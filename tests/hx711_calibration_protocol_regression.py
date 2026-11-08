#!/usr/bin/env python3
"""Guard the Arduino Nano wire format and calibration-status contract."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
firmware = (root / "hx711_advanced.ino").read_text(encoding="utf-8")
serial = (root / "src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
header = (root / "include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
protocol = (root / "include/sensor/SerialProtocolConfig.hpp").read_text(encoding="utf-8")
parser = (root / "src/sensor/ProtocolParser.cpp").read_text(encoding="utf-8")

required_firmware = [
    "#define HX711_DT_PIN  2",
    "#define HX711_SCK_PIN 3",
    "#define SERIAL_BAUD_RATE 115200",
    "struct DataPacket",
    "uint32_t timestamp;",
    "int32_t value;",
    "uint8_t crc;",
    "Serial.write(0xAA);",
    "Serial.write(0x55);",
    "packet.crc = crc8(data_ptr, sizeof(DataPacket) - 1);",
    "bool is_calibrated = false;",
    'Serial.print(F("ms,CAL:"));',
    "is_calibrated ? 1 : 0",
    "strtod(buffer + 4, &end_ptr)",
    "isfinite(parsed_weight)",
    "isfinite(proposed_factor)",
    "return LONG_MIN;",
    "return INT32_MIN;  // Preserve timeout separately from a valid zero sample",
    "if (raw != LONG_MIN)",
    "if (count & 0x800000L)",
    "count |= ~0xFFFFFFL;",
]
for token in required_firmware:
    assert token in firmware, f"missing Nano firmware invariant: {token}"
assert "atof(buffer + 4)" not in firmware
assert "count ^= 0x800000" not in firmware, "HX711 samples must be sign-extended, not offset-flipped"
assert "*end_ptr == '\\t'" in firmware, "calibration parser must use a single escaped tab character"
assert "*end_ptr == '\\0'" in firmware, "calibration parser must use a single escaped NUL terminator"

assert "hasCalibratedValue" in header
assert "data.hasCalibratedValue = calibrationConfirmed;" in serial
assert "protocol::Crc8InitialValue" in parser
assert "protocol::Crc8Polynomial" in parser
assert "protocol::MeasurementPayloadSize" in parser
assert "protocol::MeasurementFrameSize" in parser
assert "protocol::FrameTrailerFirst" in parser
assert "protocol::FrameTrailerSecond" in parser
assert "MeasurementPayloadSize = 9" in protocol
assert "MeasurementFrameSize = 11" in protocol
assert "FrameTrailerFirst = 0xAA" in protocol
assert "FrameTrailerSecond = 0x55" in protocol
assert "NanoHx711DataPin = 2" in protocol
assert "NanoHx711ClockPin = 3" in protocol

print("HX711 Nano wire-format and calibration regression: PASS")
