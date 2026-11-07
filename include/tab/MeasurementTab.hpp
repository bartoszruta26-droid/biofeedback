#ifndef TAB_MEASUREMENTTAB_HPP
#define TAB_MEASUREMENTTAB_HPP

#include <QWidget>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QVector>
#include <QString>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QCheckBox>
#include <QUuid>
#include <QHeaderView>
#include <QTextEdit>
#include <memory>
#include <QComboBox>

#include "sensor/SerialCommunication.hpp"
#include "sensor/MeasurementSerialAdapter.hpp"
#include "training/RepetitionDetector.hpp"
#include "core/DebugManager.hpp"
#include "formulas.hpp"

namespace tab {

/**
 * @brief Jednostki wyświetlania siły
 */
enum class ForceUnit {
    Newtons,      // Siła w Newtonach [N]
    Kilograms,    // Siła w kilogramach-siły [kgf] (1 kgf ≈ 9.81 N)
    Raw           // Wartość surowa z czujnika (ADC counts)
};

/**
 * @brief Widget do wizualizacji trendów długoterminowych
 */
class TrendsPlotWidget : public QWidget {
    Q_OBJECT
public:
    explicit TrendsPlotWidget(QWidget *parent = nullptr);
    
    void setData(const QVector<double>& dates, const QVector<double>& avgForces, 
                 const QVector<double>& maxForces);
    void clearData();
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_dates;
    QVector<double> m_avgForces;
    QVector<double> m_maxForces;
    double m_maxForce;
};

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
    void setUnit(ForceUnit unit);
    
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> m_forces;
    QVector<double> m_times;
    double m_targetForce;
    double m_maxTimeWindow;
    double m_maxForce;
    double m_maxObservedForce;
    double m_minObservedForce;
    ForceUnit m_unit;
    QString getUnitLabel() const;
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
    double startTime;            // Czas rozpoczęcia od początku sesji [s]
    double endTime;              // Czas zakończenia od początku sesji [s]
    double timeInTop8pct;       // Czas w górnych 8% siły [s]
    double timeInBottom8pct;    // Czas w dolnych 8% siły [s]
    double areaUnderCurve;      // Pole pod krzywą (impuls) [N·s]
    double avgForce;            // Średnia siła [N]
    double medianForce;         // Mediana siły [N]
    double modeForce;           // Dominanta siły [N]
    bool isValid;
    RepetitionData biomechanicalData;
    BiomechanicalParams biomechanicalParams;
    
    RepetitionStats() : repNumber(0), peakForce(0), timeToPeak(0), 
                        riseRate(0), fallRate(0), duration(0), startTime(0), endTime(0),
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
    double totalWork;                   // Praca mechaniczna [J]; status w biomechanicalParams
    double avgRiseRate;                 // Średnia szybkość narastania [N/s]
    double avgFallRate;                 // Średnia szybkość opadania [N/s]
    double seriesSpeed;                 // Szybkość wykonania serii [rep/s]
    double consistencyScore;            // Wskaźnik powtarzalności [%]
    double impulseTotal;                // Całkowity impuls siły [N·s]
    QString sessionDate;                // Data sesji pomiarowej
    BiomechanicalParams biomechanicalParams; // 65 parametrów z oceną dostępności
    
    SeriesStats() : seriesNumber(0), avgPeakForce(0), medianPeakForce(0),
                    fatigueIndex(0), totalWork(0), avgRiseRate(0),
                    avgFallRate(0), seriesSpeed(0), consistencyScore(0),
                    impulseTotal(0) {}
};

/**
 * @brief Dane sesji pomiarowej do trendów długoterminowych
 */
struct RecordedMeasurementSample {
    quint64 sequence{0};
    qint64 timestampUnixMs{0};
    quint32 deviceTimestampMs{0};
    double elapsedTimeSeconds{0.0};
    double value{0.0};
    qint32 rawValue{0};
    bool hasRawValue{false};
    double calibratedMassKg{0.0};
    bool hasCalibratedMass{false};
    QString unit;
    QString source;
    QString quality;
};

