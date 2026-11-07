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
    "stopMeasurement();\n            m_lblStatus->setText(tr(\"Status: SESJA ZAKOŃCZONA\"))",
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

print("MeasurementTab lifecycle, monotonic timing and sample archive regression passed")
