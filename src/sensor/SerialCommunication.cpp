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
#include <cmath>
#include <limits>
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
#include <unordered_map>
#include <utility>
#include <system_error>
#include <condition_variable>
#ifndef _WIN32
#include <poll.h>
#endif

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

namespace {
thread_local std::vector<const void*> activeSubscriptionCallbacks;

template <typename Callback>
struct CallbackSlot {
    explicit CallbackSlot(Callback value) : callback(std::move(value)) {}
    std::mutex mutex;
    std::condition_variable idle;
    bool active = true;
    std::size_t inFlight = 0;
    Callback callback;
};

template <typename Slot, typename Invoke>
void invokeSubscription(const std::shared_ptr<Slot>& slot, Invoke invoke)
{
    {
        std::lock_guard<std::mutex> lock(slot->mutex);
        if (!slot->active) return;
        ++slot->inFlight;
    }
    activeSubscriptionCallbacks.push_back(slot.get());
    try { invoke(slot->callback); } catch (...) {}
    activeSubscriptionCallbacks.pop_back();
    {
        std::lock_guard<std::mutex> lock(slot->mutex);
        if (--slot->inFlight == 0) slot->idle.notify_all();
    }
}

template <typename Slot>
void deactivateSubscription(const std::shared_ptr<Slot>& slot)
{
    if (!slot) return;
    std::unique_lock<std::mutex> lock(slot->mutex);
    slot->active = false;
    if (std::find(activeSubscriptionCallbacks.begin(), activeSubscriptionCallbacks.end(), slot.get()) == activeSubscriptionCallbacks.end())
        slot->idle.wait(lock, [&slot]() { return slot->inFlight == 0; });
}
} // namespace

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
    std::intptr_t portHandle = -1;
    std::atomic<bool> isConnected{false};
    int baudRate = DEFAULT_BAUD_RATE;
    std::string portName;
    mutable std::mutex watchdogMutex;
    int watchdogTimeoutMs = WATCHDOG_DEFAULT_TIMEOUT_MS;
    std::chrono::steady_clock::time_point lastActivityTime;
    mutable std::mutex calibrationMutex;
    double zeroOffset = 0.0;
    double scaleFactor = 1.0;
    bool isCalibrated = false;
    ArduinoInfo arduinoInfo;
    DataCallback dataCallback;
    std::mutex callbackMutex;
    std::unordered_map<SubscriptionId, std::shared_ptr<CallbackSlot<DataCallback>>> dataSubscribers;
    std::unordered_map<SubscriptionId, std::shared_ptr<CallbackSlot<ConnectionCallback>>> connectionSubscribers;
    SubscriptionId nextSubscriptionId = 1;
    ConnectionCallback connectionCallback;
    std::atomic<bool> asyncReading{false};
    std::mutex asyncThreadMutex;
    std::thread asyncThread;
    mutable std::mutex readMutex;
    std::string lastError;
    bool verbose = true;
    int32_t timestampOffset = 0;
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
        
        {
            std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
            m_impl->baudRate = baudRate;
            m_impl->portName = portName;
        }
        
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
        {
            std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
            m_impl->portHandle = reinterpret_cast<std::intptr_t>(h);
        }
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
        {
            std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
            m_impl->portHandle = fd;
        }
