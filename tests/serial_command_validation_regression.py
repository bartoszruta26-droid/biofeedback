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
positions = {item: body.find(item) for item in required}
missing = [item for item, position in positions.items() if position < 0]
if missing:
    raise SystemExit("Missing serial command validation: " + ", ".join(missing))

write_position = body.find("writeToPort(")
if write_position < 0:
    raise SystemExit("Serial write call not found")
late_checks = [item for item, position in positions.items() if position >= write_position]
if late_checks:
    raise SystemExit("Validation must precede serial write: " + ", ".join(late_checks))

if "MaxResponseBytes = 4096" not in body:
    raise SystemExit("Serial command response must remain bounded")
if "std::chrono::seconds(2)" not in body:
    raise SystemExit("Serial command response must retain a bounded timeout")

print("Serial command validation and bounded I/O policy passed.")
