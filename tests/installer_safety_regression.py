#!/usr/bin/env python3
"""Static safety checks for installer cleanup and production diagnostics."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
installer = (root / "biofeedback_installer.sh").read_text(encoding="utf-8")
main = (root / "src/main.cpp").read_text(encoding="utf-8")
debug = (root / "src/core/DebugManager.cpp").read_text(encoding="utf-8")

assert 'GROUP="dialout", MODE="0660"' in installer
assert 'MODE="0666"' not in installer
assert 'USUŃ DANE POMIAROWE' in installer
assert 'realpath -e -- "$patient_data_dir"' in installer
assert 'realpath -e -- "$REPO_DIR/data"' in installer
assert 'rm -rf data/*' not in installer
assert 'rm -rf "$REPO_DIR/data"/*' not in installer
assert "constexpr bool DEBUG_VERBOSE = false;" in main
assert "constexpr bool DEBUG_EXCEPTION_TRACE = false;" in main
assert "constexpr bool DEBUG_SIGNALS = false;" in main
assert "constexpr bool DEBUG_DEBUGMANAGER = false;" in debug
assert "m_minimumLogLevel(DebugLevel::INFO)" in debug
assert "m_consoleOutputEnabled(false)" in debug
print("installer_safety_regression: PASS")
