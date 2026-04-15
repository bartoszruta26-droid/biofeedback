/**
 * @file SerialCommunication.cpp
 * @brief Implementacja komunikacji szeregowej z Arduino NANO
 * 
 * Zawiera implementację metod klasy SerialCommunication do obsługi:
 * - CRC8 (Cyclic Redundancy Check)
 * - Watchdog (monitorowanie połączenia)
 * - Timestamp (synchronizacja czasu)
 * - Value (odczyt wartości z czujnika)
 * - Tarowanie (kalibracja)
 * - Identyfikacja Arduino i portu
 */

#include "sensor/SerialCommunication.hpp"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
#endif

namespace sensor {

// ============================================================================
// CRC8 Implementation
// ============================================================================

uint8_t SerialCommunication::calculateCRC8(const uint8_t* data, size_t length) const {
    uint8_t crc = 0x00;
    const uint8_t polynomial = 0x07; // CRC-8 polynomial (x^8 + x^2 + x + 1)
    
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

bool SerialCommunication::verifyCRC8(const uint8_t* data, size_t length, uint8_t expectedCRC) const {
    return calculateCRC8(data, length) == expectedCRC;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

SerialCommunication::SerialCommunication() 
    : m_portHandle(-1)
    , m_isConnected(false)
    , m_baudRate(DEFAULT_BAUD_RATE)
    , m_watchdogTimeout(WATCHDOG_DEFAULT_TIMEOUT_MS)
    , m_lastActivityTime(std::chrono::steady_clock::now())
    , m_zeroOffset(0.0f)
    , m_scaleFactor(1.0f)
    , m_isCalibrated(false) {
}

SerialCommunication::~SerialCommunication() {
    disconnect();
}

// ============================================================================
// Port Identification
// ============================================================================

std::vector<std::string> SerialCommunication::scanAvailablePorts() const {
    std::vector<std::string> availablePorts;
    
#ifdef _WIN32
    // Windows: enumerate COM ports
    for (int i = 0; i < 256; ++i) {
        std::stringstream ss;
        ss << "\\\\.\\COM" << i;
        HANDLE hComm = CreateFile(ss.str().c_str(), 
                                   GENERIC_READ | GENERIC_WRITE,
                                   0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hComm != INVALID_HANDLE_VALUE) {
            std::string portName = "COM" + std::to_string(i);
            availablePorts.push_back(portName);
            CloseHandle(hComm);
        }
    }
#else
    // Linux/Mac: scan /dev directory
    DIR* dir = opendir("/dev");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            
            // Common Arduino/NANO patterns
            bool isArduino = (name.find("ttyUSB") == 0 ||      // Linux USB serial
                             name.find("ttyACM") == 0 ||       // Linux ACM devices
                             name.find("cu.usbserial") == 0 || // Mac FTDI
                             name.find("cu.usbmodem") == 0);   // Mac CDC
            
            if (isArduino) {
                availablePorts.push_back("/dev/" + name);
            }
        }
        closedir(dir);
    }
#endif
    
    return availablePorts;
}

bool SerialCommunication::identifyPort(std::string& foundPort, ArduinoInfo& info) {
    auto ports = scanAvailablePorts();
    
    for (const auto& port : ports) {
        if (connect(port)) {
            if (getArduinoInfo(info)) {
                foundPort = port;
                return true;
            }
            disconnect();
        }
    }
    return false;
}

// ============================================================================
// Connection Management
// ============================================================================

bool SerialCommunication::connect(const std::string& port, uint32_t baudRate) {
    if (m_isConnected) {
        disconnect();
    }
    
    m_baudRate = baudRate;
    
#ifdef _WIN32
    std::string fullPortName = "\\\\.\\" + port;
    m_portHandle = CreateFile(fullPortName.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (m_portHandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(m_portHandle, &dcb)) {
        CloseHandle(m_portHandle);
        m_portHandle = -1;
        return false;
    }
    
    dcb.BaudRate = m_baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    
    if (!SetCommState(m_portHandle, &dcb)) {
        CloseHandle(m_portHandle);
        m_portHandle = -1;
        return false;
    }
    
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(m_portHandle, &timeouts);
    
#else
    // POSIX systems
    m_portHandle = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_portHandle < 0) {
        return false;
    }
    
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    
    if (tcgetattr(m_portHandle, &tty) != 0) {
        close(m_portHandle);
        m_portHandle = -1;
        return false;
    }
    
    // Set baud rate
    speed_t speed;
    switch (m_baudRate) {
        case 9600: speed = B9600; break;
        case 19200: speed = B19200; break;
        case 38400: speed = B38400; break;
        case 57600: speed = B57600; break;
        case 115200: speed = B115200; break;
        default: speed = B115200;
    }
    
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);
    
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;
    
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10; // 1 second timeout
    
