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
#include "sensor/ProtocolParser.hpp"
#include "core/DebugManager.hpp"
#include <chrono>
#include <cmath>
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
#include <filesystem>
#include <set>

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
    std::atomic<bool> isConnected{false};
    // Protect connection metadata snapshots independently from serial I/O.
    mutable std::mutex stateMutex;
    int baudRate = protocol::DefaultBaudRate;
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
    // Serialize complete open/close state transitions, separately from I/O.
    std::mutex connectionMutex;
    std::mutex readMutex;
    // Persistent parser for the device's continuous binary stream. Keeping parser
    // state across reads preserves frames split across USB/UART transactions.
    ProtocolParser streamParser;
    std::atomic<bool> continuousMode{false};
    mutable std::mutex errorMutex;
    std::string lastError;
    std::atomic<bool> verbose{false};
    mutable std::mutex timestampMutex;
    int32_t timestampOffset = 0;
    mutable std::mutex watchdogMutex;
    mutable std::mutex calibrationMutex;
    void markActivity() {
        std::lock_guard<std::mutex> lock(watchdogMutex);
        lastActivityTime = std::chrono::steady_clock::now();
    }
    void setLastError(std::string error) {
        std::lock_guard<std::mutex> lock(errorMutex);
        lastError = std::move(error);
    }
    std::string getLastErrorSnapshot() const {
        std::lock_guard<std::mutex> lock(errorMutex);
        return lastError;
    }
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
    std::vector<std::string> ports;

    try {
#ifdef _WIN32
        // Probe standard Windows COM names without assuming a particular USB
        // serial driver or Arduino board identifier.
        for (int i = 1; i <= 256; ++i) {
            const std::string name = "COM" + std::to_string(i);
            const std::string devicePath = "\\\\.\\" + name;
            HANDLE handle = CreateFileA(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE,
                                        0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (handle != INVALID_HANDLE_VALUE) {
                ports.push_back(name);
                CloseHandle(handle);
            }
        }
#else
        namespace fs = std::filesystem;
        std::set<std::string> seenDevices;

        const auto addDevice = [&ports, &seenDevices](const fs::path& candidate) {
            std::error_code ec;
            if (!fs::exists(candidate, ec) || ec) return;
            if (!fs::is_character_file(candidate, ec) || ec) return;

            // Prefer stable /dev/serial/by-id aliases when present, while
            // avoiding duplicate entries for their /dev/ttyUSB* targets.
            fs::path canonical = fs::canonical(candidate, ec);
            const std::string identity = ec ? candidate.string() : canonical.string();
            if (seenDevices.insert(identity).second)
                ports.push_back(candidate.string());
        };

        const auto startsWith = [](const std::string& value, const char* prefix) {
            return value.rfind(prefix, 0) == 0;
        };

        std::error_code ec;

        // Add stable udev aliases first so the user sees a descriptive board
        // name instead of a transient ttyUSB/ttyACM number.
        for (const fs::path& aliasDirectory : {
                 fs::path("/dev/serial/by-id"), fs::path("/dev/serial/by-path")}) {
            ec.clear();
            if (!fs::is_directory(aliasDirectory, ec) || ec) continue;
            for (fs::directory_iterator it(aliasDirectory, ec), end; !ec && it != end; it.increment(ec))
                addDevice(it->path());
        }

        const fs::path devRoot("/dev");
        ec.clear();
        if (fs::is_directory(devRoot, ec) && !ec) {
            for (fs::directory_iterator it(devRoot, ec), end; !ec && it != end; it.increment(ec)) {
                const std::string name = it->path().filename().string();
                if (startsWith(name, "ttyUSB") || startsWith(name, "ttyACM") ||
                    startsWith(name, "ttyS") || startsWith(name, "ttyAMA") ||
                    startsWith(name, "ttyTHS") || startsWith(name, "ttyGS") ||
                    startsWith(name, "rfcomm") || startsWith(name, "cu.usbserial") ||
                    startsWith(name, "cu.usbmodem")) {
                    addDevice(it->path());
                }
            }
        }
#endif
        std::sort(ports.begin(), ports.end());
    } catch (const std::exception& e) {
        core::DebugManager::instance().sendDebugMessage(
            QString("Serial port scan failed: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "SerialCommunication::scanAvailablePorts");
    } catch (...) {
        core::DebugManager::instance().sendDebugMessage(
            "Unknown error during serial port scan",
            core::DebugLevel::ERROR,
            "SerialCommunication::scanAvailablePorts");
    }

    return ports;
}

bool SerialCommunication::connect(const SerialConfig& config) { return openPort(config.portName, config.baudRate); }

bool SerialCommunication::connect(const std::string& portName, int baudRate) { return openPort(portName, baudRate); }

bool SerialCommunication::openPort(const std::string& portName, int baudRate) {
    // Reap any asynchronous worker before acquiring the connection-state lock:
    // callbacks running on that worker may themselves request a connection change.
    stopAsyncReading();
    std::unique_lock<std::mutex> connectionLock(m_impl->connectionMutex);
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
                QString("Invalid baud rate %1. Using default %2.").arg(baudRate).arg(protocol::DefaultBaudRate),
                core::DebugLevel::WARNING,
                "SerialCommunication::openPort"
            );
            baudRate = protocol::DefaultBaudRate;
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
            // openPort already owns connectionMutex; tear down the old handle
            // inline rather than recursively calling disconnect().
            std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
#ifdef _WIN32
            if (m_impl->portHandle != -1) CloseHandle(reinterpret_cast<HANDLE>(m_impl->portHandle));
#else
            if (m_impl->portHandle >= 0) close(static_cast<int>(m_impl->portHandle));
#endif
            m_impl->isConnected.store(false, std::memory_order_release);
            m_impl->portHandle = -1;
            { std::lock_guard<std::mutex> stateLock(m_impl->stateMutex); m_impl->portName.clear(); }
        }
        
        { std::lock_guard<std::mutex> stateLock(m_impl->stateMutex); m_impl->baudRate = baudRate; }
        intptr_t newPortHandle = -1;
        
#ifdef _WIN32
        HANDLE h = CreateFileA(("\\\\.\\" + portName).c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            const DWORD errorCode = GetLastError();
            m_impl->setLastError("Cannot open " + portName + ": Windows error " + std::to_string(errorCode));
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
            m_impl->setLastError("Cannot configure serial port timeouts");
            return false;
        }
        newPortHandle = reinterpret_cast<intptr_t>(h);
#else
        int fd = open(portName.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK);
        if (fd < 0) {
            const std::string error = std::strerror(errno);
            m_impl->setLastError("Cannot open " + portName + ": " + error);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to open port %1: %2").arg(QString::fromStdString(portName)).arg(strerror(errno)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        termios tty{};
        if (tcgetattr(fd, &tty)) {
            const std::string error = std::strerror(errno);
            m_impl->setLastError("Cannot read terminal settings for " + portName + ": " + error);
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
            const std::string error = std::strerror(errno);
            m_impl->setLastError("Cannot configure terminal " + portName + ": " + error);
            close(fd);
            core::DebugManager::instance().sendDebugMessage(
                QString("Failed to set terminal attributes for %1").arg(QString::fromStdString(portName)),
                core::DebugLevel::ERROR,
                "SerialCommunication::openPort"
            );
            return false; 
        }
        int fl = fcntl(fd, F_GETFL); fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
        newPortHandle = fd;
#endif
        // Publish the native handle and connected state atomically with respect
        // to disconnect/read/write. A disconnect between handle creation and
        // publication must not leave an unowned open descriptor behind.
        {
            std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
            m_impl->portHandle = newPortHandle;
            {
                std::lock_guard<std::mutex> stateLock(m_impl->stateMutex);
                m_impl->portName = portName;
            }
            m_impl->isConnected.store(true, std::memory_order_release);
        }
        m_impl->markActivity();
        connectionLock.unlock();
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
    std::unique_lock<std::mutex> connectionLock(m_impl->connectionMutex);

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
        m_impl->isConnected.store(false, std::memory_order_release);
        m_impl->portHandle = -1;
        { std::lock_guard<std::mutex> stateLock(m_impl->stateMutex); m_impl->portName.clear(); }
    }
    // Release both lifecycle and I/O locks before invoking user callbacks:
    // callbacks may synchronously call connect() or disconnect().
    connectionLock.unlock();
    notifyConnectionCallbacks(false, "Disconnected");
}

