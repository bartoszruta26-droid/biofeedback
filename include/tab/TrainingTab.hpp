#ifndef TAB_TRAININGTAB_HPP
#define TAB_TRAININGTAB_HPP

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QPainter>
#include <QPaintEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <memory>
#include <deque>
#include <mutex>
#include <vector>
#include "training/RepetitionDetector.hpp"
#include "training/TrainingSessionClock.hpp"
#include "training/SessionStateMachine.hpp"
#include "measurement/MeasurementEngine.hpp"
#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include "core/DebugManager.hpp"

namespace tab {

class TrainingAnalyticsWidget;

/**
 * @brief Widget do wizualizacji postępu treningu
 */
class TrainingProgressWidget : public QWidget {
    Q_OBJECT
public:
    explicit TrainingProgressWidget(QWidget *parent = nullptr);
    
    void setCurrentExercise(int current, int total);
    void setCurrentSeries(int current, int total);
    void setCurrentRep(int current, int total);
    void setTargetForce(double force);
    void setCurrentForce(double force);
    void clearForceHistory();
    void setSessionTime(int seconds);
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_currentExercise;
    int m_totalExercises;
    int m_currentSeries;
    int m_totalSeries;
    int m_currentRep;
    int m_totalReps;
    double m_targetForce;
    double m_currentForce;
    std::deque<double> m_forceHistory;
    int m_sessionTime;
    
    static QColor getForceColorStatic(double current, double target);
};

/**
 * @brief Struktura reprezentująca pojedyncze ćwiczenie w sesji treningowej
 */
struct TrainingExercise {
    QString name;                 // Nazwa ćwiczenia
    int seriesCount;              // Liczba serii
    int repsPerSeries;            // Liczba powtórzeń na serię
    double targetForce;           // Docelowa siła [N]
    double restTime;              // Czas odpoczynku między seriami [s]
    QString gameType;             // Typ gry do użycia (Pong, FlappyBird, etc.)
    bool isCompleted;             // Czy ćwiczenie zakończone
    
    TrainingExercise() : seriesCount(3), repsPerSeries(8), targetForce(50.0),
                         restTime(60.0), isCompleted(false) {}
};

/**
 * @brief Pojedyncze, wykryte powtórzenie przypisane do serii.
 */
struct TrainingRepetitionRecord {
    int seriesIndex{0};
    int repetitionIndex{0};
    biofeedback::training::RepetitionRecord measurement;
};

/**
 * @brief Statystyki pojedynczego ćwiczenia
 */
struct ExerciseStats {
    QString exerciseName;
    int completedSeries;
    int completedReps;
    double avgPeakForce;
    double maxPeakForce;
    double avgReactionTime;
    double consistencyScore;
    bool isCompleted;
    int gameScore{0};
    QVector<TrainingRepetitionRecord> repetitions;
    QVector<biofeedback::training::SeriesSummary> series;
    
    ExerciseStats() : completedSeries(0), completedReps(0), avgPeakForce(0),
                      maxPeakForce(0), avgReactionTime(0), consistencyScore(0),
                      isCompleted(false) {}
};

/**
 * @class TrainingTab
 * @brief Zakładka do prowadzenia sesji treningowych z wykorzystaniem gier
 * 
 * TrainingTab umożliwia:
 * - Konfigurowanie sesji treningowej (wybór ćwiczeń, parametrów)
 * - Prowadzenie treningu z użyciem wybranej gry (Pong, FlappyBird, etc.)
 * - Monitorowanie postępu w czasie rzeczywistym
 * - Integrację z SerialCommunication do odczytu danych z sensora
 * - Zapisywanie wyników treningu
 */
class TrainingTab : public QWidget
{
    Q_OBJECT
    friend class TrainingTabProcessRestartRegression;

public:
    explicit TrainingTab(QWidget *parent = nullptr);
    ~TrainingTab() override;

    /**
     * @brief Dodaje ćwiczenie do planu treningowego
     */
    void addExercise(const TrainingExercise& exercise);
    
    /**
     * @brief Usuwa ćwiczenie z planu
     */
    void removeExercise(int index);
    
    /**
     * @brief Czyści cały plan treningowy
     */
    void clearPlan();
    
