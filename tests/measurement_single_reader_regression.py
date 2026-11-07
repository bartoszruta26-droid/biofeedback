#!/usr/bin/env python3
"""Guard against multiple consumers reading the same serial stream."""
from pathlib import Path

source = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/MeasurementTab.hpp").read_text(encoding="utf-8")
for forbidden in ("readData(", "tryReadData(", "m_timer", "onTimerTick"):
    if forbidden in source or forbidden in header:
        raise SystemExit(f"MeasurementTab must not poll the serial port ({forbidden})")

manual = source[source.index("void MeasurementTab::onReadSingleSample()"):
                 source.index("void MeasurementTab::", source.index("void MeasurementTab::onReadSingleSample()") + 30)]
if "m_lastSensorData" not in manual or "readSingleSample(valueToUse)" not in manual:
    raise SystemExit("Manual sample action must reuse the latest asynchronously received frame")

receiver = source[source.index("void MeasurementTab::onSensorDataReceived"):
                  source.index("void MeasurementTab::", source.index("void MeasurementTab::onSensorDataReceived") + 30)]
if "readSingleSample(valueToUse)" not in receiver:
    raise SystemExit("Measurement samples must be consumed from the shared callback")

print("Single asynchronous measurement reader policy passed")