bool SerialCommunication::writeToPort(const uint8_t* data, size_t length) {
    if (!m_impl->isConnected || m_impl->portHandle == -1 || data == nullptr || length == 0) {
        m_impl->setLastError("Serial write rejected: invalid connection or buffer");
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
            m_impl->setLastError("Serial write failed");
            return false;
        }
        offset += static_cast<size_t>(written);
    }
#else
    const int fd = static_cast<int>(m_impl->portHandle);
    const int originalFlags = fcntl(fd, F_GETFL);
    if (originalFlags < 0 || fcntl(fd, F_SETFL, originalFlags | O_NONBLOCK) < 0) {
        m_impl->setLastError("Serial write configuration failed");
        return false;
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    bool success = true;
    while (offset < length) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            m_impl->setLastError("Serial write timed out");
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
            m_impl->setLastError("Serial write timed out");
            success = false;
            break;
        }
        if (ready < 0 ||
            (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0 ||
            (descriptor.revents & POLLOUT) == 0) {
            m_impl->setLastError("Serial write failed");
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

        m_impl->setLastError("Serial write failed");
        success = false;
        break;
    }

    if (fcntl(fd, F_SETFL, originalFlags) < 0) {
        m_impl->setLastError("Serial write completed but port flags could not be restored");
        success = false;
    }
    if (!success) return false;
#endif
    return true;
}


