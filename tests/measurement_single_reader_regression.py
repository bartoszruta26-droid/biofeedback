#!/usr/bin/env python3
"""Guard against multiple consumers reading the same serial stream."""
from pathlib import Path

source = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/MeasurementTab.hpp").read_text(encoding="utf-8")
for forbidden in ("readData(", "tryReadData(", "m_timer", "onTimerTick"):
    if forbidden in source or forbidden in header:
        raise SystemExit(f"MeasurementTab must not poll the serial port ({forbidden})")

manual_start = source.index("void MeasurementTab::onReadSingleSample()")
manual_end = source.index("void MeasurementTab::", manual_start + 30)
manual = source[manual_start:manual_end]
if "m_lastSensorData" not in manual or "readSingleSample(valueToUse)" not in manual:
    raise SystemExit("Manual sample action must reuse the latest asynchronously received frame")

receiver_start = source.index("void MeasurementTab::onSensorDataReceived")
receiver_end = source.index("void MeasurementTab::", receiver_start + 30)
receiver = source[receiver_start:receiver_end]
if "readSingleSample(valueToUse)" not in receiver:
    raise SystemExit("Measurement samples must be consumed from the shared callback")
if ", m_showRawValues(false)" not in source:
    raise SystemExit("Calibrated force in newtons must be the default measurement mode")
if "m_btnToggleRaw->setEnabled(true)" not in source:
    raise SystemExit("Raw/calibrated mode must be selectable while idle")
if "if (!m_serialPort || !m_serialPort->isConnected())" not in source:
    raise SystemExit("Measurement must refuse to start without a connected sensor")

print("Single asynchronous measurement reader policy passed")
