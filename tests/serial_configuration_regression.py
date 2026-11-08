#!/usr/bin/env python3
"""Cross-check host, configuration and MCU serial-control defaults."""
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
config = json.loads((root / "config/config.json").read_text(encoding="utf-8"))
example = json.loads((root / "config/config.json.example").read_text(encoding="utf-8"))
protocol = (root / "include/sensor/SerialProtocolConfig.hpp").read_text(encoding="utf-8")
serial_header = (root / "include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
serial_source = (root / "src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
config_source = (root / "src/core/ConfigManager.cpp").read_text(encoding="utf-8")
parser_source = (root / "src/sensor/ProtocolParser.cpp").read_text(encoding="utf-8")
nano = (root / "hx711_advanced.ino").read_text(encoding="utf-8")
pico = (root / "firmware/pico/main.cpp").read_text(encoding="utf-8")

baud_match = re.search(r"DefaultBaudRate\s*=\s*(\d+)", protocol)
assert baud_match, "missing canonical serial baud rate"
baud = int(baud_match.group(1))
assert baud == 115200
assert config["serial"]["baud_rate"] == baud
assert example["serial"]["baud_rate"] == baud
assert "protocol::DefaultBaudRate" in serial_header
assert "protocol::DefaultBaudRate" in serial_source
assert 'm_config["serial.baud_rate"] = sensor::protocol::DefaultBaudRate;' in config_source
assert re.search(r"#define SERIAL_BAUD_RATE\s+115200", nano)
assert "protocol::Crc8Polynomial" in parser_source
assert "protocol::Crc8InitialValue" in parser_source
assert "protocol::MeasurementFrameSize" in parser_source
assert "ControlProtocolVersion = 1" in protocol
assert "MaxControlCommandBytes = 63" in protocol
assert "ControlPing[] = \"PING\"" in protocol
assert "ControlStatus[] = \"STATUS\"" in protocol
assert "ControlSafe[] = \"SAFE\"" in protocol
assert "DefaultControlBaudRate = DefaultBaudRate" in protocol
assert "sensor::protocol::DefaultControlBaudRate" in pico
assert "gpio_set_function(kUartTxPin, GPIO_FUNC_UART);" in pico
assert "gpio_set_function(kUartRxPin, GPIO_FUNC_UART);" in pico

assert config == example, "config.json and config.json.example must remain identical"
for legacy in ("ir_pwm", "electrodes", "antenna", "actuators_enabled", "effectors"):
    assert legacy not in json.dumps(config).lower(), f"legacy setting remains: {legacy}"
assert config["deployment"]["profile"] == "development"
assert config["encryption"]["enabled"] is False
assert config["encryption"]["algorithm"] == "AES-256-GCM"
assert config["encryption"]["key_derivation"] == "SHA-256 key normalization (32+ byte external secret)"
assert "iterations" not in config["encryption"]
assert config["medical"]["require_authentication"] is True
assert config["devices"]["pico"]["baud_rate"] == baud
assert config["devices"]["pico"]["uart_tx_pin"] == 0
assert config["devices"]["pico"]["uart_rx_pin"] == 1
assert config["devices"]["arduino_nano"]["baud_rate"] == baud
assert config["devices"]["arduino_nano"]["hx711_data_pin"] == 2
assert config["devices"]["arduino_nano"]["hx711_clock_pin"] == 3
assert config["logging"]["console_output"] is False
assert config["logging"]["diagnostics_enabled"] is False
assert "PicoUartTxPin = 0" in protocol
assert "PicoUartRxPin = 1" in protocol
assert "NanoHx711DataPin = 2" in protocol
assert "NanoHx711ClockPin = 3" in protocol
assert "sensor::protocol::PicoUartTxPin" in pico
assert "sensor::protocol::PicoUartRxPin" in pico
assert "#define HX711_DT_PIN  2" in nano
assert "#define HX711_SCK_PIN 3" in nano
print("serial_configuration_regression: PASS")
