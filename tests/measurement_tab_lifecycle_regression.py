#!/usr/bin/env python3
"""Static lifecycle regression checks for the Qt measurement tab."""
from pathlib import Path

source = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
header = Path("include/tab/MeasurementTab.hpp").read_text(encoding="utf-8")
training_source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
training_header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")

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
    "deviceElapsedMs = deviceTimestampMs - m_deviceTimestampOriginMs",
    "m_hasDeviceTimestampOrigin = false",
    "recorded.calibratedMassKg = calibratedMassKg",
    "sample.calibratedMassKg = sampleObj[\"calibratedMassKg\"].toDouble()",
    "constexpr int liveBufferCapacity = 10000",
    "m_recordedSamples.append(recorded)",
    "bool MeasurementTab::openSampleJournal()",
    "SampleJournalWriter m_sampleJournalWriter",
    "m_sampleJournalWriter.appendJson(record)",
    "m_totalDroppedMeasurementSamples",
    "sample.ingestMonotonicNs",
    "m_uiDispatchLatencyWindowMs",
    "Opóźnienie ingest→GUI [ms]: p50=%1, p95=%2, p99=%3",
    "dziennik trwały zachowuje pełny pomiar",
    "m_sampleJournalWriter.finish()",
    "m_sampleJournalWriter.isAccepting()",
    "recoverSampleJournals(dataDir)",
    "Never carry samples or a live journal across patient boundaries",
    "if (hasActivePatientData)\n        resetSession();",
    "if (m_patientId == id) return;",
    'header["firstSequence"] = static_cast<qint64>(m_journalNextSequence.load(std::memory_order_acquire))',
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

if "m_sampleJournal.write(" in source or "m_sampleJournal.flush(" in source:
    raise SystemExit("Disk I/O must be owned by SampleJournalWriter, not the GUI thread")
if "m_sampleJournalPendingWrites" in source or "QFile m_sampleJournal" in header:
    raise SystemExit("Legacy synchronous sample journal state remains in MeasurementTab")
writer_source = Path("src/tab/SampleJournalWriter.cpp").read_text(encoding="utf-8")
for required in (
    "std::thread(&SampleJournalWriter::run, this)",
    "m_queue.size() >= m_capacity",
    "m_queue.push_back(PendingRecord{line, ticket, durable})",
    "m_changed.wait(lock",
    "file.write(line)",
    "syncParentDirectory(m_path, error)",
    "::fsync(directoryFd)",
    "flushDurably(file, error)",
    "pendingSinceSync >= 8",
    "::fsync(file.handle())",
):
    if required not in writer_source:
        raise SystemExit(f"Missing background journal writer invariant: {required}")

if 'addDebugMessage(QString("HX711: RAW=' in source:
    raise SystemExit("Per-sample debug logging must not run in the acquisition hot path")

# The raw-data table is a presentation surface, not an acquisition sink.
# Batch its widget mutations to at most 10 Hz and flush the visible tail at stop;
# the full recorded sample stream remains independent of this lossy UI view.
sample_start = source.index("void MeasurementTab::readSingleSample(")
sample_end = source.index("void MeasurementTab::updateRawTableBatch(", sample_start)
sample_body = source[sample_start:sample_end]
if "m_rawTable->insertRow" in sample_body or "m_rawTable->scrollToBottom" in sample_body:
    raise SystemExit("Per-sample raw table widget mutations remain in readSingleSample")
batch_start = source.index("void MeasurementTab::updateRawTableBatch(")
batch_end = source.index("void MeasurementTab::updateLiveDisplay(", batch_start)
batch_body = source[batch_start:batch_end]
for required in (
    "elapsedMs - m_lastRawTableRefreshMs < 100",
    "m_rawTable->setUpdatesEnabled(false)",
    "m_rawTable->setUpdatesEnabled(true)",
    "m_rawTableLastSequence = totalSamples",
    "m_rawTable->scrollToBottom()",
    "while (m_rawTable->rowCount() > 100)",
):
    if required not in batch_body:
        raise SystemExit(f"Missing batched raw-table invariant: {required}")
