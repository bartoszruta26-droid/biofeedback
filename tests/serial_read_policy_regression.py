#!/usr/bin/env python3
"""Regression guard for bounded serial reads and timeout handling."""

from pathlib import Path
import sys

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
read_helper_start = source.index("int SerialCommunication::readFromPort(")
read_helper_end = source.index("bool SerialCommunication::isConnected()", read_helper_start)
read_data_start = source.index("SensorData SerialCommunication::readData(int timeout)")
read_data_end = source.index("bool SerialCommunication::tryReadData", read_data_start)
command_start = source.index("std::string SerialCommunication::sendCommand(")
command_end = source.index("bool SerialCommunication::sendRawData", command_start)
read_helper = source[read_helper_start:read_helper_end]
read_data = source[read_data_start:read_data_end]
command = source[command_start:command_end]

checks = {
    "read helper is implemented": "int SerialCommunication::readFromPort(" in source,
    "POSIX read waits for input readiness": "descriptor.events = POLLIN;" in read_helper,
    "POSIX read uses a steady-clock deadline": "std::chrono::steady_clock::now() +" in read_helper,
    "POSIX read retries interrupted operations": "errno == EINTR" in read_helper,
    "POSIX read retries temporary backpressure": "errno == EAGAIN || errno == EWOULDBLOCK" in read_helper,
    "POSIX read detects closed port": "POLLHUP" in read_helper,
    "POSIX read restores descriptor flags": "fcntl(fd, F_SETFL, originalFlags)" in read_helper,
    "readData honors caller timeout": "std::chrono::milliseconds(std::max(timeout, 0))" in read_data,
    "readData uses shared read helper": "readFromPort(buf" in read_data,
    "readData response is bounded": "constexpr size_t MaxResponseBytes = 4096;" in read_data,
    "readData no longer calls platform read directly": "ReadFile(" not in read_data and "read(m_impl->portHandle" not in read_data,
    "sendCommand uses shared read helper": "readFromPort(buf" in command,
    "sendCommand response is bounded": "constexpr size_t MaxResponseBytes = 4096;" in command,
    "sendCommand no longer calls platform read directly": "ReadFile(" not in command and "read(m_impl->portHandle" not in command,
}
failed = [label for label, passed in checks.items() if not passed]
if failed:
    print("Serial read policy FAILED:")
    for label in failed:
        print(f" - {label}")
    sys.exit(1)
print("Serial read policy passed.")
