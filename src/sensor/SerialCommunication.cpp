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
#include <limits>
#include <utility>
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
constexpr bool DEBUG_SERIAL_COMM = false;

/// Enable verbose data packet debugging
constexpr bool DEBUG_DATA_PACKETS = false;

/// Enable connection state debugging
constexpr bool DEBUG_CONNECTION = false;

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
    #include <poll.h>
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
    intptr_t portHandle = -1;
    bool isConnected = false;
    int baudRate = DEFAULT_BAUD_RATE;
    std::string portName;
    int watchdogTimeoutMs = WATCHDOG_DEFAULT_TIMEOUT_MS;
    std::chrono::steady_clock::time_point lastActivityTime;
    double zeroOffset = 0.0;
    double scaleFactor = 1.0;
    bool isCalibrated = false;
    ArduinoInfo arduinoInfo;
    DataCallback dataCallback;
    ConnectionCallback connectionCallback;
    std::mutex callbackMutex;
    std::map<CallbackToken, DataCallback> dataSubscribers;
    std::map<CallbackToken, ConnectionCallback> connectionSubscribers;
    CallbackToken nextCallbackToken = 1;
    std::atomic<bool> asyncReading{false};
    // Protect the std::thread object and worker identity independently from I/O.
    // The lifecycle mutex serializes start/stop; self-stop only requests exit.
    std::mutex asyncLifecycleMutex;
    std::mutex asyncThreadMutex;
    std::thread asyncThread;
    std::thread::id asyncWorkerId;
    std::mutex readMutex;
    std::string lastError;
    bool verbose = false;
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
        to.WriteTotalTimeoutConstant = 1000;
        if (!SetCommTimeouts(h, &to)) {
            CloseHandle(h);
            m_impl->lastError = "Cannot configure serial port timeouts";
            return false;
        }
        m_impl->portHandle = reinterpret_cast<intptr_t>(h);
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
        m_impl->portName = portName;
        m_impl->lastActivityTime = std::chrono::steady_clock::now();
        notifyConnectionCallbacks(true, std::string("Connected to ") + portName);
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

    // Serialize handle teardown with every synchronous read/write operation.
    // readData(), sendCommand() and sendRawData() hold readMutex while using
    // the native handle, so closing it before acquiring this lock can race
    // with an in-flight system call (or close a descriptor reused elsewhere).
    {
        std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
        if (!m_impl->isConnected) return;
#ifdef _WIN32
        if (m_impl->portHandle != -1) CloseHandle(reinterpret_cast<HANDLE>(m_impl->portHandle));
#else
        if (m_impl->portHandle >= 0) close(static_cast<int>(m_impl->portHandle));
#endif
        m_impl->portHandle = -1;
        m_impl->isConnected = false;
        m_impl->portName.clear();
    }
    // Callbacks run after releasing readMutex so subscribers may safely
    // query state or initiate a new operation without self-deadlocking.
    notifyConnectionCallbacks(false, "Disconnected");
}

bool SerialCommunication::writeToPort(const uint8_t* data, size_t length) {
    if (!m_impl->isConnected || m_impl->portHandle == -1 || data == nullptr || length == 0) {
        m_impl->lastError = "Serial write rejected: invalid connection or buffer";
        return false;
    }

    size_t offset = 0;
#ifdef _WIN32
    HANDLE handle = reinterpret_cast<HANDLE>(m_impl->portHandle);
    while (offset < length) {
        const DWORD chunk = static_cast<DWORD>(std::min(
            length - offset, static_cast<size_t>(std::numeric_limits<DWORD>::max())));
        DWORD written = 0;
        if (!WriteFile(handle, data + offset, chunk, &written, nullptr) || written == 0) {
            m_impl->lastError = "Serial write failed";
            return false;
        }
        offset += static_cast<size_t>(written);
    }
#else
    const int fd = static_cast<int>(m_impl->portHandle);
    const int originalFlags = fcntl(fd, F_GETFL);
    if (originalFlags < 0 || fcntl(fd, F_SETFL, originalFlags | O_NONBLOCK) < 0) {
        m_impl->lastError = "Serial write configuration failed";
        return false;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    bool success = true;
    while (offset < length) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            m_impl->lastError = "Serial write timed out";
            success = false;
            break;
        }

        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - now);
        const int timeoutMs = static_cast<int>(std::max<int64_t>(
            1, std::min<int64_t>(remaining.count(), INT_MAX)));

        pollfd descriptor{};
        descriptor.fd = fd;
        descriptor.events = POLLOUT;

        const int ready = poll(&descriptor, 1, timeoutMs);
        if (ready < 0 && errno == EINTR) continue;
        if (ready == 0) {
            m_impl->lastError = "Serial write timed out";
            success = false;
            break;
        }
        if (ready < 0 ||
            (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0 ||
            (descriptor.revents & POLLOUT) == 0) {
            m_impl->lastError = "Serial write failed";
            success = false;
            break;
        }

        const ssize_t written = write(fd, data + offset, length - offset);
        if (written > 0) {
            offset += static_cast<size_t>(written);
            continue;
        }
        if (written < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
            continue;
        }

        m_impl->lastError = "Serial write failed";
        success = false;
        break;
    }

    if (fcntl(fd, F_SETFL, originalFlags) < 0) {
        m_impl->lastError = "Serial write completed but port flags could not be restored";
        success = false;
    }
    if (!success) return false;
