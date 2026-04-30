#include "gui/MainWindow.hpp"
#include "gui/GraphWidget.hpp"
#include "tab/PatientTab.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/OutlineTab.hpp"
#include "tab/TrainingTab.hpp"
#include "games/GameEngine.hpp"
#include "games/SinGame.hpp"
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QTabWidget>
#include <QStackedWidget>
#include <QMessageBox>

namespace gui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphWidget(nullptr)
    , m_centralWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_patientTab(nullptr)
    , m_measurementTab(nullptr)
    , m_outlineTab(nullptr)
    , m_trainingTab(nullptr)
    , m_gameStack(nullptr)
    , m_currentGame(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_clearButton(nullptr)
    , m_exportButton(nullptr)
    , m_configButton(nullptr)
    , m_weightLabel(nullptr)
    , m_weightValue(nullptr)
    , m_statusLabel(nullptr)
    , m_samplingRateSpin(nullptr)
    , m_graphDurationSpin(nullptr)
    , m_unitCombo(nullptr)
    , m_updateTimer(nullptr)
    , m_isCollectingData(false)
    , m_isConnected(false)
    , m_settingsWidget(nullptr)
{
    setWindowTitle(tr("Biofeedback - Monitor Wagi"));
    setMinimumSize(1280, 800);
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    createConnections();
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUI()
{
    setupCentralWidget();
}

void MainWindow::setupCentralWidget()
{
    m_centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create GraphWidget first - it must exist before loadSettings() is called
    m_graphWidget = new GraphWidget(this);
    m_graphWidget->setTitle(tr("Wykres Siły w Czasie Rzeczywistym"));
    m_graphWidget->setYLabel(tr("Siła [N]"));
    m_graphWidget->setUnit(tr("N"));
    m_graphWidget->setMinimumHeight(300);
    
    // Setup tabs
    setupTabs();
    
    // Add graph widget to main layout
    mainLayout->addWidget(m_graphWidget);
    
    // Add tab widget to main layout
    mainLayout->addWidget(m_tabWidget);
    
    // Setup control panel with buttons and settings
    setupControlPanel();
    
    // Setup game area (hidden by default)
    createGameArea();
    
    setCentralWidget(m_centralWidget);
    
    // Auto-start data collection after UI is ready
    QTimer::singleShot(500, this, &MainWindow::startDataCollection);
}

void MainWindow::setupTabs()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    
    // Create tabs
    m_patientTab = new tab::PatientTab(this);
    m_measurementTab = new tab::MeasurementTab(this);
    m_outlineTab = new tab::OutlineTab(this);
    m_trainingTab = new tab::TrainingTab(this);
    
    // Add tabs to widget
    m_tabWidget->addTab(m_patientTab, tr("📋 Pacjenci"));
    m_tabWidget->addTab(m_measurementTab, tr("📊 Pomiary"));
    m_tabWidget->addTab(m_outlineTab, tr("🎮 Trening (Outline)"));
    m_tabWidget->addTab(m_trainingTab, tr("🏋️ Trening (Gry)"));
    
    // Set default tab
    m_tabWidget->setCurrentIndex(0);
}

void MainWindow::setupControlPanel()
{
    // Create minimal control panel without "Akcje" and "Ustawienia" groups
    // These are now accessible through menu bars (Widok and Narzędzia)
    
    // Status bar at bottom of central widget
    m_statusLabel = new QLabel(tr("Status: Rozłączono"), this);
    m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    m_statusLabel->setContentsMargins(10, 5, 10, 5);
    
    // Add status label to main layout
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(m_centralWidget->layout());
    if (mainLayout) {
        mainLayout->addWidget(m_statusLabel);
    }
}

void MainWindow::showSettingsDialog()
{
    // Create settings widget if not exists
    if (!m_settingsWidget) {
        m_settingsWidget = new QWidget(this);
        m_settingsWidget->setWindowTitle(tr("Ustawienia"));
        m_settingsWidget->setMinimumSize(400, 300);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(m_settingsWidget);
        
        // Settings group
        QGroupBox* settingsGroup = new QGroupBox(tr("Ustawienia"), this);
        QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);
        
        // Sampling rate
        QHBoxLayout* samplingLayout = new QHBoxLayout();
        samplingLayout->addWidget(new QLabel(tr("Częstotliwość:"), this));
        m_samplingRateSpin = new QSpinBox(this);
        m_samplingRateSpin->setRange(10, 1000);
        m_samplingRateSpin->setValue(100);
        m_samplingRateSpin->setSuffix(tr(" Hz"));
        samplingLayout->addWidget(m_samplingRateSpin);
        settingsLayout->addLayout(samplingLayout);
        
        // Graph duration
        QHBoxLayout* durationLayout = new QHBoxLayout();
        durationLayout->addWidget(new QLabel(tr("Czas wykresu:"), this));
        m_graphDurationSpin = new QSpinBox(this);
        m_graphDurationSpin->setRange(10, 300);
        m_graphDurationSpin->setValue(60);
        m_graphDurationSpin->setSuffix(tr(" s"));
        durationLayout->addWidget(m_graphDurationSpin);
        settingsLayout->addLayout(durationLayout);
        
        // Unit selection
        QHBoxLayout* unitLayout = new QHBoxLayout();
        unitLayout->addWidget(new QLabel(tr("Jednostka:"), this));
        m_unitCombo = new QComboBox(this);
        m_unitCombo->addItem(tr("Niutony (N)"));
        m_unitCombo->addItem(tr("Kilogramy (kg)"));
        unitLayout->addWidget(m_unitCombo);
        settingsLayout->addLayout(unitLayout);
        
        // Weight display
        QHBoxLayout* weightLayout = new QHBoxLayout();
        m_weightLabel = new QLabel(tr("Waga:"), this);
        m_weightValue = new QLabel(tr("0.00 N"), this);
        m_weightValue->setStyleSheet("font-size: 16px; font-weight: bold; color: blue;");
        m_weightValue->setMinimumWidth(100);
        weightLayout->addWidget(m_weightLabel);
        weightLayout->addWidget(m_weightValue);
        weightLayout->addStretch();
        settingsLayout->addLayout(weightLayout);
        
        mainLayout->addWidget(settingsGroup);
        mainLayout->addStretch();
        
        // Connect settings signals
        connect(m_samplingRateSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::onSamplingRateChanged);
        connect(m_graphDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &MainWindow::onGraphDurationChanged);
    }
    
    m_settingsWidget->show();
    m_settingsWidget->raise();
    m_settingsWidget->activateWindow();
}

