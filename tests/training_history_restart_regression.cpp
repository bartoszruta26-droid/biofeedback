#include "tab/TrainingAnalyticsWidget.hpp"

#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>

#include <cassert>
#include <cmath>

using namespace tab;

static ExerciseStats makeStats(int repetitions)
{
    ExerciseStats stats;
    stats.exerciseName = QStringLiteral("Test grip");
    stats.completedReps = repetitions;
    stats.completedSeries = 1;
    stats.avgPeakForce = 42.5;
    stats.maxPeakForce = 48.0;
    stats.avgReactionTime = 0.32;
    stats.consistencyScore = 91.0;

    biofeedback::training::SeriesSummary series;
    series.repetitionCount = static_cast<std::size_t>(repetitions);
    series.averagePeakForceN = 42.5;
    series.maximumPeakForceN = 48.0;
    series.averageDurationMs = 720.0;
    series.averageTimeToPeakMs = 320.0;
    series.targetAchievementPercent = 80.0;
    series.peakForceCoefficientOfVariationPercent = 9.0;
    stats.series.append(series);

    for (int index = 0; index < repetitions; ++index) {
        TrainingRepetitionRecord record;
        record.seriesIndex = 1;
        record.repetitionIndex = index + 1;
        record.measurement.startTimestampMs = 1000U + static_cast<std::uint32_t>(index * 1000);
        record.measurement.endTimestampMs = 1720U + static_cast<std::uint32_t>(index * 1000);
        record.measurement.durationMs = 720;
        record.measurement.timeToPeakMs = 320;
        record.measurement.peakForceN = 42.5 + index;
        record.measurement.meanForceN = 30.0;
        record.measurement.sampleCount = 2;
        record.measurement.samples.push_back({record.measurement.startTimestampMs, 10.0});
        record.measurement.samples.push_back({record.measurement.endTimestampMs, 5.0});
        stats.repetitions.append(record);
    }
    return stats;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QTemporaryDir temporary;
    assert(temporary.isValid());
    const QString historyPath = temporary.filePath(QStringLiteral("training_history.json"));

    QJsonObject checkpoint;
    checkpoint.insert("format_version", 2);
    checkpoint.insert("current_exercise_index", 0);
    checkpoint.insert("current_series_index", 0);
    checkpoint.insert("current_repetition_index", 1);
    checkpoint.insert("session_elapsed_ms", 12500.0);
    checkpoint.insert("active_elapsed_ms", 9000.0);

    {
        TrainingAnalyticsWidget firstProcess;
        firstProcess.setHistoryFilePath(historyPath);
        assert(firstProcess.beginSession());
        firstProcess.setStats({makeStats(1)}, 12, 9);
        firstProcess.setExecutionState(checkpoint);
        assert(firstProcess.saveSessionCheckpoint());
    }

    // A new widget instance models a fresh application process loading the
    // durable history file after an unexpected shutdown.
    {
        TrainingAnalyticsWidget restartedProcess;
        restartedProcess.setHistoryFilePath(historyPath);
        QJsonObject recoveredEntry;
        assert(restartedProcess.recoverPendingSession(recoveredEntry));

        const auto restored = restartedProcess.currentStats();
        assert(restored.size() == 1);
        assert(restored[0].exerciseName == QStringLiteral("Test grip"));
        assert(restored[0].completedReps == 1);
        assert(restored[0].repetitions.size() == 1);
        assert(restored[0].repetitions[0].measurement.samples.size() == 2);
        assert(restored[0].repetitions[0].measurement.samples[1].timestampMs == 1720U);
        assert(std::abs(restored[0].repetitions[0].measurement.peakForceN - 42.5) < 0.001);
        assert(recoveredEntry.value("execution_state").toObject()
                   .value("current_repetition_index").toInt() == 1);

        // Updating the recovered session must replace its checkpoint row,
        // not append a duplicate session.
        checkpoint.insert("current_repetition_index", 2);
        restartedProcess.setStats({makeStats(2)}, 18, 14);
        restartedProcess.setExecutionState(checkpoint);
        assert(restartedProcess.saveSessionCheckpoint());

        QFile file(historyPath);
        assert(file.open(QIODevice::ReadOnly));
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        assert(document.isArray());
        assert(document.array().size() == 1);
        const QJsonObject updatedEntry = document.array().first().toObject();
        assert(updatedEntry.value("session_status").toString() == "in_progress");
        assert(updatedEntry.value("repetition_count").toInt() == 2);
        assert(updatedEntry.value("execution_state").toObject()
                   .value("current_repetition_index").toInt() == 2);
        assert(updatedEntry.value("exercises").toArray().first().toObject()
                   .value("repetitions").toArray().size() == 2);
    }

    return 0;
}
