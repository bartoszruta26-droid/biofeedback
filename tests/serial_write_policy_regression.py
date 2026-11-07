#!/usr/bin/env python3
"""Regression guard for complete, bounded serial writes."""

from pathlib import Path
import sys

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
checks = {
    "shared write helper implemented": "bool SerialCommunication::writeToPort(" in source,
    "write loop completes partial writes": "while (offset < length)" in source,
    "POSIX readiness timeout": "poll(&descriptor, 1, 1000)" in source,
    "POSIX interrupted poll retry": "ready < 0 && errno == EINTR" in source,
    "POSIX interrupted write retry": "written < 0 && errno == EINTR" in source,
    "POSIX temporary backpressure retry": "errno == EAGAIN || errno == EWOULDBLOCK" in source,
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
