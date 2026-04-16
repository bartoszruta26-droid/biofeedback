#include "tab/MeasurementTab.hpp"
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <algorithm>
#include <cmath>
#include <QMap>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScrollArea>

namespace tab {

// ============================================================================
// TrendsPlotWidget - Implementacja
// ============================================================================

TrendsPlotWidget::TrendsPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxForce(100.0)
{
    setMinimumHeight(250);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void TrendsPlotWidget::setData(const QVector<double>& dates, const QVector<double>& avgForces, 
                                const QVector<double>& maxForces)
{
    m_dates = dates;
    m_avgForces = avgForces;
    m_maxForces = maxForces;
    
    // Znajdź maksymalną siłę
    m_maxForce = 0;
    for (double force : m_maxForces) {
        if (force > m_maxForce) m_maxForce = force;
    }
    m_maxForce *= 1.2;  // 20% marginesu
    
    update();
}

void TrendsPlotWidget::clearData()
{
    m_dates.clear();
    m_avgForces.clear();
    m_maxForces.clear();
    m_maxForce = 100.0;
    update();
}

void TrendsPlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int width = this->width();
    int height = this->height();
    int margin = 50;
    
    int plotWidth = width - 2 * margin;
    int plotHeight = height - 2 * margin;
    
    if (plotWidth <= 0 || plotHeight <= 0 || m_dates.isEmpty()) {
        return;
    }
    
    // Tło
    painter.fillRect(margin, margin, plotWidth, plotHeight, QColor(245, 248, 255));
    
    // Linie siatki
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    for (int i = 1; i <= 4; ++i) {
        int y = margin + (plotHeight * i / 5);
        painter.drawLine(margin, y, margin + plotWidth, y);
    }
    
    // Oś X
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(margin, margin + plotHeight, margin + plotWidth, margin + plotHeight);
    
    // Oś Y
    painter.drawLine(margin, margin, margin, margin + plotHeight);
    
    // Etykiety osi Y
    painter.setFont(QFont("Arial", 8));
    painter.setPen(Qt::black);
    for (int i = 0; i <= 5; ++i) {
        int y = margin + plotHeight - (plotHeight * i / 5);
        double force = m_maxForce * i / 5;
        QString label = QString::number(force, 'f', 0);
        painter.drawText(5, y + 3, label);
    }
    
    // Rysowanie punktów i linii dla średniej siły
    if (m_avgForces.size() >= 2) {
        painter.setPen(QPen(QColor(0, 100, 200), 3, Qt::SolidLine));
        
        for (int i = 1; i < m_avgForces.size(); ++i) {
            double x1 = margin + ((i - 1) / (double)(m_avgForces.size() - 1)) * plotWidth;
            double y1 = margin + plotHeight - (m_avgForces[i-1] / m_maxForce) * plotHeight;
            double x2 = margin + (i / (double)(m_avgForces.size() - 1)) * plotWidth;
            double y2 = margin + plotHeight - (m_avgForces[i] / m_maxForce) * plotHeight;
            
            painter.drawLine(static_cast<int>(x1), static_cast<int>(y1), 
                            static_cast<int>(x2), static_cast<int>(y2));
        }
        
        // Punkty
        painter.setBrush(QBrush(QColor(0, 100, 200)));
        for (int i = 0; i < m_avgForces.size(); ++i) {
            double x = margin + (i / (double)(m_avgForces.size() - 1)) * plotWidth;
            double y = margin + plotHeight - (m_avgForces[i] / m_maxForce) * plotHeight;
            painter.drawEllipse(static_cast<int>(x) - 5, static_cast<int>(y) - 5, 10, 10);
        }
    }
    
    // Rysowanie punktów dla maksymalnej siły
    if (m_maxForces.size() >= 2) {
        painter.setPen(QPen(QColor(200, 50, 50), 2, Qt::DotLine));
        painter.setBrush(QBrush(QColor(200, 50, 50)));
        
        for (int i = 0; i < m_maxForces.size(); ++i) {
            double x = margin + (i / (double)(m_maxForces.size() - 1)) * plotWidth;
            double y = margin + plotHeight - (m_maxForces[i] / m_maxForce) * plotHeight;
            painter.drawEllipse(static_cast<int>(x) - 4, static_cast<int>(y) - 4, 8, 8);
        }
    }
    
    // Legenda
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.setPen(QColor(0, 100, 200));
    painter.drawText(margin + 10, margin + 15, "Średnia Siła");
    painter.setPen(QColor(200, 50, 50));
    painter.drawText(margin + 120, margin + 15, "Maks. Siła");
    
    // Etykiety
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 9));
    painter.drawText(margin + plotWidth / 2 - 60, height - 10, "Kolejne Sesje");
    painter.drawText(5, margin - 5, "Siła [N]");
}

// ============================================================================
// ForcePlotWidget - Implementacja
// ============================================================================

ForcePlotWidget::ForcePlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_targetForce(50.0)
    , m_maxTimeWindow(10.0)
    , m_maxForce(100.0)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ForcePlotWidget::addSample(double force, double time)
{
    m_forces.append(force);
    m_times.append(time);
    
    if (force > m_maxForce) {
        m_maxForce = force * 1.2;
    }
    
    // Usuń stare dane poza oknem czasowym
    while (!m_times.isEmpty() && (time - m_times.first()) > m_maxTimeWindow) {
        m_forces.removeFirst();
        m_times.removeFirst();
    }
    
    update();
}

void ForcePlotWidget::clearData()
{
    m_forces.clear();
    m_times.clear();
    m_maxForce = 100.0;
    update();
}

void ForcePlotWidget::setTargetForce(double force)
{
    m_targetForce = force;
    update();
}

void ForcePlotWidget::setMaxTimeWindow(double seconds)
{
    m_maxTimeWindow = seconds;
}

void ForcePlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int width = this->width();
    int height = this->height();
    int margin = 40;
    
    int plotWidth = width - 2 * margin;
    int plotHeight = height - 2 * margin;
    
    if (plotWidth <= 0 || plotHeight <= 0 || m_forces.isEmpty()) {
        return;
    }
    
    // Tło
    painter.fillRect(margin, margin, plotWidth, plotHeight, QColor(250, 250, 255));
    
    // Linie siatki
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DashLine));
    for (int i = 1; i <= 4; ++i) {
        int y = margin + (plotHeight * i / 5);
        painter.drawLine(margin, y, margin + plotWidth, y);
    }
    
    // Oś czasu (X)
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(margin, margin + plotHeight, margin + plotWidth, margin + plotHeight);
    
    // Oś siły (Y)
    painter.drawLine(margin, margin, margin, margin + plotHeight);
    
    // Etykiety osi Y
    painter.setFont(QFont("Arial", 8));
    painter.setPen(Qt::black);
    for (int i = 0; i <= 5; ++i) {
        int y = margin + plotHeight - (plotHeight * i / 5);
        double force = m_maxForce * i / 5;
        QString label = QString::number(force, 'f', 0);
        painter.drawText(5, y + 3, label);
    }
    
    // Linia docelowej siły
    if (m_targetForce > 0 && m_maxForce > 0) {
        int targetY = margin + plotHeight - static_cast<int>((m_targetForce / m_maxForce) * plotHeight);
        painter.setPen(QPen(QColor(255, 100, 100), 2, Qt::DashLine));
        painter.drawLine(margin, targetY, margin + plotWidth, targetY);
        painter.setPen(QColor(255, 100, 100));
        painter.drawText(margin + plotWidth - 60, targetY - 5, "Cel");
    }
    
    // Rysowanie wykresu siły
    if (m_forces.size() < 2) {
        return;
    }
    
    double minTime = m_times.first();
    double maxTime = m_times.last();
    double timeRange = maxTime - minTime;
    if (timeRange <= 0) timeRange = 1.0;
    
    painter.setPen(QPen(QColor(0, 100, 200), 2, Qt::SolidLine));
    
    for (int i = 1; i < m_forces.size(); ++i) {
        double x1 = margin + ((m_times[i-1] - minTime) / timeRange) * plotWidth;
        double y1 = margin + plotHeight - (m_forces[i-1] / m_maxForce) * plotHeight;
        double x2 = margin + ((m_times[i] - minTime) / timeRange) * plotWidth;
        double y2 = margin + plotHeight - (m_forces[i] / m_maxForce) * plotHeight;
        
        painter.drawLine(static_cast<int>(x1), static_cast<int>(y1), 
                        static_cast<int>(x2), static_cast<int>(y2));
    }
    
    // Wypełnienie pod wykresem
    painter.setBrush(QBrush(QColor(0, 100, 200, 50)));
    painter.setPen(Qt::NoPen);
    
    QVector<QPoint> fillPoints;
    fillPoints.append(QPoint(margin, margin + plotHeight));
    for (int i = 0; i < m_forces.size(); ++i) {
        double x = margin + ((m_times[i] - minTime) / timeRange) * plotWidth;
        double y = margin + plotHeight - (m_forces[i] / m_maxForce) * plotHeight;
        fillPoints.append(QPoint(static_cast<int>(x), static_cast<int>(y)));
    }
    fillPoints.append(QPoint(margin + plotWidth, margin + plotHeight));
    
    painter.drawPolygon(fillPoints);
    
    // Etykiety osi X
    painter.setPen(Qt::black);
    painter.drawText(margin + plotWidth / 2 - 20, height - 15, "Czas [s]");
    painter.drawText(5, margin - 5, "Siła [N]");
}

// ============================================================================
// MeasurementTab - Implementacja
// ============================================================================

