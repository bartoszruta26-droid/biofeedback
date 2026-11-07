#!/usr/bin/env python3
"""Guard state-machine integration and durable interrupted-session autosave."""
from pathlib import Path

header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")
source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
analytics_header = Path("include/tab/TrainingAnalyticsWidget.hpp").read_text(encoding="utf-8")
analytics = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")
machine = Path("include/training/SessionStateMachine.hpp").read_text(encoding="utf-8")
checks = {
    "state machine owned by TrainingTab": "SessionStateMachine m_stateMachine" in header,
    "start guarded by state machine": "if (!m_stateMachine.canStart()) return;" in source,
    "active transition on start": "if (!m_stateMachine.start()) return;" in source,
    "pause transition": "!m_stateMachine.pause()" in source,
    "resume transition": "!m_stateMachine.resume()" in source,
    "rest transition": "m_stateMachine.beginRest();" in source,
    "end-rest transition": "m_stateMachine.endRest();" in source,
    "terminal completed/interrupted states": "m_stateMachine.complete()" in source and "m_stateMachine.interrupt()" in source,
    "automatic persistence API": "bool saveSessionAutomatically();" in analytics_header,
    "checkpoint API": "bool saveSessionCheckpoint();" in analytics_header,
    "checkpoint records update in place": "replacedActiveSession" in analytics and '"in_progress"' in analytics,
    "automatic persistence called on stop": "saveSessionAutomatically()" in source,
    "persistence supports zero repetitions": "repetitions > 0 ? weightedPeak / repetitions : 0.0" in analytics,
    "failed save keeps session locked": "if (!m_activeSessionHistoryFilePath.isEmpty() && !m_sessionSaved)\n        return false;" in analytics,
    "state machine has all terminal states": "Completed, Interrupted" in machine,
}
for label, ok in checks.items():
    if not ok:
        raise SystemExit(f"Training state/persistence regression failed: {label}")
print("Training state machine integration and interrupted-session autosave guards passed")