if "updateRawTableBatch(true);" not in source:
    raise SystemExit("Stopping a measurement must flush pending raw-table presentation samples")
if "quint64 m_rawTableLastSequence{0};" not in header:
    raise SystemExit("Raw-table presentation sequence must be tracked independently")



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
# MeasurementTab still has two checked invokeMethod scheduling sites.
if source.count("const bool queued = QMetaObject::invokeMethod") < 2:
    raise SystemExit("MeasurementTab: initial and follow-up drain scheduling must check invokeMethod")
if source.count("m_measurementQueue.clear();") < 2:
    raise SystemExit("MeasurementTab: failed scheduling must clear stranded UI backlog")
if source.count("m_measurementDrainScheduled = false;") < 2:
    raise SystemExit("MeasurementTab: failed scheduling must release the drain flag")

# GUI queues are deliberately short: at 80 SPS, 64 frames cap stale
# presentation backlog at 800 ms. The durable journal remains lossless.
if "MaximumQueuedSamples = 64" not in source:
    raise SystemExit("MeasurementTab GUI backlog must be capped at 64 samples")
if "MaximumQueuedSamples = 64" not in training_source:
    raise SystemExit("TrainingTab GUI backlog must be capped at 64 samples")
if "m_measurementQueue.clear();" not in source[source.index("void MeasurementTab::enqueueMeasurementSample("):]:
    raise SystemExit("MeasurementTab overload must discard stale frames to preserve freshness")

# TrainingTab retains bounded FIFO batches so Qt can service other events.
training_drain_start = training_source.index("void TrainingTab::drainMeasurementSamples(")
training_drain_end = training_source.index("void TrainingTab::scheduleLiveUiRefresh(", training_drain_start)
training_drain = training_source[training_drain_start:training_drain_end]
if "constexpr std::size_t MaximumBatch = 16;" not in training_drain:
    raise SystemExit("TrainingTab must keep GUI analysis batches bounded")
if "scheduleAgain" not in training_drain or "QMetaObject::invokeMethod" not in training_drain:
    raise SystemExit("TrainingTab must reschedule remaining FIFO work without dropping it")

# The training detector receives every real sample; only the visible UI and
# game control are refreshed from the latest value at a bounded 60 Hz cadence.
for required in (
    "m_repetitionDetector.process(sample.deviceTimestampMs, force, true, repetition)",
    "m_liveUiTimer->setInterval(16)",
    "m_liveUiTimer->setTimerType(Qt::PreciseTimer)",
    "sample.ingestMonotonicNs",
    "Opóźnienie ingest→GUI treningu [ms]: p50=%1, p95=%2, p99=%3",
    "void TrainingTab::scheduleLiveUiRefresh()",
    "void TrainingTab::refreshLiveUi()",
    "m_progressWidget->setCurrentForce(force)",
    "m_lblCurrentForce->setText(QString(\"%1 N\").arg(force, 0, 'f', 2));",
    "m_currentGame->handleSensorInput(gameInput)",
):
    if required not in training_source:
        raise SystemExit(f"Missing low-latency training invariant: {required}")
if "QTimer* m_liveUiTimer{nullptr};" not in training_header:
    raise SystemExit("TrainingTab live UI timer state is missing")


# Training's detector consumes each frame; visible widgets and game input
# are updated from the latest-value snapshot by a precise 60 Hz timer.

drain_start = training_source.index("void TrainingTab::drainMeasurementSamples(")
drain_end = training_source.index("void TrainingTab::scheduleLiveUiRefresh(", drain_start)
drain_body = training_source[drain_start:drain_end]
if "MaximumBatch = 16" not in drain_body or "scheduleAgain" not in drain_body:
    raise SystemExit("Training analysis must preserve bounded FIFO batches")