MeasurementTab::MeasurementTab(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_btnStartStop(nullptr)
    , m_btnReadManual(nullptr)
    , m_btnSave(nullptr)
    , m_btnLoad(nullptr)
    , m_btnReset(nullptr)
    , m_liveBox(nullptr)
    , m_forcePlot(nullptr)
    , m_lblCurrentForce(nullptr)
    , m_lblStatus(nullptr)
    , m_forceBar(nullptr)
    , m_lblSeriesInfo(nullptr)
    , m_seriesProgress(nullptr)
    , m_statsBox(nullptr)
    , m_statsTable(nullptr)
    , m_rawBox(nullptr)
    , m_rawTable(nullptr)
    , m_timer(nullptr)
    , m_isMeasuring(false)
    , m_currentSeries(1)
    , m_currentRepInSeries(0)
    , m_totalReps(0)
    , m_sampleRate(100.0)  // 100 Hz
    , m_targetForce(50.0)  // 50 N domyślnie
    , m_sessionStartTime(0)
    , m_inContraction(false)
    , m_repStartForce(0)
    , m_repPeakForce(0)
    , m_repStartTime(0)
    , m_repPeakTime(0)
    , m_contractionThreshold(5.0)  // 5 N próg detekcji
    , m_trendsBox(nullptr)
    , m_trendsScrollArea(nullptr)
    , m_trendsContent(nullptr)
    , m_chkShowTrends(nullptr)
{
    // Inicjalizacja wskaźników przycisków JSON
    m_btnSaveJSON = nullptr;
    m_btnLoadJSON = nullptr;
    
    m_timer = new QTimer(this);
    m_timer->setInterval(10);  // 10 ms = 100 Hz
    connect(m_timer, &QTimer::timeout, this, &MeasurementTab::onTimerTick);
    
    setupUI();
    setupTrendsUI();
}

MeasurementTab::~MeasurementTab()
{
    // Qt automatycznie czyści widgety z parentem
}

void MeasurementTab::ensureCurrentSeriesInitialized()
{
    if (!m_completedSeriesStats.isEmpty()) {
        return;
    }

    SeriesStats initialSeries;
    initialSeries.seriesNumber = 1;
    m_completedSeriesStats.append(initialSeries);
}

void MeasurementTab::ensureSessionClockStarted()
{
    if (m_sessionStartTime <= 0.0) {
        m_sessionStartTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
    }
}

void MeasurementTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Sekcja kontrolna
    m_controlLayout = new QHBoxLayout();
    
    m_btnStartStop = new QPushButton("START POMIARU", this);
    m_btnStartStop->setMinimumHeight(40);
    m_btnStartStop->setFont(QFont("Arial", 12, QFont::Bold));
    m_btnStartStop->setStyleSheet("background-color: #4CAF50; color: white;");
    connect(m_btnStartStop, &QPushButton::clicked, this, &MeasurementTab::onToggleMeasurement);
    
    m_btnReadManual = new QPushButton("WCZYTAJ POJEDYNCZO", this);
    m_btnReadManual->setMinimumHeight(40);
    connect(m_btnReadManual, &QPushButton::clicked, this, &MeasurementTab::onReadSingleSample);
    
    m_btnSave = new QPushButton("ZAPISZ CSV", this);
    connect(m_btnSave, &QPushButton::clicked, this, &MeasurementTab::onSaveData);
    
    m_btnLoad = new QPushButton("WCZYTAJ CSV", this);
    connect(m_btnLoad, &QPushButton::clicked, this, &MeasurementTab::onLoadData);
    
    m_btnSaveJSON = new QPushButton("ZAPISZ POMIAR JSON", this);
    connect(m_btnSaveJSON, &QPushButton::clicked, this, &MeasurementTab::onSaveMeasurementJSON);
    
    m_btnLoadJSON = new QPushButton("OTWÓRZ POMIAR JSON", this);
    connect(m_btnLoadJSON, &QPushButton::clicked, this, &MeasurementTab::onLoadMeasurementJSON);
    
    m_btnReset = new QPushButton("RESET SESJI", this);
    connect(m_btnReset, &QPushButton::clicked, this, [this]() { resetSession(); });
    
    m_chkShowTrends = new QCheckBox("POKAŻ TRENDY DŁUGOTERMINOWE", this);
    m_chkShowTrends->setFont(QFont("Arial", 10, QFont::Bold));
    connect(m_chkShowTrends, &QCheckBox::toggled, this, &MeasurementTab::onShowTrends);
    
    m_controlLayout->addWidget(m_btnStartStop);
    m_controlLayout->addWidget(m_btnReadManual);
    m_controlLayout->addWidget(m_btnSave);
    m_controlLayout->addWidget(m_btnLoad);
    m_controlLayout->addWidget(m_btnSaveJSON);
    m_controlLayout->addWidget(m_btnLoadJSON);
    m_controlLayout->addWidget(m_chkShowTrends);
    m_controlLayout->addWidget(m_btnReset);
    
    m_mainLayout->addLayout(m_controlLayout);
    
    // Panel żywy z wykresem
    m_liveBox = new QGroupBox("Podgląd Na Żywo", this);
    QVBoxLayout* liveLayout = new QVBoxLayout();
    
    m_forcePlot = new ForcePlotWidget(this);
    m_forcePlot->setTargetForce(m_targetForce);
    m_forcePlot->setMaxTimeWindow(10.0);
    
    m_lblCurrentForce = new QLabel("Aktualna siła: 0.0 N", this);
    m_lblCurrentForce->setFont(QFont("Arial", 16, QFont::Bold));
    m_lblCurrentForce->setAlignment(Qt::AlignCenter);
    
    m_forceBar = new QProgressBar(this);
    m_forceBar->setRange(0, 100);
    m_forceBar->setValue(0);
    m_forceBar->setFormat("%v N");
    
    m_lblStatus = new QLabel("Status: GOTOWY", this);
    m_lblStatus->setFont(QFont("Arial", 11));
    m_lblStatus->setStyleSheet("color: blue; font-weight: bold;");
    
    m_lblSeriesInfo = new QLabel("Seria: 0/3 | Powtórzenia: 0/8", this);
    m_lblSeriesInfo->setFont(QFont("Arial", 11));
    
    m_seriesProgress = new QProgressBar(this);
    m_seriesProgress->setRange(0, 24);  // 3 serie x 8 powtórzeń
    m_seriesProgress->setValue(0);
    m_seriesProgress->setFormat("Postęp: %v/24");
    
    liveLayout->addWidget(m_forcePlot);
    liveLayout->addWidget(m_lblCurrentForce);
    liveLayout->addWidget(m_forceBar);
    liveLayout->addWidget(m_lblStatus);
    liveLayout->addWidget(m_lblSeriesInfo);
    liveLayout->addWidget(m_seriesProgress);
    
    m_liveBox->setLayout(liveLayout);
    m_mainLayout->addWidget(m_liveBox);
    
    // Tabela statystyk
    m_statsBox = new QGroupBox("Statystyki Serii i Powtórzeń", this);
    QVBoxLayout* statsLayout = new QVBoxLayout();
    
    m_statsTable = new QTableWidget(this);
    m_statsTable->setColumnCount(12);
    m_statsTable->setHorizontalHeaderLabels({
        "Seria", "Powt.", "Szczyt [N]", "Czas do szczytu [s]", 
        "Rise Rate [N/s]", "Fall Rate [N/s]", "Top 8% [s]", "Bottom 8% [s]",
        "Impuls [N·s]", "Średnia [N]", "Mediana [N]", "Dominanta [N]"
    });
    m_statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_statsTable->setAlternatingRowColors(true);
    
    statsLayout->addWidget(m_statsTable);
    m_statsBox->setLayout(statsLayout);
    m_mainLayout->addWidget(m_statsBox);
    
    // Tabela surowych danych
    m_rawBox = new QGroupBox("Surowe Dane (ostatnie 100 próbek)", this);
    QVBoxLayout* rawLayout = new QVBoxLayout();
    
    m_rawTable = new QTableWidget(this);
    m_rawTable->setColumnCount(2);
    m_rawTable->setHorizontalHeaderLabels({"Czas [s]", "Siła [N]"});
    m_rawTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_rawTable->setMaximumHeight(150);
    
    rawLayout->addWidget(m_rawTable);
    m_rawBox->setLayout(rawLayout);
    m_mainLayout->addWidget(m_rawBox);
}

void MeasurementTab::setupTrendsUI()
{
    m_trendsBox = new QGroupBox("Trendy Długoterminowe", this);
    m_trendsBox->setVisible(false);
    
    QVBoxLayout* trendsLayout = new QVBoxLayout();
    
    m_trendsScrollArea = new QScrollArea(this);
    m_trendsScrollArea->setWidgetResizable(true);
    m_trendsScrollArea->setMinimumHeight(300);
    
    m_trendsContent = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(m_trendsContent);
    
    QLabel* lblInfo = new QLabel("Wykresy trendów długoterminowych dla pacjenta: " + m_patientId, this);
    lblInfo->setFont(QFont("Arial", 11, QFont::Bold));
    contentLayout->addWidget(lblInfo);
    
    m_trendsScrollArea->setWidget(m_trendsContent);
    trendsLayout->addWidget(m_trendsScrollArea);
    
    m_trendsBox->setLayout(trendsLayout);
    m_mainLayout->addWidget(m_trendsBox);
}

void MeasurementTab::setPatientId(const QString& id)
{
    m_patientId = id;
}

void MeasurementTab::setTargetForce(double force)
{
    m_targetForce = force;
    m_forcePlot->setTargetForce(force);
}

bool MeasurementTab::isMeasurementActive() const
{
    return m_isMeasuring;
}

void MeasurementTab::startMeasurement()
{
    if (m_isMeasuring) return;

    ensureCurrentSeriesInitialized();
    m_isMeasuring = true;
    ensureSessionClockStarted();
    m_timer->start();
    
    m_btnStartStop->setText("STOP POMIARU");
    m_btnStartStop->setStyleSheet("background-color: #f44336; color: white;");
    m_lblStatus->setText("Status: REJESTRACJA...");
    m_lblStatus->setStyleSheet("color: red; font-weight: bold;");
    
    emit measurementStarted();
}

