#!/usr/bin/env python3
"""Regression guards for durable training cursor, plan recovery and crash boundaries."""
from pathlib import Path

tab = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")
analytics = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")
machine = Path("include/training/SessionStateMachine.hpp").read_text(encoding="utf-8")
clock = Path("include/training/TrainingSessionClock.hpp").read_text(encoding="utf-8")

checks = {
    "plan is persisted": '"exercise_plan"' in tab and '"series_count"' in tab and '"repetitions_per_series"' in tab,
    "execution cursor is persisted": '"current_exercise_index"' in tab and '"current_series_index"' in tab and '"current_repetition_index"' in tab,
    "monotonic session and active time are persisted": '"session_elapsed_ms"' in tab and '"active_elapsed_ms"' in tab,
    "remaining rest is persisted in milliseconds": '"rest_remaining_ms"' in tab,
    "repetition checkpoints are written": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::onRepetitionDetected"):tab.index("void TrainingTab::finishCurrentSeries")],
    "series and exercise transitions checkpoint": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::finishCurrentSeries"):tab.index("void TrainingTab::advanceAfterRest")],
    "rest transitions checkpoint": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::advanceAfterRest"):tab.index("void TrainingTab::calculateExerciseStats")],
    "recovery offered after loading patient history": "recoverPendingSession(recovered)" in tab and "Odzyskiwanie treningu" in tab,
    "recovery validates cursor bounds": "exerciseIndex >= plan.size()" in tab and "seriesIndex >= plan[exerciseIndex].seriesCount" in tab,
    "restoring starts paused and retains resume state": "SessionState::Paused, resumeState" in tab,
    "session clock restores elapsed values": "m_trainingClock.restore(sessionMs, activeMs)" in tab and "m_sessionElapsedOffsetMs + elapsedMs" in tab,
    "state machine restore validates state": "bool restore(SessionState state" in machine,
    "session clock restore is monotonic": "void restore(std::int64_t sessionMs, std::int64_t activeMs)" in clock,
    "recovery record is updated in place": "replacedActiveSession" in analytics,
    "checkpoint JSON is atomically persisted": "QSaveFile file(path)" in analytics and "!file.commit()" in analytics,
    "no patient identifier is serialized": '"patient_pesel"' not in tab and '"patient_id"' not in tab,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    raise SystemExit("Training checkpoint recovery regression failed: " + ", ".join(failed))
print("Training checkpoint recovery regression passed")