#endif
        m_impl->isConnected.store(true);
        { std::lock_guard<std::mutex> watchdogLock(m_impl->watchdogMutex); m_impl->lastActivityTime = std::chrono::steady_clock::now(); }
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
        notifyConnectionCallbacks(true, "Connected to " + portName);
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
    bool wasConnected = false;
    {
        // Lock order: asyncThreadMutex -> readMutex. This prevents a new reader
        // from starting between the stop/join phase and closing the port.
        std::unique_lock<std::mutex> threadLock(m_impl->asyncThreadMutex);
        m_impl->asyncReading.store(false);
        if (m_impl->asyncThread.joinable()) {
            if (m_impl->asyncThread.get_id() == std::this_thread::get_id())
                return; // A reader callback must request disconnect from another thread.
            m_impl->asyncThread.join();
        }

        std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
        wasConnected = m_impl->isConnected.load();
        if (!wasConnected) return;
#ifdef _WIN32
        if (m_impl->portHandle != -1)
            CloseHandle(reinterpret_cast<HANDLE>(m_impl->portHandle));
#else
        if (m_impl->portHandle >= 0) close(static_cast<int>(m_impl->portHandle));
#endif
        m_impl->portHandle = -1;
        m_impl->isConnected.store(false);
        m_impl->portName.clear();
    }
    notifyConnectionCallbacks(false, "Serial connection closed");
}
bool SerialCommunication::isConnected() const { return m_impl->isConnected; }
std::string SerialCommunication::getPortName() const {
    std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
    return m_impl->isConnected.load() ? m_impl->portName : std::string();
}
int SerialCommunication::getBaudRate() const {
    std::lock_guard<std::mutex> stateLock(m_impl->readMutex);
    return m_impl->baudRate;
}

ArduinoInfo SerialCommunication::identifyArduino(int timeout) {
    ArduinoInfo info;
    if (!m_impl->isConnected) return info;
    std::string resp = sendCommand("ID", true, timeout);
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
    std::cerr << "[SerialComm] ERROR: Arduino not found after " << maxAttempts << " attempts." << std::endl;
    return false;
}

ArduinoInfo SerialCommunication::getArduinoInfo() const { return m_impl->arduinoInfo; }

