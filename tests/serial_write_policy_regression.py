#!/usr/bin/env python3
"""Regression guard for complete, bounded serial writes."""

from pathlib import Path
import sys

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
checks = {
    "native-width port handle avoids 64-bit Windows truncation": "intptr_t portHandle = -1;" in source and "reinterpret_cast<intptr_t>(h)" in source,
    "raw writes share the read/write mutex": "bool SerialCommunication::sendRawData(const uint8_t* data, size_t len) {\n    std::lock_guard<std::mutex> lock(m_impl->readMutex);" in source,
    "shared write helper implemented": "bool SerialCommunication::writeToPort(" in source,
    "write loop completes partial writes": "while (offset < length)" in source,
    "POSIX absolute write deadline": "const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);" in source,
    "POSIX interrupted poll retry": "ready < 0 && errno == EINTR" in source,
    "POSIX interrupted write retry": "errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK" in source,
    "POSIX write is temporarily nonblocking": "originalFlags | O_NONBLOCK" in source,
    "POSIX readiness event is verified": "(descriptor.revents & POLLOUT) == 0" in source,
    "Windows partial writes advance offset": "offset += static_cast<size_t>(written)" in source,
    "readData uses checked write": "if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size()))" in source,
    "sendCommand uses checked write": "if (!writeToPort(reinterpret_cast<const uint8_t*>(toSend.data()), toSend.size()))" in source,
    "sendRawData uses checked write": "if (!writeToPort(data, len)) return false;" in source,
    "Windows write timeout configured": "to.WriteTotalTimeoutConstant = 1000;" in source,
}
failed = [label for label, passed in checks.items() if not passed]
if failed:
    print("Serial write policy FAILED:")
    for label in failed:
        print(f" - {label}")
    sys.exit(1)
print("Serial write policy passed.")