void MeasurementTab::stopMeasurement()
{
    if (!m_isMeasuring) return;
    
    m_timer->stop();
    m_isMeasuring = false;
    
    // Jeśli trwa powtórzenie, zakończ je
    if (m_inContraction && !m_currentRepForces.isEmpty()) {
        calculateRepetitionStats();
    }
    
    m_btnStartStop->setText("START POMIARU");
    m_btnStartStop->setStyleSheet("background-color: #4CAF50; color: white;");
    m_lblStatus->setText("Status: ZATRZYMANO");
    m_lblStatus->setStyleSheet("color: orange; font-weight: bold;");
    
    emit measurementStopped();
}

void MeasurementTab::resetSession()
{
    stopMeasurement();
    
    m_currentSeries = 1;
    m_currentRepInSeries = 0;
    m_totalReps = 0;
    m_sessionStartTime = 0;
    
    m_rawForceBuffer.clear();
    m_timeBuffer.clear();
    m_currentRepForces.clear();
    m_currentRepTimes.clear();
    m_completedSeriesStats.clear();
    
    m_inContraction = false;
    m_repStartForce = 0;
    m_repPeakForce = 0;
    m_repStartTime = 0;
    m_repPeakTime = 0;
    
    m_forcePlot->clearData();
    m_lblCurrentForce->setText("Aktualna siła: 0.0 N");
    m_forceBar->setValue(0);
    m_lblStatus->setText("Status: GOTOWY");
    m_lblStatus->setStyleSheet("color: blue; font-weight: bold;");
    m_lblSeriesInfo->setText("Seria: 0/3 | Powtórzenia: 0/8");
    m_seriesProgress->setValue(0);
    
    m_statsTable->setRowCount(0);
    m_rawTable->setRowCount(0);
}

void MeasurementTab::simulateSensorData()
{
    // Symulacja realistycznego sygnału z czujnika tensometrycznego
    // Pacjent wykonuje naciągnięcia lateksowej taśmy
    
    static double simTime = 0;
    static int simPhase = 0;  // 0: relaks, 1: narastanie, 2: szczyt, 3: opadanie
    static double peakTarget = 0;
    
    double dt = 1.0 / m_sampleRate;
    simTime += dt;
    
    double baseForce = 2.0;  // Siła spoczynkowa
    double noise = (QRandomGenerator::global()->bounded(100) - 50) / 50.0;  // ±1N szum
    
    if (!m_inContraction && QRandomGenerator::global()->bounded(100) < 5) {
        // Rozpocznij symulowane powtórzenie
        m_inContraction = true;
        m_repStartTime = simTime;
        m_repPeakForce = baseForce;
        peakTarget = m_targetForce * (0.7 + 0.3 * QRandomGenerator::global()->bounded(100) / 100.0);
        simPhase = 1;
    }
    
    double currentForce = baseForce;
    
    if (m_inContraction) {
        switch (simPhase) {
            case 1:  // Narastanie (0.3-0.6s)
                currentForce = baseForce + (peakTarget - baseForce) * ((simTime - m_repStartTime) / 0.4);
                if (currentForce >= peakTarget * 0.95) {
                    m_repPeakTime = simTime;
                    simPhase = 2;
                }
                break;
            case 2:  // Szczyt (0.2-0.4s)
                currentForce = peakTarget + noise * 2;
                if (simTime - m_repPeakTime > 0.3) {
                    simPhase = 3;
                }
                break;
            case 3:  // Opadanie (0.3-0.6s)
                currentForce = peakTarget * (1.0 - (simTime - m_repPeakTime) / 0.4);
                if (currentForce <= baseForce + 2) {
                    m_inContraction = false;
                    simPhase = 0;
                }
                break;
        }
    }
    
    currentForce += noise;
    if (currentForce < 0) currentForce = 0;
    
    readSingleSample(currentForce);
}

void MeasurementTab::readSingleSample(double forceValue)
{
    ensureCurrentSeriesInitialized();
    ensureSessionClockStarted();

    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0 - m_sessionStartTime;
    
    m_rawForceBuffer.append(forceValue);
    m_timeBuffer.append(currentTime);
    
    m_currentRepForces.append(forceValue);
    m_currentRepTimes.append(currentTime);
    
    emit newForceSample(forceValue, currentTime);
    
    updateLiveDisplay(forceValue);
    detectRepetitions();
    
    // Aktualizuj tabelę surowych danych (ostatnie 100)
    if (m_rawTable->rowCount() >= 100) {
        m_rawTable->removeRow(0);
    }
    int row = m_rawTable->rowCount();
    m_rawTable->insertRow(row);
    m_rawTable->setItem(row, 0, new QTableWidgetItem(QString::number(currentTime, 'f', 3)));
    m_rawTable->setItem(row, 1, new QTableWidgetItem(QString::number(forceValue, 'f', 2)));
    m_rawTable->scrollToBottom();
}

void MeasurementTab::updateLiveDisplay(double force)
{
    m_lblCurrentForce->setText(QString("Aktualna siła: %1 N").arg(force, 0, 'f', 1));
    m_forceBar->setValue(static_cast<int>(force));
    m_forcePlot->addSample(force, m_timeBuffer.last());
}

void MeasurementTab::detectRepetitions()
{
    if (m_currentRepForces.isEmpty()) return;
    
    double currentForce = m_currentRepForces.last();
    double currentTime = m_currentRepTimes.last();
    
    // Detekcja początku skurcza
    if (!m_inContraction && currentForce > m_contractionThreshold) {
        m_inContraction = true;
        m_repStartTime = currentTime;
        m_repStartForce = currentForce;
        m_repPeakForce = currentForce;
        m_repPeakTime = currentTime;
        m_currentRepForces.clear();
        m_currentRepTimes.clear();
        m_currentRepForces.append(currentForce);
        m_currentRepTimes.append(currentTime);
    }
    // Śledzenie szczytu
    else if (m_inContraction && currentForce > m_repPeakForce) {
        m_repPeakForce = currentForce;
        m_repPeakTime = currentTime;
    }
    // Detekcja końca skurcza
    else if (m_inContraction && currentForce < m_contractionThreshold * 0.5) {
        // Koniec powtórzenia
        calculateRepetitionStats();
        
        m_inContraction = false;
        m_currentRepForces.clear();
        m_currentRepTimes.clear();
    }
}

