/**
 * @file SerialCommunication.cpp
 * @brief Implementacja komunikacji szeregowej z Arduino NANO HX711
 * 
 * @section DEBUG_FEATURES Funkcje Debugowania
 * - Szczegółowe logowanie operacji na portach szeregowych
 * - Śledzenie stanu połączenia i aktywności
 * - Monitorowanie błędów CRC i retry operations
 * - Statystyki transmisji danych
 * 
 * @section ERROR_HANDLING Obsługa Błędów
 * - Walidacja parametrów przed operacjami
 * - Obsługa wyjątków podczas otwierania portu
 * - Retry logic dla tymczasowych błędów
 * - Graceful degradation przy utracie połączenia
 */

#include "sensor/SerialCommunication.hpp"
#include "core/DebugManager.hpp"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <climits>
#include <cstdio>
#include <stdexcept>
#include <system_error>

// ============================================================================
// DEBUG FLAGS AND CONFIGURATION
// ============================================================================

/// Enable detailed debug logging for serial operations
constexpr bool DEBUG_SERIAL_COMM = true;

/// Enable verbose data packet debugging
constexpr bool DEBUG_DATA_PACKETS = false;

/// Enable connection state debugging
constexpr bool DEBUG_CONNECTION = true;

/// Enable CRC validation debugging
constexpr bool DEBUG_CRC = false;

/// Maximum number of connection retries
constexpr int MAX_CONNECTION_RETRIES = 3;

/// Delay between connection retries in milliseconds
constexpr int CONNECTION_RETRY_DELAY_MS = 500;

#define DEFAULT_BAUD_RATE 115200
#define WATCHDOG_DEFAULT_TIMEOUT_MS 5000

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <cerrno>
#endif

namespace sensor {

// ============================================================================
// HELPER FUNCTIONS - CRC CALCULATION
// ============================================================================

uint8_t SerialCommunication::calculateCRC8(const uint8_t* data, size_t length) {
    try {
        if (data == nullptr || length == 0) {
#if DEBUG_CRC
            core::DebugManager::instance().sendDebugMessage(
                "CRC calculation failed: null data or zero length",
                core::DebugLevel::WARNING,
                "SerialCommunication::calculateCRC8"
            );
#endif
            return 0xFF;
        }
        
        uint8_t crc = 0xFF;
        const uint8_t polynomial = 0x07;
        for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];
            for (uint8_t bit = 0; bit < 8; ++bit) {
                if (crc & 0x80) crc = (crc << 1) ^ polynomial;
                else crc <<= 1;
            }
        }
        
#if DEBUG_CRC
        core::DebugManager::instance().sendDebugMessage(
            QString("CRC8 calculated: 0x%1").arg(crc, 2, 16, QChar('0')),
            core::DebugLevel::VERBOSE,
            "SerialCommunication::calculateCRC8"
        );
#endif
        return crc;
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("CRC calculation exception: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "SerialCommunication::calculateCRC8"
        );
        return 0xFF;
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during CRC calculation",
            core::DebugLevel::CRITICAL,
            "SerialCommunication::calculateCRC8"
        );
        return 0xFF;
    }
}

bool SerialCommunication::verifyCRC8(const uint8_t* data, size_t length) {
    try {
        if (length < 1) {
#if DEBUG_CRC
            core::DebugManager::instance().sendDebugMessage(
                "CRC verification failed: data too short",
                core::DebugLevel::WARNING,
                "SerialCommunication::verifyCRC8"
            );
#endif
            return false;
        }
        
        bool isValid = calculateCRC8(data, length - 1) == data[length - 1];
        
#if DEBUG_CRC
        core::DebugManager::instance().sendDebugMessage(
            QString("CRC verification: %1").arg(isValid ? "PASSED" : "FAILED"),
            core::DebugLevel::DEBUG,
            "SerialCommunication::verifyCRC8"
        );
#endif
        return isValid;
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("CRC verification exception: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "SerialCommunication::verifyCRC8"
        );
        return false;
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during CRC verification",
            core::DebugLevel::CRITICAL,
            "SerialCommunication::verifyCRC8"
        );
        return false;
    }
}

