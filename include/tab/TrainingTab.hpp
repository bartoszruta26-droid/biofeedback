#ifndef TAB_TRAININGTAB_HPP
#define TAB_TRAININGTAB_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QPainter>
#include <QPaintEvent>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <memory>
#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"

namespace tab {

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
    int m_sessionTime;
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
    void stopTraining();
    
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
    
    /**
     * @brief Podłącza silnik gry
     */
    void setGameEngine(games::GameEngine* game);
    
    /**
     * @brief Pobiera aktualny wynik treningu
     */
    QVector<ExerciseStats> getTrainingStats() const;
    
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
    void onSensorDataReceived(const sensor::SensorData& data);
    void onGameEnded(int score);
    void onGameStarted();
    void onScoreChanged(int score);

private:
    void setupUI();
    void updateDisplay();
    void updateProgressWidgets();
    void loadNextExercise();
    void loadNextSeries();
    void loadNextRep();
    void checkExerciseCompletion();
    void calculateExerciseStats();
    void startRestTimer();
    void updateRestDisplay();
    
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
    
    // Widget gry (embedowany)
    QGroupBox* m_gameBox;
    games::GameEngine* m_currentGame;
    
    // Tabela statystyk
    QGroupBox* m_statsBox;
    QWidget* m_statsContainer;
    
    // Logika i dane
    QTimer* m_timer;
    QTimer* m_restTimer;
    bool m_isTraining;
    bool m_isPaused;
    
    QVector<TrainingExercise> m_exercisePlan;
    QVector<ExerciseStats> m_exerciseStats;
    
    int m_currentExerciseIndex;
    int m_currentSeriesIndex;
    int m_currentRepIndex;
    
    int m_sessionTime;
    int m_restTimeRemaining;
    bool m_inRestPeriod;
    
    double m_currentForce;
    double m_peakForceInRep;
    
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    
    // Pomocnicze
    QString getExerciseDisplayName(int index) const;
    QColor getForceColor(double current, double target) const;
};

} // namespace tab

#endif // TAB_TRAININGTAB_HPP
