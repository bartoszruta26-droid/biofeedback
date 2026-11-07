#!/usr/bin/env python3
"""Regression guard for single-line serial command validation."""

from pathlib import Path
import sys

source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
start = source.find("std::string SerialCommunication::sendCommand(")
end = source.find("\nbool SerialCommunication::sendRawData(", start)
if start < 0 or end < 0:
    raise SystemExit("sendCommand implementation not found")
body = source[start:end]

required = [
    "MaxCommandLength = 128",
    "cmd.empty()",
    "cmd.size() > MaxCommandLength",
    'cmd.find_first_of("\\r\\n")',
    "cmd.find('\\0')",
    "ch < 0x20 || ch > 0x7e",
    'm_impl->lastError = "Rejected invalid serial command"',
]
missing = [item for item in required if item not in body]
if missing:
    print("Serial command validation FAILED:")
    for item in missing:
        print(f" - missing {item}")
    sys.exit(1)

validation_end = body.find("if (!m_impl->isConnected) return")
if validation_end < 0 or validation_end < body.find("cmd.find_first_of"):
    raise SystemExit("connection/write path must follow command validation")

print("Serial command validation policy passed.")
