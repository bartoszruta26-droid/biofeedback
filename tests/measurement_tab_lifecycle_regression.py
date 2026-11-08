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

# The realtime pipeline must use a monotonic session clock and keep UI-only
# buffers bounded independently from the full JSON sample archive.
for required in (
    "QElapsedTimer m_sessionElapsedTimer",
    "m_sessionElapsedTimer.elapsed() / 1000.0",
    "m_sessionElapsedOffsetSeconds",
    "recorded.calibratedMassKg = calibratedMassKg",
    "sample.calibratedMassKg = sampleObj[\"calibratedMassKg\"].toDouble()",
    "constexpr int liveBufferCapacity = 10000",
    "m_recordedSamples.append(recorded)",
    "bool MeasurementTab::openSampleJournal()",
    "bool MeasurementTab::appendSampleJournal(const RecordedMeasurementSample& sample)",
    "m_sampleJournalPendingWrites >= 32",
    "::fsync(m_sampleJournal.handle())",
    "recoverSampleJournals(dataDir)",
    "Never carry samples or a live journal across patient boundaries",
    "if (hasActivePatientData)\n        resetSession();",
    "if (m_patientId == id) return;",
    'header["firstSequence"] = static_cast<qint64>(m_recordedSamples.size()) + 1',
    'expectedSequence = object["firstSequence"].toVariant().toULongLong()',
    "Prefer a finalized JSON session over its still-present journal",
    "if (file.atEnd()) break;",
    "Materialize the recovered stream as a normal, selectable JSON",
    "QSaveFile recoveredFile(recoveredPath)",
    'record["recordType"] = "sample"',
    "void MeasurementTab::showCurrentMeasurementData()",
    "showCurrentMeasurementData();",
    "void MeasurementTab::updateBiomechanicalTable()",
    "m_elasticConstantInput->value()",
    "m_patientMassInput->value()",
    "recalculateBiomechanicalResults()",
    "m_biomechSeriesSelector->addItem",
    "m_biomechSeriesSelector->currentData().toInt()",
    'root["elasticConstantNPerM"] = session.elasticConstantNPerM',
    'root["patientMassKg"] = session.patientMassKg',
    'root["schemaVersion"] = 3',
    "currentSeries.biomechanicalParams =",
    "m_completedSeriesStats.last().seriesNumber == m_currentSeries",
    "currentSeries.seriesNumber = m_currentSeries",
    "m_currentSeries > 3",
    "stopMeasurement();\n            emit sessionFinished(m_completedSeriesStats);",
    "biomechanicalData.samples.emplace_back",
    "series.biomechanicalParams.assignValues(seriesValues)",
    "calculateAllParams(biomechanicalSeries)",
    'seriesObj["biomechanicalAvailability"] = seriesBiomechanicalAvailability',
    'repObj["biomechanicalInput"] = biomechanicalInput',
    'sampleObject["timestamp"] = static_cast<qint64>(sample.timestamp)',
    "m_forcePlot->addSample(sample.value, sample.elapsedTimeSeconds)",
    'root["samples"] = samplesArray',
    'const QJsonArray samplesArray = root["samples"].toArray()',
    "deserializeSessionFromJSON(jsonContent, false)",
    "if (includeSamples) {",
    "const QJsonArray biomechanicalSamples = biomechanicalInput[\"samples\"].toArray()",
    "currentSeries.reps.last().endTime -",
    "currentSeries.reps.first().startTime",
):
    if required not in source and required not in header:
        raise SystemExit(f"Missing realtime/archive invariant: {required}")

if 'addDebugMessage(QString("HX711: RAW=' in source:
    raise SystemExit("Per-sample debug logging must not run in the acquisition hot path")


# Sensor-to-game control is a state input, so the GUI mailbox must coalesce
# stale values instead of replaying a long FIFO backlog.
game_source = Path("src/gui/MainWindow.cpp").read_text(encoding="utf-8")
game_header = Path("include/gui/MainWindow.hpp").read_text(encoding="utf-8")
training_source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
for required in (
    "m_pendingGameSensorInput = value",
    "m_gameSensorInputAvailable = true",
    "const bool queued = QMetaObject::invokeMethod",
    "m_gameInputDrainScheduled = false",
    "forwardSensorToGame(latestValue)",
):
    if required not in game_source:
        raise SystemExit(f"Missing low-latency game input invariant: {required}")
if "std::deque<double> m_gameSensorInputQueue" in game_header:
    raise SystemExit("MainWindow must not replay stale game-control samples from a FIFO queue")
if "MaximumQueuedInputs" in game_source or "scheduleAgain" in game_source:
    raise SystemExit("Game control must not drain a stale multi-batch input backlog")

# Failed Qt event scheduling must release the scheduled flag and account for
# the discarded UI backlog, both on initial enqueue and on batch rescheduling.
for label, content in (("MeasurementTab", source), ("TrainingTab", training_source)):
    if content.count("const bool queued = QMetaObject::invokeMethod") < 2:
        raise SystemExit(f"{label}: initial and follow-up drain scheduling must check invokeMethod")
    if content.count("m_measurementQueue.clear();") < 2:
        raise SystemExit(f"{label}: failed scheduling must clear stranded UI backlog")
    if content.count("m_measurementDrainScheduled = false;") < 2:
        raise SystemExit(f"{label}: failed scheduling must release the drain flag")


# ForcePlotWidget must keep the visible-window trace in O(1)-amortized append
# and expiry time; per-sample full-window scans and QVector front shifts add lag.
plot_header = header
plot_source = source
for required in (
    "std::deque<double> m_forces",
    "std::deque<double> m_times",
    "std::deque<std::pair<double, double>> m_maxCandidates",
    "std::deque<std::pair<double, double>> m_minCandidates",
    "m_maxCandidates.emplace_back(time, force)",
    "m_minCandidates.emplace_back(time, force)",
):
    if required not in plot_header and required not in plot_source:
        raise SystemExit(f"Missing constant-time live plot invariant: {required}")
add_sample_start = plot_source.index("void ForcePlotWidget::addSample")
add_sample_end = plot_source.index("void ForcePlotWidget::clearData", add_sample_start)
add_sample_body = plot_source[add_sample_start:add_sample_end]
if "removeFirst()" in add_sample_body or "for (double sample : m_forces)" in add_sample_body:
    raise SystemExit("ForcePlotWidget must not shift or rescan the full visible trace per sample")

print("Measurement, training and game realtime dispatch regression passed")
