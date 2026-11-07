#include "gui/MainWindow.hpp"
#include "gui/GraphWidget.hpp"
#include "tab/PatientTab.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/OutlineTab.hpp"
#include "tab/TrainingTab.hpp"
#include "tab/DebugTab.hpp"
#include "games/GameEngine.hpp"
#include "games/SinGame.hpp"
#include "measurement/Units.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QTabWidget>
#include <QStackedWidget>
#include <QMessageBox>
#include <QGridLayout>
#include <QFrame>
#include <QMetaObject>
#include <cmath>
#include <memory>

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
    , m_debugTab(nullptr)
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
    , m_dashboardConnectionValue(nullptr)
    , m_dashboardForceValue(nullptr)
    , m_dashboardMeasurementValue(nullptr)
    , m_samplingRateSpin(nullptr)
    , m_graphDurationSpin(nullptr)
    , m_unitCombo(nullptr)
    , m_devicePortCombo(nullptr)
    , m_connectDeviceButton(nullptr)
    , m_updateTimer(nullptr)
    , m_settingsWidget(nullptr)
    , m_isCollectingData(false)
    , m_isConnected(false)
    , m_debugTerminal(nullptr)
    , m_debugMaxLines(500)
    , m_serialPort(nullptr)
{
    setWindowTitle(tr("Biofeedback - Monitor Wagi"));
    setMinimumSize(1280, 800);
    setStyleSheet("QMainWindow{background:#f3f6fb;color:#18263a;} QTabWidget::pane{border:1px solid #dce4ef;background:white;border-radius:10px;} QTabBar::tab{background:#e9eef5;color:#40516a;padding:11px 18px;} QTabBar::tab:selected{background:white;color:#087f82;font-weight:700;} QFrame#dashboardCard{background:white;border:1px solid #dce4ef;border-radius:12px;} QLabel#dashboardValue{color:#14243a;font-size:21pt;font-weight:700;} QPushButton{background:#087f82;color:white;border:0;border-radius:8px;padding:10px 16px;font-weight:600;} QPushButton:hover{background:#06696c;}");
    
    // Shared serial service; UI tabs subscribe independently to its data stream.
    m_serialPort = std::make_shared<sensor::SerialCommunication>();
    m_serialConnectionCallbackToken = m_serialPort->addConnectionCallback(
        [this](bool connected, const std::string& message) {
            const QString deviceName = connected ? QString::fromStdString(m_serialPort->getPortName()) : QString();
            const QString detail = QString::fromStdString(message);
            QMetaObject::invokeMethod(this, [this, connected, deviceName, detail]() {
                updateConnectionStatus(connected, deviceName);
                if (m_connectDeviceButton) m_connectDeviceButton->setText(connected ? tr("Rozłącz") : tr("Połącz"));
                if (!detail.isEmpty()) addDebugMessage(detail, connected ? "INFO" : "WARNING");
            }, Qt::QueuedConnection);
        });

    m_serialDataCallbackToken = m_serialPort->addDataCallback([this](const sensor::SensorData& data) {
        if (!data.isValid || !data.hasCalibratedValue) return;
        const double forceNewtons = biofeedback::measurement::gramsToNewtons(data.calibratedValue);
        if (!std::isfinite(forceNewtons) || forceNewtons < 0.0) return;
        constexpr double gameReferenceForceN = 50.0;
        const double normalizedGameInput = qBound(0.0, forceNewtons / gameReferenceForceN * 100.0, 100.0);
        QMetaObject::invokeMethod(this, [this, normalizedGameInput]() {
            forwardSensorToGame(normalizedGameInput);
        }, Qt::QueuedConnection);
    });

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
    if (m_serialPort) {
        m_serialPort->removeDataCallback(m_serialDataCallbackToken);
        m_serialDataCallbackToken = 0;
        m_serialPort->removeConnectionCallback(m_serialConnectionCallbackToken);
        m_serialConnectionCallbackToken = 0;
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }
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
    
    // GraphWidget is now managed by MeasurementTab - do not create it here
    m_graphWidget = nullptr;  // Will be accessed via measurementTab()->graphWidget()
    
    // Setup tabs
    setupTabs();
    
    // Add tab widget to main layout
    mainLayout->addWidget(m_tabWidget);
    
    // Setup debug terminal
    setupDebugTerminal();
    
    // Setup control panel with buttons and settings
    setupControlPanel();
    
    // Setup game area (hidden by default) - used for global game management
    createGameArea();
    
    setCentralWidget(m_centralWidget);
    
    // Auto-start data collection will be triggered from Application.cpp after login
    // Do not start here to avoid conflicts with login dialog
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
    m_debugTab = new tab::DebugTab(this);
    
    // Udostępnij połączenie szeregowe wszystkim zakładkom
    if (m_serialPort) {
        m_measurementTab->setSerialConnection(m_serialPort);
        m_outlineTab->setSerialConnection(m_serialPort);
        m_trainingTab->setSerialConnection(m_serialPort);
        m_debugTab->setSerialConnection(m_serialPort);
    }

    connect(m_patientTab, &tab::PatientTab::currentPatientChanged,
            m_trainingTab, &tab::TrainingTab::setCurrentPatientContext);
    connect(m_trainingTab, &tab::TrainingTab::trainingStarted,
            m_patientTab, [this]() { m_patientTab->setEnabled(false); });
    connect(m_trainingTab, &tab::TrainingTab::trainingStopped,
            m_patientTab, [this]() {
                if (!m_measurementTab->isMeasurementActive())
                    m_patientTab->setEnabled(true);
            });
    connect(m_measurementTab, &tab::MeasurementTab::measurementStarted,
            m_patientTab, [this]() { m_patientTab->setEnabled(false); });
    connect(m_measurementTab, &tab::MeasurementTab::measurementStopped,
            m_patientTab, [this]() {
                if (!m_trainingTab->isTrainingActive())
                    m_patientTab->setEnabled(true);
            });
    
    QWidget* dashboard = new QWidget(m_tabWidget);
    QVBoxLayout* dashboardLayout = new QVBoxLayout(dashboard);
    dashboardLayout->setContentsMargins(28, 26, 28, 26);
    QLabel* dashboardTitle = new QLabel(tr("Centrum Biofeedback"), dashboard);
    dashboardTitle->setStyleSheet("font-size:24px;font-weight:700;color:#14243a;");
    dashboardLayout->addWidget(dashboardTitle);
    QGridLayout* cards = new QGridLayout();
    cards->setHorizontalSpacing(14);
    cards->setVerticalSpacing(14);
    auto makeDashboardCard = [dashboard](const QString& heading, const QString& value,
                                         const QString& detail, QLabel** valueLabelOut) {
        QFrame* card = new QFrame(dashboard);
        card->setObjectName("dashboardCard");
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(18, 16, 18, 16);
        QLabel* headingLabel = new QLabel(heading, card);
        headingLabel->setStyleSheet("color:#64748b;font-size:10pt;font-weight:600;");
        QLabel* valueLabel = new QLabel(value, card);
        valueLabel->setObjectName("dashboardValue");
        QLabel* detailLabel = new QLabel(detail, card);
        detailLabel->setStyleSheet("color:#74839a;font-size:9pt;");
        cardLayout->addWidget(headingLabel);
        cardLayout->addWidget(valueLabel);
        cardLayout->addWidget(detailLabel);
        if (valueLabelOut) *valueLabelOut = valueLabel;
        return card;
    };
    cards->addWidget(makeDashboardCard(tr("POŁĄCZENIE"), tr("Rozłączono"),
        tr("Urządzenie pomiarowe"), &m_dashboardConnectionValue), 0, 0);
    cards->addWidget(makeDashboardCard(tr("OSTATNI ODCZYT"), tr("-- N"),
        tr("Oczekiwanie na próbkę"), &m_dashboardForceValue), 0, 1);
    cards->addWidget(makeDashboardCard(tr("POMIAR"), tr("Bezczynny"),
        tr("Brak aktywnej sesji"), &m_dashboardMeasurementValue), 1, 0);
    cards->addWidget(makeDashboardCard(tr("TRYB"), tr("Gotowy"),
        tr("Aplikacja desktopowa Qt"), nullptr), 1, 1);
    dashboardLayout->addLayout(cards);
    QLabel* hint = new QLabel(tr("Wybierz moduł, aby przejść do pacjentów, pomiarów lub treningu."), dashboard);
    hint->setStyleSheet("color:#74839a;font-size:11pt;");
    dashboardLayout->addWidget(hint);
    QHBoxLayout* actions = new QHBoxLayout();
    QPushButton* patientsButton = new QPushButton(tr("Pacjenci"), dashboard);
    QPushButton* measurementsButton = new QPushButton(tr("Otwórz pomiary"), dashboard);
    QPushButton* trainingButton = new QPushButton(tr("Trening"), dashboard);
    actions->addWidget(patientsButton); actions->addWidget(measurementsButton); actions->addWidget(trainingButton); actions->addStretch();
    dashboardLayout->addLayout(actions);

    QLabel* deviceHeading = new QLabel(tr("Połączenie z urządzeniem"), dashboard);
    deviceHeading->setStyleSheet("font-size:14px;font-weight:700;color:#24364d;");
    dashboardLayout->addWidget(deviceHeading);
    QHBoxLayout* deviceControls = new QHBoxLayout();
    m_devicePortCombo = new QComboBox(dashboard);
    m_devicePortCombo->setMinimumWidth(260);
    m_devicePortCombo->setMinimumHeight(40);
    QPushButton* scanPortsButton = new QPushButton(tr("Skanuj porty"), dashboard);
    m_connectDeviceButton = new QPushButton(tr("Połącz"), dashboard);
    deviceControls->addWidget(m_devicePortCombo, 2);
    deviceControls->addWidget(scanPortsButton);
    deviceControls->addWidget(m_connectDeviceButton);
    deviceControls->addStretch();
    dashboardLayout->addLayout(deviceControls);
    connect(scanPortsButton, &QPushButton::clicked, this, &MainWindow::refreshSerialPorts);
    connect(m_connectDeviceButton, &QPushButton::clicked, this, &MainWindow::toggleSerialConnection);
    refreshSerialPorts();

    dashboardLayout->addStretch();
    m_tabWidget->addTab(dashboard, tr("⌂ Pulpit"));
    m_tabWidget->addTab(m_patientTab, tr("📋 Pacjenci"));
    m_tabWidget->addTab(m_measurementTab, tr("📊 Pomiary"));
    m_tabWidget->addTab(m_outlineTab, tr("🎮 Trening (Outline)"));
    m_tabWidget->addTab(m_trainingTab, tr("🏋️ Trening (Gry)"));
    m_tabWidget->addTab(m_debugTab, tr("🔍 Debug Terminal"));
    connect(patientsButton, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentWidget(m_patientTab); });
    connect(measurementsButton, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentWidget(m_measurementTab); });
    connect(trainingButton, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentWidget(m_trainingTab); });
    m_tabWidget->setCurrentWidget(dashboard);
}

