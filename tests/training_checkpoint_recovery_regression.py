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
    "crash after first repetition is recoverable": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::onRepetitionDetected"):tab.index("void TrainingTab::finishCurrentSeries")],
    "crash mid-series is recoverable": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::loadNextRep"):tab.index("void TrainingTab::onRepetitionDetected")],
    "crash during rest is recoverable": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::finishCurrentSeries"):tab.index("void TrainingTab::advanceAfterRest")],
    "crash after exercise change is recoverable": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::loadNextExercise"):tab.index("void TrainingTab::loadNextSeries")],
    "rest transitions checkpoint": "saveExecutionCheckpoint()" in tab[tab.index("void TrainingTab::advanceAfterRest"):tab.index("void TrainingTab::calculateExerciseStats")],
    "recovery offered after loading patient history": "recoverPendingSession(recovered)" in tab and "Odzyskiwanie treningu" in tab,
    "recovery validates cursor bounds": "exerciseIndex >= stagedPlan.size()" in tab and "seriesIndex >= stagedPlan[exerciseIndex].seriesCount" in tab,
    "restoring starts paused and retains resume state": "restore(biofeedback::training::SessionState::Paused," in tab,
    "session clock restores elapsed values": "m_trainingClock.restore(sessionMs, activeMs)" in tab and "m_sessionElapsedOffsetMs + elapsedMs" in tab,
    "state machine restore validates state": "bool restore(SessionState state" in machine,
    "session clock restore is monotonic": "void restore(std::int64_t sessionMs, std::int64_t activeMs)" in clock,
    "recovery record is updated in place": "replacedActiveSession" in analytics,
    "checkpoint JSON is atomically persisted": "QSaveFile file(path)" in analytics and "!file.commit()" in analytics,
    "checkpoint restore validates before mutating live state": (
        "// Phase 1: parse and validate everything" in tab and
        "// Phase 2: commit." in tab and
        tab.index("// Phase 1: parse and validate everything") <
        tab.index("// Phase 2: commit.")
    ),
    "checkpoint restore stages detector before commit": (
        "biofeedback::training::RepetitionDetector stagedDetector" in tab and
        "if (!stagedDetector.restore(detectorSnapshot)) return false;" in tab and
        tab.index("if (!stagedDetector.restore(detectorSnapshot)) return false;") <
        tab.index("// Phase 2: commit.")
    ),
    "version 2 requires a detector snapshot": "checkpointVersion >= 2 && !hasDetectorSnapshot" in tab,
    "recovery rejects incomplete or mismatched exercise statistics": (
        "if (stagedStats.size() != stagedPlan.size()) return false;" in tab and
        "stagedStats[index].exerciseName != stagedPlan[index].name" in tab
    ),
    "restored elapsed values are finite and ordered": (
        "activeMsValue > sessionMsValue" in tab and
        "restMsValue > 86400000.0" in tab
    ),
    "no patient identifier is serialized": '"patient_pesel"' not in tab and '"patient_id"' not in tab,
}
failed = [name for name, ok in checks.items() if not ok]
if failed:
    raise SystemExit("Training checkpoint recovery regression failed: " + ", ".join(failed))
print("Training checkpoint recovery regression passed")
