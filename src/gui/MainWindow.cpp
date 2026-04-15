#include "gui/MainWindow.hpp"
#include <QApplication>
#include <QSettings>
#include <QDateTime>

namespace gui {

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
{
    setWindowTitle(tr("Biofeedback - Monitor Wagi"));
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
    
    // Widget wykresu
    m_graphWidget = new GraphWidget(this);
    m_graphWidget->setTitle(tr("Wykres Siły w Czasie Rzeczywistym"));
    m_graphWidget->setYLabel(tr("Siła [N]"));
    m_graphWidget->setUnit(tr("N"));
    m_graphWidget->setMinimumHeight(400);
    mainLayout->addWidget(m_graphWidget);
    
    // Panel sterowania
    QGroupBox* controlGroup = new QGroupBox(tr("Panel Sterowania"), this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
    
    // Przyciski akcji
    m_startButton = new QPushButton(tr("▶ Start"), this);
    m_startButton->setMinimumHeight(40);
    m_startButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    
    m_stopButton = new QPushButton(tr("■ Stop"), this);
    m_stopButton->setMinimumHeight(40);
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet("background-color: #f44336; color: white; font-weight: bold;");
    
    m_clearButton = new QPushButton(tr("🗑 Czyść"), this);
    m_clearButton->setMinimumHeight(40);
    
    m_exportButton = new QPushButton(tr("💾 Eksportuj"), this);
    m_exportButton->setMinimumHeight(40);
    
    m_configButton = new QPushButton(tr("⚙ Konfiguracja"), this);
    m_configButton->setMinimumHeight(40);
    
    controlLayout->addWidget(m_startButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_clearButton);
    controlLayout->addWidget(m_exportButton);
    controlLayout->addWidget(m_configButton);
    controlLayout->addStretch();
    
    mainLayout->addWidget(controlGroup);
    
    // Panel informacji
    QGroupBox* infoGroup = new QGroupBox(tr("Informacje"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    // Wartość wagi
    QHBoxLayout* weightLayout = new QHBoxLayout();
    m_weightLabel = new QLabel(tr("Aktualna siła:"), this);
    m_weightLabel->setFont(QFont("Arial", 12));
    m_weightValue = new QLabel(tr("0.0 N"), this);
    m_weightValue->setFont(QFont("Arial", 24, QFont::Bold));
    m_weightValue->setStyleSheet("color: #2196F3;");
    weightLayout->addWidget(m_weightLabel);
    weightLayout->addWidget(m_weightValue);
    weightLayout->addStretch();
    infoLayout->addLayout(weightLayout);
    
    // Ustawienia
    QHBoxLayout* settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(new QLabel(tr("Częstotliwość próbkowania [Hz]:")));
    m_samplingRateSpin = new QSpinBox(this);
    m_samplingRateSpin->setRange(1, 1000);
    m_samplingRateSpin->setValue(100);
    m_samplingRateSpin->setSuffix(" Hz");
    settingsLayout->addWidget(m_samplingRateSpin);
    
    settingsLayout->addSpacing(20);
    
    settingsLayout->addWidget(new QLabel(tr("Zakres wykresu [s]:")));
    m_graphDurationSpin = new QSpinBox(this);
    m_graphDurationSpin->setRange(5, 300);
    m_graphDurationSpin->setValue(60);
    m_graphDurationSpin->setSuffix(" s");
    settingsLayout->addWidget(m_graphDurationSpin);
    
    settingsLayout->addSpacing(20);
    
    settingsLayout->addWidget(new QLabel(tr("Jednostka:")));
    m_unitCombo = new QComboBox(this);
    m_unitCombo->addItem("N (Newton)", "N");
    m_unitCombo->addItem("g (gramy)", "g");
    m_unitCombo->addItem("kg (kilogramy)", "kg");
    settingsLayout->addWidget(m_unitCombo);
    settingsLayout->addStretch();
    
    infoLayout->addLayout(settingsLayout);
    
    // Status
    m_statusLabel = new QLabel(tr("Status: Rozłączono"), this);
    m_statusLabel->setFont(QFont("Arial", 11));
    m_statusLabel->setStyleSheet("color: orange; font-weight: bold;");
    infoLayout->addWidget(m_statusLabel);
    
    mainLayout->addWidget(infoGroup);
    
    setCentralWidget(m_centralWidget);
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
    
    // Menu Widok
    QMenu* viewMenu = menuBar->addMenu(tr("&Widok"));
    m_actionClear = viewMenu->addAction(tr("&Wyczyść wykres"));
    m_actionClear->setShortcut(QKeySequence::Delete);
    
    // Menu Narzędzia
    QMenu* toolsMenu = menuBar->addMenu(tr("&Narzędzia"));
    m_actionConfig = toolsMenu->addAction(tr("&Konfiguracja"));
    m_actionConfig->setShortcut(QKeySequence::Preferences);
    
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
    // Przyciski
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::startDataCollection);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopDataCollection);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearGraph);
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::exportData);
    connect(m_configButton, &QPushButton::clicked, this, &MainWindow::openConfiguration);
    
    // Akcje menu
    connect(m_actionExit, &QAction::triggered, this, &MainWindow::close);
    connect(m_actionConfig, &QAction::triggered, this, &MainWindow::openConfiguration);
    connect(m_actionExport, &QAction::triggered, this, &MainWindow::exportData);
    connect(m_actionClear, &QAction::triggered, this, &MainWindow::clearGraph);
    connect(m_actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    
    // Ustawienia
    connect(m_samplingRateSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSamplingRateChanged);
    connect(m_graphDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onGraphDurationChanged);
    
    // Timer aktualizacji
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);  // 100ms
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateTimerTimeout);
    m_updateTimer->start();
}

void MainWindow::loadSettings()
{
    QSettings settings("Biofeedback", "MainWindow");
    resize(settings.value("window/size", QSize(1024, 768)).toSize());
    move(settings.value("window/position", QPoint(100, 100)).toPoint());
    
    int samplingRate = settings.value("sampling/rate", 100).toInt();
    int graphDuration = settings.value("graph/duration", 60).toInt();
    
    m_samplingRateSpin->setValue(samplingRate);
    m_graphDurationSpin->setValue(graphDuration);
    m_graphWidget->setTimeRange(graphDuration);
}

void MainWindow::saveSettings()
{
    QSettings settings("Biofeedback", "MainWindow");
    settings.setValue("window/size", size());
    settings.setValue("window/position", pos());
    settings.setValue("sampling/rate", m_samplingRateSpin->value());
    settings.setValue("graph/duration", m_graphDurationSpin->value());
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
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    
    emit dataCollectionStarted();
}

void MainWindow::stopDataCollection()
{
    if (!m_isCollectingData) return;
    
    m_isCollectingData = false;
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    
    emit dataCollectionStopped();
}

void MainWindow::clearGraph()
{
    m_graphWidget->clear();
}

void MainWindow::exportData()
{
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
    m_graphWidget->setTimeRange(seconds);
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

} // namespace gui
