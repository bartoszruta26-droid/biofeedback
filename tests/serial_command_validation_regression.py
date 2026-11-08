#!/usr/bin/env python3
"""Regression guard for the production serial command input boundary."""

from pathlib import Path

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
start = source.find("std::string SerialCommunication::sendCommand(")
end = source.find("\nbool SerialCommunication::sendRawData(", start)
if start < 0 or end < 0:
    raise SystemExit("sendCommand implementation not found")
body = source[start:end]

required = (
    "MaxCommandLength = 128",
    "cmd.empty()",
    "cmd.size() > MaxCommandLength",
    'cmd.find_first_of("\\r\\n")',
    "cmd.find('\\0')",
    "ch < 0x20 || ch > 0x7e",
    'm_impl->setLastError("Rejected invalid serial command")',
)
missing = [item for item in required if item not in body]
if missing:
    raise SystemExit("Missing serial command validation: " + ", ".join(missing))

validation_end = body.find("if (!m_impl->isConnected) return")
write_position = body.find("writeToPort(")
if validation_end < 0 or write_position < 0 or validation_end > write_position:
    raise SystemExit("Command validation must precede connection and serial write")

if "MaxResponseBytes = 4096" not in body:
    raise SystemExit("Serial command response must remain bounded")
if "std::chrono::seconds(2)" not in body:
    raise SystemExit("Serial command response must retain a bounded timeout")

print("Serial command validation and bounded I/O policy passed.")