void MeasurementTab::calculateRepetitionStats()
{
    if (m_currentRepForces.size() < 2) return;
    
    m_currentRepStats.repNumber = m_currentRepInSeries + 1;
    m_currentRepStats.peakForce = m_repPeakForce;
    m_currentRepStats.timeToPeak = m_repPeakTime - m_repStartTime;
    
    // Rise rate (szybkość narastania)
    if (m_repPeakTime - m_repStartTime > 0) {
        m_currentRepStats.riseRate = (m_repPeakForce - m_repStartForce) / (m_repPeakTime - m_repStartTime);
    }
    
    // Fall rate (szybkość opadania) - znajdź punkt gdzie siła spada poniżej progu
    double fallEndTime = m_repPeakTime;
    for (int i = m_currentRepForces.size() - 1; i >= 0; --i) {
        if (m_currentRepForces[i] < m_repPeakForce * 0.1) {
            fallEndTime = m_currentRepTimes[i];
            break;
        }
    }
    if (fallEndTime - m_repPeakTime > 0) {
        m_currentRepStats.fallRate = (m_repPeakForce - m_currentRepForces.last()) / (fallEndTime - m_repPeakTime);
    }
    
    // Czas trwania
    m_currentRepStats.duration = m_currentRepTimes.last() - m_repStartTime;
    
    // Czas w górnych 8% siły
    double thresholdTop = m_repPeakForce * 0.92;
    double timeInTop = 0;
    for (int i = 1; i < m_currentRepForces.size(); ++i) {
        if (m_currentRepForces[i] >= thresholdTop && m_currentRepForces[i-1] >= thresholdTop) {
            timeInTop += m_currentRepTimes[i] - m_currentRepTimes[i-1];
        }
    }
    m_currentRepStats.timeInTop8pct = timeInTop;
    
    // Czas w dolnych 8% siły (relaksacja między powtórzeniami)
    double thresholdBottom = m_repPeakForce * 0.08;
    double timeInBottom = 0;
    for (int i = 1; i < m_currentRepForces.size(); ++i) {
        if (m_currentRepForces[i] <= thresholdBottom && m_currentRepForces[i-1] <= thresholdBottom) {
            timeInBottom += m_currentRepTimes[i] - m_currentRepTimes[i-1];
        }
    }
    m_currentRepStats.timeInBottom8pct = timeInBottom;
    
    // Pole pod krzywą (impuls)
    m_currentRepStats.areaUnderCurve = calculateAreaUnderCurve(m_currentRepForces, m_currentRepTimes);
    
    // Średnia siła
    double sum = 0;
    for (double f : m_currentRepForces) sum += f;
    m_currentRepStats.avgForce = sum / m_currentRepForces.size();
    
    // Mediana
    QVector<double> sortedForces = m_currentRepForces;
    std::sort(sortedForces.begin(), sortedForces.end());
    m_currentRepStats.medianForce = calculateMedian(sortedForces);
    
    // Dominanta (najczęstsza wartość w histogramie)
    m_currentRepStats.modeForce = calculateMode(m_currentRepForces);
    
    m_currentRepStats.isValid = true;
    
    // Dodaj do bieżącej serii
    SeriesStats& currentSeries = m_completedSeriesStats.last();
    currentSeries.reps.append(m_currentRepStats);
    
    // Emituj sygnał
    emit repetitionCompleted(m_currentSeries, m_currentRepStats.repNumber, m_currentRepStats);
    
    // Aktualizuj tabelę
    updateStatsTable();
    
    // Sprawdź czy seria zakończona
    m_currentRepInSeries++;
    if (m_currentRepInSeries >= 8) {
        // Analiza serii
        currentSeries.avgPeakForce = 0;
        currentSeries.totalWork = 0;
        currentSeries.avgRiseRate = 0;
        currentSeries.avgFallRate = 0;
        
        QVector<double> peakForces;
        for (const auto& rep : currentSeries.reps) {
            currentSeries.avgPeakForce += rep.peakForce;
            currentSeries.totalWork += rep.areaUnderCurve;
            currentSeries.avgRiseRate += rep.riseRate;
            currentSeries.avgFallRate += rep.fallRate;
            peakForces.append(rep.peakForce);
        }
        
        currentSeries.avgPeakForce /= currentSeries.reps.size();
        currentSeries.avgRiseRate /= currentSeries.reps.size();
        currentSeries.avgFallRate /= currentSeries.reps.size();
        currentSeries.medianPeakForce = calculateMedian(peakForces);
        currentSeries.impulseTotal = currentSeries.totalWork;
        
        // Wskaźnik zmęczenia (spadek siły względem pierwszego powtórzenia)
        if (currentSeries.reps.size() >= 2 && currentSeries.reps.first().peakForce > 0) {
            double lastPeak = currentSeries.reps.last().peakForce;
            double firstPeak = currentSeries.reps.first().peakForce;
            currentSeries.fatigueIndex = ((firstPeak - lastPeak) / firstPeak) * 100.0;
        }
        
        // Szybkość serii
        double seriesDuration = currentSeries.reps.last().duration - currentSeries.reps.first().duration;
        if (seriesDuration > 0) {
            currentSeries.seriesSpeed = currentSeries.reps.size() / seriesDuration;
        }
        
        // Wskaźnik powtarzalności (odwrotność odchylenia standardowego)
        double variance = 0;
        for (double pf : peakForces) {
            variance += (pf - currentSeries.avgPeakForce) * (pf - currentSeries.avgPeakForce);
        }
        variance /= peakForces.size();
        double stdDev = std::sqrt(variance);
        currentSeries.consistencyScore = std::max(0.0, 100.0 - (stdDev / currentSeries.avgPeakForce) * 100.0);
        
        emit seriesCompleted(m_currentSeries, currentSeries);
        
        // Przejdź do następnej serii lub zakończ sesję
        m_currentRepInSeries = 0;
        m_currentSeries++;
        
        if (m_currentSeries > 3) {
            m_lblStatus->setText("Status: SESJA ZAKOŃCZONA");
            m_lblStatus->setStyleSheet("color: green; font-weight: bold;");
            emit sessionFinished(m_completedSeriesStats);
        } else {
            m_lblStatus->setText(QString("Status: PRZERWA - ZA CHWILĘ SERIA %1").arg(m_currentSeries));
        }
    }
    
    updateSeriesProgress();
}

