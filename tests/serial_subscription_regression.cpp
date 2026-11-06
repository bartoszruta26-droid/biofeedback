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
    std::atomic<bool> calibratedValueValid{false};
    std::atomic<bool> rawFlagCorrect{false};
    std::atomic<bool> blockingCallbackEntered{false};
    std::atomic<bool> releaseBlockingCallback{false};

    const auto connectionId = serial.subscribeConnectionCallback(
        [&](bool connected, const std::string&) {
            if (connected) ++connectedEvents;
            else ++disconnectedEvents;
        });
    const auto firstId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (data.isValid && data.value == 123456) {
                ++firstSubscriberCalls;
                if (std::abs(data.calibratedValue - 123456.0) < 1e-6) calibratedValueValid.store(true);
                if (!data.hasRawValue) rawFlagCorrect.store(true);
            }
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
        int responseCount = 0;
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
            frame[0] = 0xAA;
            std::memcpy(frame + 1, &timestamp, sizeof(timestamp));
            std::memcpy(frame + 5, &value, sizeof(value));
            frame[9] = sensor::SerialCommunication::calculateCRC8(frame, 9);
            frame[10] = 0x55;
            // First response has a deliberately corrupted CRC; parser must skip it.
            if (responseCount++ == 0) frame[9] ^= 0xFF;
            (void)write(masterFd, frame, sizeof(frame));
        }
    });

    if (std::isfinite(serial.convertToCalibrated(123456))) {
        std::cerr << "Host conversion should require an explicit calibration\n";
        emulatorRunning = false;
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }
    serial.setCalibrationPoint(123456, 42.5);
    if (std::abs(serial.convertToCalibrated(123456) - 42.5) > 1e-6) {
        std::cerr << "Explicit host calibration conversion failed\n";
        emulatorRunning = false;
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }
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
                              waitUntil(calibratedValueValid, true, std::chrono::seconds(1)) &&
                              waitUntil(rawFlagCorrect, true, std::chrono::seconds(1));
    if (!bothReceived) {
        std::cerr << "Serial sample check failed: first=" << firstSubscriberCalls.load()
                  << ", second=" << secondSubscriberCalls.load()
                  << ", callbackEntered=" << blockingCallbackEntered.load()
                  << ", calibrated=" << calibratedValueValid.load() << "\\n";
        releaseBlockingCallback.store(true);
        emulatorRunning = false;
        serial.stopAsyncReading();
        emulator.join();
        serial.disconnect();
        close(masterFd);
        return 1;
    }

    // Register the blocking callback only after normal subscribers have
    // demonstrated delivery; unordered callback order must not starve them.
    const auto blockingId = serial.subscribeDataCallback(
        [&](const sensor::SensorData& data) {
            if (!data.isValid || data.value != 123456) return;
            blockingCallbackEntered.store(true);
            while (!releaseBlockingCallback.load())
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
    if (!blockingId ||
        !waitUntil(blockingCallbackEntered, true, std::chrono::seconds(2))) {
        std::cerr << "Blocking callback was not entered\n";
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

    const auto commandTimeoutStart = std::chrono::steady_clock::now();
    (void)serial.sendCommand("NO_RESPONSE", true, 100);
    const auto commandTimeoutElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - commandTimeoutStart).count();
    if (commandTimeoutElapsed > 500) {
        std::cerr << "sendCommand timeout exceeded: " << commandTimeoutElapsed << " ms\n";
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