    tcflush(m_portHandle, TCIFLUSH);
    
    if (tcsetattr(m_portHandle, TCSANOW, &tty) != 0) {
        close(m_portHandle);
        m_portHandle = -1;
        return false;
    }
#endif
    
    m_isConnected = true;
    m_lastActivityTime = std::chrono::steady_clock::now();
    
    // Small delay to allow Arduino to reset
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    return true;
}

void SerialCommunication::disconnect() {
    if (!m_isConnected) {
        return;
    }
    
#ifdef _WIN32
    if (m_portHandle != -1 && m_portHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(reinterpret_cast<HANDLE>(m_portHandle));
    }
#else
    if (m_portHandle >= 0) {
        close(m_portHandle);
    }
#endif
    
    m_portHandle = -1;
    m_isConnected = false;
}

bool SerialCommunication::isConnected() const {
    return m_isConnected;
}

// ============================================================================
// Watchdog Implementation
// ============================================================================

void SerialCommunication::setWatchdogTimeout(uint32_t timeoutMs) {
    m_watchdogTimeout = timeoutMs;
}

uint32_t SerialCommunication::getWatchdogTimeout() const {
    return m_watchdogTimeout;
}

bool SerialCommunication::checkWatchdog() {
    if (!m_isConnected) {
        return false;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastActivityTime).count();
    
    if (elapsed > static_cast<int64_t>(m_watchdogTimeout)) {
        // Connection timeout - attempt reconnection or notify
        return false;
    }
    
    return true;
}

void SerialCommunication::resetWatchdog() {
    m_lastActivityTime = std::chrono::steady_clock::now();
}

// ============================================================================
// Timestamp Synchronization
// ============================================================================

int64_t SerialCommunication::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

bool SerialCommunication::syncTimestamp(int64_t& arduinoTimestamp) {
    if (!m_isConnected) {
        return false;
    }
    
    int64_t hostTimestamp = getCurrentTimestamp();
    
    // Send sync command to Arduino
    std::vector<uint8_t> cmd = {'T', 'S', 'Y', 'N', 'C'};
    cmd.push_back(calculateCRC8(cmd.data(), cmd.size()));
    
    if (!writeData(cmd)) {
        return false;
    }
    
    // Wait for response
    std::vector<uint8_t> response;
    if (!readData(response, 13)) { // Expected response size
        return false;
    }
    
    // Parse timestamp from response
    if (response.size() >= 9) {
        arduinoTimestamp = 0;
        for (int i = 0; i < 8; ++i) {
            arduinoTimestamp |= (static_cast<int64_t>(response[i]) << (i * 8));
        }
        
        // Calculate offset for future synchronization
        m_timestampOffset = hostTimestamp - arduinoTimestamp;
        return true;
    }
    
    return false;
}

int64_t SerialCommunication::getSynchronizedTimestamp() const {
    return getCurrentTimestamp() - m_timestampOffset;
}

// ============================================================================
// Data Reading/Writing
// ============================================================================

