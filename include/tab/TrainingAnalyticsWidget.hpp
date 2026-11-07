#pragma once

#include <QWidget>
#include <QVector>
#include <QJsonArray>
#include <QUuid>

#include "tab/TrainingTab.hpp"

class QLabel;
class QPushButton;
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
    void setStats(const QVector<ExerciseStats>& stats, int durationSeconds = 0,
                  int activeDurationSeconds = 0);
    void setSessionCompleted(bool completed);
    void setHistoryFilePath(const QString& filePath);
    bool beginSession();
    bool saveSessionAutomatically();
    bool saveSessionCheckpoint();

private slots:
    void saveCurrentSession();
    bool persistCurrentSession(bool showFeedback, bool finalize = true);
    void onHistoryEntryActivated(int row, int column);

private:
    void loadHistory();
    void rebuildHistoryTable();
    void rebuildSummary(const QVector<ExerciseStats>& stats);
    void rebuildSeriesTable(const QVector<ExerciseStats>& stats);
    void rebuildRepetitionTable(const QVector<ExerciseStats>& stats);
    void rebuildPeakChart(const QVector<ExerciseStats>& stats);
    void rebuildForceTraceChart(const QVector<ExerciseStats>& stats);

    QLabel* m_summaryLabel{nullptr};
    QPushButton* m_saveHistoryButton{nullptr};
    QTableWidget* m_historyTable{nullptr};
    QVector<ExerciseStats> m_currentStats;
    int m_durationSeconds{0};
    int m_activeDurationSeconds{0};
    bool m_sessionCompleted{false};
    QJsonArray m_historyEntries;
    bool m_historyReadable{true};
    QString m_historyFilePath;
    QString m_activeSessionHistoryFilePath;
    QString m_activeSessionId;
    QString m_pendingHistoryFilePath;
    bool m_sessionSaved{false};
    bool m_sessionInProgress{false};
    QTableWidget* m_seriesTable{nullptr};
    QTableWidget* m_repetitionTable{nullptr};
    QtCharts::QChartView* m_peakChart{nullptr};
    QtCharts::QChartView* m_forceTraceChart{nullptr};
    QTabWidget* m_tabs{nullptr};
};

} // namespace tab