SensorData SerialCommunication::readData(int timeout) {
    SensorData data;
    if (!m_impl->isConnected) return data;
    
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    if (!m_impl->isConnected.load()) return data;
    
    // Send DATA command
    std::string toSend = "DATA\n";
#ifdef _WIN32
    DWORD w; WriteFile(reinterpret_cast<HANDLE>(m_impl->portHandle), toSend.c_str(), toSend.size(), &w, nullptr);
#else
    write(m_impl->portHandle, toSend.c_str(), toSend.size());
#endif
    { std::lock_guard<std::mutex> watchdogLock(m_impl->watchdogMutex); m_impl->lastActivityTime = std::chrono::steady_clock::now(); }
    
    // Read response - look for binary packet followed by text
    char buf[256];
    std::vector<uint8_t> rawData;
    auto start = std::chrono::steady_clock::now();
    bool gotBinaryData = false;
    
    while (true) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        const int remaining = std::max(0, timeout - static_cast<int>(elapsed));
        if (remaining == 0) break;
        int rd = 0;
#ifdef _WIN32
        COMMTIMEOUTS timeouts{};
        if (GetCommTimeouts(reinterpret_cast<HANDLE>(m_impl->portHandle), &timeouts)) {
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(remaining);
            SetCommTimeouts(reinterpret_cast<HANDLE>(m_impl->portHandle), &timeouts);
        }
        DWORD dw = 0;
        if (ReadFile(reinterpret_cast<HANDLE>(m_impl->portHandle), buf, 255, &dw, nullptr))
            rd = static_cast<int>(dw);
#else
        pollfd descriptor{static_cast<int>(m_impl->portHandle), POLLIN, 0};
        const int ready = poll(&descriptor, 1, remaining);
        if (ready < 0 && errno == EINTR) continue;
        if (ready <= 0 || !(descriptor.revents & (POLLIN | POLLHUP))) break;
        rd = static_cast<int>(read(m_impl->portHandle, buf, 255));
#endif
        if (rd > 0) {
            for (int i = 0; i < rd; ++i) {
                rawData.push_back(static_cast<uint8_t>(buf[i]));

                // Firmware frame: 0xAA | timestamp(4) | calibrated force(4) |
                // CRC-8(1) | 0x55. CRC covers the first 9 bytes, including sync.
                if (rawData.size() < 11) continue;

                for (std::size_t packetStart = 0;
                     packetStart + 11 <= rawData.size(); ++packetStart) {
                    if (rawData[packetStart] != 0xAA ||
                        rawData[packetStart + 10] != 0x55) continue;

                    uint8_t packet[10];
                    std::memcpy(packet, rawData.data() + packetStart, sizeof(packet));
                    if (!verifyCRC8(packet, 10)) continue;

                    std::memcpy(&data.timestamp, packet + 1, sizeof(data.timestamp));
                    std::memcpy(&data.value, packet + 5, sizeof(data.value));
                    data.calibratedValue = static_cast<double>(data.value);
                    data.crc = packet[9];
                    data.isValid = true;
                    gotBinaryData = true;
                    break;
                }
                if (gotBinaryData) break;
            }

            // Also check for text response as fallback
            std::string resp(rawData.begin(), rawData.end());
            if (!gotBinaryData && resp.find("FORCE:") != std::string::npos) {
                const size_t fp = resp.find("FORCE:");
                const size_t gp = resp.find("g,", fp);
                bool parsedForce = false;
                if (gp != std::string::npos) {
                    try {
                        data.calibratedValue = std::stod(resp.substr(fp + 6, gp - fp - 6));
                        parsedForce = std::isfinite(data.calibratedValue);
                    } catch (...) {}
                }
                if (parsedForce) {
                    const size_t rp = resp.find("RAW:", gp);
                    if (rp != std::string::npos) {
                        const size_t cp = resp.find(",", rp + 4);
                        if (cp != std::string::npos) {
                            try {
                                data.value = std::stoi(resp.substr(rp + 4, cp - rp - 4));
                                data.hasRawValue = true;
                            } catch (...) {}
                        }
                        const size_t tp = resp.find("TS:", rp);
                        if (tp != std::string::npos) {
                            const size_t mp = resp.find("ms", tp);
                            if (mp != std::string::npos) {
                                try {
                                    data.timestamp = std::stoul(resp.substr(tp + 3, mp - tp - 3));
                                } catch (...) {}
                            }
                        }
                    }
                    data.isValid = true;
                    break;
                }
            }
            
            if (gotBinaryData) break;
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
    
    { std::lock_guard<std::mutex> watchdogLock(m_impl->watchdogMutex); m_impl->lastActivityTime = std::chrono::steady_clock::now(); }
    return data;
}

bool SerialCommunication::tryReadData(SensorData& data) { data = readData(100); return data.isValid; }
void SerialCommunication::setDataCallback(DataCallback cb) {
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->dataCallback = std::move(cb);
}

SerialCommunication::SubscriptionId SerialCommunication::subscribeDataCallback(DataCallback callback) {
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    const SubscriptionId id = m_impl->nextSubscriptionId++;
    m_impl->dataSubscribers.emplace(id, std::make_shared<CallbackSlot<DataCallback>>(std::move(callback)));
    return id;
}

void SerialCommunication::unsubscribeDataCallback(SubscriptionId id) {
    if (id == 0) return;
    std::shared_ptr<CallbackSlot<DataCallback>> slot;
    {
        std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
        const auto it = m_impl->dataSubscribers.find(id);
        if (it == m_impl->dataSubscribers.end()) return;
        slot = it->second;
        m_impl->dataSubscribers.erase(it);
    }
    deactivateSubscription(slot);
}
void SerialCommunication::notifyConnectionCallbacks(bool connected, const std::string& message) {
    ConnectionCallback primaryCallback;
    std::vector<std::shared_ptr<CallbackSlot<ConnectionCallback>>> subscribers;
    {
        std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
        primaryCallback = m_impl->connectionCallback;
        subscribers.reserve(m_impl->connectionSubscribers.size());
        for (const auto& entry : m_impl->connectionSubscribers) {
            subscribers.push_back(entry.second);
        }
    }
    if (primaryCallback) {
        try { primaryCallback(connected, message); } catch (...) {}
    }
    for (const auto& slot : subscribers)
        invokeSubscription(slot, [&](const ConnectionCallback& callback) {
            if (callback) callback(connected, message);
        });
}

void SerialCommunication::setConnectionCallback(ConnectionCallback cb) {
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->connectionCallback = std::move(cb);
}

SerialCommunication::SubscriptionId SerialCommunication::subscribeConnectionCallback(ConnectionCallback callback) {
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    const SubscriptionId id = m_impl->nextSubscriptionId++;
    m_impl->connectionSubscribers.emplace(id, std::make_shared<CallbackSlot<ConnectionCallback>>(std::move(callback)));
    return id;
}

void SerialCommunication::unsubscribeConnectionCallback(SubscriptionId id) {
    if (id == 0) return;
    std::shared_ptr<CallbackSlot<ConnectionCallback>> slot;
    {
        std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
        const auto it = m_impl->connectionSubscribers.find(id);
        if (it == m_impl->connectionSubscribers.end()) return;
        slot = it->second;
        m_impl->connectionSubscribers.erase(it);
    }
    deactivateSubscription(slot);
}

void SerialCommunication::startAsyncReading() {
    std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
    if (!m_impl->isConnected.load() || m_impl->asyncReading.load()) return;
    if (m_impl->asyncThread.joinable()) m_impl->asyncThread.join();

    m_impl->asyncReading.store(true);
    m_impl->asyncThread = std::thread([this]() {
        while (m_impl->asyncReading.load() && m_impl->isConnected.load()) {
            SensorData data = readData(100);
            if (data.isValid) {
                DataCallback primaryCallback;
                std::vector<std::shared_ptr<CallbackSlot<DataCallback>>> subscribers;
                {
                    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
                    primaryCallback = m_impl->dataCallback;
                    subscribers.reserve(m_impl->dataSubscribers.size());
                    for (const auto& entry : m_impl->dataSubscribers)
                        subscribers.push_back(entry.second);
                }
                if (primaryCallback) {
                    try { primaryCallback(data); } catch (...) {}
                }
                for (const auto& slot : subscribers)
                    invokeSubscription(slot, [&](const DataCallback& callback) {
                        if (callback) callback(data);
                    });
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(12));
        }
    });
}

void SerialCommunication::stopAsyncReading() {
    std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
    m_impl->asyncReading.store(false);
    if (m_impl->asyncThread.joinable() &&
        m_impl->asyncThread.get_id() != std::this_thread::get_id())
        m_impl->asyncThread.join();
}

void SerialCommunication::configureWatchdog(int timeoutMs, bool enabled) {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    m_impl->watchdogTimeoutMs = enabled ? std::max(1, timeoutMs) : INT_MAX;
}
void SerialCommunication::resetWatchdog() {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
}
bool SerialCommunication::isWatchdogOK() const {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_impl->lastActivityTime).count() < m_impl->watchdogTimeoutMs;
}
int SerialCommunication::getTimeSinceLastActivity() const {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_impl->lastActivityTime).count());
}