class SerialCommunication::Impl {
public:
    int portHandle = -1;
    bool isConnected = false;
    int baudRate = DEFAULT_BAUD_RATE;
    int watchdogTimeoutMs = WATCHDOG_DEFAULT_TIMEOUT_MS;
    std::chrono::steady_clock::time_point lastActivityTime;
    double zeroOffset = 0.0;
    double scaleFactor = 1.0;
    bool isCalibrated = false;
    ArduinoInfo arduinoInfo;
    DataCallback dataCallback;
    ConnectionCallback connectionCallback;
    std::atomic<bool> asyncReading{false};
    std::thread asyncThread;
    std::mutex readMutex;
    std::string lastError;
    bool verbose = true;
};

SerialCommunication::SerialCommunication() : m_impl(std::make_unique<Impl>()) {
#if DEBUG_SERIAL_COMM
    core::DebugManager::instance().sendDebugMessage(
        "SerialCommunication object created",
        core::DebugLevel::DEBUG,
        "SerialCommunication::SerialCommunication"
    );
#endif
}

SerialCommunication::~SerialCommunication() {
#if DEBUG_SERIAL_COMM
    core::DebugManager::instance().sendDebugMessage(
        "SerialCommunication object destroyed",
        core::DebugLevel::DEBUG,
        "SerialCommunication::~SerialCommunication"
    );
#endif
    disconnect();
}

std::vector<std::string> SerialCommunication::scanAvailablePorts() {
#if DEBUG_CONNECTION
    core::DebugManager::instance().sendDebugMessage(
        "Scanning for available serial ports...",
        core::DebugLevel::INFO,
        "SerialCommunication::scanAvailablePorts"
    );
#endif

    std::vector<std::string> ports;
    
    try {
#ifdef _WIN32
        for (int i = 1; i <= 256; ++i) {
            std::stringstream ss; ss << "\\\\.\\" << "COM" << i;
            HANDLE h = CreateFileA(ss.str().c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (h != INVALID_HANDLE_VALUE) { 
                ports.push_back("COM" + std::to_string(i)); 
                CloseHandle(h);
#if DEBUG_CONNECTION
                core::DebugManager::instance().sendDebugMessage(
                    QString("Found port: %1").arg(QString::fromStdString("COM" + std::to_string(i))),
                    core::DebugLevel::VERBOSE,
                    "SerialCommunication::scanAvailablePorts"
                );
#endif
            }
        }
#else
        DIR* dir = opendir("/dev");
        if (dir) {
            struct dirent* e;
            while ((e = readdir(dir))) {
                std::string n = e->d_name;
                if (n.find("ttyUSB")==0 || n.find("ttyACM")==0 || n.find("cu.usbserial")==0 || n.find("cu.usbmodem")==0) {
                    std::string portPath = "/dev/" + n;
                    ports.push_back(portPath);
#if DEBUG_CONNECTION
                    core::DebugManager::instance().sendDebugMessage(
                        QString("Found port: %1").arg(QString::fromStdString(portPath)),
                        core::DebugLevel::VERBOSE,
                        "SerialCommunication::scanAvailablePorts"
                    );
#endif
                }
            }
            closedir(dir);
        } else {
            core::DebugManager::instance().sendDebugMessage(
                "Failed to open /dev directory for scanning",
                core::DebugLevel::WARNING,
                "SerialCommunication::scanAvailablePorts"
            );
        }
#endif
        
#if DEBUG_CONNECTION
        core::DebugManager::instance().sendDebugMessage(
            QString("Scan complete. Found %1 ports.").arg(ports.size()),
            core::DebugLevel::INFO,
            "SerialCommunication::scanAvailablePorts"
        );
#endif
        
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("Exception during port scanning: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "SerialCommunication::scanAvailablePorts"
        );
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during port scanning",
            core::DebugLevel::CRITICAL,
            "SerialCommunication::scanAvailablePorts"
        );
    }
    
    return ports;
}

bool SerialCommunication::connect(const SerialConfig& config) { return openPort(config.portName, config.baudRate); }

