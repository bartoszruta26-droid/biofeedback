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
    '"sample_count"', '"game_points"',
):
    if field not in save:
        raise SystemExit(f"Training session history is missing {field}")
if "QSaveFile" not in save or "!file.commit()" not in save:
    raise SystemExit("Training history must use atomic file replacement")
if "QFileDevice::ReadOwner | QFileDevice::WriteOwner" not in save:
    raise SystemExit("Training history must use owner-only file permissions")
print("Complete training history regression passed")