for required in (
    "m_repetitionDetector.process(sample.deviceTimestampMs, force, true, repetition)",
    "m_liveUiTimer->setInterval(16)",
    "m_liveUiTimer->setTimerType(Qt::PreciseTimer)",
    "void TrainingTab::scheduleLiveUiRefresh()",
    "void TrainingTab::refreshLiveUi()",
    "m_lblCurrentForce->setText(QString(\"%1 N\").arg(force, 0, 'f', 2));",
    "m_currentGame->handleSensorInput(gameInput)",
):
    if required not in training_source:
        raise SystemExit(f"Missing latest-value training invariant: {required}")
if "QTimer* m_liveUiTimer{nullptr};" not in training_header:
    raise SystemExit("Training live refresh timer declaration missing")

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


# Journal queue admission precedes publication to the lossy GUI queue; fsync is batched.
engine_source = Path("src/measurement/MeasurementEngine.cpp").read_text(encoding="utf-8")
journal_writer_source = Path("src/tab/SampleJournalWriter.cpp").read_text(encoding="utf-8")
for required in (
    "m_measurementEngine->setCommitHook",
    "m_measurementEngine->setCommitHook({})",
    "m_journalAdmission.store(true",
    "m_journalAdmission.store(false",
    "m_measurementSessionGeneration.fetch_add(1",
    "sessionGeneration != m_measurementSessionGeneration.load",
    "m_sampleJournalWriter.appendJson(record)",
    "m_journalNextSequence.store(sequence + 1",
    "bool MeasurementTab::restoreSampleJournalIntoSession()",
    "expectedSequence != m_journalNextSequence.load",
    "if (m_isMeasuring)",
    "if (m_sampleJournalFailed)",
    "QFile::remove(m_sampleJournalPath)",
):
    if required not in source:
        raise SystemExit(f"Missing durable acquisition commit invariant: {required}")
# The ingest gate preserves commit/sequence order, while the state mutex is
# released before invoking the potentially blocking persistence hook.
for required in (
    "std::unique_lock<std::mutex> ingestLock(m_ingestMutex)",
    "CommitHook commitHook",
    "commitHook = m_commitHook",
    "committed = commitHook(sample)",
):
    if required not in engine_source:
        raise SystemExit(f"Missing serialized out-of-lock commit invariant: {required}")
lock_end = engine_source.index("\n    }\n\n    if (commitHook)")
hook_start = engine_source.index("if (commitHook)", lock_end)
hook_call = engine_source.index("committed = commitHook(sample)", hook_start)
history_start = engine_source.index("m_history.push_back(sample)", hook_call)
if not (lock_end < hook_start < hook_call < history_start):
    raise SystemExit("Commit hook must run outside the state mutex and before history publication")
if "QJsonDocument(record.jsonObject).toJson(QJsonDocument::Compact)" not in journal_writer_source:
    raise SystemExit("Journal JSON serialization must run on the writer thread")

# The bounded presentation/analysis queue must never block the serial reader.
# The durable journal remains lossless; when the short GUI buffer saturates,
# stale queued frames are discarded and the detector is reset.
for required in (
    "bool m_measurementQueueAccepting{true}",
    "MaximumQueuedSamples = 64",
    "if (m_measurementQueue.size() >= MaximumQueuedSamples)",
    "m_droppedMeasurementSamples += static_cast<std::size_t>(discarded)",
    "m_measurementQueue.clear()",
    "m_totalDroppedMeasurementSamples += discarded",
    "m_measurementQueueAccepting = false",
):
    if required not in source and required not in header:
        raise SystemExit(f"Missing bounded analysis-queue invariant: {required}")
enqueue_start = source.index("void MeasurementTab::enqueueMeasurementSample(")
enqueue_end = source.index("void MeasurementTab::drainMeasurementSamples(", enqueue_start)
enqueue_body = source[enqueue_start:enqueue_end]
if "m_measurementQueueSpaceAvailable.wait" in enqueue_body:
    raise SystemExit("GUI presentation backlog must not block durable acquisition")