bool SerialCommunication::connect(const std::string& portName, int baudRate) { return openPort(portName, baudRate); }

bool SerialCommunication::openPort(const std::string& portName, int baudRate) {
    try {
        // Walidacja parametrów wejściowych
        if (portName.empty()) {
            core::DebugManager::instance().sendDebugMessage(
                "Cannot open port: empty port name provided",
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false;
        }
        
        // Walidacja baud rate
        if (baudRate <= 0 || baudRate > 921600) {
            core::DebugManager::instance().sendDebugMessage(
                QString("Invalid baud rate %1. Using default %2.").arg(baudRate).arg(DEFAULT_BAUD_RATE),
                core::DebugLevel::WARNING,
                "SerialCommunication::openPort"
            );
            baudRate = DEFAULT_BAUD_RATE;
        }

#if DEBUG_CONNECTION
        core::DebugManager::instance().sendDebugMessage(
            QString("Opening port %1 with baud rate %2").arg(QString::fromStdString(portName)).arg(baudRate),
            core::DebugLevel::INFO,
            "SerialCommunication::openPort"
        );
#endif
        
        if (m_impl->isConnected) {
#if DEBUG_CONNECTION
            core::DebugManager::instance().sendDebugMessage(
                "Disconnecting existing connection before opening new port",
                core::DebugLevel::DEBUG,
                "SerialCommunication::openPort"
            );
#endif
            disconnect();
        }
        
        m_impl->baudRate = baudRate;
        
#ifdef _WIN32
        HANDLE h = CreateFileA(("\\\\.\\" + portName).c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h == INVALID_HANDLE_VALUE) { 
            m_impl->lastError = "Cannot open " + portName;
            DWORD errorCode = GetLastError();
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to open port %1. Windows error code: %2").arg(QString::fromStdString(portName)).arg(errorCode),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        DCB dcb{}; dcb.DCBlength = sizeof(DCB);
        if (!GetCommState(h, &dcb)) { 
            CloseHandle(h);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to get port state for %1").arg(QString::fromStdString(portName)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        dcb.BaudRate = baudRate; dcb.ByteSize = 8; dcb.Parity = NOPARITY; dcb.StopBits = ONESTOPBIT;
        if (!SetCommState(h, &dcb)) { 
            CloseHandle(h);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to set port configuration for %1").arg(QString::fromStdString(portName)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        COMMTIMEOUTS to{}; to.ReadIntervalTimeout = 50; to.ReadTotalTimeoutConstant = 50;
        SetCommTimeouts(h, &to);
        m_impl->portHandle = reinterpret_cast<int>(h);
#else
        int fd = open(portName.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK);
        if (fd < 0) { 
            m_impl->lastError = "Cannot open " + portName;
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to open port %1: %2").arg(QString::fromStdString(portName)).arg(strerror(errno)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        termios tty{};
        if (tcgetattr(fd, &tty)) { 
            close(fd);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to get terminal attributes for %1").arg(QString::fromStdString(portName)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        speed_t spd = B115200;
        switch(baudRate) { case 9600:spd=B9600;break; case 19200:spd=B19200;break; case 38400:spd=B38400;break; case 57600:spd=B57600;break; case 230400:spd=B230400;break; }
        cfsetospeed(&tty, spd); cfsetispeed(&tty, spd);
        tty.c_cflag |= (CREAD|CLOCAL|CS8); tty.c_cflag &= ~(PARENB|CSTOPB|CRTSCTS);
        tty.c_lflag &= ~(ICANON|ECHO|ISIG); tty.c_oflag &= ~OPOST; tty.c_iflag &= ~(IXON|IXOFF);
        tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 10;
        tcflush(fd, TCIFLUSH);
        if (tcsetattr(fd, TCSANOW, &tty)) { 
            close(fd);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to set terminal attributes for %1").arg(QString::fromStdString(portName)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        int fl = fcntl(fd, F_GETFL); fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
        m_impl->portHandle = fd;
#endif
        m_impl->isConnected = true;
        m_impl->lastActivityTime = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
#if DEBUG_CONNECTION
        core::DebugManager::instance().sendDebugMessage(
            QString("Successfully connected to %1 at %2 baud").arg(QString::fromStdString(portName)).arg(baudRate),
            core::DebugLevel::INFO,
            "SerialCommunication::openPort"
        );
#endif
        
        if (m_impl->verbose) {
            std::cout << "[SerialComm] Connected to " << portName << " at " << baudRate << " baud" << std::endl;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("Exception during port opening: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "SerialCommunication::openPort"
        );
        return false;
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception during port opening",
            core::DebugLevel::CRITICAL,
            "SerialCommunication::openPort"
        );
        return false;
    }
}

void SerialCommunication::disconnect() {
    stopAsyncReading();
    if (!m_impl->isConnected) return;
#ifdef _WIN32
    if (m_impl->portHandle > 0) CloseHandle(reinterpret_cast<HANDLE>(m_impl->portHandle));
#else
    if (m_impl->portHandle >= 0) close(m_impl->portHandle);
#endif
    m_impl->portHandle = -1; m_impl->isConnected = false;
}

bool SerialCommunication::isConnected() const { return m_impl->isConnected; }
std::string SerialCommunication::getPortName() const { return m_impl->isConnected ? "Unknown" : ""; }

ArduinoInfo SerialCommunication::identifyArduino(int timeout) {
    ArduinoInfo info;
    if (!m_impl->isConnected) return info;
    std::string resp = sendCommand("ID", true);
    if (resp.find("DEVICE:") != std::string::npos) {
        std::istringstream iss(resp); std::string line;
        while (std::getline(iss, line)) {
            if (line.find("DEVICE:") == 0) info.deviceName = line.substr(7);
            else if (line.find("ID:") == 0) info.serialNumber = line.substr(3);
            else if (line.find("FW:") == 0) info.firmwareVersion = line.substr(3);
        }
        info.isConnected = true; info.boardId = 1;
        m_impl->arduinoInfo = info;
    }
    return info;
}

bool SerialCommunication::autoConnect(int maxAttempts) {
    int attempt = 0;
    while (maxAttempts <= 0 || attempt < maxAttempts) {
        for (const auto& p : scanAvailablePorts()) {
            std::cout << "[SerialComm] Trying " << p << std::endl;
            if (connect(p, DEFAULT_BAUD_RATE)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (identifyArduino(2000).isConnected) {
                    std::cout << "[SerialComm] Found Arduino on " << p << std::endl;
                    return true;
                }
                disconnect();
            }
        }
        attempt++;
        std::cout << "[SerialComm] Arduino not found. Retry " << attempt << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cerr << "[SerialComm] ERROR: Arduino not found. Continuing search..." << std::endl;
    while (true) {
        for (const auto& p : scanAvailablePorts()) {
            if (connect(p, DEFAULT_BAUD_RATE)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (identifyArduino(2000).isConnected) {
                    std::cout << "[SerialComm] FOUND Arduino!" << std::endl;
                    return true;
                }
                disconnect();
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

ArduinoInfo SerialCommunication::getArduinoInfo() const { return m_impl->arduinoInfo; }

SensorData SerialCommunication::readData(int timeout) {
    SensorData data;
    if (!m_impl->isConnected) return data;
    
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    
    // Send DATA command
    std::string toSend = "DATA\n";
#ifdef _WIN32
    DWORD w; WriteFile(reinterpret_cast<HANDLE>(m_impl->portHandle), toSend.c_str(), toSend.size(), &w, nullptr);
#else
    write(m_impl->portHandle, toSend.c_str(), toSend.size());
#endif
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    
    // Read response - look for binary packet followed by text
    char buf[256];
    std::vector<uint8_t> rawData;
    auto start = std::chrono::steady_clock::now();
    bool gotBinaryData = false;
    
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::steady_clock::now() - start).count() < 2000) {
        int rd = 0;
#ifdef _WIN32
        DWORD dw; if (ReadFile(reinterpret_cast<HANDLE>(m_impl->portHandle), buf, 255, &dw, nullptr)) rd = dw;
#else
        rd = read(m_impl->portHandle, buf, 255);
#endif
        if (rd > 0) {
            for (int i = 0; i < rd; ++i) {
                rawData.push_back(static_cast<uint8_t>(buf[i]));
                
                // Check for binary packet structure (9 bytes: 4 timestamp + 4 value + 1 CRC)
                // followed by end markers 0xAA 0x55
                if (rawData.size() >= 11) {
                    // Look for end marker pattern
                    if (rawData[rawData.size()-2] == 0xAA && rawData[rawData.size()-1] == 0x55) {
                        // Extract binary packet (9 bytes before end markers)
                        if (rawData.size() >= 11) {
                            size_t packetStart = rawData.size() - 11;
                            
                            // Safety check - ensure we have enough data
                            if (packetStart + 9 <= rawData.size()) {
                                // Copy data to local buffer to avoid vector reallocation issues
                                uint8_t packet[9];
                                for (size_t j = 0; j < 9; ++j) {
                                    packet[j] = rawData[packetStart + j];
                                }
                                
                                // Parse binary packet: timestamp(4) + value(4) + crc(1)
                                uint32_t timestamp;
                                int32_t value;
                                uint8_t crc;
                                
                                std::memcpy(&timestamp, packet, 4);
                                std::memcpy(&value, packet + 4, 4);
                                crc = packet[8];
                                
                                // Verify CRC
                                if (verifyCRC8(packet, 9)) {
                                    data.timestamp = timestamp;
                                    data.value = value;
                                    data.crc = crc;
                                    data.isValid = true;
                                    gotBinaryData = true;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            
            // Also check for text response as fallback
            std::string resp(rawData.begin(), rawData.end());
            if (!gotBinaryData && resp.find("FORCE:") != std::string::npos) {
                size_t fp = resp.find("FORCE:");
                size_t gp = resp.find("g,", fp);
                if (gp != std::string::npos) {
                    try {
                        data.calibratedValue = std::stod(resp.substr(fp+6, gp-fp-6));
                    } catch (...) {}
                }
                size_t rp = resp.find("RAW:", gp);
                if (rp != std::string::npos) {
                    size_t cp = resp.find(",", rp+4);
                    if (cp != std::string::npos) {
                        try {
                            data.value = std::stoi(resp.substr(rp+4, cp-rp-4));
                        } catch (...) {}
                    }
                    size_t tp = resp.find("TS:", rp);
                    if (tp != std::string::npos) {
                        size_t mp = resp.find("ms", tp);
                        if (mp != std::string::npos) {
                            try {
                                data.timestamp = std::stoul(resp.substr(tp+3, mp-tp-3));
                            } catch (...) {}
                        }
                    }
                }
                data.isValid = true;
                break;
            }
            
            if (resp.find("[DATA_SENT]") != std::string::npos && gotBinaryData) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    if (m_impl->verbose && data.isValid) {
        std::cout << "[SerialComm] Data: TS=" << data.timestamp 
                  << ", Value=" << data.value 
                  << ", Calibrated=" << data.calibratedValue
                  << ", CRC=" << (int)data.crc << std::endl;
    } else if (m_impl->verbose && !data.isValid) {
        std::cout << "[SerialComm] Failed to parse data. Raw bytes: ";
        for (size_t i = 0; i < std::min(rawData.size(), size_t(20)); ++i) {
            printf("%02X ", rawData[i]);
        }
        std::cout << std::endl;
    }
    
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    return data;
}

bool SerialCommunication::tryReadData(SensorData& data) { data = readData(100); return data.isValid; }
void SerialCommunication::setDataCallback(DataCallback cb) { m_impl->dataCallback = cb; }
void SerialCommunication::setConnectionCallback(ConnectionCallback cb) { m_impl->connectionCallback = cb; }

void SerialCommunication::startAsyncReading() {
    if (m_impl->asyncReading) return;
    m_impl->asyncReading = true;
    m_impl->asyncThread = std::thread([this]() {
        while (m_impl->asyncReading && m_impl->isConnected) {
            SensorData d = readData(100);
            if (d.isValid && m_impl->dataCallback) m_impl->dataCallback(d);
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
        }
    });
}

void SerialCommunication::stopAsyncReading() {
    m_impl->asyncReading = false;
    if (m_impl->asyncThread.joinable()) m_impl->asyncThread.join();
}

void SerialCommunication::configureWatchdog(int timeoutMs, bool enabled) { m_impl->watchdogTimeoutMs = enabled ? timeoutMs : INT_MAX; }
void SerialCommunication::resetWatchdog() { m_impl->lastActivityTime = std::chrono::steady_clock::now(); }
bool SerialCommunication::isWatchdogOK() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_impl->lastActivityTime).count() < m_impl->watchdogTimeoutMs;
}
int SerialCommunication::getTimeSinceLastActivity() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_impl->lastActivityTime).count();
}

double SerialCommunication::tare(int samples) { if (m_impl->isConnected) sendCommand("TARE", true); m_impl->zeroOffset = 0.0; return 0.0; }
void SerialCommunication::setCalibrationPoint(int32_t raw, double known) { if (raw != 0) { m_impl->scaleFactor = known/raw; m_impl->isCalibrated = true; } }
bool SerialCommunication::calibrate(int32_t, int32_t, double knownWeight) {
    if (!m_impl->isConnected) return false;
    std::string r = sendCommand("CAL:" + std::to_string(knownWeight), true);
    m_impl->isCalibrated = (r.find("Calibrated!") != std::string::npos);
    return m_impl->isCalibrated;
}
double SerialCommunication::convertToCalibrated(int32_t raw) const { return (raw - m_impl->zeroOffset) * m_impl->scaleFactor; }
double SerialCommunication::getCalibrationScale() const { return m_impl->scaleFactor; }
int32_t SerialCommunication::getCalibrationOffset() const { return static_cast<int32_t>(m_impl->zeroOffset); }
void SerialCommunication::resetCalibration() { m_impl->zeroOffset = 0.0; m_impl->scaleFactor = 1.0; m_impl->isCalibrated = false; }

uint32_t SerialCommunication::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
void SerialCommunication::syncTimestamp(uint32_t) {}
int32_t SerialCommunication::getTimestampOffset() const { return 0; }

std::string SerialCommunication::sendCommand(const std::string& cmd, bool waitForResponse) {
    if (!m_impl->isConnected) return "";
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    std::string toSend = cmd + "\n";
#ifdef _WIN32
    DWORD w; WriteFile(reinterpret_cast<HANDLE>(m_impl->portHandle), toSend.c_str(), toSend.size(), &w, nullptr);
#else
    write(m_impl->portHandle, toSend.c_str(), toSend.size());
#endif
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    if (!waitForResponse) return "";
    std::string resp; char buf[256];
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() < 2000) {
        int rd = 0;
#ifdef _WIN32
        DWORD dw; if (ReadFile(reinterpret_cast<HANDLE>(m_impl->portHandle), buf, 255, &dw, nullptr)) rd = dw;
#else
        rd = read(m_impl->portHandle, buf, 255);
#endif
        if (rd > 0) { buf[rd] = 0; resp += buf; if (resp.find("[DATA_SENT]")!=std::string::npos || resp.find("OK")!=std::string::npos || resp.find("FORCE:")!=std::string::npos) break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (m_impl->verbose && !resp.empty()) std::cout << "[SerialComm] " << resp << std::endl;
    return resp;
}

bool SerialCommunication::sendRawData(const uint8_t* data, size_t len) {
    if (!m_impl->isConnected || !data || len == 0) return false;
#ifdef _WIN32
    DWORD w; return WriteFile(reinterpret_cast<HANDLE>(m_impl->portHandle), data, len, &w, nullptr) && w == len;
#else
    return write(m_impl->portHandle, data, len) == static_cast<ssize_t>(len);
#endif
}

void SerialCommunication::setVerbose(bool v) { m_impl->verbose = v; }
std::string SerialCommunication::getLastError() const { return m_impl->lastError; }
void SerialCommunication::setCRCPolynomial(uint8_t) {}

} // namespace sensor
