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
if "m_lastSensorData" not in manual or "readSingleSample(value, m_lastSensorData.timestamp, m_lastSensorData.value, true," not in manual:
    raise SystemExit("Manual capture must reuse the latest asynchronous frame, timestamp and raw ADC value")
if "if (m_isMeasuring)" not in manual or "AKTYWNY POMIAR ZAPISUJE PRÓBKI AUTOMATYCZNIE" not in manual:
    raise SystemExit("Manual capture must not replay a sample while the asynchronous session is recording")

# The desktop adapter always commits unmodified ADC counts. The durable journal
# stores those counts independently of the currently selected presentation unit.
if '"desktop-hx711", "count"' not in source:
    raise SystemExit("Desktop acquisition must always use raw count units")
if 'record["unit"] = "count"' not in source or 'record["rawValue"] = sample.rawValue' not in source:
    raise SystemExit("Every acquired sample must be durably archived as RAW with its ADC value")
handler_start = source.index("void MeasurementTab::onMeasurementSample")
handler_end = source.index("// MeasurementTab - Konwersja jednostek", handler_start)
handler = source[handler_start:handler_end]
if "sample.isValid()" not in handler or 'sample.unit != "count"' not in handler:
    raise SystemExit("RAW display must consume valid canonical count samples")
if "rawToNewtons(raw, forceNewtons)" not in handler:
    raise SystemExit("Calibrated display must convert raw counts in the desktop GUI")
if "CalibrationMath::convertRaw" not in source or "CalibrationProfileStore::load" not in source:
    raise SystemExit("Desktop calibration must be loaded from the validated configuration profile")
if "m_measurementEngine->subscribe" not in source:
    raise SystemExit("MeasurementTab must subscribe to the shared measurement engine")
if "m_showRawValues(true)" not in source or "m_currentUnit(ForceUnit::Raw)" not in source:
    raise SystemExit("RAW values must be the default desktop presentation")
if "m_btnToggleRaw->setEnabled(true)" not in source:
    raise SystemExit("RAW diagnostic mode must be selectable while idle")
if "if (!m_serialPort || !m_serialPort->isConnected())" not in source:
    raise SystemExit("Measurement must refuse to start without a connected sensor")
print("Single-reader measurement regression passed")
