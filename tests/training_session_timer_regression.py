#!/usr/bin/env python3
"""Guard monotonic training clocks, pause-aware rest, and interruption metadata."""
from pathlib import Path

header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")
source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
analytics_h = Path("include/tab/TrainingAnalyticsWidget.hpp").read_text(encoding="utf-8")
analytics = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")

checks = {
    "monotonic session clock": "QElapsedTimer m_sessionClock" in header,
    "deterministic active/session clock model": "TrainingSessionClock m_trainingClock" in header,
    "rest clock monotonic": "QElapsedTimer m_restClock" in header,
    "active time excludes pause and rest": "m_trainingClock.update(m_sessionElapsedOffsetMs + elapsedMs, !m_isPaused && !m_inRestPeriod)" in source,
    "session clock keeps ticking during pause": "m_timer->stop();" not in source[source.index("void TrainingTab::pauseTraining"):source.index("void TrainingTab::resumeTraining")],
    "rest countdown based on elapsed monotonic time": "m_restDurationMs - m_restClock.elapsed()" in source,
    "rest duration freezes on pause": "m_restDurationMs = static_cast<qint64>(m_restTimeRemaining) * 1000;" in source,
    "completed/interrupted stop distinction": "void TrainingTab::stopTraining(bool completed)" in source,
    "automatic completion marked completed": "stopTraining(true);" in source,
    "active duration exposed to analytics": "activeDurationSeconds" in analytics_h and "m_trainingClock.activeSeconds()" in source,
    "session status persisted": '"session_status"' in analytics,
    "active duration persisted": '"active_duration_seconds"' in analytics,
}
for label, passed in checks.items():
    if not passed:
        raise SystemExit(f"Training clock/session regression failed: {label}")
if "m_timer->setInterval(1000);  // 1 Hz" not in source:
    raise SystemExit("Training session UI timer must remain 1 Hz")
print("Training monotonic clock, pause-aware rest, and interrupted-session regression passed")