void MainWindow::setupDebugTerminal()
{
    // Terminal debugowy jest teraz tylko w DebugTab
    // Ta metoda inicjalizuje DebugManager i rejestruje DebugTab jako odbiorcę
    
    // Initial debug message via DebugManager
    addDebugMessage("=== Debug Terminal Initialized ===", "INFO");
    addDebugMessage(QString("Application started at %1").arg(QDateTime::currentDateTime().toString()), "INFO");
    
    // Initialize debug terminals in tabs (they will use DebugManager)
    // Note: setupDebugTerminal methods removed from TrainingTab and MeasurementTab
    // Debug functionality is now centralized in DebugManager
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

void MainWindow::addDebugMessage(const QString& message, const QString& type)
{
    if (!m_debugTerminal) return;
    
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString coloredMessage;
    
    if (type == "ERROR") {
        coloredMessage = QString("<span style='color: #ff6b6b;'>[%1] [ERROR] %2</span><br>").arg(timestamp).arg(message);
    } else if (type == "WARNING") {
        coloredMessage = QString("<span style='color: #ffd93d;'>[%1] [WARNING] %2</span><br>").arg(timestamp).arg(message);
    } else if (type == "DATA") {
        coloredMessage = QString("<span style='color: #6bcb77;'>[%1] [DATA] %2</span><br>").arg(timestamp).arg(message);
    } else if (type == "SERIAL") {
        coloredMessage = QString("<span style='color: #4d96ff;'>[%1] [SERIAL] %2</span><br>").arg(timestamp).arg(message);
    } else {
        coloredMessage = QString("<span style='color: #c4c4c4;'>[%1] [INFO] %2</span><br>").arg(timestamp).arg(message);
    }
    
    m_debugTerminal->append(coloredMessage);
    
    // Limit number of lines to prevent memory issues
    QStringList lines = m_debugTerminal->toPlainText().split("\n");
    if (lines.size() > m_debugMaxLines) {
        QTextCursor cursor(m_debugTerminal->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.deleteChar(); // Delete the newline after the block
        cursor.deleteChar(); // Delete the block itself
    }
    
    // Auto-scroll to bottom
    m_debugTerminal->verticalScrollBar()->setValue(m_debugTerminal->verticalScrollBar()->maximum());
}

void MainWindow::clearDebugTerminal()
{
    if (m_debugTerminal) {
        m_debugTerminal->clear();
        addDebugMessage("Debug terminal cleared", "INFO");
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
    viewMenu->addSeparator();
    QAction* actionClearDebug = viewMenu->addAction(tr("Wyczyść terminal debugowania"));
    connect(actionClearDebug, &QAction::triggered, this, &MainWindow::clearDebugTerminal);
    
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
    
    // GraphWidget is now managed by MeasurementTab - no need for tab change handling here
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
    
    // GraphWidget is now managed by MeasurementTab - settings are handled there
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

void MainWindow::refreshSerialPorts()
{
    if (!m_devicePortCombo) return;
    const QString previousPort = m_devicePortCombo->currentData().toString();
    m_devicePortCombo->clear();
    const auto ports = sensor::SerialCommunication::scanAvailablePorts();
    for (const auto& port : ports) {
        const QString value = QString::fromStdString(port);
        m_devicePortCombo->addItem(value, value);
    }
    if (ports.empty()) {
        m_devicePortCombo->addItem(tr("Brak wykrytych portów"), QString());
        return;
    }
    const int previousIndex = m_devicePortCombo->findData(previousPort);
    if (previousIndex >= 0) m_devicePortCombo->setCurrentIndex(previousIndex);
}

void MainWindow::toggleSerialConnection()
{
    if (!m_serialPort) {
        QMessageBox::warning(this, tr("Urządzenie"), tr("Usługa portu szeregowego nie jest dostępna."));
        return;
    }
    if (m_serialPort->isConnected()) {
        m_serialPort->disconnect();
        updateConnectionStatus(false);
        if (m_connectDeviceButton) m_connectDeviceButton->setText(tr("Połącz"));
        addDebugMessage(tr("Rozłączono urządzenie pomiarowe"), "SERIAL");
        return;
    }

    const QString port = m_devicePortCombo ? m_devicePortCombo->currentData().toString() : QString();
    if (port.isEmpty()) {
        QMessageBox::information(this, tr("Urządzenie"), tr("Najpierw wybierz wykryty port szeregowy."));
        refreshSerialPorts();
        return;
    }
    if (!m_serialPort->connect(port.toStdString(), DEFAULT_BAUD_RATE)) {
        updateConnectionStatus(false);
        QMessageBox::warning(this, tr("Połączenie"), tr("Nie udało się otworzyć portu %1.").arg(port));
        addDebugMessage(tr("Nie udało się połączyć z %1").arg(port), "ERROR");
        return;
    }
    m_serialPort->startAsyncReading();
    updateConnectionStatus(true, port);
    if (m_connectDeviceButton) m_connectDeviceButton->setText(tr("Rozłącz"));
    addDebugMessage(tr("Uruchomiono odczyt z portu %1 przy %2 baud").arg(port).arg(DEFAULT_BAUD_RATE), "SERIAL");
}

void MainWindow::updateWeightDisplay(double weight)
{
    const QString value = QString("%1 N").arg(weight, 0, 'f', 2);
    if (m_weightValue) m_weightValue->setText(value);
    if (m_dashboardForceValue) m_dashboardForceValue->setText(value);
}

void MainWindow::updateConnectionStatus(bool connected, const QString& deviceName)
{
    m_isConnected = connected;
    m_currentDevice = connected ? deviceName : QString();
    if (m_connectDeviceButton) m_connectDeviceButton->setText(connected ? tr("Rozłącz") : tr("Połącz"));
    
    const QString device = deviceName.isEmpty() ? tr("Urządzenie") : deviceName;
    if (connected) {
        if (m_statusLabel) { m_statusLabel->setText(tr("Status: Połączono z %1").arg(device)); m_statusLabel->setStyleSheet("color:#087f82;font-weight:bold;"); }
        if (m_dashboardConnectionValue) m_dashboardConnectionValue->setText(tr("Połączono"));
    } else {
        if (m_statusLabel) { m_statusLabel->setText(tr("Status: Rozłączono")); m_statusLabel->setStyleSheet("color:#b7791f;font-weight:bold;"); }
        if (m_dashboardConnectionValue) m_dashboardConnectionValue->setText(tr("Rozłączono"));
    }
}

void MainWindow::startDataCollection()
{
    if (m_isCollectingData) return;
    
    m_isCollectingData = true;
    if (m_dashboardMeasurementValue) m_dashboardMeasurementValue->setText(tr("Aktywny"));
    if (m_startButton) m_startButton->setEnabled(false);
    if (m_stopButton) m_stopButton->setEnabled(true);
    m_actionStart->setEnabled(false);
    m_actionStop->setEnabled(true);
    
    // Add debug message when data collection starts
    addDebugMessage("Data collection started via menu Widok->Rozpocznij", "INFO");
    
    emit dataCollectionStarted();
}

void MainWindow::stopDataCollection()
{
    if (!m_isCollectingData) return;
    
    m_isCollectingData = false;
    if (m_dashboardMeasurementValue) m_dashboardMeasurementValue->setText(tr("Bezczynny"));
    if (m_startButton) m_startButton->setEnabled(true);
    if (m_stopButton) m_stopButton->setEnabled(false);
    m_actionStart->setEnabled(true);
    m_actionStop->setEnabled(false);
    
    // Add debug message when data collection stops
    addDebugMessage("Data collection stopped via menu Widok->Zatrzymaj", "INFO");
    
    emit dataCollectionStopped();
}

void MainWindow::clearGraph()
{
    // GraphWidget is now managed by MeasurementTab - delegate to it
    if (m_measurementTab) {
        m_measurementTab->resetSession();  // This clears the ForcePlotWidget data
    }
}

void MainWindow::exportData()
{
    // Export functionality is now handled by MeasurementTab
    if (m_measurementTab) {
        m_measurementTab->onSaveMeasurementJSON();  // Or onSaveData() for CSV export
    } else {
        QMessageBox::warning(this, tr("Błąd"), tr("Zakładka pomiarów nie jest zainicjalizowana."));
    }
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
    // GraphWidget is now managed by MeasurementTab - settings are handled there
    emit graphDurationChanged(seconds);
}

void MainWindow::onAutoScaleToggled(bool enabled)
{
    emit autoScaleToggled(enabled);
}

void MainWindow::onUpdateTimerTimeout()
{
    // Okresowa aktualizacja UI
    // Dodajemy debug message co 10 wywołań (co ~1 sekundę)
    static int callCount = 0;
    callCount++;
    if (callCount % 10 == 0 && m_debugTerminal) {
        // Można dodać okresowe logowanie statusu
    }
}

void MainWindow::startGame(const QString& gameId, const tab::ExerciseData& exerciseData)
{
    qDebug() << "Starting game:" << gameId << "for exercise:" << exerciseData.name;
    
    // Stop existing game if any
    stopGame();
    
    // Create appropriate game based on ID
    if (gameId == "sin_game") {
        if (!m_serialPort || !m_serialPort->isConnected()) {
            QMessageBox::warning(this, tr("Brak urządzenia"),
                                 tr("Podłącz czujnik HX711 przed uruchomieniem gry."));
            return;
        }
        auto* sinGame = new games::SinGame(this);
        sinGame->setSerialConnection(m_serialPort);
        m_currentGame = sinGame;
        
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