int SerialCommunication::readFromPort(uint8_t* buffer, size_t maxLength, int timeout) {
    if (!m_impl->isConnected || m_impl->portHandle == -1 ||
        buffer == nullptr || maxLength == 0 || timeout < 0) {
        m_impl->setLastError("Serial read rejected: invalid connection, buffer, or timeout");
        return -1;
    }

#ifdef _WIN32
    HANDLE handle = reinterpret_cast<HANDLE>(m_impl->portHandle);
    COMMTIMEOUTS originalTimeouts{};
    if (!GetCommTimeouts(handle, &originalTimeouts)) {
        m_impl->setLastError("Serial read timeout configuration unavailable");
        return -1;
    }

    COMMTIMEOUTS readTimeouts = originalTimeouts;
    readTimeouts.ReadIntervalTimeout = timeout == 0 ? MAXDWORD : 0;
    readTimeouts.ReadTotalTimeoutMultiplier = 0;
    readTimeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(timeout);
    if (!SetCommTimeouts(handle, &readTimeouts)) {
        m_impl->setLastError("Serial read timeout configuration failed");
        return -1;
    }

    DWORD bytesRead = 0;
    const DWORD requested = static_cast<DWORD>(std::min(
        maxLength, static_cast<size_t>(INT_MAX)));
    const BOOL readSucceeded = ReadFile(handle, buffer, requested, &bytesRead, nullptr);
    const BOOL restoreSucceeded = SetCommTimeouts(handle, &originalTimeouts);
    if (!restoreSucceeded) {
        m_impl->setLastError("Serial read completed but timeout settings could not be restored");
        return -1;
    }
    if (!readSucceeded) {
        m_impl->setLastError("Serial read failed");
        return -1;
    }
    return static_cast<int>(bytesRead);
#else
    const int fd = static_cast<int>(m_impl->portHandle);
    const int originalFlags = fcntl(fd, F_GETFL);
    if (originalFlags < 0 || fcntl(fd, F_SETFL, originalFlags | O_NONBLOCK) < 0) {
        m_impl->setLastError("Serial read configuration failed");
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
            m_impl->setLastError("Serial read failed");
            result = -1;
            break;
        }
        if ((descriptor.revents & POLLHUP) != 0 &&
            (descriptor.revents & POLLIN) == 0) {
            m_impl->setLastError("Serial port closed during read");
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
        m_impl->setLastError("Serial read failed");
        result = -1;
        break;
    }

    if (fcntl(fd, F_SETFL, originalFlags) < 0) {
        m_impl->setLastError("Serial read completed but port flags could not be restored");
        return -1;
    }
    return result;
#endif
}

