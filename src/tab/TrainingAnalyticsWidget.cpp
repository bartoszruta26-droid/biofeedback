#include "tab/TrainingAnalyticsWidget.hpp"

#include <QHeaderView>
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

    m_peakChart = new QtCharts::QChartView(m_tabs);
    m_peakChart->setRenderHint(QPainter::Antialiasing);
    m_peakChart->setMinimumHeight(260);
    m_tabs->addTab(m_peakChart, tr("Wykres siły"));
    m_tabs->addTab(m_seriesTable, tr("Analiza serii"));
    m_tabs->addTab(m_repetitionTable, tr("Powtórzenia"));
    layout->addWidget(m_tabs, 1);
}

void TrainingAnalyticsWidget::setStats(const QVector<ExerciseStats>& stats) {
    rebuildSummary(stats);
    rebuildSeriesTable(stats);
    rebuildRepetitionTable(stats);
    rebuildPeakChart(stats);
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
