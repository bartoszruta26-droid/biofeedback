#include "tab/TrainingAnalyticsWidget.hpp"
#include "core/RuntimePaths.hpp"

#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <algorithm>

namespace tab {

TrainingAnalyticsWidget::TrainingAnalyticsWidget(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    m_summaryLabel = new QLabel(tr("Brak zarejestrowanych powtórzeń."), this);
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);
    m_saveHistoryButton = new QPushButton(tr("Zapisz sesję w historii"), this);
    m_saveHistoryButton->setEnabled(false);
    layout->addWidget(m_saveHistoryButton, 0, Qt::AlignLeft);
    connect(m_saveHistoryButton, &QPushButton::clicked,
            this, &TrainingAnalyticsWidget::saveCurrentSession);

    m_tabs = new QTabWidget(this);
    m_seriesTable = new QTableWidget(m_tabs);
    m_seriesTable->setColumnCount(8);
    m_seriesTable->setHorizontalHeaderLabels({
        tr("Ćwiczenie"), tr("Seria"), tr("Powtórzenia"),
        tr("Śr. szczyt [N]"), tr("Maks. szczyt [N]"),
        tr("Śr. czas [ms]"), tr("Cel [%]"), tr("CV [%]")
    });
    m_repetitionTable = new QTableWidget(m_tabs);
    m_repetitionTable->setColumnCount(8);
    m_repetitionTable->setHorizontalHeaderLabels({
        tr("Ćwiczenie"), tr("Seria"), tr("Powt."), tr("Szczyt [N]"),
        tr("Średnia [N]"), tr("Czas [ms]"), tr("Do szczytu [ms]"),
        tr("Próbki")
    });
    for (auto* table : {m_seriesTable, m_repetitionTable}) {
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        table->horizontalHeader()->setStretchLastSection(true);
        table->verticalHeader()->setVisible(false);
    }

    m_historyTable = new QTableWidget(m_tabs);
    m_historyTable->setColumnCount(8);
    m_historyTable->setHorizontalHeaderLabels({
        tr("Zapisano"), tr("Czas [s]"), tr("Ćwiczenia"), tr("Serie"),
        tr("Powtórzenia"), tr("Śr. szczyt [N]"), tr("Maks. [N]"), tr("Punkty gier")
    });
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setAlternatingRowColors(true);
    m_historyTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->verticalHeader()->setVisible(false);

    m_peakChart = new QtCharts::QChartView(m_tabs);
    m_peakChart->setRenderHint(QPainter::Antialiasing);
    m_peakChart->setMinimumHeight(260);
    m_tabs->addTab(m_peakChart, tr("Wykres siły"));
    m_tabs->addTab(m_seriesTable, tr("Analiza serii"));
    m_tabs->addTab(m_repetitionTable, tr("Powtórzenia"));
    m_tabs->addTab(m_historyTable, tr("Historia sesji"));
    layout->addWidget(m_tabs, 1);
    m_historyFilePath = QDir(biofeedback::runtime::appDataDirectory())
        .filePath(QStringLiteral("training_history.json"));
    loadHistory();
}

void TrainingAnalyticsWidget::setStats(const QVector<ExerciseStats>& stats, int durationSeconds) {
    m_currentStats = stats;
    m_durationSeconds = durationSeconds;
    bool hasRepetitions = false;
    for (const auto& exercise : stats)
        hasRepetitions = hasRepetitions || !exercise.repetitions.isEmpty();
    m_saveHistoryButton->setEnabled(hasRepetitions && m_historyReadable && !m_sessionSaved);
    rebuildSummary(stats);
    rebuildSeriesTable(stats);
    rebuildRepetitionTable(stats);
    rebuildPeakChart(stats);
}

void TrainingAnalyticsWidget::setHistoryFilePath(const QString& filePath) {
    const QString requestedPath = filePath.trimmed().isEmpty()
        ? QDir(biofeedback::runtime::appDataDirectory()).filePath(QStringLiteral("training_history.json"))
        : filePath;
    if (!m_activeSessionHistoryFilePath.isEmpty() && !m_sessionSaved) {
        // Do not mix a newly selected patient's history into the previous
        // patient's unsaved session file.
        m_pendingHistoryFilePath = requestedPath;
        return;
    }

    m_historyFilePath = requestedPath;
    m_historyReadable = true;
    m_currentStats.clear();
    m_durationSeconds = 0;
    m_sessionSaved = false;
    setStats({}, 0);
    loadHistory();
}