/** Full sample stream retained for lossless session export/import. */
struct MeasurementSession {
    QString sessionId;                  // Unikalny identyfikator sesji
    QString patientId;                  // ID pacjenta
    QString date;                       // Data pomiaru
    QVector<SeriesStats> series;        // Serie z tej sesji
    QVector<RecordedMeasurementSample> samples; // Pełny przebieg próbkowania
    
    MeasurementSession() {}
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
    void readSingleSample(double forceValue, std::uint32_t deviceTimestampMs = 0,
                          std::int32_t rawValue = 0, bool hasRawValue = false,
                          double calibratedMassKg = 0.0, bool hasCalibratedMass = false);
    
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
    
    /**
     * @brief Eksportuje dane sesji do JSON (dla trendów długoterminowych)
     */
    bool exportToJSON(const QString& filename);
    
    /**
     * @brief Importuje dane sesji z JSON (dla trendów długoterminowych)
     */
    bool importFromJSON(const QString& filename);
    
    /**
     * @brief Wczytuje wszystkie pliki JSON pacjenta i wyświetla trendy długoterminowe
     */
    void loadPatientTrends();
    
    /**
     * @brief Wczytuje wszystkie historyczne pomiary pacjenta do porównania 65 parametrów
     */
    void loadAllHistoricalMeasurements();
    
    /**
     * @brief Wyświetla dane aktualnie rejestrowanego pomiaru na wykresie
     */
    void showCurrentMeasurementData();
    
    /**
     * @brief Ustawia ID pacjenta z PatientTab i ładuje jego historyczne pomiary
     * @param pesel PESEL pacjenta
     * @param patientDataPath Ścieżka do danych pacjenta
     */
    void onPatientChanged(const QString& pesel, const QString& patientDataPath);
    
    /**
     * @brief Ustawia połączenie szeregowe z Arduino
     * @param serial Połączenie szeregowe
     */
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    
    /**
     * @brief Sprawdza czy Arduino jest podłączone
     * @return true jeśli Arduino Nano z HX711 jest podłączone
     */
    bool isArduinoConnected() const;
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> measurementEngine() const { return m_measurementEngine; }
    
    /**
     * @brief Dodaje wiadomość do centralnego terminala debugowego (DebugTab)
     * @param message Treść wiadomości
     * @param type Typ wiadomości (INFO, ERROR, WARNING, DATA, SERIAL)
     * 
     * Ta metoda wysyła wiadomość do DebugManager, który przekazuje ją do DebugTab.
     * MeasurementTab nie posiada już własnego terminala debugowego.
     */
    void addDebugMessage(const QString& message, const QString& type = "INFO");

signals:
    void measurementStarted();
    void measurementStopped();
    void newForceSample(double force, double timestamp, bool isRaw = false);
    void repetitionCompleted(int seriesNum, int repNum, const RepetitionStats& stats);
    void seriesCompleted(int seriesNum, const SeriesStats& stats);
    void sessionFinished(const QVector<SeriesStats>& allStats);
    void arduinoConnectionStatus(bool connected, const QString& message);

public slots:
    void onSaveData();
    void onLoadData();
    void onShowTrends();
    void onSaveMeasurementJSON();   // Zapis pojedynczego pomiaru do JSON
    void onLoadMeasurementJSON();   // Otwieranie pojedynczego pomiaru JSON
    void onSensorDataReceived(const sensor::SensorData& data);

private slots:
    void onToggleMeasurement();
    void onReadSingleSample();
    void onSelectionChanged();

private:
    void onMeasurementSample(const biofeedback::measurement::MeasurementSample& sample);
    void ensureCurrentSeriesInitialized();
    void ensureSessionClockStarted();
    void setupUI();
    void updateLiveDisplay(double force);
    void calculateRepetitionStats();
    void updateStatsTable();
    void updateBiomechanicalTable();
    void updateSeriesProgress();
    double calculateMedian(const QVector<double>& values) const;
    double calculateMode(const QVector<double>& values) const;
    double calculateAreaUnderCurve(const QVector<double>& forces, 
                                   const QVector<double>& times) const;
    void setupTrendsUI();
    void updateTrendsView();
    QString serializeSessionToJSON(const MeasurementSession& session) const;
    MeasurementSession deserializeSessionFromJSON(const QString& json, bool includeSamples = true) const;
    