void MeasurementTab::updateStatsTable()
{
    m_statsTable->setRowCount(m_statsTable->rowCount() + 1);
    int row = m_statsTable->rowCount() - 1;
    
    const auto& stats = m_currentRepStats;
    
    m_statsTable->setItem(row, 0, new QTableWidgetItem(QString::number(m_currentSeries)));
    m_statsTable->setItem(row, 1, new QTableWidgetItem(QString::number(stats.repNumber)));
    m_statsTable->setItem(row, 2, new QTableWidgetItem(QString::number(stats.peakForce, 'f', 2)));
    m_statsTable->setItem(row, 3, new QTableWidgetItem(QString::number(stats.timeToPeak, 'f', 3)));
    m_statsTable->setItem(row, 4, new QTableWidgetItem(QString::number(stats.riseRate, 'f', 2)));
    m_statsTable->setItem(row, 5, new QTableWidgetItem(QString::number(stats.fallRate, 'f', 2)));
    m_statsTable->setItem(row, 6, new QTableWidgetItem(QString::number(stats.timeInTop8pct, 'f', 3)));
    m_statsTable->setItem(row, 7, new QTableWidgetItem(QString::number(stats.timeInBottom8pct, 'f', 3)));
    m_statsTable->setItem(row, 8, new QTableWidgetItem(QString::number(stats.areaUnderCurve, 'f', 2)));
    m_statsTable->setItem(row, 9, new QTableWidgetItem(QString::number(stats.avgForce, 'f', 2)));
    m_statsTable->setItem(row, 10, new QTableWidgetItem(QString::number(stats.medianForce, 'f', 2)));
    m_statsTable->setItem(row, 11, new QTableWidgetItem(QString::number(stats.modeForce, 'f', 2)));
    
    m_statsTable->scrollToBottom();
}

void MeasurementTab::updateSeriesProgress()
{
    int totalReps = (m_currentSeries - 1) * 8 + m_currentRepInSeries;
    m_seriesProgress->setValue(totalReps);
    m_lblSeriesInfo->setText(QString("Seria: %1/3 | Powtórzenia: %2/8")
                             .arg(qMin(m_currentSeries, 3))
                             .arg(m_currentRepInSeries));
}

double MeasurementTab::calculateMedian(const QVector<double>& values) const
{
    if (values.isEmpty()) return 0;
    
    QVector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    
    int n = sorted.size();
    if (n % 2 == 0) {
        return (sorted[n/2 - 1] + sorted[n/2]) / 2.0;
    } else {
        return sorted[n/2];
    }
}

double MeasurementTab::calculateMode(const QVector<double>& values) const
{
    if (values.isEmpty()) return 0;
    
    // Histogram z 10 przedziałami
    double minVal = *std::min_element(values.begin(), values.end());
    double maxVal = *std::max_element(values.begin(), values.end());
    double range = maxVal - minVal;
    if (range <= 0) return minVal;
    
    double binSize = range / 10.0;
    QMap<int, int> histogram;
    
    for (double v : values) {
        int bin = static_cast<int>((v - minVal) / binSize);
        histogram[bin]++;
    }
    
    // Znajdź najczęstszy przedział
    int maxBin = 0;
    int maxCount = 0;
    for (auto it = histogram.begin(); it != histogram.end(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            maxBin = it.key();
        }
    }
    
    return minVal + maxBin * binSize + binSize / 2.0;
}

double MeasurementTab::calculateAreaUnderCurve(const QVector<double>& forces, 
                                               const QVector<double>& times) const
{
    if (forces.size() < 2 || times.size() < 2) return 0;
    
    double area = 0;
    for (int i = 1; i < forces.size(); ++i) {
        double dt = times[i] - times[i-1];
        double avgForce = (forces[i] + forces[i-1]) / 2.0;
        area += avgForce * dt;
    }
    
    return area;
}

SeriesStats MeasurementTab::analyzeSeries(int seriesIndex) const
{
    if (seriesIndex < 1 || seriesIndex > m_completedSeriesStats.size()) {
        return SeriesStats();
    }
    return m_completedSeriesStats[seriesIndex - 1];
}

QVector<SeriesStats> MeasurementTab::getAllSeriesStats() const
{
    return m_completedSeriesStats;
}

void MeasurementTab::onToggleMeasurement()
{
    if (m_isMeasuring) {
        stopMeasurement();
    } else {
        startMeasurement();
    }
}

void MeasurementTab::onReadSingleSample()
{
    // Symulacja odczytu z czujnika - w rzeczywistości tutaj byłoby czytanie z ADC/UART
    double simulatedForce = QRandomGenerator::global()->bounded(1000) / 10.0;  // 0-100 N
    readSingleSample(simulatedForce);
}

void MeasurementTab::onTimerTick()
{
    simulateSensorData();
}

void MeasurementTab::onSelectionChanged()
{
    // Placeholder dla przyszłej funkcjonalności
}

void MeasurementTab::onSaveData()
{
    QString filename = QFileDialog::getSaveFileName(this, "Zapisz dane sesji", 
                                                     "sesja_pomiarowa.csv", "CSV Files (*.csv)");
    if (filename.isEmpty()) return;
    
    exportToCSV(filename);
}

void MeasurementTab::onLoadData()
{
    QString filename = QFileDialog::getOpenFileName(this, "Wczytaj dane sesji",
                                                     "", "CSV Files (*.csv)");
    if (filename.isEmpty()) return;
    
    importFromCSV(filename);
}

void MeasurementTab::onSaveMeasurementJSON()
{
    if (m_completedSeriesStats.isEmpty()) {
        QMessageBox::warning(this, "Ostrzeżenie", "Brak danych pomiarowych do zapisu!");
        return;
    }
    
    if (m_patientId.isEmpty()) {
        QMessageBox::warning(this, "Ostrzeżenie", "Najpierw ustaw ID pacjenta!");
        return;
    }
    
    // Utwórz katalog dla pacjenta jeśli nie istnieje
    QString dataDir = QDir::currentPath() + "/data/patients/" + m_patientId;
    QDir dir;
    if (!dir.exists(dataDir)) {
        dir.mkpath(dataDir);
    }
    
    // Generuj nazwę pliku z datą i numerem sesji
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    int sessionNum = m_patientSessions.size() + 1;
    QString filename = dataDir + "/pomiar_" + timestamp + "_sesja" + QString::number(sessionNum) + ".json";
    
    exportToJSON(filename);
    
    // Dodaj do listy sesji pacjenta
    loadPatientTrends();
}

void MeasurementTab::onLoadMeasurementJSON()
{
    QString filename = QFileDialog::getOpenFileName(this, "Otwórz pomiar JSON",
                                                     "", "JSON Files (*.json)");
    if (filename.isEmpty()) return;
    
    importFromJSON(filename);
    
    // Po wczytaniu, zaktualizuj trendy
    if (!m_patientId.isEmpty()) {
        loadPatientTrends();
    }
}