    /**
     * @brief Pobiera listę ćwiczeń
     */
    QVector<TrainingExercise> getExercisePlan() const;
    
    /**
     * @brief Ustawia aktualne ćwiczenie
     */
    void setCurrentExerciseIndex(int index);
    
    /**
     * @brief Rozpoczyna sesję treningową
     */
    void startTraining();
    
    /**
     * @brief Zatrzymuje sesję treningową
     */
    void stopTraining(bool completed = false);
    
    /**
     * @brief Pauzuje trening
     */
    void pauseTraining();
    
    /**
     * @brief Wznawia trening z pauzy
     */
    void resumeTraining();
    
    /**
     * @brief Sprawdza czy trening jest aktywny
     */
    bool isTrainingActive() const;
    
    /**
     * @brief Sprawdza czy trening jest spauzowany
     */
    bool isTrainingPaused() const;
    
    /**
     * @brief Podłącza obiekt SerialCommunication
     */
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void setMeasurementEngine(std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine);
    void setSensorCalibrationReady(bool ready);
    
    /**
     * @brief Podłącza silnik gry
     */
    void setGameEngine(games::GameEngine* game);
    void showGameWindow();
    void setCurrentPatientContext(const QString& patientPesel, const QString& patientDataPath);
    
    /**
     * @brief Pobiera aktualny wynik treningu
     */
    QVector<ExerciseStats> getTrainingStats() const;
    QString currentPatientPesel() const { return m_currentPatientPesel; }
    QJsonObject buildSharedTrainingRecord(const QString& patientUuid) const;
    
    /**
     * @brief Eksportuje wyniki treningu do CSV
     */
    bool exportResultsToCSV(const QString& filename);
    
    /**
     * @brief Importuje plan treningowy z pliku
     */
    bool importTrainingPlan(const QString& filename);
    
    /**
     * @brief Eksportuje plan treningowy do pliku
     */
    bool exportTrainingPlan(const QString& filename);

    /**
     * @brief Dodaje wiadomość do centralnego terminala debugowego (DebugTab)
     * @param message Wiadomość do wyświetlenia
     * @param type Typ wiadomości (INFO, ERROR, WARNING, DATA, SERIAL)
     * 
     * Ta metoda wysyła wiadomość do DebugManager, który przekazuje ją do DebugTab.
     * TrainingTab nie posiada już własnego terminala debugowego.
     */
    void addDebugMessage(const QString& message, const QString& type = "INFO");

signals:
    void trainingStarted();
    void trainingStopped();
    void trainingPaused();
    void trainingResumed();
    void exerciseCompleted(int index, const ExerciseStats& stats);
    void seriesCompleted(int exerciseIndex, int seriesIndex);
    void repetitionCompleted(int exerciseIndex, int seriesIndex, int repIndex);
    void newForceSample(double force);
    void trainingFinished(const QVector<ExerciseStats>& allStats);

public slots:
    void onSaveResults();
    void onLoadPlan();
    void onExportPlan();

private slots:
    void onStartStopClicked();
    void onPauseResumeClicked();
    void onExerciseSelected(int index);
    void onTimerTick();
    void updateSessionClocks();

    void onGameEnded(int score);
    void onGameStarted();
    void onScoreChanged(int score);

private:
    void enqueueMeasurementSample(const biofeedback::measurement::MeasurementSample& sample);
    void drainMeasurementSamples();
    void onMeasurementSample(const biofeedback::measurement::MeasurementSample& sample);
    void scheduleLiveUiRefresh();
    void refreshLiveUi();
    void setupUI();
    void updateDisplay();
    void updateProgressWidgets();
    void refreshAnalytics();
    void configureGameForExercise();
    void loadNextExercise();
    void loadNextSeries();
    void loadNextRep();
    void calculateExerciseStats();
    void onRepetitionDetected(const biofeedback::training::RepetitionRecord& repetition);
    void finishCurrentSeries();
    void advanceAfterRest();
    void startRestTimer();
    void updateRestDisplay();
    QJsonObject buildExecutionCheckpoint() const;
    bool restoreExecutionCheckpoint(const QJsonObject& checkpoint);
    bool saveExecutionCheckpoint();
    
    // Komponenty UI
    QVBoxLayout* m_mainLayout;
    
