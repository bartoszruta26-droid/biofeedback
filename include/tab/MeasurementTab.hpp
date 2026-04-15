#ifndef TAB_MEASUREMENTTAB_HPP
#define TAB_MEASUREMENTTAB_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QPainter>
#include <QPaintEvent>

namespace tab {

/**
 * @brief Widget do wizualizacji wykresu siły w czasie rzeczywistym
 */
class ForcePlotWidget : public QWidget {
    Q_OBJECT
public:
    explicit ForcePlotWidget(QWidget *parent = nullptr);
    
    void addSample(double force, double time);
    void clearData();
    void setTargetForce(double force);
    void setMaxTimeWindow(double seconds);
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_forces;
    QVector<double> m_times;
    double m_targetForce;
    double m_maxTimeWindow;
    double m_maxForce;
};

/**
 * @brief Statystyki pojedynczego powtórzenia (naciągnięcia)
 */
struct RepetitionStats {
    int repNumber;              // Numer powtórzenia
    double peakForce;           // Maksymalny skurcz [N]
    double timeToPeak;          // Czas do osiągnięcia szczytu [s]
    double riseRate;            // Szybkość narastania siły [N/s]
    double fallRate;            // Szybkość opadania siły [N/s]
    double duration;            // Czas trwania powtórzenia [s]
    double timeInTop8pct;       // Czas w górnych 8% siły [s]
    double timeInBottom8pct;    // Czas w dolnych 8% siły [s]
    double areaUnderCurve;      // Pole pod krzywą (impuls) [N·s]
    double avgForce;            // Średnia siła [N]
    double medianForce;         // Mediana siły [N]
    double modeForce;           // Dominanta siły [N]
    bool isValid;
    
    RepetitionStats() : repNumber(0), peakForce(0), timeToPeak(0), 
                        riseRate(0), fallRate(0), duration(0),
                        timeInTop8pct(0), timeInBottom8pct(0),
                        areaUnderCurve(0), avgForce(0), medianForce(0),
                        modeForce(0), isValid(false) {}
};

/**
 * @brief Statystyki całej serii (8 powtórzeń)
 */
struct SeriesStats {
    int seriesNumber;                   // Numer serii (1-3)
    QVector<RepetitionStats> reps;      // Lista powtórzeń
    double avgPeakForce;                // Średnia siła szczytowa serii
    double medianPeakForce;             // Mediana sił szczytowych
    double fatigueIndex;                // Wskaźnik zmęczenia [% spadku]
    double totalWork;                   // Całkowita praca (suma pól pod krzywą) [N·s]
    double avgRiseRate;                 // Średnia szybkość narastania [N/s]
    double avgFallRate;                 // Średnia szybkość opadania [N/s]
    double seriesSpeed;                 // Szybkość wykonania serii [rep/s]
    double consistencyScore;            // Wskaźnik powtarzalności [%]
    double impulseTotal;                // Całkowity impuls [N·s]
    
    SeriesStats() : seriesNumber(0), avgPeakForce(0), medianPeakForce(0),
                    fatigueIndex(0), totalWork(0), avgRiseRate(0),
                    avgFallRate(0), seriesSpeed(0), consistencyScore(0),
                    impulseTotal(0) {}
};

/**
 * @class MeasurementTab
 * @brief Widget zakładki do rejestracji i analizy ćwiczeń z lateksową taśmą
 * 
 * MeasurementTab umożliwia:
 * - Rejestrację siły naciągu z czujnika tensometrycznego
 * - Wykonywanie 3 serii po 8 powtórzeń
 * - Obliczanie zaawansowanych parametrów biomechanicznych
 * - Wizualizację wykresu siły w czasie rzeczywistym
 * - Eksport/Import danych sesji
 */
class MeasurementTab : public QWidget
{
    Q_OBJECT

public:
    explicit MeasurementTab(QWidget *parent = nullptr);
    ~MeasurementTab() override;

    /**
     * @brief Ustawia ID pacjenta dla bieżącej sesji
     */
    void setPatientId(const QString& id);
    
    /**
     * @brief Ustawia siłę docelową (np. 80% MVC)
     */
    void setTargetForce(double force);
    