bool SerialCommunication::isConnected() const { return m_impl->isConnected.load(std::memory_order_acquire); }
std::string SerialCommunication::getPortName() const {
    std::lock_guard<std::mutex> stateLock(m_impl->stateMutex);
    return m_impl->isConnected.load(std::memory_order_acquire) ? m_impl->portName : "";
}
int SerialCommunication::getBaudRate() const {
    std::lock_guard<std::mutex> stateLock(m_impl->stateMutex);
    return m_impl->baudRate;
}

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
        { std::lock_guard<std::mutex> stateLock(m_impl->stateMutex); m_impl->arduinoInfo = info; }
    }
    return info;
}

bool SerialCommunication::autoConnect(int maxAttempts) {
    int attempt = 0;
    while (maxAttempts <= 0 || attempt < maxAttempts) {
        for (const auto& p : scanAvailablePorts()) {
            std::cout << "[SerialComm] Trying " << p << std::endl;
            if (connect(p, protocol::DefaultBaudRate)) {
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

ArduinoInfo SerialCommunication::getArduinoInfo() const {
    std::lock_guard<std::mutex> stateLock(m_impl->stateMutex);
    return m_impl->arduinoInfo;
}

std::vector<SensorData> SerialCommunication::readDataBatch(int timeout) {
    std::vector<SensorData> samples;
    if (timeout < 0) {
        m_impl->setLastError("Serial read rejected: negative timeout");
        return samples;
    }
    if (!m_impl->isConnected) return samples;

    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    const std::string toSend = "DATA\n";
    if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size())) {
        return samples;
    }
    m_impl->markActivity();

    uint8_t buf[256];
    constexpr std::size_t MaxResponseBytes = 4096;
    std::vector<uint8_t> rawData;
    rawData.reserve(512);
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeout);
    ProtocolParser binaryParser;

    while (rawData.size() < MaxResponseBytes &&
           std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        const int rd = readFromPort(buf,
            std::min(sizeof(buf), MaxResponseBytes - rawData.size()),
            static_cast<int>(std::max<int64_t>(1, remaining.count())));
        if (rd < 0) break;
        if (rd == 0) continue;

        binaryParser.feed(buf, static_cast<std::size_t>(rd));
        for (int i = 0; i < rd && rawData.size() < MaxResponseBytes; ++i) {
            rawData.push_back(buf[i]);
        }

        // Preserve every complete CRC-valid frame in wire order.
        while (const auto frame = binaryParser.takeFrame()) {
            SensorData sample;
            sample.timestamp = frame->timestampMs;
            sample.hasTimestamp = true;
            sample.value = frame->rawValue;
            sample.crc = frame->crc;
            sample.isValid = true;
            sample.hasFrame = true;
            samples.push_back(sample);
        }

        const std::string response(rawData.begin(), rawData.end());
        if (response.find("[DATA_SENT]") != std::string::npos && !samples.empty()) {
            break;
        }
    }

    // FORCE text describes the DATA transaction, not each extra continuous frame.
    const std::string response(rawData.begin(), rawData.end());
    const std::size_t forcePos = response.find("FORCE:");
    if (forcePos != std::string::npos) {
        const std::size_t gramsEnd = response.find("g,", forcePos);
        bool calibrationConfirmed = false;
        const std::size_t calibrationPos = response.find("CAL:", forcePos);
        if (calibrationPos != std::string::npos) {
            try {
                calibrationConfirmed = std::stoi(response.substr(calibrationPos + 4)) == 1;
            } catch (...) {}
        }
        // A text-only response still carries its raw value and device timestamp.
        // Parse those fields before creating the calibrated sample; otherwise
        // emplacing the sample for FORCE would skip the fallback RAW/TS parser.
        SensorData fallback;
        const std::size_t rawPos = response.find("RAW:", forcePos);
        if (rawPos != std::string::npos) {
            const std::size_t rawEnd = response.find(",", rawPos + 4);
            if (rawEnd != std::string::npos) {
                try {
                    fallback.value = std::stoi(
                        response.substr(rawPos + 4, rawEnd - rawPos - 4));
                } catch (...) {}
            }
            const std::size_t timestampPos = response.find("TS:", rawPos);
            if (timestampPos != std::string::npos) {
                const std::size_t timestampEnd = response.find("ms", timestampPos);
                if (timestampEnd != std::string::npos) {
                    try {
                        fallback.timestamp = static_cast<uint32_t>(std::stoul(
                            response.substr(timestampPos + 3,
                                timestampEnd - timestampPos - 3)));
                        fallback.hasTimestamp = true;
                    } catch (...) {}
                }
            }
        }

        if (samples.empty()) {
            fallback.hasFrame = !rawData.empty();
            samples.push_back(fallback);
        }

        if (gramsEnd != std::string::npos) {
            try {
                const double grams = std::stod(
                    response.substr(forcePos + 6, gramsEnd - forcePos - 6));
                if (std::isfinite(grams)) {
                    samples.front().calibratedValue = grams;
                    samples.front().hasCalibratedValue = calibrationConfirmed;
                    samples.front().hasFrame = !rawData.empty();
                    samples.front().isValid = true;
                }
            } catch (...) {}
        }
    } else if (samples.empty() && !rawData.empty()) {
        SensorData responseOnly;
        responseOnly.hasFrame = true;
        samples.push_back(responseOnly);
    }

    if (m_impl->verbose && !samples.empty() && samples.front().isValid) {
        const auto& first = samples.front();
        std::cout << "[SerialComm] Batch: " << samples.size()
                  << " frame(s), first TS=" << first.timestamp
                  << ", Value=" << first.value
                  << ", CRC=" << static_cast<int>(first.crc) << std::endl;
    }
    if (!rawData.empty()) m_impl->markActivity();
    return samples;
}

SensorData SerialCommunication::readData(int timeout) {
    auto samples = readDataBatch(timeout);
    return samples.empty() ? SensorData{} : samples.front();
}

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
    // Check before taking the lifecycle lock: an external stop may hold that
    // lock while joining this worker, so a callback must never wait on it.
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncWorkerId == std::this_thread::get_id()) return;
    }

    std::lock_guard<std::mutex> lifecycleLock(m_impl->asyncLifecycleMutex);
    if (m_impl->asyncReading.load(std::memory_order_acquire)) return;

    std::thread previousWorker;
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        if (m_impl->asyncThread.joinable()) previousWorker = std::move(m_impl->asyncThread);
    }
    if (previousWorker.joinable()) previousWorker.join();
    if (!m_impl->isConnected) return;

    // Switch the firmware to push mode once. The previous implementation sent
    // DATA and waited for DATA_SENT on every iteration, limiting acquisition
    // to roughly 10 Hz and adding a transaction timeout to every sample.
    {
        std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
        m_impl->streamParser.reset();
        const std::string command = "CONT\n";
        if (!writeToPort(reinterpret_cast<const uint8_t*>(command.data()), command.size()))
            return;
    }
    m_impl->continuousMode.store(true, std::memory_order_release);
    m_impl->asyncReading.store(true, std::memory_order_release);
    {
        std::lock_guard<std::mutex> threadLock(m_impl->asyncThreadMutex);
        m_impl->asyncThread = std::thread([this]() {
            {
                std::lock_guard<std::mutex> workerLock(m_impl->asyncThreadMutex);
                m_impl->asyncWorkerId = std::this_thread::get_id();
            }

            uint8_t buffer[128];
            while (m_impl->asyncReading.load(std::memory_order_acquire) &&
                   m_impl->isConnected) {
                int bytesRead = 0;
                {
                    std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
                    bytesRead = readFromPort(buffer, sizeof(buffer), 20);
                }

                if (bytesRead > 0) {
                    m_impl->markActivity();
                    m_impl->streamParser.feed(buffer, static_cast<std::size_t>(bytesRead));
                    while (const auto frame = m_impl->streamParser.takeFrame()) {
                        SensorData sample;
                        sample.timestamp = frame->timestampMs;
                        sample.hasTimestamp = true;
                        sample.value = frame->rawValue;
                        sample.crc = frame->crc;
                        sample.isValid = true;
                        sample.hasFrame = true;
                        notifyDataCallbacks(sample);
                    }
                } else if (bytesRead < 0) {
                    // A disconnected/failed descriptor must not turn the reader
                    // into a hot loop. External stop() still reaps this worker.
                    if (!m_impl->isConnected) break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
            }

            m_impl->asyncReading.store(false, std::memory_order_release);
            {
                std::lock_guard<std::mutex> workerLock(m_impl->asyncThreadMutex);
                m_impl->asyncWorkerId = std::thread::id{};
            }
        });
    }
}

