#include "sensor/SerialCommunication.hpp"

#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

#include <poll.h>
#include <pty.h>
#include <unistd.h>

int main() {
    int masterFd = -1;
    int slaveFd = -1;
    char slaveName[256] = {};
    if (openpty(&masterFd, &slaveFd, slaveName, nullptr, nullptr) != 0) {
        std::cerr << "openpty failed\n";
        return 1;
    }
    close(slaveFd);

    sensor::SerialCommunication serial;
    serial.setVerbose(false);

    std::atomic<int> connectedEvents{0};
    std::atomic<int> disconnectedEvents{0};
    std::atomic<int> firstSubscriberCalls{0};
    std::atomic<int> secondSubscriberCalls{0};

    const auto connectionId = serial.subscribeConnectionCallback(
        [&](bool connected, const std::string&) {
            if (connected) ++connectedEvents;
            else ++disconnectedEvents;
        });
    const auto firstId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (data.isValid && data.value == 123456) ++firstSubscriberCalls;
        });
    const auto secondId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (data.isValid && data.value == 123456) ++secondSubscriberCalls;
        });

    if (!connectionId || !firstId || !secondId) {
        std::cerr << "Subscription registration failed\n";
        close(masterFd);
        return 1;
    }
    if (!serial.connect(slaveName, 115200)) {
        std::cerr << "Serial PTY connection failed: " << serial.getLastError() << "\n";
        close(masterFd);
        return 1;
    }

    if (serial.getPortName() != slaveName || serial.getBaudRate() != 115200) {
        std::cerr << "Serial connection metadata mismatch: port="
                  << serial.getPortName() << ", baud=" << serial.getBaudRate() << "\n";
        serial.disconnect();
        close(masterFd);
        return 1;
    }

    std::atomic<bool> emulatorRunning{true};
    std::thread emulator([&]() {
        while (emulatorRunning.load()) {
            pollfd pfd{masterFd, POLLIN, 0};
            const int ready = poll(&pfd, 1, 100);
            if (ready <= 0 || !(pfd.revents & POLLIN)) continue;

            char command[64];
            const ssize_t count = read(masterFd, command, sizeof(command));
            if (count <= 0) continue;

            uint8_t frame[11] = {};
            const uint32_t timestamp = 1234;
            const int32_t value = 123456;
            std::memcpy(frame, &timestamp, sizeof(timestamp));
            std::memcpy(frame + 4, &value, sizeof(value));
            frame[8] = sensor::SerialCommunication::calculateCRC8(frame, 8);
            frame[9] = 0xAA;
            frame[10] = 0x55;
            (void)write(masterFd, frame, sizeof(frame));
            // The current parser exits its read window when it sees this marker.
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            static constexpr char responseMarker[] = "[DATA_SENT]";
            (void)write(masterFd, responseMarker, sizeof(responseMarker) - 1);
        }
    });

    serial.startAsyncReading();
    const auto waitUntil = [](const std::atomic<int>& value, int minimum,
                              std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (value.load() >= minimum) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return value.load() >= minimum;
    };

    const bool bothReceived = waitUntil(firstSubscriberCalls, 1, std::chrono::seconds(5)) &&
                              waitUntil(secondSubscriberCalls, 1, std::chrono::seconds(1));
    if (!bothReceived) {
        std::cerr << "Both subscribers did not receive the same sample\n";
        emulatorRunning = false;
        serial.stopAsyncReading();
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }

    // Allow any already-copied callback snapshot to finish before measuring detachment.
    serial.unsubscribeDataCallback(firstId);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    const int firstAfterUnsubscribe = firstSubscriberCalls.load();
    const int secondBefore = secondSubscriberCalls.load();
    const bool secondContinued = waitUntil(secondSubscriberCalls, secondBefore + 2,
                                           std::chrono::seconds(3));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    const bool firstDetached = firstSubscriberCalls.load() == firstAfterUnsubscribe;

    serial.stopAsyncReading();
    emulatorRunning = false;
    emulator.join();
    serial.disconnect();
    close(masterFd);

    if (!secondContinued || !firstDetached) {
        std::cerr << "Unsubscribe isolation failed\n";
        return 1;
    }
    if (connectedEvents.load() < 1 || disconnectedEvents.load() < 1) {
        std::cerr << "Connection status subscribers did not receive both transitions\n";
        return 1;
    }

    serial.unsubscribeDataCallback(secondId);
    serial.unsubscribeConnectionCallback(connectionId);
    std::cout << "PASS: multi-subscriber delivery, isolated unsubscribe, connection events\n";
    return 0;
}
