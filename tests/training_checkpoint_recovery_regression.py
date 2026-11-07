#!/usr/bin/env python3
"""Regression guards for durable training cursor, plan recovery and crash boundaries."""
from pathlib import Path

tab = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")
analytics = Path("src/tab/TrainingAnalyticsWidget.cpp").read_text(encoding="utf-8")
machine = Path("include/training/SessionStateMachine.hpp").read_text(encoding="utf-8")
clock = Path("include/training/TrainingSessionClock.hpp").read_text(encoding="utf-8")
restart_test = Path("tests/training_history_restart_regression.cpp").read_text(encoding="utf-8")
cmake = Path("CMakeLists.txt").read_text(encoding="utf-8")
process_restart_test = Path("tests/training_tab_process_restart_regression.cpp").read_text(encoding="utf-8")

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
    "restart recovery activates and deserializes the persisted history entry": (
        "onHistoryEntryActivated(row, 0);" in analytics and
        "m_currentStats = restored;" in analytics and
        "m_executionState = candidate.value(\"execution_state\").toObject();" in analytics
    ),
    "history reload restores every repetition sample": (
        "const QJsonArray repetitions = object.value(\"repetitions\").toArray();" in analytics and
        "const QJsonArray samples = repetitionObject.value(\"samples\").toArray();" in analytics and
        "record.measurement.samples.push_back(sample);" in analytics
    ),
    "full TrainingTab restart test launches separate OS processes": (
        "QProcess process;" in process_restart_test and
        "process.start(executable, {stage, root});" in process_restart_test and
        "std::_Exit(0);" in process_restart_test
    ),
    "full TrainingTab restart test verifies plan, cursor and detector samples": (
        "m_exercisePlan.size() == 2" in process_restart_test and
        "m_currentExerciseIndex == 1" in process_restart_test and
        "m_currentSeriesIndex == 0" in process_restart_test and
        "m_currentRepIndex == 1" in process_restart_test and
        "detector.samples.size() == 2" in process_restart_test
    ),
    "full TrainingTab restart test continues an in-flight repetition exactly once": (
        "training.onRepetitionDetected(completed);" in process_restart_test and
        "m_exerciseStats[1].completedReps == 1" in process_restart_test and
        "m_exerciseStats[1].repetitions.size() == 1" in process_restart_test
    ),
    "full TrainingTab restart test preserves exact paused rest milliseconds over repeated crashes": (
        "m_restDurationMs == 12345" in process_restart_test and
        "verify-rest-again" in process_restart_test and
        "m_stateMachine.resting()" in process_restart_test
    ),
    "full TrainingTab process restart test is registered in CTest": (
        "training_tab_process_restart_regression" in cmake and
        "training_tab_process_restart_regression" in Path(".github/workflows/security-regression.yml").read_text(encoding="utf-8")
    ),
    "restart-level test recreates the analytics widget": (
        "TrainingAnalyticsWidget firstProcess;" in restart_test and
        "TrainingAnalyticsWidget restartedProcess;" in restart_test and
        "recoverPendingSession(recoveredEntry)" in restart_test
    ),
    "restart-level test verifies no duplicate history row": (
        "document.array().size() == 1" in restart_test and
        "repetition_count\").toInt() == 2" in restart_test
    ),
    "restart-level test is registered for offscreen Qt": (
        "training_history_restart_regression" in cmake and
        "QT_QPA_PLATFORM=offscreen" in cmake
    ),
    "checkpoint persists the full exercise and repetition history": (
        "entry.insert(\"exercises\", exerciseEntries);" in analytics and
        "repetitionEntry.insert(\"samples\", sampleEntries);" in analytics
    ),
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
