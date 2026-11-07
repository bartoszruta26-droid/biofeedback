#!/usr/bin/env python3
"""Guard against concurrent serial readers in the desktop measurement tab."""
from pathlib import Path

source = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/MeasurementTab.hpp").read_text(encoding="utf-8")
for forbidden in ("readData(", "tryReadData(", "m_timer", "onTimerTick"):
    if forbidden in source or forbidden in header:
        raise SystemExit(f"MeasurementTab must use the shared asynchronous stream ({forbidden})")

manual_start = source.index("void MeasurementTab::onReadSingleSample()")
manual_end = source.index("void MeasurementTab::onSelectionChanged()", manual_start)
manual = source[manual_start:manual_end]
if "m_lastSensorData" not in manual or "readSingleSample(value)" not in manual:
    raise SystemExit("Manual capture must reuse the latest asynchronous frame")

receiver_start = source.index("void MeasurementTab::onSensorDataReceived")
receiver_end = source.index("// MeasurementTab - Konwersja jednostek", receiver_start)
receiver = source[receiver_start:receiver_end]
if "readSingleSample(value)" not in receiver:
    raise SystemExit("Live samples must flow from the shared callback into measurement")
if "m_showRawValues(false)" not in source:
    raise SystemExit("Calibrated force in SI units must be the default")
if "m_btnToggleRaw->setEnabled(true)" not in source:
    raise SystemExit("RAW diagnostic mode must be selectable while idle")
if "if (!m_serialPort || !m_serialPort->isConnected())" not in source:
    raise SystemExit("Measurement must refuse to start without a connected sensor")
print("Single-reader measurement regression passed")