void SerialCommunication::stopAsyncReading() {
    m_impl->asyncReading.store(false, std::memory_order_release);
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

    if (m_impl->continuousMode.exchange(false, std::memory_order_acq_rel) &&
        m_impl->isConnected) {
        const std::string command = "STOP\n";
        std::lock_guard<std::mutex> ioLock(m_impl->readMutex);
        (void)writeToPort(reinterpret_cast<const uint8_t*>(command.data()), command.size());
    }
}

void SerialCommunication::configureWatchdog(int timeoutMs, bool enabled) {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    m_impl->watchdogTimeoutMs = enabled ? timeoutMs : INT_MAX;
}
void SerialCommunication::resetWatchdog() { m_impl->markActivity(); }
bool SerialCommunication::isWatchdogOK() const {
    std::lock_guard<std::mutex> lock(m_impl->watchdogMutex);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - m_impl->lastActivityTime).count();
    return elapsed < m_impl->watchdogTimeoutMs;
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
    if (raw == 0) return;
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->scaleFactor = known / raw;
    m_impl->isCalibrated = true;
}
bool SerialCommunication::calibrate(int32_t, int32_t, double knownWeight) {
    if (!m_impl->isConnected) return false;
    const std::string response = sendCommand("CAL:" + std::to_string(knownWeight), true);
    const bool calibrated = response.find("Calibrated!") != std::string::npos;
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    m_impl->isCalibrated = calibrated;
    return calibrated;
}
double SerialCommunication::convertToCalibrated(int32_t raw) const {
    std::lock_guard<std::mutex> lock(m_impl->calibrationMutex);
    return (raw - m_impl->zeroOffset) * m_impl->scaleFactor;
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
    const uint32_t hostTimestamp = getCurrentTimestamp();
    const int32_t offset = (m_impl->isConnected && arduinoTimestamp != 0)
        ? static_cast<int32_t>(hostTimestamp - arduinoTimestamp) : 0;
    {
        std::lock_guard<std::mutex> lock(m_impl->timestampMutex);
        m_impl->timestampOffset = offset;
    }
#if DEBUG_SERIAL_COMM
    core::DebugManager::instance().sendDebugMessage(
        QString("Timestamp sync: Host=%1, Arduino=%2, Offset=%3 ms")
            .arg(hostTimestamp).arg(arduinoTimestamp).arg(offset),
        core::DebugLevel::DEBUG,
        "SerialCommunication::syncTimestamp"
    );
#endif
}
int32_t SerialCommunication::getTimestampOffset() const {
    std::lock_guard<std::mutex> lock(m_impl->timestampMutex);
    return m_impl->timestampOffset;
}

