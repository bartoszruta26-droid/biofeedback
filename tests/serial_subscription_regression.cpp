#include "sensor/SerialCommunication.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
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
    std::atomic<bool> blockingCallbackEntered{false};
    std::atomic<bool> releaseBlockingCallback{false};

    const auto connectionId = serial.subscribeConnectionCallback(
        [&](bool connected, const std::string&) {
            if (connected) ++connectedEvents;
            else ++disconnectedEvents;
        });
    const auto firstId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (data.isValid && data.value == 123456 && std::abs(data.calibratedValue - 42.5) < 1e-6) ++firstSubscriberCalls;
        });
    const auto secondId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (data.isValid && data.value == 123456 && data.calibratedValue == 123456.0) ++secondSubscriberCalls;
        });

    const auto blockingId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (!data.isValid || data.value != 123456) return;
            blockingCallbackEntered.store(true);
            while (!releaseBlockingCallback.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });

    if (!connectionId || !firstId || !secondId || !blockingId) {
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

    serial.setCalibrationPoint(123456, 42.5);
    serial.startAsyncReading();
    const auto waitUntil = [](const auto& value, int minimum,
                              std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (value.load() >= minimum) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return value.load() >= minimum;
    };

    const bool bothReceived = waitUntil(firstSubscriberCalls, 1, std::chrono::seconds(5)) &&
                              waitUntil(secondSubscriberCalls, 1, std::chrono::seconds(1)) &&
                              waitUntil(blockingCallbackEntered, true, std::chrono::seconds(1));
    if (!bothReceived) {
        std::cerr << "Both subscribers did not receive the same sample\n";
        releaseBlockingCallback.store(true);
        emulatorRunning = false;
        serial.stopAsyncReading();
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }

    // Unsubscribe must wait for a callback that has already entered.
    std::atomic<bool> blockingUnsubscribeReturned{false};
    std::thread blockingUnsubscriber([&]() {
        serial.unsubscribeDataCallback(blockingId);
        blockingUnsubscribeReturned.store(true);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const bool waitedForInFlightCallback = !blockingUnsubscribeReturned.load();
    releaseBlockingCallback.store(true);
    blockingUnsubscriber.join();
    if (!waitedForInFlightCallback || !blockingUnsubscribeReturned.load()) {
        std::cerr << "Unsubscribe did not wait for the in-flight callback\n";
        emulatorRunning = false;
        serial.stopAsyncReading();
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }

    // Unsubscribe one receiver while preserving delivery to the others.
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

    const auto timeoutStart = std::chrono::steady_clock::now();
    (void)serial.readData(100);
    const auto timeoutElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - timeoutStart).count();
    if (timeoutElapsed > 500) {
        std::cerr << "readData(100) exceeded its timeout: " << timeoutElapsed << " ms\n";
        serial.disconnect();
        close(masterFd);
        return 1;
    }

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
    serial.unsubscribeDataCallback(blockingId);
    serial.unsubscribeConnectionCallback(connectionId);
    std::cout << "PASS: multi-subscriber delivery, in-flight unsubscribe barrier, connection events\n";
    return 0;
}