void MainWindow::createGameArea()
{
    m_gameStack = new QStackedWidget(this);
    
    // Placeholder when no game is active
    QWidget* placeholder = new QWidget();
    QVBoxLayout* placeholderLayout = new QVBoxLayout(placeholder);
    QLabel* placeholderLabel = new QLabel(tr("Gra zostanie uruchomiona po rozpoczęciu sesji treningowej"));
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setMinimumHeight(300);
    placeholderLabel->setStyleSheet("font-size: 18px; color: #666;");
    placeholderLayout->addWidget(placeholderLabel);
    
    m_gameStack->addWidget(placeholder);
    
    // Initially hidden - will be shown when game starts
    m_gameStack->hide();
}

void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();
    
    // Menu Plik
    QMenu* fileMenu = menuBar->addMenu(tr("&Plik"));
    m_actionExport = fileMenu->addAction(tr("&Eksportuj dane do CSV"));
    m_actionExport->setShortcut(QKeySequence::Save);
    fileMenu->addSeparator();
    m_actionExit = fileMenu->addAction(tr("W&yjście"));
    m_actionExit->setShortcut(QKeySequence::Quit);
    
    // Menu Widok - zawiera Akcje
    QMenu* viewMenu = menuBar->addMenu(tr("&Widok"));
    m_actionStart = viewMenu->addAction(tr("▶ &Rozpocznij"));
    m_actionStop = viewMenu->addAction(tr("⏹ &Zatrzymaj"));
    viewMenu->addSeparator();
    m_actionClear = viewMenu->addAction(tr("&Wyczyść wykres"));
    m_actionClear->setShortcut(QKeySequence::Delete);
    
    // Disable stop action initially
    m_actionStop->setEnabled(false);
    
    // Menu Narzędzia - zawiera Ustawienia
    QMenu* toolsMenu = menuBar->addMenu(tr("&Narzędzia"));
    m_actionConfig = toolsMenu->addAction(tr("&Ustawienia"));
    m_actionConfig->setShortcut(QKeySequence::Preferences);
    toolsMenu->addSeparator();
    QAction* actionShowSettings = toolsMenu->addAction(tr("Pokaż &panel ustawień"));
    connect(actionShowSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    
    // Menu Pomoc
    QMenu* helpMenu = menuBar->addMenu(tr("&Pomoc"));
    m_actionAbout = helpMenu->addAction(tr("&O aplikacji"));
    m_actionAbout->setShortcut(QKeySequence::HelpContents);
}

void MainWindow::setupToolBar()
{
    // Można dodać pasek narzędzi w przyszłości
}

