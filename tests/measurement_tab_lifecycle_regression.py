#!/usr/bin/env python3
"""Static lifecycle regression checks for the Qt measurement tab."""
from pathlib import Path

source = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/MeasurementTab.hpp").read_text(encoding="utf-8")

for label, content in (("implementation", source), ("header", header)):
    if "connectToArduinoAsync" in content:
        raise SystemExit(f"{label}: obsolete automatic-connect worker remains")

if "std::thread" in source or ".detach()" in source:
    raise SystemExit("MeasurementTab must not launch an unowned background thread")

if "processSample(data)" in source:
    raise SystemExit("MeasurementTab must not ingest a sample already ingested by its adapter")

if "MeasurementSerialAdapter" not in source:
    raise SystemExit("MeasurementTab must retain the shared measurement adapter")
if "m_repetitionDetector.process(deviceTimestampMs, forceValue, true, repetition)" not in source:
    raise SystemExit("MeasurementTab must use the shared repetition detector")
if "m_repetitionDetector.reset()" not in source:
    raise SystemExit("MeasurementTab must reset repetition state at session boundaries")

print("MeasurementTab lifecycle and single-ingestion policy passed")
