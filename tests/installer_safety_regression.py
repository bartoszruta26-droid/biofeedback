#!/usr/bin/env python3
"""Check scoped serial permissions and safe uninstall defaults."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
legacy = (root / "biofeedback_installer.sh").read_text(encoding="utf-8")
packaged = (root / "packaging/linux/install.sh").read_text(encoding="utf-8")
uninstaller = (root / "packaging/linux/uninstall.sh").read_text(encoding="utf-8")
main = (root / "src/main.cpp").read_text(encoding="utf-8")
debug = (root / "src/core/DebugManager.cpp").read_text(encoding="utf-8")
encryption = (root / "src/data/Encryption.cpp").read_text(encoding="utf-8")
example = (root / "config/config.json.example").read_text(encoding="utf-8")

assert 'KERNEL=="ttyUSB[0-9]*"' in legacy
assert 'KERNEL=="ttyACM[0-9]*"' in legacy
assert 'GROUP="dialout", MODE="0660", TAG+="uaccess"' in legacy
assert 'ATTRS{idVendor}=="2341"' not in legacy
assert 'MODE="0666"' not in legacy
assert 'BIOFEEDBACK_INSTALL_DIR' in packaged
assert '[[ -L "$INSTALL_DIR" ]]' in packaged
assert 'BIOFEEDBACK_REMOVE_DATA' in uninstaller
assert 'USUŃ DANE POMIAROWE' in uninstaller
assert 'Dane pacjentów pozostawiono bez zmian' in uninstaller
assert 'rm -rf -- "$canonical_data"' in uninstaller
assert '[[ -L "$DATA_DIR" ]]' in uninstaller
assert 'Odmowa usunięcia ścieżki spoza katalogu instalacji' in uninstaller

assert '"baud_rate": 115200' in example
assert '"algorithm": "AES-256-GCM"' in example
assert '"key_derivation": "PBKDF2-HMAC-SHA256"' in example
assert '"require_authentication": true' in example
assert '"actuators_enabled": false' in example
assert "constexpr bool DEBUG_VERBOSE = false;" in main
assert "constexpr bool DEBUG_EXCEPTION_TRACE = false;" in main
assert "constexpr bool DEBUG_SIGNALS = false;" in main
assert "constexpr bool DEBUG_DEBUGMANAGER = false;" in debug
assert "m_minimumLogLevel(DebugLevel::INFO)" in debug
assert "m_consoleOutputEnabled(false)" in debug
print("installer_safety_regression: PASS")

for flag in ("DEBUG_ENCRYPTION", "DEBUG_XOR_OPERATIONS", "DEBUG_BASE64_OPERATIONS", "DEBUG_KEY_GENERATION", "DEBUG_ERROR_HANDLING"):
    assert f"#define {flag} 0" in encryption
assert "bool Encryption::m_consoleOutputEnabled = false;" in encryption