void MainWindow::setupStatusBar()
{
    QStatusBar* statusBar = this->statusBar();
    statusBar->showMessage(tr("Gotowy"));
}

void MainWindow::createConnections()
{
    // Przyciski (jeśli istnieją - teraz dostępne głównie przez menu)
    if (m_startButton) {
        connect(m_startButton, &QPushButton::clicked, this, &MainWindow::startDataCollection);
    }
    if (m_stopButton) {
        connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopDataCollection);
    }
    if (m_clearButton) {
        connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearGraph);
    }
    if (m_exportButton) {
        connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    }
    if (m_configButton) {
        connect(m_configButton, &QPushButton::clicked, this, &MainWindow::openConfiguration);
    }
    
    // Akcje menu - Widok (zawiera Akcje)
    connect(m_actionStart, &QAction::triggered, this, &MainWindow::startDataCollection);
    connect(m_actionStop, &QAction::triggered, this, &MainWindow::stopDataCollection);
    connect(m_actionClear, &QAction::triggered, this, &MainWindow::clearGraph);
    
    // Akcje menu - Plik
    connect(m_actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(m_actionExport, &QAction::triggered, this, &MainWindow::exportData);
    
    // Akcje menu - Narzędzia (zawiera Ustawienia)
    connect(m_actionConfig, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    
    // Timer aktualizacji
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);  // 100ms
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimerTimeout);
    m_updateTimer->start();
    
    // Connect tabs to game system
    connect(m_outlineTab, &tab::OutlineTab::requestGameStart,
            this, [this](const QString& gameId, const tab::ExerciseData& exerciseData) {
                startGame(gameId, exerciseData);
            });
    
    connect(m_outlineTab, &tab::OutlineTab::gameFinished,
            this, &MainWindow::onGameFinished);
}