double SerialCommunication::tare(int samples) {
    (void)samples;
    if (m_impl->isConnected) sendCommand("TARE", true);
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->zeroOffset = 0.0;
    return 0.0;
}
void SerialCommunication::setCalibrationPoint(int32_t raw, double known) {
    if (raw == 0 || !std::isfinite(known)) return;
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->zeroOffset = 0.0;
    m_impl->scaleFactor = known / static_cast<double>(raw);
    m_impl->isCalibrated = true;
}
bool SerialCommunication::calibrate(int32_t rawZero, int32_t rawSpan, double knownWeight) {
    if (!m_impl->isConnected || rawSpan == rawZero ||
        !std::isfinite(knownWeight) || knownWeight <= 0.0) return false;
    const std::string response = sendCommand("CAL:" + std::to_string(knownWeight), true);
    if (response.find("Calibrated!") == std::string::npos) return false;

    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->zeroOffset = static_cast<double>(rawZero);
    m_impl->scaleFactor = knownWeight / (static_cast<double>(rawSpan) - static_cast<double>(rawZero));
    m_impl->isCalibrated = true;
    return true;
}
double SerialCommunication::convertToCalibrated(int32_t raw) const {
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    if (!m_impl->isCalibrated) return std::numeric_limits<double>::quiet_NaN();
    return (static_cast<double>(raw) - m_impl->zeroOffset) * m_impl->scaleFactor;
}
double SerialCommunication::getCalibrationScale() const {
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    return m_impl->scaleFactor;
}
int32_t SerialCommunication::getCalibrationOffset() const {
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    return static_cast<int32_t>(m_impl->zeroOffset);
}
void SerialCommunication::resetCalibration() {
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->zeroOffset = 0.0;
    m_impl->scaleFactor = 1.0;
    m_impl->isCalibrated = false;
}