bool MeasurementTab::exportToCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można zapisać pliku: " + filename);
        return false;
    }
    
    QTextStream out(&file);
    
    // Nagłówek
    out << "Pacjent,Seria,Powtorzenie,Szczyt_N,CzasDoSzczytu_s,RiseRate_N_s,FallRate_N_s,"
        << "Duration_s,TimeInTop8_s,TimeInBottom8_s,Impuls_Ns,AvgForce_N,MedianForce_N,ModeForce_N\n";
    
    // Dane
    for (const auto& series : m_completedSeriesStats) {
        for (const auto& rep : series.reps) {
            out << m_patientId << ","
                << series.seriesNumber << ","
                << rep.repNumber << ","
                << rep.peakForce << ","
                << rep.timeToPeak << ","
                << rep.riseRate << ","
                << rep.fallRate << ","
                << rep.duration << ","
                << rep.timeInTop8pct << ","
                << rep.timeInBottom8pct << ","
                << rep.areaUnderCurve << ","
                << rep.avgForce << ","
                << rep.medianForce << ","
                << rep.modeForce << "\n";
        }
    }
    
    file.close();
    QMessageBox::information(this, "Sukces", "Dane zapisane do: " + filename);
    return true;
}

bool MeasurementTab::importFromCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku: " + filename);
        return false;
    }
    
    resetSession();
    
    QTextStream in(&file);
    QString line = in.readLine();  // Nagłówek
    
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 14) {
            m_patientId = parts[0];
            
            int seriesNum = parts[1].toInt();
            int repNum = parts[2].toInt();
            
            // Utwórz serię jeśli nie istnieje
            while (m_completedSeriesStats.size() < seriesNum) {
                SeriesStats newSeries;
                newSeries.seriesNumber = m_completedSeriesStats.size() + 1;
                m_completedSeriesStats.append(newSeries);
            }
            
            RepetitionStats stats;
            stats.repNumber = repNum;
            stats.peakForce = parts[3].toDouble();
            stats.timeToPeak = parts[4].toDouble();
            stats.riseRate = parts[5].toDouble();
            stats.fallRate = parts[6].toDouble();
            stats.duration = parts[7].toDouble();
            stats.timeInTop8pct = parts[8].toDouble();
            stats.timeInBottom8pct = parts[9].toDouble();
            stats.areaUnderCurve = parts[10].toDouble();
            stats.avgForce = parts[11].toDouble();
            stats.medianForce = parts[12].toDouble();
            stats.modeForce = parts[13].toDouble();
            stats.isValid = true;
            
            m_completedSeriesStats[seriesNum - 1].reps.append(stats);
        }
    }
    
    file.close();
    
    // Odśwież widok
    m_statsTable->setRowCount(0);
    for (const auto& series : m_completedSeriesStats) {
        for (const auto& rep : series.reps) {
            m_currentRepStats = rep;
            m_currentSeries = series.seriesNumber;
            m_currentRepInSeries = rep.repNumber;
            updateStatsTable();
        }
    }
    
    updateSeriesProgress();
    QMessageBox::information(this, "Sukces", "Dane wczytane z: " + filename);
    return true;
}

bool MeasurementTab::exportToJSON(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można zapisać pliku: " + filename);
        return false;
    }
    
    MeasurementSession session;
    session.sessionId = QUuid::createUuid().toString();
    session.patientId = m_patientId;
    session.date = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    session.series = m_completedSeriesStats;
    
    QString json = serializeSessionToJSON(session);
    QTextStream out(&file);
    out << json;
    file.close();
    
    QMessageBox::information(this, "Sukces", "Dane sesji zapisane do: " + filename);
    return true;
}

bool MeasurementTab::importFromJSON(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku: " + filename);
        return false;
    }
    
    QTextStream in(&file);
    QString jsonContent = in.readAll();
    file.close();
    
    MeasurementSession session = deserializeSessionFromJSON(jsonContent);
    if (!session.sessionId.isEmpty()) {
        m_patientSessions.append(session);
        m_patientId = session.patientId;
        m_completedSeriesStats = session.series;
        
        // Odśwież widok
        m_statsTable->setRowCount(0);
        for (const auto& series : m_completedSeriesStats) {
            for (const auto& rep : series.reps) {
                m_currentRepStats = rep;
                m_currentSeries = series.seriesNumber;
                m_currentRepInSeries = rep.repNumber;
                updateStatsTable();
            }
        }
        updateSeriesProgress();
        QMessageBox::information(this, "Sukces", "Sesja wczytana z: " + filename);
        return true;
    }
    
    return false;
}

void MeasurementTab::loadPatientTrends()
{
    if (m_patientId.isEmpty()) {
        QMessageBox::warning(this, "Ostrzeżenie", "Najpierw ustaw ID pacjenta!");
        return;
    }
    
    QString dataDir = QDir::currentPath() + "/data/patients/" + m_patientId;
    QDir dir(dataDir);
    if (!dir.exists()) {
        QMessageBox::information(this, "Informacja", "Brak zapisanych sesji dla tego pacjenta.");
        return;
    }
    
    m_patientSessions.clear();
    QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    for (const QString& fileName : jsonFiles) {
        QString filePath = dataDir + "/" + fileName;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString jsonContent = in.readAll();
            file.close();
            
            MeasurementSession session = deserializeSessionFromJSON(jsonContent);
            if (!session.sessionId.isEmpty()) {
                m_patientSessions.append(session);
            }
        }
    }
    
    if (m_patientSessions.isEmpty()) {
        QMessageBox::information(this, "Informacja", "Brak zapisanych sesji JSON dla tego pacjenta.");
        return;
    }
    
    updateTrendsView();
}

void MeasurementTab::onShowTrends()
{
    bool showTrends = m_chkShowTrends->isChecked();
    m_trendsBox->setVisible(showTrends);
    
    if (showTrends) {
        loadPatientTrends();
    }
}