void MainWindow::loadSettings()
{
    QSettings settings("Biofeedback", "MainWindow");
    resize(settings.value("window/size", QSize(1024, 768)).toSize());
    move(settings.value("window/position", QPoint(100, 100)).toPoint());
    
    int samplingRate = settings.value("sampling/rate", 100).toInt();
    int graphDuration = settings.value("graph/duration", 60).toInt();
    
    // Protect against null pointer - widgets may be created in showSettingsDialog
    if (m_samplingRateSpin) {
        m_samplingRateSpin->setValue(samplingRate);
    }
    if (m_graphDurationSpin) {
        m_graphDurationSpin->setValue(graphDuration);
    }
    
    // Protect against null pointer - GraphWidget should be initialized by now
    if (m_graphWidget) {
        m_graphWidget->setTimeRange(graphDuration);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("Biofeedback", "MainWindow");
    settings.setValue("window/size", size());
    settings.setValue("window/position", pos());
    if (m_samplingRateSpin) {
        settings.setValue("sampling/rate", m_samplingRateSpin->value());
    }
    if (m_graphDurationSpin) {
        settings.setValue("graph/duration", m_graphDurationSpin->value());
    }
}

void MainWindow::updateWeightDisplay(double weight)
{
    m_weightValue->setText(QString("%1 N").arg(weight, 0, 'f', 2));
}

void MainWindow::updateConnectionStatus(bool connected, const QString& deviceName)
{
    m_isConnected = connected;
    
    if (connected) {
        m_statusLabel->setText(tr("Status: Połączono z %1").arg(deviceName.isEmpty() ? tr("Urządzenie") : deviceName));
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    } else {
        m_statusLabel->setText(tr("Status: Rozłączono"));
        m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    }
}

void MainWindow::startDataCollection()
{
    if (m_isCollectingData) return;
    
    m_isCollectingData = true;
    if (m_startButton) m_startButton->setEnabled(false);
    if (m_stopButton) m_stopButton->setEnabled(true);
    m_actionStart->setEnabled(false);
    m_actionStop->setEnabled(true);
    
    emit dataCollectionStarted();
}

void MainWindow::stopDataCollection()
{
    if (!m_isCollectingData) return;
    
    m_isCollectingData = false;
    if (m_startButton) m_startButton->setEnabled(true);
    if (m_stopButton) m_stopButton->setEnabled(false);
    m_actionStart->setEnabled(true);
    m_actionStop->setEnabled(false);
    
    emit dataCollectionStopped();
}

void MainWindow::clearGraph()
{
    if (m_graphWidget) {
        m_graphWidget->clear();
    }
}

void MainWindow::exportData()
{
    if (!m_graphWidget) {
        QMessageBox::warning(this, tr("Błąd"), tr("Wykres nie jest zainicjalizowany."));
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Eksportuj dane do CSV"),
        QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") + "_measurements.csv",
        tr("Pliki CSV (*.csv)")
    );
    
    if (filename.isEmpty()) return;
    
    QVector<DataPoint> data = m_graphWidget->getData();
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Błąd"), tr("Nie można zapisać pliku: ") + filename);
        return;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // Nagłówek
    out << "Timestamp,Value\n";
    
    // Dane
    for (const DataPoint& point : data) {
        out << point.timestamp.toString(Qt::ISODate) << "," 
            << QString::number(point.value, 'f', 4) << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, tr("Sukces"), 
                            tr("Wyeksportowano %1 punktów danych do:\n%2")
                            .arg(data.size()).arg(filename));
}

void MainWindow::openConfiguration()
{
    // Dialog konfiguracji można rozszerzyć w przyszłości
    QMessageBox::information(this, tr("Konfiguracja"), 
                            tr("Okno konfiguracji będzie dostępne w przyszłej wersji."));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("O aplikacji"),
        tr("<h2>Biofeedback - Monitor Wagi</h2>"
           "<p>Wersja 1.0.0</p>"
           "<p>Aplikacja do monitorowania i wizualizacji danych z czujników siły.</p>"
           "<p>&copy; 2024 Biofeedback Team</p>"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isCollectingData) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Potwierdzenie"),
            tr("Trwa zbieranie danych. Czy na pewno chcesz zamknąć aplikację?"),
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    
    saveSettings();
    event->accept();
}

void MainWindow::onSamplingRateChanged(int rate)
{
    emit samplingRateChanged(rate);
}

void MainWindow::onGraphDurationChanged(int seconds)
{
    if (m_graphWidget) {
        m_graphWidget->setTimeRange(seconds);
    }
    emit graphDurationChanged(seconds);
}

void MainWindow::onAutoScaleToggled(bool enabled)
{
    emit autoScaleToggled(enabled);
}

void MainWindow::onUpdateTimerTimeout()
{
    // Okresowa aktualizacja UI
}

void MainWindow::startGame(const QString& gameId, const tab::ExerciseData& exerciseData)
{
    qDebug() << "Starting game:" << gameId << "for exercise:" << exerciseData.name;
    
    // Stop existing game if any
    stopGame();
    
    // Create appropriate game based on ID
    if (gameId == "sin_game") {
        m_currentGame = new games::SinGame(this);
        
        // Connect game signals
        connect(m_currentGame, &games::GameEngine::gameEnded,
                this, [this](int score) {
                    onGameFinished(score);
                });
        
        // Add game to stack
        m_gameStack->addWidget(m_currentGame);
        m_gameStack->setCurrentWidget(m_currentGame);
        m_gameStack->show();
        
        // Start the game
        m_currentGame->start();
        
        qDebug() << "SinGame started successfully";
    }
    else if (gameId == "tan_game") {
        // Placeholder for TanGame - can be implemented similarly
        QMessageBox::information(this, tr("Gra"), 
                                tr("Gra TanGame będzie dostępna w przyszłej wersji."));
    }
    else if (gameId.isEmpty() || gameId == "-- Wybierz grę --") {
        QMessageBox::warning(this, tr("Brak gry"), 
                            tr("Nie wybrano gry do uruchomienia."));
        return;
    }
    else {
        QMessageBox::warning(this, tr("Nieznana gra"), 
                            tr("Nieznany identyfikator gry: %1").arg(gameId));
        return;
    }
}

void MainWindow::stopGame()
{
    if (m_currentGame) {
        m_currentGame->stop();
        m_gameStack->removeWidget(m_currentGame);
        delete m_currentGame;
        m_currentGame = nullptr;
        m_gameStack->hide();
        qDebug() << "Game stopped";
    }
}

void MainWindow::forwardSensorToGame(double value)
{
    if (m_currentGame && m_currentGame->isRunning() && !m_currentGame->isPaused()) {
        m_currentGame->handleSensorInput(value);
    }
}

void MainWindow::onGameFinished(int score)
{
    qDebug() << "Game finished with score:" << score;
    
    // Notify outline tab
    if (m_outlineTab) {
        m_outlineTab->onGameFinished(score);
    }
    
    // Stop and cleanup game
    stopGame();
    
    // Show result
    QMessageBox::information(this, tr("Koniec gry"), 
                            tr("Twój wynik: %1 punktów").arg(score));
}

void MainWindow::onSensorDataForGame(double value)
{
    forwardSensorToGame(value);
}

} // namespace gui