#endif
    return true;
}


int SerialCommunication::readFromPort(uint8_t* buffer, size_t maxLength, int timeout) {
    if (!m_impl->isConnected || m_impl->portHandle == -1 ||
        buffer == nullptr || maxLength == 0 || timeout < 0) {
        m_impl->lastError = "Serial read rejected: invalid connection, buffer, or timeout";
        return -1;
    }

#ifdef _WIN32
    HANDLE handle = reinterpret_cast<HANDLE>(m_impl->portHandle);
    COMMTIMEOUTS originalTimeouts{};
    if (!GetCommTimeouts(handle, &originalTimeouts)) {
        m_impl->lastError = "Serial read timeout configuration unavailable";
        return -1;
    }

    COMMTIMEOUTS readTimeouts = originalTimeouts;
    readTimeouts.ReadIntervalTimeout = timeout == 0 ? MAXDWORD : 0;
    readTimeouts.ReadTotalTimeoutMultiplier = 0;
    readTimeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(timeout);
    if (!SetCommTimeouts(handle, &readTimeouts)) {
        m_impl->lastError = "Serial read timeout configuration failed";
        return -1;
    }

    DWORD bytesRead = 0;
    const DWORD requested = static_cast<DWORD>(std::min(
        maxLength, static_cast<size_t>(INT_MAX)));
    const BOOL readSucceeded = ReadFile(handle, buffer, requested, &bytesRead, nullptr);
    const BOOL restoreSucceeded = SetCommTimeouts(handle, &originalTimeouts);
    if (!restoreSucceeded) {
        m_impl->lastError = "Serial read completed but timeout settings could not be restored";
        return -1;
    }
    if (!readSucceeded) {
        m_impl->lastError = "Serial read failed";
        return -1;
    }
    return static_cast<int>(bytesRead);
#else
    const int fd = static_cast<int>(m_impl->portHandle);
    const int originalFlags = fcntl(fd, F_GETFL);
    if (originalFlags < 0 || fcntl(fd, F_SETFL, originalFlags | O_NONBLOCK) < 0) {
        m_impl->lastError = "Serial read configuration failed";
        return -1;
    }

    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeout);
    int result = 0;
    bool pollAttempted = false;
    for (;;) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline && pollAttempted) break;
        int remainingMs = 0;
        if (now < deadline) {
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - now);
            remainingMs = static_cast<int>(std::max<int64_t>(
                1, std::min<int64_t>(remaining.count(), INT_MAX)));
        }

        pollfd descriptor{};
        descriptor.fd = fd;
        descriptor.events = POLLIN;
        pollAttempted = true;
        const int ready = poll(&descriptor, 1, remainingMs);
        if (ready < 0 && errno == EINTR) continue;
        if (ready == 0) break;
        if (ready < 0 ||
            (descriptor.revents & (POLLERR | POLLNVAL)) != 0) {
            m_impl->lastError = "Serial read failed";
            result = -1;
            break;
        }
        if ((descriptor.revents & POLLHUP) != 0 &&
            (descriptor.revents & POLLIN) == 0) {
            m_impl->lastError = "Serial port closed during read";
            result = -1;
            break;
        }
        if ((descriptor.revents & POLLIN) == 0) continue;

        const ssize_t bytesRead = read(fd, buffer, maxLength);
        if (bytesRead > 0) {
            result = static_cast<int>(bytesRead);
            break;
        }
        if (bytesRead == 0) break;
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
        m_impl->lastError = "Serial read failed";
        result = -1;
        break;
    }

    if (fcntl(fd, F_SETFL, originalFlags) < 0) {
        m_impl->lastError = "Serial read completed but port flags could not be restored";
        return -1;
    }
    return result;