    /**
     * @brief Sprawdza czy pomiar jest aktywny
     */
    bool isMeasurementActive() const;
    
    /**
     * @brief Rozpoczyna sesję pomiarową
     */
    void startMeasurement();
    
    /**
     * @brief Zatrzymuje sesję pomiarową
     */
    void stopMeasurement();
    
    /**
     * @brief Resetuje sesję (czyści dane)
     */
    void resetSession();
    
    /**
     * @brief Symuluje dane z czujnika (do testów)
     */
    void simulateSensorData();
    
    /**
     * @brief Ręczne wczytanie pojedynczej próbki siły
     */
    void readSingleSample(double forceValue);
    
    /**
     * @brief Analizuje serię i zwraca statystyki
     */
    SeriesStats analyzeSeries(int seriesIndex) const;
    
    /**
     * @brief Zwraca wszystkie obliczone statystyki
     */
    QVector<SeriesStats> getAllSeriesStats() const;
    
    /**
     * @brief Eksportuje dane sesji do CSV
     */
    bool exportToCSV(const QString& filename);
    
    /**
     * @brief Importuje dane sesji z CSV
     */
    bool importFromCSV(const QString& filename);

signals:
    void measurementStarted();
    void measurementStopped();
    void newForceSample(double force, double timestamp);
    void repetitionCompleted(int seriesNum, int repNum, const RepetitionStats& stats);
    void seriesCompleted(int seriesNum, const SeriesStats& stats);
    void sessionFinished(const QVector<SeriesStats>& allStats);

public slots:
    void onSaveData();
    void onLoadData();

private slots:
    void onToggleMeasurement();
    void onReadSingleSample();
    void onTimerTick();
    void onSelectionChanged();

private:
    void setupUI();
    void updateLiveDisplay(double force);
    void detectRepetitions();
    void calculateRepetitionStats();
    void updateStatsTable();
    void updateSeriesProgress();
    double calculateMedian(const QVector<double>& values) const;
    double calculateMode(const QVector<double>& values) const;
    double calculateAreaUnderCurve(const QVector<double>& forces, 
                                   const QVector<double>& times) const;
    
    // Komponenty UI
    QVBoxLayout* m_mainLayout;
    
    // Sekcja kontrolna
    QHBoxLayout* m_controlLayout;
    QPushButton* m_btnStartStop;
    QPushButton* m_btnReadManual;
    QPushButton* m_btnSave;
    QPushButton* m_btnLoad;
    QPushButton* m_btnReset;
    
    // Panel żywy z wykresem
    QGroupBox* m_liveBox;
    ForcePlotWidget* m_forcePlot;
    QLabel* m_lblCurrentForce;
    QLabel* m_lblStatus;
    QProgressBar* m_forceBar;
    QLabel* m_lblSeriesInfo;
    QProgressBar* m_seriesProgress;
    
    // Tabela statystyk
    QGroupBox* m_statsBox;
    QTableWidget* m_statsTable;
    
    // Tabela surowych danych
    QGroupBox* m_rawBox;
    QTableWidget* m_rawTable;
    
    // Logika i dane
    QTimer* m_timer;
    bool m_isMeasuring;
    QString m_patientId;
    
    int m_currentSeries;      // 1-3
    int m_currentRepInSeries; // 1-8
    int m_totalReps;
    
    double m_sampleRate;      // Hz
    double m_targetForce;     // N
    double m_sessionStartTime;
    
    // Bufory danych
    QVector<double> m_rawForceBuffer;
    QVector<double> m_timeBuffer;
    
    // Bufor bieżącego powtórzenia
    QVector<double> m_currentRepForces;
    QVector<double> m_currentRepTimes;
    
    // Detekcja powtórzeń
    bool m_inContraction;
    double m_repStartForce;
    double m_repPeakForce;
    double m_repStartTime;
    double m_repPeakTime;
    double m_contractionThreshold;
    
    // Wyniki
    QVector<SeriesStats> m_completedSeriesStats;
    RepetitionStats m_currentRepStats;
};

} // namespace tab

#endif // TAB_MEASUREMENTTAB_HPP