if "m_measurementQueue.push_back(sample)" not in enqueue_body:
    raise SystemExit("GUI queue must retain ordered samples while capacity is available")
destructor_start = source.index("MeasurementTab::~MeasurementTab()")
destructor_end = source.index("void MeasurementTab::ensureCurrentSeriesInitialized", destructor_start)
destructor_body = source[destructor_start:destructor_end]
if destructor_body.index("m_measurementEngine->setCommitHook({})") > destructor_body.index("closeSampleJournal()"):
    raise SystemExit("Teardown must disable and synchronize commit hook before closing journal")

# Acquisition runs at 80 SPS; the plot uses a separate 120 Hz interpolated presentation clock.
# Training labels/game input consume each real sample; Qt coalesces paint events.
training_source = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
game_engine_source = Path("src/games/GameEngine.cpp").read_text(encoding="utf-8")
if "m_currentGame->handleSensorInput(gameInput);" not in training_source:
    raise SystemExit("Training must forward every acquired sample to the active game")
if "m_lblCurrentForce->setText(QString(\"%1 N\").arg(force, 0, 'f', 2));" not in training_source:
    raise SystemExit("Training force display must consume the newest acquired sample")
if "stats.completedReps = stats.repetitions.size();" not in training_source:
    raise SystemExit("Training repetition total must derive from completed records")
if "++m_currentRepIndex;" not in training_source or "finishCurrentSeries();" not in training_source:
    raise SystemExit("Training must advance the repetition cursor and finish a series at its configured target")
if "stats.completedSeries = stats.series.size();" not in training_source:
    raise SystemExit("Completed series count must derive from stored series summaries")
if "m_currentSeriesIndex = nextSeriesIndex;" not in training_source or "updateProgressWidgets();" not in training_source:
    raise SystemExit("Training progress must refresh after each series transition")
if "m_repaintTimer.setInterval(8);" not in source:
    raise SystemExit("Measurement force plot repaint must target approximately 120 Hz")
if "constexpr double displayStepSeconds = 1.0 / 120.0;" not in source:
    raise SystemExit("Measurement force plot must interpolate on a 120 Hz display grid")
if "Fritsch-Carlson limiter" not in source or "qBound(qMin(y0, y1), value, qMax(y0, y1))" not in source:
    raise SystemExit("Measurement plot must use bounded shape-preserving interpolation")
if "m_lastSampleArrival.elapsed() > 250" not in source:
    raise SystemExit("120 Hz repaint timer must stop after the sensor stream becomes idle")
if "m_repaintTimer.setTimerType(Qt::PreciseTimer);" not in source:
    raise SystemExit("Measurement force plot must use precise timer scheduling")
if "Qt::PreciseTimer" not in game_engine_source:
    raise SystemExit("Game frame timer must use precise scheduling")
if "QPainterPath tracePath;" not in source or "QPointF point(x, y);" not in source:
    raise SystemExit("Measurement trace must retain subpixel coordinates for smooth rendering")
if "path.lineTo(point);" not in source or "appendAt(time, interpolate(segment, time), tracePath, started)" not in source:
    raise SystemExit("Measurement trace must render the interpolated 120 Hz points")
if "m_forcePlot->addSample(forceValue, currentTime)" not in source:
    raise SystemExit("Measurement plot must receive every acquired sample")
if "m_progressWidget->setCurrentForce(force);" not in training_source:
    raise SystemExit("Training live force trace must receive the latest acquired sample")
training_header = Path("include/tab/TrainingTab.hpp").read_text(encoding="utf-8")
if "std::deque<double> m_forceHistory" not in training_header or "MaximumTraceSamples = 600" not in training_source:
    raise SystemExit("Training live trace must retain a bounded six-second sample window")
if "QPainterPath trace;" not in training_source or "Qt::RoundJoin" not in training_source:
    raise SystemExit("Training live trace must use antialiased rounded vector rendering")
if "trace.cubicTo(control1, control2, p2);" not in training_source:
    raise SystemExit("Training live trace must use shape-preserving cubic interpolation")