#endif
}

bool SerialCommunication::isConnected() const { return m_impl->isConnected; }
std::string SerialCommunication::getPortName() const { return m_impl->isConnected ? m_impl->portName : ""; }
int SerialCommunication::getBaudRate() const { return m_impl->baudRate; }

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
    std::cerr << "[SerialComm] ERROR: Arduino not found after " << maxAttempts << " attempts." << std::endl;
    return false;
}

ArduinoInfo SerialCommunication::getArduinoInfo() const { return m_impl->arduinoInfo; }

SensorData SerialCommunication::readData(int timeout) {
    SensorData data;
    if (timeout < 0) {
        m_impl->lastError = "Serial read rejected: negative timeout";
        return data;
    }
    if (!m_impl->isConnected) return data;
    
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    
    // Send DATA command
    std::string toSend = "DATA\n";
if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size())) {
        return data;
    }
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    
    // Read response within the caller-provided total deadline.
    uint8_t buf[256];
    constexpr size_t MaxResponseBytes = 4096;
    std::vector<uint8_t> rawData;
    rawData.reserve(512);
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(std::max(timeout, 0));
    bool gotBinaryData = false;

    while (rawData.size() < MaxResponseBytes &&
           std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        const int rd = readFromPort(buf, std::min(sizeof(buf), MaxResponseBytes - rawData.size()),
                                    static_cast<int>(std::max<int64_t>(1, remaining.count())));
        if (rd < 0) break;
        if (rd > 0) {
            for (int i = 0; i < rd; ++i) {
                if (rawData.size() >= MaxResponseBytes) break;
                rawData.push_back(buf[i]);
                
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
    }
    
    data.hasFrame = !rawData.empty();

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
    
    if (!rawData.empty()) {
        m_impl->lastActivityTime = std::chrono::steady_clock::now();
    }
    return data;
}

bool SerialCommunication::tryReadData(SensorData& data) { data = readData(100); return data.isValid; }

void SerialCommunication::setDataCallback(DataCallback callback)
{
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->dataCallback = std::move(callback);
}

void SerialCommunication::setConnectionCallback(ConnectionCallback callback)
{
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->connectionCallback = std::move(callback);
}

SerialCommunication::CallbackToken SerialCommunication::addDataCallback(DataCallback callback)
{
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    const CallbackToken token = m_impl->nextCallbackToken++;
    m_impl->dataSubscribers.emplace(token, std::move(callback));
    return token;
}

SerialCommunication::CallbackToken SerialCommunication::addConnectionCallback(ConnectionCallback callback)
{
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    const CallbackToken token = m_impl->nextCallbackToken++;
    m_impl->connectionSubscribers.emplace(token, std::move(callback));
    return token;
}

void SerialCommunication::removeDataCallback(CallbackToken token)
{
    if (token == 0) return;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->dataSubscribers.erase(token);
}

void SerialCommunication::removeConnectionCallback(CallbackToken token)
{
    if (token == 0) return;
    std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
    m_impl->connectionSubscribers.erase(token);
}

void SerialCommunication::notifyDataCallbacks(const SensorData& data)
{
    std::vector<DataCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
        if (m_impl->dataCallback) callbacks.push_back(m_impl->dataCallback);
        for (const auto& item : m_impl->dataSubscribers) callbacks.push_back(item.second);
    }
    for (const auto& callback : callbacks) {
        try { callback(data); } catch (...) { /* isolate subscriber failures from serial reader */ }
    }
}

void SerialCommunication::notifyConnectionCallbacks(bool connected, const std::string& message)
{
    std::vector<ConnectionCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_impl->callbackMutex);
        if (m_impl->connectionCallback) callbacks.push_back(m_impl->connectionCallback);
        for (const auto& item : m_impl->connectionSubscribers) callbacks.push_back(item.second);
    }
    for (const auto& callback : callbacks) {
        try { callback(connected, message); } catch (...) { /* isolate subscriber failures from device state */ }
    }
}

