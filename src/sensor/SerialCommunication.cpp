/**
 * @file SerialCommunication.cpp
 * @brief Implementacja komunikacji szeregowej z Arduino NANO HX711
 */

#include "sensor/SerialCommunication.hpp"
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

uint8_t SerialCommunication::calculateCRC8(const uint8_t* data, size_t length) {
    uint8_t crc = 0xFF;
    const uint8_t polynomial = 0x07;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (crc & 0x80) crc = (crc << 1) ^ polynomial;
            else crc <<= 1;
        }
    }
    return crc;
}

bool SerialCommunication::verifyCRC8(const uint8_t* data, size_t length) {
    if (length < 1) return false;
    return calculateCRC8(data, length - 1) == data[length - 1];
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

SerialCommunication::SerialCommunication() : m_impl(std::make_unique<Impl>()) {}
SerialCommunication::~SerialCommunication() { disconnect(); }

std::vector<std::string> SerialCommunication::scanAvailablePorts() {
    std::vector<std::string> ports;
#ifdef _WIN32
    for (int i = 1; i <= 256; ++i) {
        std::stringstream ss; ss << "\\\\.\\" << "COM" << i;
        HANDLE h = CreateFileA(ss.str().c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
        if (h != INVALID_HANDLE_VALUE) { ports.push_back("COM" + std::to_string(i)); CloseHandle(h); }
    }
#else
    DIR* dir = opendir("/dev");
    if (dir) {
        struct dirent* e;
        while ((e = readdir(dir))) {
            std::string n = e->d_name;
            if (n.find("ttyUSB")==0 || n.find("ttyACM")==0 || n.find("cu.usbserial")==0 || n.find("cu.usbmodem")==0)
                ports.push_back("/dev/" + n);
        }
        closedir(dir);
    }
#endif
    return ports;
}

bool SerialCommunication::connect(const SerialConfig& config) { return openPort(config.portName, config.baudRate); }

bool SerialCommunication::openPort(const std::string& portName, int baudRate) {
    if (m_impl->isConnected) disconnect();
    m_impl->baudRate = baudRate;
#ifdef _WIN32
    HANDLE h = CreateFileA(("\\\\.\\" + portName).c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) { m_impl->lastError = "Cannot open " + portName; return false; }
    DCB dcb{}; dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(h, &dcb)) { CloseHandle(h); return false; }
    dcb.BaudRate = baudRate; dcb.ByteSize = 8; dcb.Parity = NOPARITY; dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(h, &dcb)) { CloseHandle(h); return false; }
    COMMTIMEOUTS to{}; to.ReadIntervalTimeout = 50; to.ReadTotalTimeoutConstant = 50;
    SetCommTimeouts(h, &to);
    m_impl->portHandle = reinterpret_cast<int>(h);
#else
    int fd = open(portName.c_str(), O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd < 0) { m_impl->lastError = "Cannot open " + portName; return false; }
    termios tty{};
    if (tcgetattr(fd, &tty)) { close(fd); return false; }
    speed_t spd = B115200;
    switch(baudRate) { case 9600:spd=B9600;break; case 19200:spd=B19200;break; case 38400:spd=B38400;break; case 57600:spd=B57600;break; case 230400:spd=B230400;break; }
    cfsetospeed(&tty, spd); cfsetispeed(&tty, spd);
    tty.c_cflag |= (CREAD|CLOCAL|CS8); tty.c_cflag &= ~(PARENB|CSTOPB|CRTSCTS);
    tty.c_lflag &= ~(ICANON|ECHO|ISIG); tty.c_oflag &= ~OPOST; tty.c_iflag &= ~(IXON|IXOFF);
    tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 10;
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &tty)) { close(fd); return false; }
    int fl = fcntl(fd, F_GETFL); fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
    m_impl->portHandle = fd;
#endif
    m_impl->isConnected = true;
    m_impl->lastActivityTime = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (m_impl->verbose) std::cout << "[SerialComm] Connected to " << portName << std::endl;
    return true;
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
    std::string resp = sendCommand("DATA", true);
    size_t fp = resp.find("FORCE:");
    if (fp != std::string::npos) {
        size_t gp = resp.find("g,", fp);
        if (gp != std::string::npos) data.calibratedValue = std::stod(resp.substr(fp+6, gp-fp-6));
        size_t rp = resp.find("RAW:", gp);
        if (rp != std::string::npos) {
            size_t commaPos = resp.find(",", rp+4);
            if (commaPos != std::string::npos) data.value = std::stoi(resp.substr(rp+4, commaPos-rp-4));
        }
        size_t tp = resp.find("TS:", rp);
        if (tp != std::string::npos) {
            size_t mp = resp.find("ms", tp);
            if (mp != std::string::npos) data.timestamp = std::stoul(resp.substr(tp+3, mp-tp-3));
        }
        data.isValid = true;
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