uint32_t SerialCommunication::getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void SerialCommunication::syncTimestamp(uint32_t arduinoTimestamp) {
    if (!m_impl->isConnected || arduinoTimestamp == 0) {
        m_impl->timestampOffset = 0;
        return;
    }
    uint32_t hostTimestamp = getCurrentTimestamp();
    m_impl->timestampOffset = static_cast<int32_t>(hostTimestamp - arduinoTimestamp);
#if DEBUG_SERIAL_COMM
    core::DebugManager::instance().sendDebugMessage(
        QString("Timestamp sync: Host=%1, Arduino=%2, Offset=%3 ms")
            .arg(hostTimestamp).arg(arduinoTimestamp).arg(m_impl->timestampOffset),
        core::DebugLevel::DEBUG,
        "SerialCommunication::syncTimestamp"
    );
#endif
}

int32_t SerialCommunication::getTimestampOffset() const { 
    return m_impl->timestampOffset; 
}

std::string SerialCommunication::sendCommand(const std::string& cmd, bool waitForResponse, int timeoutMs) {
    if (!m_impl->isConnected) return "";
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    if (!m_impl->isConnected.load()) return "";
    const std::string toSend = cmd + "\n";
#ifdef _WIN32
    DWORD written = 0;
    if (!WriteFile(reinterpret_cast<HANDLE>(m_impl->portHandle), toSend.c_str(),
                   static_cast<DWORD>(toSend.size()), &written, nullptr) ||
        written != toSend.size()) return "";
#else
    if (write(m_impl->portHandle, toSend.c_str(), toSend.size()) !=
        static_cast<ssize_t>(toSend.size())) return "";
#endif
    { std::lock_guard<std::mutex> watchdogLock(m_impl->watchdogMutex); m_impl->lastActivityTime = std::chrono::steady_clock::now(); }
    if (!waitForResponse || timeoutMs <= 0) return "";

    std::string response;
    char buffer[256];
    const auto start = std::chrono::steady_clock::now();
    while (true) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        const int remaining = std::max(0, timeoutMs - static_cast<int>(elapsed));
        if (remaining == 0) break;
        int bytesRead = 0;
#ifdef _WIN32
        COMMTIMEOUTS timeouts{};
        if (GetCommTimeouts(reinterpret_cast<HANDLE>(m_impl->portHandle), &timeouts)) {
            timeouts.ReadIntervalTimeout = MAXDWORD;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(remaining);
            SetCommTimeouts(reinterpret_cast<HANDLE>(m_impl->portHandle), &timeouts);
        }
        DWORD count = 0;
        if (ReadFile(reinterpret_cast<HANDLE>(m_impl->portHandle), buffer, 255, &count, nullptr))
            bytesRead = static_cast<int>(count);
#else
        pollfd descriptor{static_cast<int>(m_impl->portHandle), POLLIN, 0};
        const int ready = poll(&descriptor, 1, remaining);
        if (ready < 0 && errno == EINTR) continue;
        if (ready <= 0 || !(descriptor.revents & (POLLIN | POLLHUP))) break;
        bytesRead = static_cast<int>(read(m_impl->portHandle, buffer, 255));
#endif
        if (bytesRead > 0) {
            buffer[bytesRead] = 0;
            response.append(buffer, static_cast<std::size_t>(bytesRead));
            if (response.find("[DATA_SENT]") != std::string::npos ||
                response.find("OK") != std::string::npos ||
                response.find("FORCE:") != std::string::npos ||
                response.find("DEVICE:") != std::string::npos ||
                response.find("Calibrated!") != std::string::npos) break;
        }
    }
    if (m_impl->verbose && !response.empty())
        std::cout << "[SerialComm] " << response << std::endl;
    return response;
}

bool SerialCommunication::sendRawData(const uint8_t* data, size_t len) {
    if (!data || len == 0) return false;
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    if (!m_impl->isConnected.load()) return false;
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
