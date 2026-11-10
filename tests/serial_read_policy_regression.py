#!/usr/bin/env python3
"""Regression guard for bounded serial reads and timeout handling."""

from pathlib import Path
import sys

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
read_helper_start = source.index("int SerialCommunication::readFromPort(")
read_helper_end = source.index("bool SerialCommunication::isConnected()", read_helper_start)
read_data_start = source.index("std::vector<SensorData> SerialCommunication::readDataBatch(int timeout)")
read_data_end = source.index("void SerialCommunication::setDataCallback", read_data_start)
command_start = source.index("std::string SerialCommunication::sendCommand(")
command_end = source.index("bool SerialCommunication::sendRawData", command_start)
read_helper = source[read_helper_start:read_helper_end]
read_data = source[read_data_start:read_data_end]
command = source[command_start:command_end]

checks = {
    "read helper is implemented": "int SerialCommunication::readFromPort(" in source,
    "Windows read timeout is applied per call": "readTimeouts.ReadTotalTimeoutConstant = static_cast<DWORD>(timeout);" in read_helper,
    "zero Windows timeout is nonblocking": "timeout == 0 ? MAXDWORD : 0" in read_helper,
    "Windows read timeout is restored": "SetCommTimeouts(handle, &originalTimeouts)" in read_helper,
    "Windows read buffer is bounded to int": "static_cast<size_t>(INT_MAX)" in read_helper,
    "POSIX read waits for input readiness": "descriptor.events = POLLIN;" in read_helper,
    "POSIX read uses a steady-clock deadline": "std::chrono::steady_clock::now() +" in read_helper,
    "POSIX read bounds interrupted retries after deadline": "if (now >= deadline && pollAttempted) break;" in read_helper,
    "POSIX read retries interrupted operations": "errno == EINTR" in read_helper,
    "POSIX read retries temporary backpressure": "errno == EAGAIN || errno == EWOULDBLOCK" in read_helper,
    "POSIX read detects closed port": "POLLHUP" in read_helper,
    "POSIX read restores descriptor flags": "fcntl(fd, F_SETFL, originalFlags)" in read_helper,
    "readData rejects negative timeout": 'm_impl->setLastError("Serial read rejected: negative timeout");' in read_data,
    "readData honors caller timeout": "std::chrono::milliseconds(timeout)" in read_data,
    "empty reads do not refresh watchdog activity": "if (!rawData.empty()) m_impl->markActivity();" in read_data,
    "readData uses shared read helper": "readFromPort(buf" in read_data,
    "readData response is bounded": "constexpr std::size_t MaxResponseBytes = 4096;" in read_data,
    "readData no longer calls platform read directly": "ReadFile(" not in read_data and "read(m_impl->portHandle" not in read_data,
    "sendCommand uses shared read helper": "readFromPort(buf" in command,
    "sendCommand response is bounded": "constexpr size_t MaxResponseBytes = 4096;" in command,
    "sendCommand no longer calls platform read directly": "ReadFile(" not in command and "read(m_impl->portHandle" not in command,
}

header = Path("include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
checks.update({
    "batch read API is declared": "std::vector<SensorData> readDataBatch(int timeout = 1000);" in header,
    "batch read consumes every parser frame": "while (const auto frame = binaryParser.takeFrame())" in read_data,
    "batch read does not discard extra parser frames": "while (binaryParser.takeFrame()) {}" not in read_data,
    "async reader enables firmware continuous mode": 'const std::string command = "CONT\\n";' in source[source.index("void SerialCommunication::startAsyncReading"):],
    "async reader retains parser state across UART reads": "m_impl->streamParser.feed(buffer" in source,
    "async reader dispatches each parsed frame": "while (const auto frame = m_impl->streamParser.takeFrame())" in source,
    "async reader does not poll DATA once per batch": "readDataBatch(100)" not in source[source.index("void SerialCommunication::startAsyncReading"):],
    "async reader no longer reads a single sample": "SensorData data = readData(100);" not in source[source.index("void SerialCommunication::startAsyncReading"):],
})

failed = [label for label, passed in checks.items() if not passed]
if failed:
    print("Serial read policy FAILED:")
    for label in failed:
        print(f" - {label}")
    sys.exit(1)
print("Serial read policy passed.")

# The Nano continuous stream is binary-framed; line-oriented FORCE output
# is limited to explicit DATA/FORCE transactions, never every 80 Hz frame.
firmware = Path("hx711_advanced.ino").read_text(encoding="utf-8")
continuous_start = firmware.index("void threadContinuousData()")
continuous_end = firmware.index("// ============================================================", continuous_start)
continuous_body = firmware[continuous_start:continuous_end]
if "sendForceReadout()" in continuous_body:
    raise SystemExit("Continuous stream must not append text FORCE lines to every binary sample")
if "sendBinaryData()" not in continuous_body:
    raise SystemExit("Continuous stream must retain CRC-framed binary samples")
