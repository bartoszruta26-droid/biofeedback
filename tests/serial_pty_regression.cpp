#include "sensor/SerialCommunication.hpp"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <cerrno>
#include <chrono>
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
    const auto disconnectStart = std::chrono::steady_clock::now();
    serial.disconnect();
    const auto disconnectElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - disconnectStart).count();
    reader.join();
    if (serial.isConnected() || disconnectElapsed < 80 || disconnectElapsed > 1000) {
        std::cerr << "Disconnect did not serialize with in-flight read: "
                  << disconnectElapsed << " ms\n";
        ::close(master);
        return 1;
    }

    ::close(master);
    std::cout << "SerialCommunication PTY integration checks passed\n";
    return 0;
}