void SerialCommunication::startAsyncReading() {
    std::lock_guard<std::mutex> lifecycleLock(m_impl->asyncLifecycleMutex);

    // Repeated starts while the worker is active are idempotent. A callback
    // running on the worker cannot synchronously restart and join itself.
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncWorkerId == std::this_thread::get_id()) return;
    }
    if (m_impl->asyncReading.load(std::memory_order_acquire)) return;

    // Reap a completed worker before assigning a new std::thread.
    std::thread previousWorker;
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncThread.joinable()) previousWorker = std::move(m_impl->asyncThread);
    }
    if (previousWorker.joinable()) previousWorker.join();

    if (!m_impl->isConnected) return;
    m_impl->asyncReading.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        m_impl->asyncThread = std::thread([this]() {
            {
                std::lock_guard<std::mutex> workerLock(m_impl->asyncThreadMutex);
                m_impl->asyncWorkerId = std::this_thread::get_id();
            }
            while (m_impl->asyncReading.load(std::memory_order_acquire) &&
                   m_impl->isConnected) {
                SensorData data = readData(100);
                if (data.hasFrame) notifyDataCallbacks(data);
                std::this_thread::sleep_for(std::chrono::milliseconds(12));
            }
            {
                std::lock_guard<std::mutex> workerLock(m_impl->asyncThreadMutex);
                m_impl->asyncWorkerId = std::thread::id{};
            }
        });
    }
}

void SerialCommunication::stopAsyncReading() {
    m_impl->asyncReading.store(false, std::memory_order_release);
    // A callback may request stop/disconnect from this worker. Never join self;
    // leave the joinable handle for the next external stop, restart, or destructor.
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncWorkerId == std::this_thread::get_id()) return;
    }

    std::lock_guard<std::mutex> lifecycleLock(m_impl->asyncLifecycleMutex);
    std::thread worker;
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncThread.joinable()) worker = std::move(m_impl->asyncThread);
    }
    if (worker.joinable()) worker.join();
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

std::string SerialCommunication::sendCommand(const std::string& cmd, bool waitForResponse) {
    // The wire protocol is line-oriented. Never allow a caller to append
    // another command or inject control bytes into the serial stream.
    constexpr std::size_t MaxCommandLength = 128;
    if (cmd.empty() || cmd.size() > MaxCommandLength ||
        cmd.find_first_of("\r\n") != std::string::npos ||
        cmd.find('\0') != std::string::npos) {
        m_impl->lastError = "Rejected invalid serial command";
        return "";
    }
    for (const unsigned char ch : cmd) {
        if (ch < 0x20 || ch > 0x7e) {
            m_impl->lastError = "Rejected invalid serial command";
            return "";
        }
    }
    if (!m_impl->isConnected) return "";
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    std::string toSend = cmd + "\n";
if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size())) {
        return "";
    }
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    if (!waitForResponse) return "";
    constexpr size_t MaxResponseBytes = 4096;
    std::string resp;
    resp.reserve(512);
    uint8_t buf[256];
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (resp.size() < MaxResponseBytes &&
           std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        const int rd = readFromPort(buf, std::min(sizeof(buf), MaxResponseBytes - resp.size()),
                                    static_cast<int>(std::max<int64_t>(1, remaining.count())));
        if (rd < 0) break;
        if (rd > 0) {
            resp.append(reinterpret_cast<const char*>(buf), static_cast<size_t>(rd));
            if (resp.find("[DATA_SENT]") != std::string::npos ||
                resp.find("OK") != std::string::npos ||
                resp.find("FORCE:") != std::string::npos) break;
        }
    }
    if (m_impl->verbose && !resp.empty()) std::cout << "[SerialComm] " << resp << std::endl;
    return resp;
}


bool SerialCommunication::sendRawData(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    if (!writeToPort(data, len)) return false;
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    return true;
}

void SerialCommunication::setVerbose(bool v) { m_impl->verbose = v; }
std::string SerialCommunication::getLastError() const { return m_impl->lastError; }
void SerialCommunication::setCRCPolynomial(uint8_t) {}

} // namespace sensor
