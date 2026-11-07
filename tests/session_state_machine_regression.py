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
    "parallel training flags removed": all(flag not in header and flag not in source for flag in ("m_isTraining", "m_isPaused", "m_inRestPeriod")),
    "running state comes from state machine": "m_stateMachine.running()" in source,
    "pause state comes from state machine": "m_stateMachine.paused()" in source,
    "rest state comes from state machine": "m_stateMachine.resting()" in source,
    "start guarded by state machine": "if (!m_stateMachine.canStart()) return;" in source,
    "active transition on start": "if (!m_stateMachine.start()) return;" in source,
    "pause transition": "!m_stateMachine.pause()" in source,
    "resume transition": "!m_stateMachine.resume()" in source,
    "rest transition records its destination": "m_stateMachine.beginRest(" in source and "RestTarget::NextExercise" in source,
    "end-rest transition is guarded": "m_stateMachine.endRest()" in source,
    "terminal completed/interrupted states": "m_stateMachine.complete()" in source and "m_stateMachine.interrupt()" in source,
    "automatic persistence API": "bool saveSessionAutomatically();" in analytics_header,
    "checkpoint API": "bool saveSessionCheckpoint();" in analytics_header,
    "checkpoint records update in place": "replacedActiveSession" in analytics and '"in_progress"' in analytics,
    "automatic persistence called on stop": "saveSessionAutomatically()" in source,
    "persistence supports zero repetitions": "repetitions > 0 ? weightedPeak / repetitions : 0.0" in analytics,
    "failed save keeps session locked": "if (!m_activeSessionHistoryFilePath.isEmpty() && !m_sessionSaved)\n        return false;" in analytics,
    "checkpoint restores plan and cursor": '"exercise_plan"' in source and "restoreExecutionCheckpoint" in source and "current_repetition_index" in source,
    "recovery prompts before resuming": "Odzyskiwanie treningu" in source and "recoverPendingSession" in source,
    "rest remainder is checkpointed": '"rest_remaining_ms"' in source and "m_restDurationMs - m_restClock.elapsed()" in source,
    "recovered session is resumed paused": "restore(biofeedback::training::SessionState::Paused," in source,
    "state machine supports restoration": "bool restore(SessionState state" in machine,
    "state machine has all terminal states": "Completed, Interrupted" in machine,
}
for label, ok in checks.items():
    if not ok:
        raise SystemExit(f"Training state/persistence regression failed: {label}")
print("Training state machine integration and interrupted-session autosave guards passed")