bool TrainingAnalyticsWidget::beginSession() {
    if (!m_activeSessionHistoryFilePath.isEmpty() && !m_sessionSaved) {
        const bool hasRepetitions = std::any_of(
            m_currentStats.cbegin(), m_currentStats.cend(),
            [](const ExerciseStats& exercise) { return !exercise.repetitions.isEmpty(); });
        if (hasRepetitions) return false;
    }
    m_activeSessionHistoryFilePath = m_historyFilePath;
    m_sessionSaved = false;
    m_saveHistoryButton->setEnabled(false);
    return true;
}

void TrainingAnalyticsWidget::loadHistory() {
    const QString path = m_historyFilePath.isEmpty()
        ? QDir(biofeedback::runtime::appDataDirectory()).filePath(QStringLiteral("training_history.json"))
        : m_historyFilePath;
    QFile file(path);
    if (!file.exists()) {
        m_historyEntries = QJsonArray();
        rebuildHistoryTable();
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        m_historyReadable = false;
        m_saveHistoryButton->setEnabled(false);
        m_summaryLabel->setText(tr("Nie można odczytać lokalnej historii treningów."));
        return;
    }
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        m_historyReadable = false;
        m_saveHistoryButton->setEnabled(false);
        m_summaryLabel->setText(tr("Historia treningów ma nieprawidłowy format. Plik zachowano bez zmian."));
        return;
    }
    m_historyEntries = document.array();
    rebuildHistoryTable();
}

void TrainingAnalyticsWidget::rebuildHistoryTable() {
    m_historyTable->setRowCount(0);
    for (const auto& value : m_historyEntries) {
        if (!value.isObject()) continue;
        const QJsonObject entry = value.toObject();
        const QStringList values = {
            entry.value("saved_at").toString(),
            QString::number(entry.value("duration_seconds").toInt()),
            QString::number(entry.value("exercise_count").toInt()),
            QString::number(entry.value("series_count").toInt()),
            QString::number(entry.value("repetition_count").toInt()),
            QString::number(entry.value("average_peak_force_n").toDouble(), 'f', 2),
            QString::number(entry.value("maximum_peak_force_n").toDouble(), 'f', 2),
            QString::number(entry.value("game_points").toInt())
        };
        const int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);
        for (int column = 0; column < values.size(); ++column)
            m_historyTable->setItem(row, column, new QTableWidgetItem(values[column]));
    }
}