std::string SerialCommunication::sendCommand(const std::string& cmd, bool waitForResponse) {
    // The wire protocol is line-oriented. Never allow a caller to append
    // another command or inject control bytes into the serial stream.
    constexpr std::size_t MaxCommandLength = 128;
    if (cmd.empty() || cmd.size() > MaxCommandLength ||
        cmd.find_first_of("\r\n") != std::string::npos ||
        cmd.find('\0') != std::string::npos) {
        m_impl->setLastError("Rejected invalid serial command");
        return "";
    }
    for (const unsigned char ch : cmd) {
        if (ch < 0x20 || ch > 0x7e) {
            m_impl->setLastError("Rejected invalid serial command");
            return "";
        }
    }
    if (!m_impl->isConnected) return "";
    std::lock_guard<std::mutex> lock(m_impl->readMutex);
    std::string toSend = cmd + "\n";
if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size())) {
        return "";
    }
    m_impl->markActivity();
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
    m_impl->markActivity();
    return true;
}

void SerialCommunication::setVerbose(bool v) { m_impl->verbose.store(v, std::memory_order_release); }
std::string SerialCommunication::getLastError() const { return m_impl->getLastErrorSnapshot(); }
void SerialCommunication::setCRCPolynomial(uint8_t) {}

} // namespace sensor
