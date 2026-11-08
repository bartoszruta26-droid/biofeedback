#!/usr/bin/env python3
"""Guard the firmware/app calibration-status contract."""
from pathlib import Path

firmware = Path("hx711_advanced.ino").read_text(encoding="utf-8")
serial = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
header = Path("include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
if "bool is_calibrated = false;" not in firmware:
    raise SystemExit("Firmware must fail closed after boot until calibrated")
if 'Serial.print(F("ms,CAL:"));' not in firmware or "is_calibrated ? 1 : 0" not in firmware:
    raise SystemExit("Firmware FORCE record must expose explicit CAL status")
if "calibration_factor = known_weight / raw_value;" not in firmware or "is_calibrated = true;" not in firmware:
    raise SystemExit("CAL command must set calibration factor and active flag")
if "hasCalibratedValue" not in header or "data.hasCalibratedValue = calibrationConfirmed;" not in serial:
    raise SystemExit("Host parser must preserve explicit calibration validity")
print("HX711 calibration protocol regression passed")