void TrainingAnalyticsWidget::saveCurrentSession() {
    int repetitions = 0, seriesCount = 0, gamePoints = 0, exerciseCount = 0;
    double weightedPeak = 0.0, maximumPeak = 0.0;
    for (const auto& exercise : m_currentStats) {
        if (!exercise.repetitions.isEmpty()) ++exerciseCount;
        repetitions += exercise.completedReps;
        seriesCount += exercise.completedSeries;
        gamePoints += exercise.gameScore;
        weightedPeak += exercise.avgPeakForce * exercise.completedReps;
        maximumPeak = std::max(maximumPeak, exercise.maxPeakForce);
    }
    if (repetitions == 0) {
        QMessageBox::information(this, tr("Historia treningów"),
                                 tr("Sesja nie zawiera zarejestrowanych powtórzeń."));
        return;
    }

    QJsonObject entry;
    entry.insert("schema_version", 2);
    entry.insert("saved_at", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    entry.insert("duration_seconds", m_durationSeconds);
    entry.insert("exercise_count", exerciseCount);
    entry.insert("series_count", seriesCount);
    entry.insert("repetition_count", repetitions);
    entry.insert("average_peak_force_n", weightedPeak / repetitions);
    entry.insert("maximum_peak_force_n", maximumPeak);
    entry.insert("game_points", gamePoints);

    // Persist the complete measured session, not only its summary row.
    QJsonArray exerciseEntries;
    for (const auto& exercise : m_currentStats) {
        QJsonObject exerciseEntry;
        exerciseEntry.insert("exercise_name", exercise.exerciseName);
        exerciseEntry.insert("completed_series", exercise.completedSeries);
        exerciseEntry.insert("completed_repetitions", exercise.completedReps);
        exerciseEntry.insert("average_peak_force_n", exercise.avgPeakForce);
        exerciseEntry.insert("maximum_peak_force_n", exercise.maxPeakForce);
        exerciseEntry.insert("average_time_to_peak_seconds", exercise.avgReactionTime);
        exerciseEntry.insert("consistency_score_percent", exercise.consistencyScore);
        exerciseEntry.insert("game_points", exercise.gameScore);

        QJsonArray seriesEntries;
        for (int index = 0; index < exercise.series.size(); ++index) {
            const auto& series = exercise.series[index];
            QJsonObject seriesEntry;
            seriesEntry.insert("series_index", index + 1);
            seriesEntry.insert("repetition_count", static_cast<qint64>(series.repetitionCount));
            seriesEntry.insert("average_peak_force_n", series.averagePeakForceN);
            seriesEntry.insert("maximum_peak_force_n", series.maximumPeakForceN);
            seriesEntry.insert("average_duration_ms", series.averageDurationMs);
            seriesEntry.insert("average_time_to_peak_ms", series.averageTimeToPeakMs);
            seriesEntry.insert("target_achievement_percent", series.targetAchievementPercent);
            seriesEntry.insert("peak_force_cv_percent", series.peakForceCoefficientOfVariationPercent);
            seriesEntries.append(seriesEntry);
        }
        exerciseEntry.insert("series", seriesEntries);

        QJsonArray repetitionEntries;
        for (const auto& record : exercise.repetitions) {
            const auto& measurement = record.measurement;
            QJsonObject repetitionEntry;
            repetitionEntry.insert("series_index", record.seriesIndex);
            repetitionEntry.insert("repetition_index", record.repetitionIndex);
            repetitionEntry.insert("start_timestamp_ms", static_cast<qint64>(measurement.startTimestampMs));
            repetitionEntry.insert("end_timestamp_ms", static_cast<qint64>(measurement.endTimestampMs));
            repetitionEntry.insert("duration_ms", static_cast<qint64>(measurement.durationMs));
            repetitionEntry.insert("time_to_peak_ms", static_cast<qint64>(measurement.timeToPeakMs));
            repetitionEntry.insert("peak_force_n", measurement.peakForceN);
            repetitionEntry.insert("mean_force_n", measurement.meanForceN);
            repetitionEntry.insert("sample_count", static_cast<qint64>(measurement.sampleCount));
            repetitionEntries.append(repetitionEntry);
        }
        exerciseEntry.insert("repetitions", repetitionEntries);
        exerciseEntries.append(exerciseEntry);
    }
    entry.insert("exercises", exerciseEntries);

    QJsonArray updated = m_historyEntries;
    updated.append(entry);

    const QString path = !m_activeSessionHistoryFilePath.isEmpty()
        ? m_activeSessionHistoryFilePath
        : (m_historyFilePath.isEmpty()
            ? QDir(biofeedback::runtime::appDataDirectory()).filePath(QStringLiteral("training_history.json"))
            : m_historyFilePath);
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    const QByteArray payload = QJsonDocument(updated).toJson(QJsonDocument::Indented);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Historia treningów"),
                             tr("Nie udało się otworzyć pliku historii: %1").arg(file.errorString()));
        return;
    }
    if (file.write(payload) != payload.size() || !file.commit()) {
        QMessageBox::warning(this, tr("Historia treningów"),
                             tr("Nie udało się zapisać historii: %1").arg(file.errorString()));
        return;
    }
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    m_historyEntries = updated;
    m_sessionSaved = true;
    m_activeSessionHistoryFilePath.clear();
    if (!m_pendingHistoryFilePath.isEmpty()) {
        m_historyFilePath = m_pendingHistoryFilePath;
        m_pendingHistoryFilePath.clear();
        loadHistory();
    } else {
        rebuildHistoryTable();
    }
    m_saveHistoryButton->setEnabled(false);
    QMessageBox::information(this, tr("Historia treningów"),
                             tr("Pełny zapis sesji (ćwiczenia, serie i powtórzenia) zapisano lokalnie. Dane nie zawierają identyfikatora pacjenta."));
}

