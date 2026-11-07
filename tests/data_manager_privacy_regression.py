#!/usr/bin/env python3
"""Regression checks for privacy-safe DataManager logging defaults."""

from pathlib import Path
import sys

header = Path("include/data/DataManager.hpp").read_text(encoding="utf-8")
source = Path("src/data/DataManager.cpp").read_text(encoding="utf-8")

checks = {
    "header detailed logging disabled": "DEBUG_DATA_MANAGER_ENABLED = false" in header,
    "header encryption diagnostics disabled": "DEBUG_ENCRYPTION_OPS_ENABLED = false" in header,
    "header file diagnostics disabled": "DEBUG_FILE_IO_ENABLED = false" in header,
    "header console output disabled": "consoleOutputEnabled{false}" in header,
    "header default level is warning": "minimumLogLevel{DataManagerLogLevel::WARNING}" in header,
    "implementation detailed logging disabled": "DEBUG_DATA_MANAGER = false" in source,
    "implementation encryption diagnostics disabled": "DEBUG_ENCRYPTION_OPS = false" in source,
    "implementation file diagnostics disabled": "DEBUG_FILE_IO = false" in source,
    "implementation console output disabled": ", consoleOutputEnabled(false)" in source,
    "implementation default level is warning": ", minimumLogLevel(DataManagerLogLevel::WARNING)" in source,
    "patient identifier is not written by parse-success log": "PatientData parsed successfully. ID:" not in source,
    "patient serialization exception is redacted": "Failed to serialize PatientData: " not in source,
    "patient parsing exception is redacted": "Exception during PatientData parsing: " not in source,
}

failed = [name for name, passed in checks.items() if not passed]
for name, passed in checks.items():
    print(("PASS" if passed else "FAIL") + ": " + name)
if failed:
    sys.exit(1)
print(f"All {len(checks)} DataManager privacy checks passed.")