bool SerialCommunication::writeData(const std::vector<uint8_t>& data) {
    if (!m_isConnected || data.empty()) {
        return false;
    }
    
    resetWatchdog();
    
#ifdef _WIN32
    DWORD bytesWritten;
    BOOL result = WriteFile(reinterpret_cast<HANDLE>(m_portHandle),
                            data.data(),
                            static_cast<DWORD>(data.size()),
                            &bytesWritten,
                            nullptr);
    return (result && bytesWritten == data.size());
#else
    ssize_t bytesWritten = write(m_portHandle, data.data(), data.size());
    return (bytesWritten == static_cast<ssize_t>(data.size()));
#endif
}

bool SerialCommunication::readData(std::vector<uint8_t>& buffer, size_t expectedLength, 
                                    uint32_t timeoutMs) {
    if (!m_isConnected) {
        return false;
    }
    
    buffer.clear();
    buffer.reserve(expectedLength);
    
    auto startTime = std::chrono::steady_clock::now();
    
    while (buffer.size() < expectedLength) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        
        if (elapsed > timeoutMs) {
            return false; // Timeout
        }
        
        uint8_t byte;
        ssize_t bytesRead = 0;
        
#ifdef _WIN32
        DWORD dwBytesRead;
        BOOL result = ReadFile(reinterpret_cast<HANDLE>(m_portHandle),
                               &byte, 1, &dwBytesRead, nullptr);
        if (result && dwBytesRead > 0) {
            bytesRead = 1;
        }
#else
        bytesRead = read(m_portHandle, &byte, 1);
#endif
        
        if (bytesRead > 0) {
            buffer.push_back(byte);
            resetWatchdog();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    return true;
}

// ============================================================================
// Sensor Value Reading
// ============================================================================

bool SerialCommunication::readSensorData(SensorData& data) {
    if (!m_isConnected) {
        return false;
    }
    
    // Send read command
    std::vector<uint8_t> cmd = {'R', 'E', 'A', 'D'};
    cmd.push_back(calculateCRC8(cmd.data(), cmd.size()));
    
    if (!writeData(cmd)) {
        return false;
    }
    
    // Expect response: [value:4][timestamp:8][status:1][crc:1] = 14 bytes
    std::vector<uint8_t> response;
    if (!readData(response, 14, 1000)) {
        return false;
    }
    
    // Verify CRC
    uint8_t receivedCRC = response.back();
    if (!verifyCRC8(response.data(), response.size() - 1, receivedCRC)) {
        return false; // CRC error
    }
    
    // Parse value (4 bytes float)
    float rawValue = 0.0f;
    std::memcpy(&rawValue, response.data(), 4);
    
    // Parse timestamp (8 bytes int64)
    data.timestamp = 0;
    for (int i = 0; i < 8; ++i) {
        data.timestamp |= (static_cast<int64_t>(response[4 + i]) << (i * 8));
    }
    
    // Parse status
    data.status = static_cast<SensorStatus>(response[12]);
    
    // Apply calibration
    data.rawValue = rawValue;
    data.calibratedValue = calibrateValue(rawValue);
    
    return true;
}

float SerialCommunication::calibrateValue(float rawValue) const {
    return (rawValue - m_zeroOffset) * m_scaleFactor;
}

// ============================================================================
// Calibration (Tarowanie)
// ============================================================================

bool SerialCommunication::zeroCalibrate() {
    if (!m_isConnected) {
        return false;
    }
    
    // Send zero calibration command
    std::vector<uint8_t> cmd = {'C', 'A', 'L', 'Z', 'E', 'R', 'O'};
    cmd.push_back(calculateCRC8(cmd.data(), cmd.size()));
    
    if (!writeData(cmd)) {
        return false;
    }
    
    // Wait for acknowledgment
    std::vector<uint8_t> response;
    if (!readData(response, 5, 2000)) {
        return false;
    }
    
    if (response[0] == 'O' && response[1] == 'K') {
        m_zeroOffset = 0.0f;
        m_isCalibrated = true;
        return true;
    }
    
    return false;
}

bool SerialCommunication::twoPointCalibrate(float knownValue1, float knownValue2) {
    if (!m_isConnected) {
        return false;
    }
    
    // First point: zero
    if (!zeroCalibrate()) {
        return false;
    }
    
    // Read raw value at first point
    SensorData data1;
    if (!readSensorData(data1)) {
        return false;
    }
    
    // Send second calibration point to Arduino
    std::vector<uint8_t> cmd = {'C', 'A', 'L', 'P', '2'};
    float tempValue = knownValue2;
    uint8_t* valueBytes = reinterpret_cast<uint8_t*>(&tempValue);
    for (int i = 0; i < 4; ++i) {
        cmd.push_back(valueBytes[i]);
    }
    cmd.push_back(calculateCRC8(cmd.data(), cmd.size()));
    
    if (!writeData(cmd)) {
        return false;
    }
    
    // Read raw value at second point
    SensorData data2;
    if (!readSensorData(data2)) {
        return false;
    }
    
    // Calculate scale factor
    float rawDiff = data2.rawValue - data1.rawValue;
    float knownDiff = knownValue2 - knownValue1;
    
    if (std::abs(rawDiff) < 0.0001f) {
        return false; // Division by zero protection
    }
    
    m_scaleFactor = knownDiff / rawDiff;
    m_isCalibrated = true;
    
    return true;
}

bool SerialCommunication::isCalibrated() const {
    return m_isCalibrated;
}

void SerialCommunication::resetCalibration() {
    m_zeroOffset = 0.0f;
    m_scaleFactor = 1.0f;
    m_isCalibrated = false;
}

// ============================================================================
// Arduino Identification
// ============================================================================

bool SerialCommunication::getArduinoInfo(ArduinoInfo& info) {
    if (!m_isConnected) {
        return false;
    }
    
    // Send identification command
    std::vector<uint8_t> cmd = {'W', 'H', 'O', 'A', 'M', 'I'};
    cmd.push_back(calculateCRC8(cmd.data(), cmd.size()));
    
    if (!writeData(cmd)) {
        return false;
    }
    
    // Expect response with device info
    std::vector<uint8_t> response;
    if (!readData(response, 64, 2000)) {
        return false;
    }
    
    // Verify CRC
    uint8_t receivedCRC = response.back();
    if (!verifyCRC8(response.data(), response.size() - 1, receivedCRC)) {
        return false;
    }
    
    // Parse response (format: [name_len][name][fw_major][fw_minor][board_id])
    size_t pos = 0;
    uint8_t nameLen = response[pos++];
    
    if (pos + nameLen > response.size() - 6) {
        return false;
    }
    
    info.deviceName = std::string(reinterpret_cast<char*>(&response[pos]), nameLen);
    pos += nameLen;
    
    info.firmwareMajor = response[pos++];
    info.firmwareMinor = response[pos++];
    
    info.boardId = 0;
    for (int i = 0; i < 4; ++i) {
        info.boardId |= (static_cast<uint32_t>(response[pos + i]) << (i * 8));
    }
    
    info.isDetected = true;
    info.detectedAt = getCurrentTimestamp();
    
    return true;
}

std::string SerialCommunication::getDeviceName() const {
    return m_deviceInfo.deviceName;
}

FirmwareVersion SerialCommunication::getFirmwareVersion() const {
    return FirmwareVersion{m_deviceInfo.firmwareMajor, m_deviceInfo.firmwareMinor};
}

// ============================================================================
// Utility Methods
// ============================================================================

void SerialCommunication::setConfig(const SerialConfig& config) {
    m_baudRate = config.baudRate;
    m_watchdogTimeout = config.watchdogTimeoutMs;
    m_zeroOffset = config.zeroOffset;
    m_scaleFactor = config.scaleFactor;
}

SerialConfig SerialCommunication::getConfig() const {
    return SerialConfig{
        m_baudRate,
        m_watchdogTimeout,
        m_zeroOffset,
        m_scaleFactor
    };
}

} // namespace sensor
