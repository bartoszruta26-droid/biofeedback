#!/usr/bin/env python3
"""Cross-module integration checks for patient routing and device timestamps."""
from pathlib import Path

main_window = Path("src/gui/MainWindow.cpp").read_text(encoding="utf-8")
patient_tab = Path("src/tab/PatientTab.cpp").read_text(encoding="utf-8")
measurement_tab = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
serial_header = Path("include/sensor/SerialCommunication.hpp").read_text(encoding="utf-8")
serial_source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
adapter = Path("src/sensor/MeasurementSerialAdapter.cpp").read_text(encoding="utf-8")
engine_header = Path("include/measurement/MeasurementEngine.hpp").read_text(encoding="utf-8")
engine_source = Path("src/measurement/MeasurementEngine.cpp").read_text(encoding="utf-8")

patient_signal = "connect(m_patientTab, &tab::PatientTab::currentPatientChanged"
if main_window.count(patient_signal) < 2:
    raise SystemExit("Patient changes must reach both TrainingTab and MeasurementTab")
if "&tab::MeasurementTab::onPatientChanged" not in main_window:
    raise SystemExit("MeasurementTab is not receiving the selected patient context")
if "emit currentPatientChanged(m_currentPatientPesel, m_currentPatientDataPath)" not in patient_tab:
    raise SystemExit("PatientTab must publish the selected patient's ID and data path")
if "setPatientId(pesel);" not in measurement_tab:
    raise SystemExit("MeasurementTab must apply the selected patient ID")

if "bool hasTimestamp;" not in serial_header or "hasTimestamp(false)" not in serial_header:
    raise SystemExit("SensorData must distinguish a missing timestamp from timestamp value zero")
if "sample.hasTimestamp = true;" not in serial_source or "fallback.hasTimestamp = true;" not in serial_source:
    raise SystemExit("Both binary batch frames and text fallback frames must mark an explicitly parsed timestamp")
if adapter.count("input.hasTimestamp = true;") < 2 or adapter.count("steady_clock::now()") < 2:
    raise SystemExit("MeasurementSerialAdapter must synthesize monotonic timestamps for legacy frames in both ingest paths")
if "bool hasTimestamp{false};" not in engine_header:
    raise SystemExit("MeasurementInput must carry explicit timestamp validity")
if "else if (!input.hasTimestamp)" not in engine_source:
    raise SystemExit("MeasurementEngine must validate timestamp presence, not reserve uint32 zero")
if "deviceTimestampMs == 0" in measurement_tab:
    raise SystemExit("MeasurementTab must not treat a valid wrapped device timestamp as a reset signal")

print("Patient routing and device timestamp integration regression passed")
