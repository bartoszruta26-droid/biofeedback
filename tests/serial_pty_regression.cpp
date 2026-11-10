#include "sensor/SerialCommunication.hpp"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <poll.h>
#include <pty.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

namespace {

bool waitForFd(int fd, short events, int timeoutMs) {
    pollfd descriptor{};
    descriptor.fd = fd;
    descriptor.events = events;
    int result;
    do {
        result = poll(&descriptor, 1, timeoutMs);
    } while (result < 0 && errno == EINTR);
    return result > 0 && (descriptor.revents & events) != 0;
}

bool readLine(int fd, std::string& line, int timeoutMs) {
    line.clear();
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        const int waitMs = static_cast<int>(remaining.count() > 0 ? remaining.count() : 1);
        if (!waitForFd(fd, POLLIN, waitMs)) return false;
        char ch = 0;
        const ssize_t count = ::read(fd, &ch, 1);
        if (count == 1) {
            line.push_back(ch);
            if (ch == '\n') return true;
        } else if (count < 0 && errno != EINTR && errno != EAGAIN) {
            return false;
        }
    }
    return false;
}

bool readBytes(int fd, std::vector<uint8_t>& bytes, std::size_t expected, int timeoutMs) {
    bytes.clear();
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(timeoutMs);
    while (bytes.size() < expected && std::chrono::steady_clock::now() < deadline) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now());
        const int waitMs = static_cast<int>(remaining.count() > 0 ? remaining.count() : 1);
        if (!waitForFd(fd, POLLIN, waitMs)) return false;
        uint8_t buffer[64];
        const ssize_t count = ::read(fd, buffer, sizeof(buffer));
        if (count > 0) bytes.insert(bytes.end(), buffer, buffer + count);
        else if (count < 0 && errno != EINTR && errno != EAGAIN) return false;
    }
    return bytes.size() == expected;
}

} // namespace