    // Panel konfiguracji
    QGroupBox* m_configBox;
    QComboBox* m_exerciseSelector;
    QPushButton* m_btnAddExercise;
    QPushButton* m_btnRemoveExercise;
    QDoubleSpinBox* m_targetForceSpin;
    QSpinBox* m_seriesSpin;
    QSpinBox* m_repsSpin;
    QSpinBox* m_restTimeSpin;
    QComboBox* m_gameTypeCombo;
    QComboBox* m_sinusWaveformCombo{nullptr};
    QSpinBox* m_sinusDifficultySpin{nullptr};
    QSpinBox* m_reactionForceDifficultySpin{nullptr};
    QSpinBox* m_mathSinusWidthSpin{nullptr};
    QSpinBox* m_mathTangentWidthSpin{nullptr};
    QSpinBox* m_mathTrapezoidWidthSpin{nullptr};
    QCheckBox* m_sinusAutoDifficultyCheck{nullptr};
    
    // Panel sterowania
    QHBoxLayout* m_controlLayout;
    QPushButton* m_btnStartStop;
    QPushButton* m_btnPauseResume;
    QPushButton* m_btnSave;
    QPushButton* m_btnLoad;
    QPushButton* m_btnExport;
    
    // Panel postępu
    QGroupBox* m_progressBox;
    TrainingProgressWidget* m_progressWidget;
    QLabel* m_lblCurrentExercise;
    QLabel* m_lblCurrentSeries;
    QLabel* m_lblCurrentRep;
    QLabel* m_lblTimer;
    QProgressBar* m_forceBar;
    QLabel* m_lblCurrentForce;
    QLabel* m_lblTargetForce;
    QLabel* m_lblRestTimer;
    
    // Game widget is owned by the independent floating fullscreen dialog.
    QGroupBox* m_gameBox;
    games::GameEngine* m_currentGame;
    QDialog* m_gameWindow;
    QPushButton* m_btnOpenGame;
    bool m_sensorCalibrationReady{false};
    QString m_activeGameType;
    QString m_currentPatientPesel;
    QString m_activeSessionId;
    QString m_sessionStartedAtUtc;
    QString m_currentPatientDataPath;
    
    // Tabela statystyk
    QGroupBox* m_statsBox;
    QWidget* m_statsContainer;
    QLabel* m_statsSummaryLabel;
    TrainingAnalyticsWidget* m_analyticsWidget;
    
    // Logika i dane
    QTimer* m_timer;
    QTimer* m_restTimer;
    QElapsedTimer m_sessionClock;
    QElapsedTimer m_restClock;
    biofeedback::training::TrainingSessionClock m_trainingClock;
    biofeedback::training::SessionStateMachine m_stateMachine;
    qint64 m_restDurationMs{0};
    qint64 m_sessionElapsedOffsetMs{0};
    int m_lastCheckpointSeconds{-1};
    
    QVector<TrainingExercise> m_exercisePlan;
    QVector<ExerciseStats> m_exerciseStats;
    
    int m_currentExerciseIndex;
    int m_currentSeriesIndex;
    int m_currentRepIndex;
    
    int m_sessionTime;
    int m_restTimeRemaining;
    biofeedback::training::RepetitionDetector m_repetitionDetector;
    std::vector<biofeedback::training::RepetitionRecord> m_currentSeriesRepetitions;

    double m_currentForce;
    double m_peakForceInRep;
    
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> m_measurementEngine;
    biofeedback::measurement::MeasurementEngine::CallbackToken m_measurementCallbackToken = 0;
    std::mutex m_measurementQueueMutex;
    // Preserve acquired samples for repetition analysis; UI refresh is coalesced separately.
    std::deque<biofeedback::measurement::MeasurementSample> m_measurementQueue;
    bool m_measurementDrainScheduled{false};
    std::size_t m_droppedMeasurementSamples{0};
    QTimer* m_liveUiTimer{nullptr};
    QVector<double> m_uiDispatchLatencyWindowMs;
    qint64 m_lastDispatchLatencyReportMs{0};
    quint64 m_pendingUiSamples{0};
    
    // Pomocnicze
    QString getExerciseDisplayName(int index) const;
    QColor getForceColor(double current, double target) const;
};

} // namespace tab

#endif // TAB_TRAININGTAB_HPP
