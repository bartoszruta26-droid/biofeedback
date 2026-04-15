#ifndef GUI_MAINWINDOW_HPP
#define GUI_MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QTabWidget>
#include <QStackedWidget>

#include "GraphWidget.hpp"

namespace tab {
    class PatientTab;
    class MeasurementTab;
    class OutlineTab;
    class TrainingTab;
}

namespace games {
    class GameEngine;
}

namespace gui {

/**
 * @class MainWindow
 * @brief Główne okno aplikacji do monitorowania i wizualizacji danych z wagi
 * 
 * MainWindow stanowi główny interfejs użytkownika aplikacji, zapewniając:
 * - Wyświetlanie wykresu rzeczywistego czasu z GraphWidget
 * - Panel sterowania z przyciskami i ustawieniami
 * - Pasek menu z opcjami konfiguracji i pomocy
 * - Pasek statusu z informacjami o połączeniu i aktualnych pomiarach
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief Pobiera wskaźnik do widgetu wykresu
     * @return Wskaźnik na GraphWidget
     */
    GraphWidget* graphWidget() const { return m_graphWidget; }

    /**
     * @brief Aktualizuje wyświetlaną wartość wagi
     * @param weight Wartość wagi do wyświetlenia
     */
    void updateWeightDisplay(double weight);

    /**
     * @brief Aktualizuje status połączenia z urządzeniem
     * @param connected true jeśli połączono, false w przeciwnym razie
     * @param deviceName Nazwa urządzenia (opcjonalna)
     */
    void updateConnectionStatus(bool connected, const QString& deviceName = QString());

    /**
     * @brief Sprawdza czy dane są zbierane
     * @return true jeśli zbieranie jest aktywne
     */
    bool isDataCollecting() const { return m_isCollectingData; }

    /**
     * @brief Pobiera zakładkę pacjentów
     */
    tab::PatientTab* patientTab() const { return m_patientTab; }

    /**
     * @brief Pobiera zakładkę pomiarów
     */
    tab::MeasurementTab* measurementTab() const { return m_measurementTab; }

    /**
     * @brief Pobiera zakładkę treningu (Outline)
     */
    tab::OutlineTab* outlineTab() const { return m_outlineTab; }

    /**
     * @brief Pobiera zakładkę TrainingTab
     */
    tab::TrainingTab* trainingTab() const { return m_trainingTab; }

    /**
     * @brief Uruchamia grę w zakładce treningu
     * @param gameId ID gry do uruchomienia
     * @param exerciseData Dane ćwiczenia
     */
    void startGame(const QString& gameId, const tab::ExerciseData& exerciseData);

    /**
     * @brief Zatrzymuje bieżącą grę
     */
    void stopGame();

    /**
     * @brief Przekazuje dane z sensora do aktywnej gry
     * @param value Wartość z sensora
     */
    void forwardSensorToGame(double value);

public slots:
    /**
     * @brief Rozpoczyna zbieranie danych
     */
    void startDataCollection();

    /**
     * @brief Zatrzymuje zbieranie danych
     */
    void stopDataCollection();

    /**
     * @brief Czyści dane z wykresu
     */
    void clearGraph();

    /**
     * @brief Eksportuje dane do pliku CSV
     */
    void exportData();

    /**
     * @brief Otwiera dialog konfiguracji
     */
    void openConfiguration();

    /**
     * @brief Wyświetla informacje o aplikacji
     */
    void showAbout();

signals:
    /**
     * @brief Sygnał emitowany gdy rozpoczęto zbieranie danych
     */
    void dataCollectionStarted();

    /**
     * @brief Sygnał emitowany gdy zatrzymano zbieranie danych
     */
    void dataCollectionStopped();

    /**
     * @brief Sygnał emitowany przy zmianie częstotliwości próbkowania
     */
    void samplingRateChanged(int rate);

    /**
     * @brief Sygnał emitowany przy zmianie zakresu wykresu
     */
    void graphDurationChanged(int seconds);

    /**
     * @brief Sygnał emitowany przy przełączeniu autoskalowania
     */
    void autoScaleToggled(bool enabled);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSamplingRateChanged(int rate);
    void onGraphDurationChanged(int seconds);
    void onAutoScaleToggled(bool enabled);
    void onUpdateTimerTimeout();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupTabs();
    void createConnections();
    void loadSettings();
    void saveSettings();
    
    // Obsługa gier
    void createGameArea();
    void loadGame(const QString& gameId);
    void onGameFinished(int score);
    void onSensorDataForGame(double value);

    // Widgety główne
    GraphWidget* m_graphWidget;
    QWidget* m_centralWidget;
    QTabWidget* m_tabWidget;
    
    // Zakładki
    tab::PatientTab* m_patientTab;
    tab::MeasurementTab* m_measurementTab;
    tab::OutlineTab* m_outlineTab;
    tab::TrainingTab* m_trainingTab;
    
    // Obszar gry
    QStackedWidget* m_gameStack;
    games::GameEngine* m_currentGame;
    
    // Elementy panelu sterowania
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_clearButton;
    QPushButton* m_exportButton;
    QPushButton* m_configButton;
    
    QLabel* m_weightLabel;
    QLabel* m_weightValue;
    QLabel* m_statusLabel;
    
    QSpinBox* m_samplingRateSpin;
    QSpinBox* m_graphDurationSpin;
    QComboBox* m_unitCombo;
    
    // Timer do aktualizacji UI
    QTimer* m_updateTimer;
    
    // Stan aplikacji
    bool m_isCollectingData;
    bool m_isConnected;
    QString m_currentDevice;
    
    // Akcje menu
    QAction* m_actionExit;
    QAction* m_actionConfig;
    QAction* m_actionExport;
    QAction* m_actionClear;
    QAction* m_actionAbout;
};

} // namespace gui

#endif // GUI_MAINWINDOW_HPP