if "LiveUiIntervalMs" in training_source:
    raise SystemExit("Training must not drop real sensor samples using a host-clock UI throttle")

print("Measurement, training and game realtime dispatch regression passed")

# Measurement analysis must process every sample in order. Only the
# presentation-facing controls are coalesced to the newest value.
sample_start = source.index("void MeasurementTab::readSingleSample(")
sample_end = source.index("void MeasurementTab::updateRawTableBatch(", sample_start)
sample_body = source[sample_start:sample_end]
if "m_recordedSamples.append(recorded)" not in sample_body:
    raise SystemExit("Full measurement samples must still be retained")
if "m_repetitionDetector.process(deviceTimestampMs, forceValue, true, repetition)" not in sample_body:
    raise SystemExit("Every acquired sample must reach repetition analysis")
if "m_forcePlot->addSample(forceValue, currentTime)" not in sample_body:
    raise SystemExit("The live plot must retain every real sensor sample")
if "updateLiveDisplay(forceValue)" not in sample_body:
    raise SystemExit("The latest sample must update the live numeric display")
if "updateRawTableBatch()" not in sample_body:
    raise SystemExit("The raw table must use its bounded batch update")


# Every-frame status callbacks must not flood the GUI event queue.
debug_source = Path("src/tab/DebugTab.cpp").read_text(encoding="utf-8")
debug_header = Path("include/tab/DebugTab.hpp").read_text(encoding="utf-8")
if "QMetaObject::invokeMethod(this, [this, data]" in debug_source:
    raise SystemExit("DebugTab still queues one GUI event per sensor frame")
if "m_pendingSensorData = data" not in debug_source:
    raise SystemExit("DebugTab latest sensor snapshot missing")
if "m_sensorUiTimer->setInterval(100)" not in debug_source:
    raise SystemExit("DebugTab status refresh must be bounded to 10 Hz")
if "m_sensorSnapshotMutex" not in debug_header:
    raise SystemExit("DebugTab latest sensor snapshot must be synchronized")

# Serial acquisition is continuous (not one DATA request per sample), and the
# Nano firmware configures the HX711 RATE input for its physical 80 SPS mode.
serial_source = Path("src/sensor/SerialCommunication.cpp").read_text(encoding="utf-8")
serial_test = Path("tests/serial_pty_regression.cpp").read_text(encoding="utf-8")
firmware = Path("hx711_advanced.ino").read_text(encoding="utf-8")
async_start = serial_source.index("void SerialCommunication::startAsyncReading()")
async_end = serial_source.index("void SerialCommunication::stopAsyncReading()", async_start)
async_body = serial_source[async_start:async_end]
if '"CONT\\n"' not in async_body or "streamParser.feed(buffer" not in async_body:
    raise SystemExit("Async serial acquisition must enable and parse the persistent continuous stream")
if "readDataBatch(100)" in async_body:
    raise SystemExit("Async acquisition must not poll DATA at 10 Hz")
if '"CONT\\n"' not in serial_test or "callbackCount.load" not in serial_test:
    raise SystemExit("PTY regression must verify multiple continuous frames")
for required in (
    "#define HX711_RATE_PIN A0",
    "digitalWrite(HX711_RATE_PIN, HIGH)",
    "#define HX711_THREAD_INTERVAL 1",
):
    if required not in firmware:
        raise SystemExit(f"HX711 firmware missing 80 SPS configuration: {required}")

detector_header = Path("include/training/RepetitionDetector.hpp").read_text(encoding="utf-8")
detector_test = Path("tests/repetition_detector_regression.cpp").read_text(encoding="utf-8")
if "minimumDurationMs{120}" not in detector_header or "maximumSampleGapMs{100}" not in detector_header:
    raise SystemExit("Repetition detector defaults must be tuned for the 80 SPS stream")
if "fastRepetitions.size() == 3" not in detector_test:
    raise SystemExit("Repetition detector must be regression-tested with three 80 SPS tubing cycles")
