#!/usr/bin/env python3
"""Regression guard for privacy-safe serial diagnostics defaults."""

from pathlib import Path
import re
import sys

source_path = Path("src/sensor/SerialCommunication.cpp")
source = source_path.read_text(encoding="utf-8")

checks = {
    "serial diagnostics disabled": r"constexpr\s+bool\s+DEBUG_SERIAL_COMM\s*=\s*false\s*;",
    "connection diagnostics disabled": r"constexpr\s+bool\s+DEBUG_CONNECTION\s*=\s*false\s*;",
    "packet diagnostics disabled": r"constexpr\s+bool\s+DEBUG_DATA_PACKETS\s*=\s*false\s*;",
    "CRC diagnostics disabled": r"constexpr\s+bool\s+DEBUG_CRC\s*=\s*false\s*;",
    "verbose serial output disabled by default": r"bool\s+verbose\s*=\s*false\s*;",
}

failed = []
for label, pattern in checks.items():
    if not re.search(pattern, source):
        failed.append(label)

if "if (m_impl->verbose && !resp.empty()) std::cout" not in source:
    failed.append("serial response output must remain behind explicit verbose opt-in")

if failed:
    print("Serial diagnostics policy FAILED:")
    for item in failed:
        print(f" - {item}")
    sys.exit(1)

print("Serial diagnostics policy passed.")
