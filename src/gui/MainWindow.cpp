#include "gui/MainWindow.hpp"
#include "gui/GraphWidget.hpp"
#include <QApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

namespace gui {

// ============================================================================
// MainWindow - Implementacja
// ============================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_graphWidget(nullptr)
    , m_centralWidget(nullptr)
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
    , m_actionExit(nullptr)
    , m_actionConfig(nullptr)
    , m_actionExport(nullptr)
    , m_actionClear(nullptr)
    , m_actionAbout(nullptr)
{
    setWindowTitle("Aplikacja Monitorowania Wagi");
    setMinimumSize(1024, 768);
    
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
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Główny widget wykresu
    m_graphWidget = new GraphWidget(this);
    m_graphWidget->setTitle("Wykres Wagi w Czasie Rzeczywistym");
    m_graphWidget->setYLabel("Waga");
    m_graphWidget->setUnit("kg");
    m_graphWidget->setMinimumHeight(400);
    mainLayout->addWidget(m_graphWidget);
    
    // Panel sterowania
    QGroupBox* controlGroup = new QGroupBox("Panel Sterowania", this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
    
    // Przyciski akcji
    m_startButton = new QPushButton("START", this);
    m_startButton->setMinimumHeight(40);
    m_startButton->setFont(QFont("Arial", 11, QFont::Bold));
    m_startButton->setStyleSheet("background-color: #4CAF50; color: white;");
    
    m_stopButton = new QPushButton("STOP", this);
    m_stopButton->setMinimumHeight(40);
    m_stopButton->setFont(QFont("Arial", 11, QFont::Bold));
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet("background-color: #f44336; color: white;");
    
    m_clearButton = new QPushButton("CZYŚĆ", this);
    m_clearButton->setMinimumHeight(40);
    
    m_exportButton = new QPushButton("EKSPORTUJ", this);
    m_exportButton->setMinimumHeight(40);
    
    m_configButton = new QPushButton("KONFIGURACJA", this);
    m_configButton->setMinimumHeight(40);
    
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_clearButton);
    controlLayout->addWidget(m_exportButton);
    controlLayout->addWidget(m_configButton);
    
    mainLayout->addWidget(controlGroup);
    
    // Panel informacji
    QGroupBox* infoGroup = new QGroupBox("Informacje", this);
    QHBoxLayout* infoLayout = new QHBoxLayout(infoGroup);
    
    m_weightLabel = new QLabel("Aktualna waga:", this);
    m_weightLabel->setFont(QFont("Arial", 11));
    
    m_weightValue = new QLabel("0.00 kg", this);
    m_weightValue->setFont(QFont("Arial", 14, QFont::Bold));
    m_weightValue->setStyleSheet("color: #2196F3;");
    
    m_statusLabel = new QLabel("Status: Rozłączono", this);
    m_statusLabel->setFont(QFont("Arial", 11));
    m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    
    // Ustawienia
    QLabel* samplingLabel = new QLabel("Częstotliwość [Hz]:", this);
    m_samplingRateSpin = new QSpinBox(this);
    m_samplingRateSpin->setRange(1, 1000);
    m_samplingRateSpin->setValue(100);
    m_samplingRateSpin->setSuffix(" Hz");
    
    QLabel* durationLabel = new QLabel("Zakres czasu [s]:", this);
    m_graphDurationSpin = new QSpinBox(this);
    m_graphDurationSpin->setRange(5, 300);
    m_graphDurationSpin->setValue(60);
    m_graphDurationSpin->setSuffix(" s");
    
    QLabel* unitLabel = new QLabel("Jednostka:", this);
    m_unitCombo = new QComboBox(this);
    m_unitCombo->addItem("kg");
    m_unitCombo->addItem("g");
    m_unitCombo->addItem("lb");
    
    infoLayout->addWidget(m_weightLabel);
    infoLayout->addWidget(m_weightValue);
    infoLayout->addStretch();
    infoLayout->addWidget(m_statusLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(samplingLabel);
    infoLayout->addWidget(m_samplingRateSpin);
    infoLayout->addWidget(durationLabel);
    infoLayout->addWidget(m_graphDurationSpin);
    infoLayout->addWidget(unitLabel);
    infoLayout->addWidget(m_unitCombo);
    
    mainLayout->addWidget(infoGroup);
    
    setCentralWidget(m_centralWidget);
}

void MainWindow::setupMenuBar()
{
    QMenuBar* menuBar = this->menuBar();
    
    // Menu Plik
    QMenu* fileMenu = menuBar->addMenu("Plik");
    
    m_actionExport = new QAction("Eksportuj dane...", this);
    m_actionExport->setShortcut(QKeySequence::Save);
    connect(m_actionExport, &QAction::triggered, this, &MainWindow::exportData);
    fileMenu->addAction(m_actionExport);
    
    fileMenu->addSeparator();
    
    m_actionExit = new QAction("Wyjdź", this);
    m_actionExit->setShortcut(QKeySequence::Quit);
    connect(m_actionExit, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(m_actionExit);
    
    // Menu Widok
    QMenu* viewMenu = menuBar->addMenu("Widok");
    
    m_actionClear = new QAction("Wyczyść wykres", this);
    m_actionClear->setShortcut(QKeySequence::Delete);
    connect(m_actionClear, &QAction::triggered, this, &MainWindow::clearGraph);
    viewMenu->addAction(m_actionClear);
    
    // Menu Narzędzia
    QMenu* toolsMenu = menuBar->addMenu("Narzędzia");
    
    m_actionConfig = new QAction("Konfiguracja...", this);
    m_actionConfig->setShortcut(QKeySequence::Preferences);
    connect(m_actionConfig, &QAction::triggered, this, &MainWindow::openConfiguration);
    toolsMenu->addAction(m_actionConfig);
    
    // Menu Pomoc
    QMenu* helpMenu = menuBar->addMenu("Pomoc");
    
    m_actionAbout = new QAction("O aplikacji...", this);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(m_actionAbout);
}

void MainWindow::setupToolBar()
{
    // Można dodać pasek narzędzi w przyszłości
}

void MainWindow::setupStatusBar()
{
    QStatusBar* status = statusBar();
    
    QLabel* leftLabel = new QLabel("Gotowy", this);
    status->addWidget(leftLabel, 1);
    
    QLabel* rightLabel = new QLabel("v1.0.0", this);
    rightLabel->setAlignment(Qt::AlignRight);
    status->addPermanentWidget(rightLabel);
}

void MainWindow::createConnections()
{
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::startDataCollection);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopDataCollection);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearGraph);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    connect(m_configButton, &QPushButton::clicked, this, &MainWindow::openConfiguration);
    