int main() {
    // Discovery must be safe in headless CI and return stable, unique device paths.
    const auto discoveredPorts = sensor::SerialCommunication::scanAvailablePorts();
    if (!std::is_sorted(discoveredPorts.begin(), discoveredPorts.end()) ||
        std::adjacent_find(discoveredPorts.begin(), discoveredPorts.end()) != discoveredPorts.end()) {
        std::cerr << "Serial port discovery returned unsorted or duplicate entries\\n";
        return 1;
    }
    for (const auto& port : discoveredPorts) {
        if (port.empty() || port.front() != '/') {
            std::cerr << "Linux serial discovery returned an invalid device path: " << port << '\\n';
            return 1;
        }
    }
    const int master = ::posix_openpt(O_RDWR | O_NOCTTY);
    if (master < 0 || ::grantpt(master) != 0 || ::unlockpt(master) != 0) {
        std::cerr << "Could not create POSIX pseudo-terminal: " << std::strerror(errno) << '\n';
        if (master >= 0) ::close(master);
        return 1;
    }

    char slaveName[256]{};
    if (::ptsname_r(master, slaveName, sizeof(slaveName)) != 0) {
        std::cerr << "Could not resolve pseudo-terminal slave path\n";
        ::close(master);
        return 1;
    }

    sensor::SerialCommunication serial;
    if (!serial.connect(slaveName, 115200)) {
        std::cerr << "Production SerialCommunication failed to open PTY: "
                  << serial.getLastError() << '\n';
        ::close(master);
        return 1;
    }
    // Concurrently exercise independent serial metadata domains.
    std::atomic<bool> metadataOk{true};
    std::thread errorWriter([&]() {
        for (int i = 0; i < 500; ++i) {
            (void)serial.sendCommand("BAD\n", false);
            (void)serial.getLastError();
        }
    });
    std::thread errorReader([&]() {
        for (int i = 0; i < 500; ++i) (void)serial.getLastError();
    });
    std::thread watchdogWriter([&]() {
        for (int i = 0; i < 500; ++i) {
            serial.configureWatchdog(1000 + (i % 1000), true);
            serial.resetWatchdog();
        }
    });
    std::thread watchdogReader([&]() {
        for (int i = 0; i < 500; ++i) {
            if (serial.getTimeSinceLastActivity() < 0) metadataOk.store(false);
            (void)serial.isWatchdogOK();
        }
    });
    std::thread calibrationWriter([&]() {
        for (int i = 1; i <= 500; ++i) {
            serial.setCalibrationPoint(i, static_cast<double>(i) / 10.0);
            serial.resetCalibration();
        }
    });
    std::thread calibrationReader([&]() {
        for (int i = 0; i < 500; ++i) {
            (void)serial.convertToCalibrated(i);
            (void)serial.getCalibrationScale();
            (void)serial.getCalibrationOffset();
        }
    });
    std::thread timestampWriter([&]() {
        for (uint32_t i = 1; i <= 500; ++i) serial.syncTimestamp(i);
    });
    std::thread timestampReader([&]() {
        for (int i = 0; i < 500; ++i) (void)serial.getTimestampOffset();
    });
    errorWriter.join();
    errorReader.join();
    watchdogWriter.join();
    watchdogReader.join();
    calibrationWriter.join();
    calibrationReader.join();
    timestampWriter.join();
    timestampReader.join();
    if (!metadataOk.load()) {
        std::cerr << "Concurrent serial metadata regression failed\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    // Concurrent connect() calls must serialize replacement of the native handle.
    std::atomic<bool> concurrentConnectsOk{true};
    std::vector<std::thread> connectors;
    for (int i = 0; i < 4; ++i) {
        connectors.emplace_back([&]() {
            if (!serial.connect(slaveName, 115200)) {
                concurrentConnectsOk.store(false, std::memory_order_release);
            }
        });
    }
    for (auto& connector : connectors) connector.join();
    if (!concurrentConnectsOk.load(std::memory_order_acquire) ||
        !serial.isConnected() || serial.getPortName() != slaveName) {
        std::cerr << "Concurrent connect() calls left inconsistent connection state\\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    std::string command;
    std::thread device([&]() {
        if (readLine(master, command, 1500) && command == "PING\n") {
            constexpr char response[] = "OK\r\n";
            (void)::write(master, response, sizeof(response) - 1);
        }
    });

    const std::string response = serial.sendCommand("PING", true);
    device.join();
    if (command != "PING\n" || response.find("OK") == std::string::npos) {
        std::cerr << "PTY command/response exchange failed; command="
                  << command << ", response=" << response << '\n';
        serial.disconnect();
        ::close(master);
        return 1;
    }

    const auto timeoutStart = std::chrono::steady_clock::now();
    const sensor::SensorData empty = serial.readData(120);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - timeoutStart).count();
    if (empty.hasFrame || empty.isValid || elapsed < 80 || elapsed > 700) {
        std::cerr << "PTY read timeout was not respected: elapsed=" << elapsed << " ms\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }
    if (!readLine(master, command, 500) || command != "DATA\n") {
        std::cerr << "PTY device did not receive DATA command\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    // The production HX711 firmware sends a binary frame followed by a
    // calibrated FORCE text record in the same DATA response.
    std::thread calibratedDevice([&]() {
        std::string dataCommand;
        if (!readLine(master, dataCommand, 500) || dataCommand != "DATA\n") return;
        uint8_t packet[11]{};
        const uint32_t deviceTimestamp = 1234;
        const int32_t rawValue = 4567;
        std::memcpy(packet, &deviceTimestamp, sizeof(deviceTimestamp));
        std::memcpy(packet + 4, &rawValue, sizeof(rawValue));
        uint8_t crc = 0xFF;
        for (int i = 0; i < 8; ++i) {
            crc ^= packet[i];
            for (int bit = 0; bit < 8; ++bit)
                crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                                   : static_cast<uint8_t>(crc << 1);
        }
        packet[8] = crc;
        packet[9] = 0xAA;
        packet[10] = 0x55;
        const std::string text = "FORCE:123.45g,RAW:4567,TS:5678ms,CAL:1\r\n[DATA_SENT]\r\n";
        std::vector<uint8_t> response(std::begin(packet), std::end(packet));
        response.insert(response.end(), text.begin(), text.end());
        (void)::write(master, response.data(), response.size());
    });
    const sensor::SensorData calibrated = serial.readData(700);
    calibratedDevice.join();
    if (!calibrated.isValid || !calibrated.hasCalibratedValue ||
        std::abs(calibrated.calibratedValue - 123.45) > 1e-6 ||
        calibrated.value != 4567) {
        std::cerr << "Binary + calibrated text frame was not parsed correctly: valid="
                  << calibrated.isValid << ", calibrated=" << calibrated.hasCalibratedValue
                  << ", grams=" << calibrated.calibratedValue << ", raw=" << calibrated.value << '\n';
        serial.disconnect();
        ::close(master);
        return 1;
    }

    // Text-only firmware remains supported but is explicitly marked calibrated.
    std::thread textOnlyDevice([&]() {
        std::string dataCommand;
        if (!readLine(master, dataCommand, 500) || dataCommand != "DATA\n") return;
        const std::string text = "FORCE:25.00g,RAW:999,TS:6789ms,CAL:1\r\n";
        (void)::write(master, text.data(), text.size());
    });
    const sensor::SensorData textOnly = serial.readData(500);
    textOnlyDevice.join();
    if (!textOnly.isValid || !textOnly.hasCalibratedValue ||
        std::abs(textOnly.calibratedValue - 25.0) > 1e-6 || textOnly.value != 999) {
        std::cerr << "Text-only calibrated frame was not parsed correctly\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    std::thread uncalibratedDevice([&]() {
        std::string dataCommand;
        if (!readLine(master, dataCommand, 500) || dataCommand != "DATA\n") return;
        const std::string text = "FORCE:12.00g,RAW:888,TS:7890ms,CAL:0\r\n";
        (void)::write(master, text.data(), text.size());
    });
    const sensor::SensorData uncalibrated = serial.readData(500);
    uncalibratedDevice.join();
    if (!uncalibrated.isValid || uncalibrated.hasCalibratedValue) {
        std::cerr << "Uncalibrated HX711 frame was incorrectly accepted as force\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    const uint8_t raw[] = {0xA5, 0x00, 0x5A, 0xFF};
    if (!serial.sendRawData(raw, sizeof(raw))) {
        std::cerr << "PTY raw write failed: " << serial.getLastError() << '\n';
        serial.disconnect();
        ::close(master);
        return 1;
    }
    std::vector<uint8_t> received;
    if (!readBytes(master, received, sizeof(raw), 500) ||
        !std::equal(received.begin(), received.end(), std::begin(raw))) {
        std::cerr << "PTY raw byte stream mismatch\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    // Keep a synchronous read in flight while disconnect is requested.
    // Teardown must wait for readMutex rather than closing a live descriptor.
    std::atomic<bool> readerStarted{false};
    std::thread reader([&]() {
        readerStarted.store(true, std::memory_order_release);
        (void)serial.readData(250);
    });
    while (!readerStarted.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    if (!readLine(master, command, 500) || command != "DATA\n") {
        std::cerr << "PTY device did not receive concurrent DATA command\n";
        reader.join();
        serial.disconnect();
        ::close(master);
        return 1;
    }
    std::atomic<bool> stopSnapshots{false};
    std::atomic<bool> snapshotsValid{true};
    std::thread snapshots([&]() {
        while (!stopSnapshots.load(std::memory_order_acquire)) {
            const std::string name = serial.getPortName();
            const int baud = serial.getBaudRate();
            (void)serial.isConnected();
            (void)serial.getArduinoInfo();
            // These independent getters are individually synchronized, not an
            // atomic multi-field snapshot. Accept the old or cleared port name
            // during teardown, then assert the final disconnected state below.
            if (baud != 115200 || (!name.empty() && name != slaveName)) {
                snapshotsValid.store(false, std::memory_order_release);
            }
        }
    });
    const auto disconnectStart = std::chrono::steady_clock::now();
    serial.disconnect();
    const auto disconnectElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - disconnectStart).count();
    stopSnapshots.store(true, std::memory_order_release);
    snapshots.join();
    reader.join();
    if (serial.isConnected() || !serial.getPortName().empty() ||
        !snapshotsValid.load(std::memory_order_acquire) ||
        disconnectElapsed < 80 || disconnectElapsed > 1000) {
        std::cerr << "Disconnect did not serialize with in-flight read: "
                  << disconnectElapsed << " ms\n";
        ::close(master);
        return 1;
    }

    // Reconnect and verify a data callback may request disconnect from the
    // asynchronous reader thread without attempting to join itself.
    if (!serial.connect(slaveName, 115200)) {
        std::cerr << "PTY reconnect before async lifecycle check failed\n";
        ::close(master);
        return 1;
    }
    // Text commands must pause the binary stream, drain stale frames and
    // restart continuous acquisition without losing command responses.
    serial.startAsyncReading();
    std::thread tareDevice([&]() {
        std::string receivedCommand;
        if (!readLine(master, receivedCommand, 1000) || receivedCommand != "CONT\\n") return;
        if (!readLine(master, receivedCommand, 1000) || receivedCommand != "STOP\\n") return;
        if (!readLine(master, receivedCommand, 1000) || receivedCommand != "TARE\\n") return;
        const std::string tareResponse = "Taring...\\r\\nTare set!\\r\\n";
        (void)::write(master, tareResponse.data(), tareResponse.size());
        (void)readLine(master, receivedCommand, 1000); // resumed CONT
    });
    const std::string tareResponse = serial.sendCommand("TARE", true);
    tareDevice.join();
    serial.stopAsyncReading();
    if (!readLine(master, command, 1000) || command != "STOP\\n") {
        std::cerr << "Continuous stream did not stop after the command transaction\\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }
    if (tareResponse.find("Tare set!") == std::string::npos) {
        std::cerr << "TARE response was lost while pausing the continuous stream\\n";
        serial.stopAsyncReading();
        serial.disconnect();
        ::close(master);
        return 1;
    }

    std::atomic<bool> callbackDisconnected{false};
    std::atomic<int> callbackCount{0};
    serial.addDataCallback([&](const sensor::SensorData& data) {
        if (data.isValid && data.hasTimestamp &&
            callbackCount.fetch_add(1, std::memory_order_acq_rel) + 1 >= 2) {
            serial.disconnect();
            callbackDisconnected.store(true, std::memory_order_release);
        }
    });
    serial.startAsyncReading();
    serial.startAsyncReading(); // idempotent while the worker is active
    if (!readLine(master, command, 1000) || command != "CONT\n") {
        std::cerr << "Async reader did not enable continuous stream mode\n";
        serial.stopAsyncReading();
        serial.disconnect();
        ::close(master);
        return 1;
    }

    // Two frames in one UART write exercise the persistent incremental parser.
    std::vector<uint8_t> callbackFrames;
    for (uint32_t index = 0; index < 2; ++index) {
        uint8_t packet[11]{};
        const uint32_t timestamp = 9000 + index * 12;
        const int32_t value = 1000 + static_cast<int32_t>(index);
        std::memcpy(packet, &timestamp, sizeof(timestamp));
        std::memcpy(packet + 4, &value, sizeof(value));
        uint8_t crc = 0xFF;
        for (int i = 0; i < 8; ++i) {
            crc ^= packet[i];
            for (int bit = 0; bit < 8; ++bit)
                crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07)
                                   : static_cast<uint8_t>(crc << 1);
        }
        packet[8] = crc;
        packet[9] = 0xAA;
        packet[10] = 0x55;
        callbackFrames.insert(callbackFrames.end(), std::begin(packet), std::end(packet));
    }
    if (::write(master, callbackFrames.data(), callbackFrames.size()) !=
        static_cast<ssize_t>(callbackFrames.size())) {
        std::cerr << "Could not provide continuous PTY frames for async callback check\n";
        serial.stopAsyncReading();
        serial.disconnect();
        ::close(master);
        return 1;
    }
    const auto callbackDeadline = std::chrono::steady_clock::now() +
                                  std::chrono::seconds(2);
    while (!callbackDisconnected.load(std::memory_order_acquire) &&
           std::chrono::steady_clock::now() < callbackDeadline) {
        std::this_thread::yield();
    }
    serial.stopAsyncReading(); // reap the worker after its callback requested disconnect
    if (!callbackDisconnected.load(std::memory_order_acquire) ||
        callbackCount.load(std::memory_order_acquire) != 2 || serial.isConnected()) {
        std::cerr << "Continuous async stream did not deliver two valid frames safely\n";
        serial.disconnect();
        ::close(master);
        return 1;
    }

    ::close(master);
    std::cout << "SerialCommunication PTY integration checks passed\n";
    return 0;
}
