#!/usr/bin/env python3
"""Cross-check host, configuration and Nano firmware serial defaults."""
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
config = json.loads((root / "config/config.json").read_text(encoding="utf-8"))
protocol = (root / "include/sensor/SerialProtocolConfig.hpp").read_text(encoding="utf-8")
serial_header = (root / "include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
serial_source = (root / "src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
config_source = (root / "src/core/ConfigManager.cpp").read_text(encoding="utf-8")
parser_source = (root / "src/sensor/ProtocolParser.cpp").read_text(encoding="utf-8")
nano = (root / "hx711_advanced.ino").read_text(encoding="utf-8")

baud = int(re.search(r"DefaultBaudRate\s*=\s*(\d+)", protocol).group(1))
assert baud == 115200
assert config["serial"]["baud_rate"] == baud
assert "protocol::DefaultBaudRate" in serial_header
assert "protocol::DefaultBaudRate" in serial_source
assert 'm_config["serial.baud_rate"] = sensor::protocol::DefaultBaudRate;' in config_source
assert re.search(r"#define SERIAL_BAUD_RATE\s+115200", nano)
assert config["encryption"]["algorithm"] == "AES-256-GCM"
assert 'm_config["encryption.algorithm"] = std::string("AES-256-GCM");' in config_source
assert config["encryption"]["key_derivation"] == "SHA-256 key normalization (32+ byte external secret)"
assert 'm_config["encryption.key_derivation"] = std::string("SHA-256 key normalization (32+ byte external secret)");' in config_source
assert "protocol::Crc8Polynomial" in parser_source
assert "protocol::Crc8InitialValue" in parser_source
assert "protocol::MeasurementFrameSize" in parser_source
print("serial_configuration_regression: PASS")