    connect(m_samplingRateSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSamplingRateChanged);
    connect(m_graphDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onGraphDurationChanged);
    
    // Timer aktualizacji UI
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);  // 10 Hz
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimerTimeout);
    m_updateTimer->start();
}

void MainWindow::loadSettings()
{
    QSettings settings("FirmApp", "WeightMonitor");
    
    // Przywróć ustawienia okna
    resize(settings.value("MainWindow/size", QSize(1024, 768)).toSize());
    move(settings.value("MainWindow/position", QPoint(100, 100)).toPoint());
    
    // Przywróć ustawienia wykresu
    int duration = settings.value("Graph/duration", 60).toInt();
    m_graphDurationSpin->setValue(duration);
    m_graphWidget->setTimeRange(duration);
    
    int rate = settings.value("Graph/samplingRate", 100).toInt();
    m_samplingRateSpin->setValue(rate);
    
    QString unit = settings.value("Graph/unit", "kg").toString();
    int unitIndex = m_unitCombo->findText(unit);
    if (unitIndex >= 0) {
        m_unitCombo->setCurrentIndex(unitIndex);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings("FirmApp", "WeightMonitor");
    
    settings.setValue("MainWindow/size", size());
    settings.setValue("MainWindow/position", pos());
    
    settings.setValue("Graph/duration", m_graphDurationSpin->value());
    settings.setValue("Graph/samplingRate", m_samplingRateSpin->value());
    settings.setValue("Graph/unit", m_unitCombo->currentText());
}

void MainWindow::updateWeightDisplay(double weight)
{
    m_weightValue->setText(QString("%1 %2").arg(weight, 0, 'f', 2).arg(m_unitCombo->currentText()));
}

void MainWindow::updateConnectionStatus(bool connected, const QString& deviceName)
{
    m_isConnected = connected;
    
    if (connected) {
        m_statusLabel->setText(QString("Status: Połączono%1").arg(
            deviceName.isEmpty() ? QString() : " - " + deviceName));
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
        m_startButton->setEnabled(true);
    } else {
        m_statusLabel->setText("Status: Rozłączono");
        m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
        m_startButton->setEnabled(false);
        m_stopButton->setEnabled(false);
        m_isCollectingData = false;
    }
}

void MainWindow::startDataCollection()
{
    if (m_isCollectingData) return;
    
    m_isCollectingData = true;
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_statusLabel->setText("Status: Zbieranie danych...");
    m_statusLabel->setStyleSheet("color: blue; font-weight: bold;");
    
    emit dataCollectionStarted();
}

void MainWindow::stopDataCollection()
{
    if (!m_isCollectingData) return;
    
    m_isCollectingData = false;
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_statusLabel->setText("Status: Zatrzymano");
    m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    
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
    if (!m_graphWidget || m_graphWidget->dataPointCount() == 0) {
        QMessageBox::information(this, "Eksport danych", 
            "Brak danych do eksportu.");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "Eksportuj dane do CSV",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/weight_data.csv",
        "Pliki CSV (*.csv);;Wszystkie pliki (*)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd eksportu",
            QString("Nie można zapisać pliku:\n%1").arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
    out << "Timestamp,Value\n";
    
    QVector<DataPoint> data = m_graphWidget->getData();
    for (const auto& point : data) {
        out << point.timestamp.toString(Qt::ISODate) << ","
            << QString::number(point.value, 'f', 4) << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Eksport zakończony",
        QString("Dane zostały wyeksportowane do:\n%1").arg(fileName));
}

void MainWindow::openConfiguration()
{
    QMessageBox::information(this, "Konfiguracja",
        "Dialog konfiguracji zostanie dodany w przyszłej wersji.");
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "O aplikacji",
        "<h2>Monitor Wagi</h2>"
        "<p>Wersja 1.0.0</p>"
        "<p>Aplikacja do monitorowania i wizualizacji danych z wagi w czasie rzeczywistym.</p>"
        "<p>&copy; 2024 FirmApp</p>");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isCollectingData) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Zbieranie danych aktywne",
            "Zbieranie danych jest nadal aktywne. Czy na pewno chcesz wyjść?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
        
        stopDataCollection();
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
    if (m_graphWidget) {
        // Implementacja autoskalowania
    }
    emit autoScaleToggled(enabled);
}

void MainWindow::onUpdateTimerTimeout()
{
    // Okresowa aktualizacja UI
    if (m_isCollectingData && !m_isConnected) {
        m_statusLabel->setText("Status: Oczekiwanie na dane...");
    }
}

} // namespace gui
