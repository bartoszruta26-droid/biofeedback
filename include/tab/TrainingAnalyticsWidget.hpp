#pragma once

#include <QWidget>
#include <QVector>

#include "tab/TrainingTab.hpp"

class QLabel;
class QTableWidget;
class QTabWidget;

namespace QtCharts {
class QChartView;
}

namespace tab {

/** Presents repetition-level and series-level training analytics. */
class TrainingAnalyticsWidget final : public QWidget {
    Q_OBJECT
public:
    explicit TrainingAnalyticsWidget(QWidget* parent = nullptr);
    void setStats(const QVector<ExerciseStats>& stats);

private:
    void rebuildSummary(const QVector<ExerciseStats>& stats);
    void rebuildSeriesTable(const QVector<ExerciseStats>& stats);
    void rebuildRepetitionTable(const QVector<ExerciseStats>& stats);
    void rebuildPeakChart(const QVector<ExerciseStats>& stats);

    QLabel* m_summaryLabel{nullptr};
    QTableWidget* m_seriesTable{nullptr};
    QTableWidget* m_repetitionTable{nullptr};
    QtCharts::QChartView* m_peakChart{nullptr};
    QTabWidget* m_tabs{nullptr};
};

} // namespace tab