void TrainingAnalyticsWidget::rebuildSummary(const QVector<ExerciseStats>& stats) {
    int exerciseCount = 0;
    int seriesCount = 0;
    int repetitionCount = 0;
    double peakSum = 0.0;
    double peakMaximum = 0.0;
    int gamePoints = 0;
    for (const auto& exercise : stats) {
        gamePoints += exercise.gameScore;
        if (exercise.completedReps > 0) ++exerciseCount;
        seriesCount += exercise.completedSeries;
        repetitionCount += exercise.completedReps;
        peakSum += exercise.avgPeakForce * exercise.completedReps;
        peakMaximum = std::max(peakMaximum, exercise.maxPeakForce);
    }
    const double average = repetitionCount > 0 ? peakSum / repetitionCount : 0.0;
    m_summaryLabel->setText(
        tr("Ćwiczenia z danymi: %1 | Ukończone serie: %2 | Powtórzenia: %3"
           " | Średni szczyt: %4 N | Maksymalny szczyt: %5 N | Punkty gier: %6")
            .arg(exerciseCount).arg(seriesCount).arg(repetitionCount)
            .arg(average, 0, 'f', 2).arg(peakMaximum, 0, 'f', 2).arg(gamePoints));
}

void TrainingAnalyticsWidget::rebuildSeriesTable(const QVector<ExerciseStats>& stats) {
    m_seriesTable->setRowCount(0);
    for (const auto& exercise : stats) {
        for (int index = 0; index < exercise.series.size(); ++index) {
            const auto& series = exercise.series[index];
            const int row = m_seriesTable->rowCount();
            m_seriesTable->insertRow(row);
            const QStringList values = {
                exercise.exerciseName, QString::number(index + 1),
                QString::number(series.repetitionCount),
                QString::number(series.averagePeakForceN, 'f', 2),
                QString::number(series.maximumPeakForceN, 'f', 2),
                QString::number(series.averageDurationMs, 'f', 0),
                QString::number(series.targetAchievementPercent, 'f', 1),
                QString::number(series.peakForceCoefficientOfVariationPercent, 'f', 1)
            };
            for (int column = 0; column < values.size(); ++column)
                m_seriesTable->setItem(row, column, new QTableWidgetItem(values[column]));
        }
    }
}

void TrainingAnalyticsWidget::rebuildRepetitionTable(const QVector<ExerciseStats>& stats) {
    m_repetitionTable->setRowCount(0);
    for (const auto& exercise : stats) {
        for (const auto& item : exercise.repetitions) {
            const auto& rep = item.measurement;
            const int row = m_repetitionTable->rowCount();
            m_repetitionTable->insertRow(row);
            const QStringList values = {
                exercise.exerciseName, QString::number(item.seriesIndex),
                QString::number(item.repetitionIndex),
                QString::number(rep.peakForceN, 'f', 2),
                QString::number(rep.meanForceN, 'f', 2),
                QString::number(rep.durationMs),
                QString::number(rep.timeToPeakMs),
                QString::number(rep.sampleCount)
            };
            for (int column = 0; column < values.size(); ++column)
                m_repetitionTable->setItem(row, column, new QTableWidgetItem(values[column]));
        }
    }
}

void TrainingAnalyticsWidget::rebuildPeakChart(const QVector<ExerciseStats>& stats) {
    auto* chart = new QtCharts::QChart();
    chart->setTitle(tr("Siła szczytowa w kolejnych powtórzeniach"));
    chart->legend()->setVisible(stats.size() > 1);
    int maximumRepetitions = 0;
    double maximumForce = 0.0;
    for (const auto& exercise : stats) {
        if (exercise.repetitions.isEmpty()) continue;
        auto* series = new QtCharts::QLineSeries(chart);
        series->setName(exercise.exerciseName);
        for (int i = 0; i < exercise.repetitions.size(); ++i) {
            const double force = exercise.repetitions[i].measurement.peakForceN;
            series->append(i + 1, force);
            maximumForce = std::max(maximumForce, force);
        }
        maximumRepetitions = std::max(maximumRepetitions, exercise.repetitions.size());
        chart->addSeries(series);
    }
    auto* axisX = new QtCharts::QValueAxis(chart);
    axisX->setTitleText(tr("Numer powtórzenia"));
    axisX->setRange(0, std::max(1, maximumRepetitions));
    auto* axisY = new QtCharts::QValueAxis(chart);
    axisY->setTitleText(tr("Siła [N]"));
    axisY->setRange(0.0, maximumForce > 0.0 ? maximumForce * 1.1 : 1.0);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    for (auto* series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }
    m_peakChart->setChart(chart);
}

} // namespace tab