void MeasurementTab::updateTrendsView()
{
    if (!m_trendsContent) return;
    
    QLayout* layout = m_trendsContent->layout();
    if (layout) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
    }
    
    QLabel* lblInfo = new QLabel("Wczytano " + QString::number(m_patientSessions.size()) + 
                                  " sesji pomiarowych dla pacjenta: " + m_patientId, this);
    lblInfo->setFont(QFont("Arial", 11, QFont::Bold));
    layout->addWidget(lblInfo);
    
    QTableWidget* trendsTable = new QTableWidget(this);
    trendsTable->setColumnCount(6);
    trendsTable->setHorizontalHeaderLabels({
        "Data", "Seria", "Śr. Szczyt [N]", "Maks. Szczyt [N]", 
        "Wskaźnik Zmęczenia [%]", "Spójność [%]"
    });
    trendsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    trendsTable->setAlternatingRowColors(true);
    
    for (const auto& session : m_patientSessions) {
        for (const auto& series : session.series) {
            int row = trendsTable->rowCount();
            trendsTable->insertRow(row);
            
            trendsTable->setItem(row, 0, new QTableWidgetItem(session.date));
            trendsTable->setItem(row, 1, new QTableWidgetItem(QString::number(series.seriesNumber)));
            trendsTable->setItem(row, 2, new QTableWidgetItem(QString::number(series.avgPeakForce, 'f', 1)));
            
            double maxPeak = 0;
            for (const auto& rep : series.reps) {
                if (rep.peakForce > maxPeak) maxPeak = rep.peakForce;
            }
            trendsTable->setItem(row, 3, new QTableWidgetItem(QString::number(maxPeak, 'f', 1)));
            trendsTable->setItem(row, 4, new QTableWidgetItem(QString::number(series.fatigueIndex, 'f', 1)));
            trendsTable->setItem(row, 5, new QTableWidgetItem(QString::number(series.consistencyScore, 'f', 1)));
        }
    }
    
    layout->addWidget(trendsTable);
    
    QGroupBox* plotGroup = new QGroupBox("Wykres Trendów - Siła Szczytowa", this);
    QVBoxLayout* plotLayout = new QVBoxLayout();
    
    TrendsPlotWidget* trendsPlot = new TrendsPlotWidget(this);
    trendsPlot->setMinimumHeight(250);
    
    QVector<double> dates;
    QVector<double> avgForces;
    QVector<double> maxForces;
    
    int idx = 0;
    for (const auto& session : m_patientSessions) {
        for (const auto& series : session.series) {
            dates.append(idx++);
            avgForces.append(series.avgPeakForce);
            
            double maxPeak = 0;
            for (const auto& rep : series.reps) {
                if (rep.peakForce > maxPeak) maxPeak = rep.peakForce;
            }
            maxForces.append(maxPeak);
        }
    }
    
    trendsPlot->setData(dates, avgForces, maxForces);
    plotLayout->addWidget(trendsPlot);
    plotGroup->setLayout(plotLayout);
    layout->addWidget(plotGroup);
}

QString MeasurementTab::serializeSessionToJSON(const MeasurementSession& session) const
{
    QJsonObject root;
    root["sessionId"] = session.sessionId;
    root["patientId"] = session.patientId;
    root["date"] = session.date;
    
    QJsonArray seriesArray;
    for (const auto& series : session.series) {
        QJsonObject seriesObj;
        seriesObj["seriesNumber"] = series.seriesNumber;
        seriesObj["avgPeakForce"] = series.avgPeakForce;
        seriesObj["medianPeakForce"] = series.medianPeakForce;
        seriesObj["fatigueIndex"] = series.fatigueIndex;
        seriesObj["totalWork"] = series.totalWork;
        seriesObj["avgRiseRate"] = series.avgRiseRate;
        seriesObj["avgFallRate"] = series.avgFallRate;
        seriesObj["seriesSpeed"] = series.seriesSpeed;
        seriesObj["consistencyScore"] = series.consistencyScore;
        seriesObj["impulseTotal"] = series.impulseTotal;
        
        QJsonArray repsArray;
        for (const auto& rep : series.reps) {
            QJsonObject repObj;
            repObj["repNumber"] = rep.repNumber;
            repObj["peakForce"] = rep.peakForce;
            repObj["timeToPeak"] = rep.timeToPeak;
            repObj["riseRate"] = rep.riseRate;
            repObj["fallRate"] = rep.fallRate;
            repObj["duration"] = rep.duration;
            repObj["timeInTop8pct"] = rep.timeInTop8pct;
            repObj["timeInBottom8pct"] = rep.timeInBottom8pct;
            repObj["areaUnderCurve"] = rep.areaUnderCurve;
            repObj["avgForce"] = rep.avgForce;
            repObj["medianForce"] = rep.medianForce;
            repObj["modeForce"] = rep.modeForce;
            repObj["isValid"] = rep.isValid;
            repsArray.append(repObj);
        }
        seriesObj["repetitions"] = repsArray;
        seriesArray.append(seriesObj);
    }
    root["series"] = seriesArray;
    
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

MeasurementSession MeasurementTab::deserializeSessionFromJSON(const QString& json) const
{
    MeasurementSession session;
    
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull()) return session;
    
    QJsonObject root = doc.object();
    session.sessionId = root["sessionId"].toString();
    session.patientId = root["patientId"].toString();
    session.date = root["date"].toString();
    
    QJsonArray seriesArray = root["series"].toArray();
    for (const auto& seriesVal : seriesArray) {
        QJsonObject seriesObj = seriesVal.toObject();
        SeriesStats series;
        series.seriesNumber = seriesObj["seriesNumber"].toInt();
        series.avgPeakForce = seriesObj["avgPeakForce"].toDouble();
        series.medianPeakForce = seriesObj["medianPeakForce"].toDouble();
        series.fatigueIndex = seriesObj["fatigueIndex"].toDouble();
        series.totalWork = seriesObj["totalWork"].toDouble();
        series.avgRiseRate = seriesObj["avgRiseRate"].toDouble();
        series.avgFallRate = seriesObj["avgFallRate"].toDouble();
        series.seriesSpeed = seriesObj["seriesSpeed"].toDouble();
        series.consistencyScore = seriesObj["consistencyScore"].toDouble();
        series.impulseTotal = seriesObj["impulseTotal"].toDouble();
        
        QJsonArray repsArray = seriesObj["repetitions"].toArray();
        for (const auto& repVal : repsArray) {
            QJsonObject repObj = repVal.toObject();
            RepetitionStats rep;
            rep.repNumber = repObj["repNumber"].toInt();
            rep.peakForce = repObj["peakForce"].toDouble();
            rep.timeToPeak = repObj["timeToPeak"].toDouble();
            rep.riseRate = repObj["riseRate"].toDouble();
            rep.fallRate = repObj["fallRate"].toDouble();
            rep.duration = repObj["duration"].toDouble();
            rep.timeInTop8pct = repObj["timeInTop8pct"].toDouble();
            rep.timeInBottom8pct = repObj["timeInBottom8pct"].toDouble();
            rep.areaUnderCurve = repObj["areaUnderCurve"].toDouble();
            rep.avgForce = repObj["avgForce"].toDouble();
            rep.medianForce = repObj["medianForce"].toDouble();
            rep.modeForce = repObj["modeForce"].toDouble();
            rep.isValid = repObj["isValid"].toBool();
            series.reps.append(rep);
        }
        session.series.append(series);
    }
    
    return session;
}

} // namespace tab