    // Konwersja jednostek
    double convertForce(double newtons, ForceUnit targetUnit) const;
    ForceUnit getCurrentUnit() const { return m_currentUnit; }
    void setCurrentUnit(ForceUnit unit);
    void updateUnitDisplay();
    
    // Komponenty UI - pomocnicze
    void updateStatsTableUnits();
    void updateRawTableUnits();
    
    // Komponenty UI
    QVBoxLayout* m_mainLayout;
    
    // Sekcja kontrolna
    QHBoxLayout* m_controlLayout;
    QPushButton* m_btnStartStop;
    QPushButton* m_btnReadManual;
    QPushButton* m_btnSave;
    QPushButton* m_btnLoad;
    QPushButton* m_btnSaveJSON;       // Zapis pojedynczego pomiaru do JSON
    QPushButton* m_btnLoadJSON;       // Otwieranie pojedynczego pomiaru JSON
    QPushButton* m_btnToggleRaw;      // Przełącznik trybu Raw/Calibrated
    QPushButton* m_btnReset;
    
    // Wybór jednostki siły
    QComboBox* m_unitSelector;
    
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
    QGroupBox* m_biomechBox;
    QTableWidget* m_biomechTable;
    
    // Tabela surowych danych
    QGroupBox* m_rawBox;
    QTableWidget* m_rawTable;
    
    // Logika i dane
    bool m_isMeasuring;
    QString m_patientId;
    
    int m_currentSeries;      // 1-3
    int m_currentRepInSeries; // 1-8
    int m_totalReps;
    
    double m_sampleRate;      // Hz
    double m_targetForce;     // N
    double m_sessionStartTime;
    QElapsedTimer m_sessionElapsedTimer;
    double m_sessionElapsedOffsetSeconds{0.0};
    
    // Bufory danych
    QVector<double> m_rawForceBuffer;
    QVector<double> m_timeBuffer;
    QVector<RecordedMeasurementSample> m_recordedSamples;
    
    // Bufor bieżącego powtórzenia
    QVector<double> m_currentRepForces;
    QVector<double> m_currentRepTimes;
    
    // Aktualna jednostka wyświetlania
    ForceUnit m_currentUnit;
    
    // Tryb wyświetlania: raw value vs calibrated
    bool m_showRawValues;
    
    // Detekcja powtórzeń
    bool m_inContraction;
    double m_repStartForce;
    double m_repPeakForce;
    double m_repStartTime;
    double m_repPeakTime;
    biofeedback::training::RepetitionDetector m_repetitionDetector;
    biofeedback::training::RepetitionRecord m_lastCompletedRepetition;
    
    // Wyniki
    QVector<SeriesStats> m_completedSeriesStats;
    RepetitionStats m_currentRepStats;
    
    // Trendy długoterminowe
    QVector<MeasurementSession> m_patientSessions;
    QGroupBox* m_trendsBox;
    QScrollArea* m_trendsScrollArea;
    QWidget* m_trendsContent;
    QCheckBox* m_chkShowTrends;
    
    // Połączenie szeregowe z Arduino
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> m_measurementEngine;
    std::unique_ptr<sensor::MeasurementSerialAdapter> m_measurementAdapter;
    biofeedback::measurement::MeasurementEngine::CallbackToken m_measurementCallbackToken = 0;
    sensor::SerialCommunication::CallbackToken m_dataCallbackToken = 0;
    sensor::SensorData m_lastSensorData;
};

} // namespace tab

#endif // TAB_MEASUREMENTTAB_HPP
