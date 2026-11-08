#!/usr/bin/env python3
"""Cross-check host, configuration and MCU serial-control defaults."""
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
pico = (root / "firmware/pico/main.cpp").read_text(encoding="utf-8")

baud = int(re.search(r"DefaultBaudRate\\s*=\\s*(\\d+)", protocol).group(1))
assert baud == 115200
assert config["serial"]["baud_rate"] == baud
assert "protocol::DefaultBaudRate" in serial_header
assert "protocol::DefaultBaudRate" in serial_source
assert 'm_config["serial.baud_rate"] = sensor::protocol::DefaultBaudRate;' in config_source
assert re.search(r"#define SERIAL_BAUD_RATE\\s+115200", nano)
assert "protocol::Crc8Polynomial" in parser_source
assert "protocol::Crc8InitialValue" in parser_source
assert "protocol::MeasurementFrameSize" in parser_source
assert "ControlProtocolVersion = 1" in protocol
assert "MaxControlCommandBytes = 63" in protocol
assert "ControlPing[] = \\"PING\\"" in protocol
assert "ControlStatus[] = \\"STATUS\\"" in protocol
assert "ControlSafe[] = \\"SAFE\\"" in protocol
assert "constexpr uint32_t kBaudRate = 115200;" in pico
assert "ACTUATORS_LOCKED" in pico
assert "gpio_set_function(kUartTxPin, GPIO_FUNC_UART);" in pico
assert "gpio_set_function(kUartRxPin, GPIO_FUNC_UART);" in pico

print("serial_configuration_regression: PASS")
