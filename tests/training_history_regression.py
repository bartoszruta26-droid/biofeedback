#!/usr/bin/env python3
"""Guard the desktop training-history schema and atomic persistence path."""
from pathlib import Path

source = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")
start = source.index("void TrainingAnalyticsWidget::saveCurrentSession")
end = source.index("void TrainingAnalyticsWidget::rebuildSummary", start)
save = source[start:end]
for field in (
    '"schema_version"', '"exercises"', '"series"', '"repetitions"',
    '"start_timestamp_ms"', '"end_timestamp_ms"', '"duration_ms"',
    '"time_to_peak_ms"', '"peak_force_n"', '"mean_force_n"',
    '"sample_count"', '"samples"', '"timestamp_ms"', '"force_n"', '"game_points"',
):
    if field not in save:
        raise SystemExit(f"Training session history is missing {field}")
if "QSaveFile" not in save or "!file.commit()" not in save:
    raise SystemExit("Training history must use atomic file replacement")
if "QFileDevice::ReadOwner | QFileDevice::WriteOwner" not in save:
    raise SystemExit("Training history must use owner-only file permissions")
widget = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")
if "bool TrainingAnalyticsWidget::beginSession()" not in widget:
    raise SystemExit("Training session must snapshot its history destination at start")
if "m_activeSessionHistoryFilePath" not in save:
    raise SystemExit("Session save must use the original patient destination")
if "rebuildForceTraceChart(restored)" not in widget or "rebuildForceTraceChart(stats)" not in widget:
    raise SystemExit("Training analytics must render force-time traces for live and restored sessions")
main = Path("src/gui/MainWindow.cpp").read_text(encoding="utf-8")
if "MeasurementTab::measurementStarted" not in main or "MeasurementTab::measurementStopped" not in main:
    raise SystemExit("Patient switching must be locked during measurement acquisition")
print("Complete training history and patient-context regression passed")
