#include "tab/MeasurementTab.hpp"
#include "measurement/Units.hpp"
#include "core/RuntimePaths.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QFileInfo>
#include <QSaveFile>
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
#include <memory>
#include <iostream>

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
    m_maxForce = 5.0;
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
    , m_maxForce(5.0)
    , m_maxObservedForce(0.0)
    , m_minObservedForce(0.0)
    , m_unit(ForceUnit::Newtons)
{
    setMinimumHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ForcePlotWidget::setUnit(ForceUnit unit)
{
    m_unit = unit;
    update();
}

QString ForcePlotWidget::getUnitLabel() const
{
    switch (m_unit) {
        case ForceUnit::Kilograms: return "kgf";
        case ForceUnit::Raw: return "ADC";
        default: return "N";
    }
}

void ForcePlotWidget::addSample(double force, double time)
{
    std::cout << "[DEBUG] ForcePlotWidget::addSample ENTER: force=" << force << " time=" << time 
              << " m_forces.size()=" << m_forces.size() << std::endl;
    
    m_forces.append(force);
    m_times.append(time);
    
    // Keep raw extrema separate from the display range. Applying the margin
    // to an already-expanded range on every sample caused exponential growth.
    m_maxObservedForce = std::max(m_maxObservedForce, force);
    m_minObservedForce = std::min(m_minObservedForce, force);
    const double observedMagnitude = std::max(
        std::abs(m_maxObservedForce), std::abs(m_minObservedForce));
    m_maxForce = std::max(5.0, observedMagnitude * 1.2);
    std::cout << "[DEBUG] ForcePlotWidget::addSample: Display range=" << m_maxForce << std::endl;
    
    // Usuń stare dane poza oknem czasowym
    while (!m_times.isEmpty() && (time - m_times.first()) > m_maxTimeWindow) {
        m_forces.removeFirst();
        m_times.removeFirst();
    }
    
    std::cout << "[DEBUG] ForcePlotWidget::addSample: Before update(), m_forces.size()=" << m_forces.size() << std::endl;
    update();
    std::cout << "[DEBUG] ForcePlotWidget::addSample EXIT" << std::endl;
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
    
    // Etykiety osi Y - teraz z zakresem od -m_maxForce do +m_maxForce
    painter.setFont(QFont("Arial", 8));
    painter.setPen(Qt::black);
    for (int i = 0; i <= 10; ++i) {
        int y = margin + plotHeight - (plotHeight * i / 10);
        double force = (-m_maxForce + (2.0 * m_maxForce * i / 10)) * (m_unit == ForceUnit::Kilograms ? 1.0 / 9.80665 : 1.0);
        QString label = QString::number(force, 'f', m_unit == ForceUnit::Kilograms ? 2 : 0);
        painter.drawText(5, y + 3, label);
    }
    
    // Narysuj linię zera
    painter.setPen(QPen(QColor(150, 150, 150), 1, Qt::DashLine));
    int zeroY = margin + plotHeight / 2;
    painter.drawLine(margin, zeroY, margin + plotWidth, zeroY);
    
    // Linia docelowej siły
    if (m_targetForce > 0 && m_maxForce > 0) {
        int targetY = margin + plotHeight - static_cast<int>((m_targetForce / m_maxForce) * plotHeight);
        painter.setPen(QPen(QColor(255, 100, 100), 2, Qt::DashLine));
        painter.drawLine(margin, targetY, margin + plotWidth, targetY);
        painter.setPen(QColor(255, 100, 100));
        painter.drawText(margin + plotWidth - 60, targetY - 5, QString("Cel [%1]").arg(getUnitLabel()));
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
        // Skalowanie z zakresem symetrycznym: -m_maxForce do +m_maxForce
        double y1 = margin + plotHeight - ((m_forces[i-1] + m_maxForce) / (2.0 * m_maxForce)) * plotHeight;
        double x2 = margin + ((m_times[i] - minTime) / timeRange) * plotWidth;
        double y2 = margin + plotHeight - ((m_forces[i] + m_maxForce) / (2.0 * m_maxForce)) * plotHeight;
        
        painter.drawLine(static_cast<int>(x1), static_cast<int>(y1), 
                        static_cast<int>(x2), static_cast<int>(y2));
    }
    
    // Wypełnienie pod wykresem - teraz względem linii zera
    painter.setBrush(QBrush(QColor(0, 100, 200, 50)));
    painter.setPen(Qt::NoPen);
    
    QVector<QPoint> fillPoints;
    // Zaczynamy od linii zera (używamy istniejącej zmiennej zeroY zdefiniowanej wcześniej)
    fillPoints.append(QPoint(margin, zeroY));
    for (int i = 0; i < m_forces.size(); ++i) {
        double x = margin + ((m_times[i] - minTime) / timeRange) * plotWidth;
        double y = margin + plotHeight - ((m_forces[i] + m_maxForce) / (2.0 * m_maxForce)) * plotHeight;
        fillPoints.append(QPoint(static_cast<int>(x), static_cast<int>(y)));
    }
    fillPoints.append(QPoint(margin + plotWidth, zeroY));
    
    painter.drawPolygon(fillPoints);
    
    // Etykiety osi X
    painter.setPen(Qt::black);
    painter.drawText(margin + plotWidth / 2 - 20, height - 15, "Czas [s]");
    painter.drawText(5, margin - 5, QString("Siła [%1]").arg(getUnitLabel()));
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
    , m_showRawValues(false)  // Domyślnie analizujemy skalibrowaną siłę
    , m_currentUnit(ForceUnit::Newtons)  // Domyślna jednostka: Newtony
    , m_unitSelector(nullptr)
    , m_trendsBox(nullptr)
    , m_trendsScrollArea(nullptr)
    , m_trendsContent(nullptr)
    , m_chkShowTrends(nullptr)
    , m_serialPort(nullptr)
{
    // Inicjalizacja wskaźników przycisków JSON
    m_btnSaveJSON = nullptr;
    m_btnLoadJSON = nullptr;
    
    // Inicjalizacja połączenia szeregowego
    m_serialPort = std::make_shared<sensor::SerialCommunication>();
    m_measurementEngine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    m_measurementAdapter = std::make_unique<sensor::MeasurementSerialAdapter>(
        *m_serialPort, m_measurementEngine, "desktop-hx711", "g");
    m_lastSensorData = sensor::SensorData();
    
    // Device discovery and connection are owned by MainWindow's explicit device controls.
    // This widget never starts a detached background connection worker.
    // MeasurementTab nie posiada własnego terminala debugowego.
    // Wszystkie wiadomości debugowe są wysyłane do DebugManager,
    // który przekazuje je do centralnej zakładki DebugTab.
    
    setupUI();
    setupTrendsUI();
}

MeasurementTab::~MeasurementTab()
{
    m_measurementAdapter.reset();
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
}

void MeasurementTab::ensureCurrentSeriesInitialized()
{
    std::cout << "[DEBUG] ensureCurrentSeriesInitialized START: m_completedSeriesStats.size()=" 
              << m_completedSeriesStats.size() << std::endl;
    
    if (!m_completedSeriesStats.isEmpty()) {
        std::cout << "[DEBUG] ensureCurrentSeriesInitialized END: already initialized" << std::endl;
        return;
    }

    SeriesStats initialSeries;
    initialSeries.seriesNumber = 1;
    m_completedSeriesStats.append(initialSeries);
    std::cout << "[DEBUG] ensureCurrentSeriesInitialized END: created initial series, size now=" 
              << m_completedSeriesStats.size() << std::endl;
}

void MeasurementTab::ensureSessionClockStarted()
{
    std::cout << "[DEBUG] ensureSessionClockStarted: m_sessionStartTime=" << m_sessionStartTime << std::endl;
    
    if (m_sessionStartTime <= 0.0) {
        m_sessionStartTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;
        std::cout << "[DEBUG] ensureSessionClockStarted: Started clock, m_sessionStartTime=" << m_sessionStartTime << std::endl;
    } else {
        std::cout << "[DEBUG] ensureSessionClockStarted: Clock already running" << std::endl;
    }
}

void MeasurementTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Sekcja kontrolna
    m_controlLayout = new QHBoxLayout();
    
    m_btnStartStop = new QPushButton("START POMIARU", this);
    m_btnStartStop->setMinimumHeight(45);
    m_btnStartStop->setFont(QFont("Arial", 12, QFont::Bold));
    m_btnStartStop->setStyleSheet("background-color: #4CAF50; color: white;");
    connect(m_btnStartStop, &QPushButton::clicked, this, &MeasurementTab::onToggleMeasurement);
    
    m_btnReadManual = new QPushButton("WCZYTAJ POJEDYNCZO", this);
    m_btnReadManual->setMinimumHeight(35);
    m_btnReadManual->setFont(QFont("Arial", 9));
    connect(m_btnReadManual, &QPushButton::clicked, this, &MeasurementTab::onReadSingleSample);
    
    m_btnSave = new QPushButton("ZAPISZ CSV", this);
    m_btnSave->setMinimumHeight(35);
    m_btnSave->setFont(QFont("Arial", 9));
    connect(m_btnSave, &QPushButton::clicked, this, &MeasurementTab::onSaveData);
    
    m_btnLoad = new QPushButton("WCZYTAJ CSV", this);
    m_btnLoad->setMinimumHeight(35);
    m_btnLoad->setFont(QFont("Arial", 9));
    connect(m_btnLoad, &QPushButton::clicked, this, &MeasurementTab::onLoadData);
    
    m_btnSaveJSON = new QPushButton("ZAPISZ POMIAR JSON", this);
    m_btnSaveJSON->setMinimumHeight(35);
    m_btnSaveJSON->setFont(QFont("Arial", 9));
    connect(m_btnSaveJSON, &QPushButton::clicked, this, &MeasurementTab::onSaveMeasurementJSON);
    
    m_btnLoadJSON = new QPushButton("OTWÓRZ POMIAR JSON", this);
    m_btnLoadJSON->setMinimumHeight(35);
    m_btnLoadJSON->setFont(QFont("Arial", 9));
    connect(m_btnLoadJSON, &QPushButton::clicked, this, &MeasurementTab::onLoadMeasurementJSON);
    
    // Przełącznik RAW jest dostępny przed pierwszą próbką; tryb jest blokowany, aby nie mieszać jednostek w jednej sesji.
    m_btnToggleRaw = new QPushButton("TRYB: WARTOŚCI RAW", this);
    m_btnToggleRaw->setCheckable(true);
    m_btnToggleRaw->setChecked(false); // Domyślnie skalibrowana siła
    m_btnToggleRaw->setEnabled(true);  // Tryb diagnostyczny RAW dostępny poza pomiarem
    m_btnToggleRaw->setMinimumHeight(35);
    m_btnToggleRaw->setFont(QFont("Arial", 9));
    connect(m_btnToggleRaw, &QPushButton::toggled, this, [this](bool checked) {
        m_showRawValues = checked;
        if (m_forcePlot)
            m_forcePlot->setUnit(checked ? ForceUnit::Raw : m_currentUnit);
        if (m_unitSelector) m_unitSelector->setEnabled(!checked);
        if (checked) {
            m_btnToggleRaw->setText("TRYB: WARTOŚCI RAW");
            m_btnToggleRaw->setStyleSheet("background-color: #FF9800; color: white;");
        } else {
            m_btnToggleRaw->setText("TRYB: WARTOŚCI SKALIBROWANE");
            m_btnToggleRaw->setStyleSheet("background-color: #2196F3; color: white;");
        }
        updateUnitDisplay();
        updateRawTableUnits();
    });
    m_btnToggleRaw->setStyleSheet("background-color: #2196F3; color: white;");
    
    m_btnReset = new QPushButton("RESET SESJI", this);
    m_btnReset->setMinimumHeight(35);
    m_btnReset->setFont(QFont("Arial", 9));
    connect(m_btnReset, &QPushButton::clicked, this, [this]() { resetSession(); });
    
    // Wybór jednostki siły
    m_unitSelector = new QComboBox(this);
    m_unitSelector->addItem("Newtony (N)", static_cast<int>(ForceUnit::Newtons));
    m_unitSelector->addItem("Kilogram-siła (kgf)", static_cast<int>(ForceUnit::Kilograms));
    m_unitSelector->setCurrentIndex(0);  // Domyślnie Newtony
    m_unitSelector->setMinimumHeight(35);
    m_unitSelector->setFont(QFont("Arial", 10));
    connect(m_unitSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        int unitValue = m_unitSelector->itemData(index).toInt();
        setCurrentUnit(static_cast<ForceUnit>(unitValue));
    });
    
    m_chkShowTrends = new QCheckBox("POKAŻ TRENDY DŁUGOTERMINOWE", this);
    m_chkShowTrends->setFont(QFont("Arial", 10, QFont::Bold));
    m_chkShowTrends->setMinimumHeight(35);
    connect(m_chkShowTrends, &QCheckBox::toggled, this, &MeasurementTab::onShowTrends);
    
    m_controlLayout->addWidget(m_btnStartStop);
    m_controlLayout->addWidget(m_btnReadManual);
    m_controlLayout->addWidget(m_btnSave);
    m_controlLayout->addWidget(m_btnLoad);
    m_controlLayout->addWidget(m_btnSaveJSON);
    m_controlLayout->addWidget(m_btnLoadJSON);
    m_controlLayout->addWidget(m_btnToggleRaw);
    m_controlLayout->addWidget(m_unitSelector);
    m_controlLayout->addWidget(m_chkShowTrends);
    m_controlLayout->addWidget(m_btnReset);
    
    m_mainLayout->addLayout(m_controlLayout);
    
    // Panel żywy z wykresem
    m_liveBox = new QGroupBox("Podgląd Na Żywo", this);
    m_liveBox->setFont(QFont("Arial", 11, QFont::Bold));
    QVBoxLayout* liveLayout = new QVBoxLayout();
    liveLayout->setSpacing(10);
    
    m_forcePlot = new ForcePlotWidget(this);
    m_forcePlot->setTargetForce(m_targetForce);
    m_forcePlot->setMaxTimeWindow(10.0);
    m_forcePlot->setMinimumHeight(250);
    
    m_lblCurrentForce = new QLabel("Aktualna siła: 0.0 N", this);
    m_lblCurrentForce->setFont(QFont("Arial", 16, QFont::Bold));
    m_lblCurrentForce->setAlignment(Qt::AlignCenter);
    m_lblCurrentForce->setMinimumHeight(30);
    
    m_forceBar = new QProgressBar(this);
    m_forceBar->setRange(-100, 100);  // Zakres od -100 do +100 dla wartości ujemnych i dodatnich
    m_forceBar->setValue(0);
    m_forceBar->setFormat("%v N");
    m_forceBar->setMinimumHeight(25);
    
    m_lblStatus = new QLabel("Status: GOTOWY", this);
    m_lblStatus->setFont(QFont("Arial", 11, QFont::Bold));
    m_lblStatus->setStyleSheet("color: blue; font-weight: bold;");
    m_lblStatus->setMinimumHeight(25);
    
    m_lblSeriesInfo = new QLabel("Seria: 0/3 | Powtórzenia: 0/8", this);
    m_lblSeriesInfo->setFont(QFont("Arial", 11));
    m_lblSeriesInfo->setMinimumHeight(25);
    
    m_seriesProgress = new QProgressBar(this);
    m_seriesProgress->setRange(0, 24);  // 3 serie x 8 powtórzeń
    m_seriesProgress->setValue(0);
    m_seriesProgress->setFormat("Postęp: %v/24");
    m_seriesProgress->setMinimumHeight(25);
    
    liveLayout->addWidget(m_forcePlot);
    liveLayout->addWidget(m_lblCurrentForce);
    liveLayout->addWidget(m_forceBar);
    liveLayout->addWidget(m_lblStatus);
    liveLayout->addWidget(m_lblSeriesInfo);
    liveLayout->addWidget(m_seriesProgress);
    
    m_liveBox->setLayout(liveLayout);
    m_mainLayout->addWidget(m_liveBox);
    

    // MeasurementTab nie posiada własnego terminala debugowego.
    // Wszystkie wiadomości debugowe są wysyłane do centralnej zakładki DebugTab
    // poprzez DebugManager.
    
    // Tabela statystyk
    m_statsBox = new QGroupBox("Statystyki Serii i Powtórzeń", this);
    m_statsBox->setFont(QFont("Arial", 11, QFont::Bold));
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
    m_statsTable->setFont(QFont("Arial", 9));
    m_statsTable->setMinimumHeight(200);
    
    statsLayout->addWidget(m_statsTable);
    m_statsBox->setLayout(statsLayout);
    m_mainLayout->addWidget(m_statsBox);
    
    // Tabela surowych danych
    m_rawBox = new QGroupBox("Surowe Dane (ostatnie 100 próbek)", this);
    m_rawBox->setFont(QFont("Arial", 11, QFont::Bold));
    QVBoxLayout* rawLayout = new QVBoxLayout();
    
    m_rawTable = new QTableWidget(this);
    m_rawTable->setColumnCount(2);
    m_rawTable->setHorizontalHeaderLabels({"Czas [s]", "Siła [N]"});
    m_rawTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_rawTable->setMaximumHeight(150);
    m_rawTable->setFont(QFont("Arial", 9));
    
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

void MeasurementTab::onPatientChanged(const QString& pesel, const QString& /*patientDataPath*/)
{
    // Ustaw ID pacjenta
    m_patientId = pesel;
    
    // Zaktualizuj etykietę w trendach długoterminowych
    if (m_trendsContent) {
        QLabel* lblInfo = m_trendsContent->findChild<QLabel*>();
        if (lblInfo) {
            lblInfo->setText("Wykresy trendów długoterminowych dla pacjenta: " + pesel);
        }
    }
    
    // Automatycznie ładuj historyczne pomiary pacjenta
    if (!pesel.isEmpty()) {
        loadAllHistoricalMeasurements();
        loadPatientTrends();
    }
}

void MeasurementTab::loadAllHistoricalMeasurements()
{
    if (m_patientId.isEmpty()) {
        return;
    }
    
    QString dataDir = biofeedback::runtime::patientDirectory(m_patientId);
    QDir dir(dataDir);
    if (!dir.exists()) {
        return;
    }
    
    // Wczytaj wszystkie historyczne sesje JSON
    QStringList jsonFiles = dir.entryList(QStringList() << "pomiar_*.json", QDir::Files);
    
    for (const QString& fileName : jsonFiles) {
        QString filePath = dataDir + "/" + fileName;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString jsonContent = in.readAll();
            file.close();
            
            MeasurementSession session = deserializeSessionFromJSON(jsonContent);
            if (!session.sessionId.isEmpty()) {
                // Sprawdź czy sesja już istnieje (unikaj duplikatów)
                bool exists = false;
                for (const auto& existingSession : m_patientSessions) {
                    if (existingSession.sessionId == session.sessionId) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    m_patientSessions.append(session);
                }
            }
        }
    }
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
    std::cout << "[DEBUG] startMeasurement START: m_isMeasuring=" << m_isMeasuring << std::endl;
    
    if (m_isMeasuring) {
        std::cout << "[DEBUG] startMeasurement END: already measuring" << std::endl;
        return;
    }
    if (!m_serialPort || !m_serialPort->isConnected()) {
        QMessageBox::warning(this, tr("Brak urządzenia"),
                             tr("Połącz czujnik HX711 przed rozpoczęciem pomiaru."));
        return;
    }

    std::cout << "[DEBUG] startMeasurement: Calling ensureCurrentSeriesInitialized" << std::endl;
    ensureCurrentSeriesInitialized();
    m_inContraction = false;
    m_currentRepForces.clear();
    m_currentRepTimes.clear();
    m_isMeasuring = true;
    std::cout << "[DEBUG] startMeasurement: Calling ensureSessionClockStarted" << std::endl;
    ensureSessionClockStarted();
    
    std::cout << "[DEBUG] startMeasurement: Locking Raw/Calibrated button" << std::endl;
    
    // Zablokuj przycisk zmiany trybu Raw/Calibrated podczas pomiaru
    if (m_btnToggleRaw) {
        m_btnToggleRaw->setEnabled(false);
        m_btnToggleRaw->setStyleSheet("background-color: #808080; color: #C0C0C0;");  // Szary kolor
    }
    
    m_btnStartStop->setText("STOP POMIARU");
    m_btnStartStop->setStyleSheet("background-color: #f44336; color: white;");
    m_lblStatus->setText("Status: REJESTRACJA...");
    m_lblStatus->setStyleSheet("color: red; font-weight: bold;");
    
    std::cout << "[DEBUG] startMeasurement: emit measurementStarted" << std::endl;
    emit measurementStarted();
    
    std::cout << "[DEBUG] startMeasurement END" << std::endl;
}

void MeasurementTab::stopMeasurement()
{
    std::cout << "[DEBUG] stopMeasurement START: m_isMeasuring=" << m_isMeasuring 
              << " m_inContraction=" << m_inContraction 
              << " m_currentRepForces.size()=" << m_currentRepForces.size() << std::endl;
    
    if (!m_isMeasuring) {
        std::cout << "[DEBUG] stopMeasurement END: not measuring" << std::endl;
        return;
    }
    
    m_isMeasuring = false;
    
    // Jeśli trwa powtórzenie, zakończ je
    if (m_inContraction && !m_currentRepForces.isEmpty() && !m_showRawValues) {
        std::cout << "[DEBUG] stopMeasurement: Ending current repetition before stopping" << std::endl;
        calculateRepetitionStats();
    }
    m_inContraction = false;
    m_currentRepForces.clear();
    m_currentRepTimes.clear();

    std::cout << "[DEBUG] stopMeasurement: Unlocking Raw/Calibrated button" << std::endl;
    
    // Odblokuj przycisk zmiany trybu Raw/Calibrated po zatrzymaniu pomiaru
    if (m_btnToggleRaw) {
        m_btnToggleRaw->setEnabled(m_rawForceBuffer.isEmpty());
        // Przywróć odpowiedni kolor w zależności od stanu
        if (m_showRawValues) {
            m_btnToggleRaw->setStyleSheet("background-color: #FF9800; color: white;");
        } else {
            m_btnToggleRaw->setStyleSheet("background-color: #2196F3; color: white;");
        }
    }
    
    m_btnStartStop->setText("START POMIARU");
    m_btnStartStop->setStyleSheet("background-color: #4CAF50; color: white;");
    m_lblStatus->setText("Status: ZATRZYMANO");
    m_lblStatus->setStyleSheet("color: orange; font-weight: bold;");
    
    std::cout << "[DEBUG] stopMeasurement: emit measurementStopped" << std::endl;
    emit measurementStopped();
    
    std::cout << "[DEBUG] stopMeasurement END" << std::endl;
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
    if (m_btnToggleRaw) m_btnToggleRaw->setEnabled(true);
    if (m_unitSelector) m_unitSelector->setEnabled(!m_showRawValues);
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
    // Usunięto ograniczenie do 0 - pozwalamy na wartości ujemne dla rozciągania
    // if (currentForce < 0) currentForce = 0;
    
    readSingleSample(currentForce);
}

void MeasurementTab::readSingleSample(double forceValue)
{
    std::cout << "[DEBUG] readSingleSample START: forceValue=" << forceValue 
              << " m_completedSeriesStats.size()=" << m_completedSeriesStats.size() << std::endl;
    
    // Safety check - ensure series is initialized
    if (m_completedSeriesStats.isEmpty()) {
        std::cout << "[DEBUG] readSingleSample: No series initialized, calling ensureCurrentSeriesInitialized" << std::endl;
        ensureCurrentSeriesInitialized();
    }
    
    // Safety check - ensure session clock is started
    if (m_sessionStartTime <= 0.0) {
        std::cout << "[DEBUG] readSingleSample: Session clock not started, calling ensureSessionClockStarted" << std::endl;
        ensureSessionClockStarted();
    }

    std::cout << "[DEBUG] readSingleSample After checks: m_completedSeriesStats.size()=" 
              << m_completedSeriesStats.size() << " m_sessionStartTime=" << m_sessionStartTime << std::endl;

    double currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0 - m_sessionStartTime;
    
    std::cout << "[DEBUG] readSingleSample: currentTime=" << currentTime << std::endl;
    
    m_rawForceBuffer.append(forceValue);
    m_timeBuffer.append(currentTime);
    if (m_btnToggleRaw) m_btnToggleRaw->setEnabled(false);
    if (m_unitSelector) m_unitSelector->setEnabled(!m_showRawValues);

    // ADC counts are diagnostic only and must never be interpreted as newtons.
    if (!m_showRawValues) {
        m_currentRepForces.append(forceValue);
        m_currentRepTimes.append(currentTime);
    }
    
    std::cout << "[DEBUG] readSingleSample: Data prepared for display - rawForceBuffer.size()=" 
              << m_rawForceBuffer.size() << " currentRepForces.size()=" << m_currentRepForces.size() << std::endl;
    
    std::cout << "[DEBUG] readSingleSample Before emit newForceSample, force=" << forceValue 
              << " timestamp=" << currentTime << std::endl;
    
    try {
        // Emit signal with all three parameters to match the signal definition
        emit newForceSample(forceValue, currentTime, m_showRawValues);
        std::cout << "[DEBUG] readSingleSample: Signal emitted successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] readSingleSample: Exception while emitting signal: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] readSingleSample: Unknown exception while emitting signal" << std::endl;
    }
    
    std::cout << "[DEBUG] readSingleSample Before updateLiveDisplay" << std::endl;
    updateLiveDisplay(forceValue);
    std::cout << "[DEBUG] readSingleSample: updateLiveDisplay completed" << std::endl;
    
    if (!m_showRawValues) {
        std::cout << "[DEBUG] readSingleSample Before detectRepetitions" << std::endl;
        detectRepetitions();
        std::cout << "[DEBUG] readSingleSample: detectRepetitions completed" << std::endl;
    }
    
    std::cout << "[DEBUG] readSingleSample END" << std::endl;
    
    // Aktualizuj tabelę surowych danych (ostatnie 100) - safety check for null pointer
    if (m_rawTable != nullptr) {
        if (m_rawTable->rowCount() >= 100) {
            m_rawTable->removeRow(0);
        }
        int row = m_rawTable->rowCount();
        m_rawTable->insertRow(row);
        m_rawTable->setItem(row, 0, new QTableWidgetItem(QString::number(currentTime, 'f', 3)));
        const double displayedSample = m_showRawValues
            ? forceValue
            : (m_currentUnit == ForceUnit::Kilograms ? forceValue / 9.80665 : forceValue);
        m_rawTable->setItem(row, 1, new QTableWidgetItem(QString::number(displayedSample, 'f', 2)));
        m_rawTable->scrollToBottom();
    } else {
        std::cerr << "[ERROR] readSingleSample: m_rawTable is null!" << std::endl;
    }
}

void MeasurementTab::updateLiveDisplay(double force)
{
    std::cout << "[DEBUG] updateLiveDisplay START: force=" << force 
              << " m_showRawValues=" << m_showRawValues << " m_currentUnit=" << static_cast<int>(m_currentUnit) << std::endl;
    
    // Wyświetl siłę w aktualnie wybranej jednostce
    double displayForce = force;
    QString unitLabel = "N";
    
    if (!m_showRawValues) {
        switch (m_currentUnit) {
            case ForceUnit::Kilograms:
                displayForce = force / 9.80665;  // N -> kgf
                unitLabel = "kgf";
                std::cout << "[DEBUG] updateLiveDisplay: Converted to kg, displayForce=" << displayForce << std::endl;
                break;
            case ForceUnit::Raw:
                // Nie powinno się zdarzyć, bo raw jest obsługiwane osobno
                displayForce = force;
                unitLabel = "ADC";
                std::cout << "[DEBUG] updateLiveDisplay: Raw mode, displayForce=" << displayForce << std::endl;
                break;
            case ForceUnit::Newtons:
            default:
                displayForce = force;
                unitLabel = "N";
                std::cout << "[DEBUG] updateLiveDisplay: Newtons mode, displayForce=" << displayForce << std::endl;
                break;
        }
    } else {
        unitLabel = "ADC";
        std::cout << "[DEBUG] updateLiveDisplay: showRawValues=true, displayForce=" << displayForce << std::endl;
    }
    
    std::cout << "[DEBUG] updateLiveDisplay: Before updating UI components" << std::endl;
    m_lblCurrentForce->setText(QString("Aktualna siła: %1 %2").arg(displayForce, 0, 'f', 1).arg(unitLabel));
    m_forceBar->setValue(static_cast<int>(displayForce));
    m_forceBar->setFormat(QString("%v %1").arg(unitLabel));
    m_forceBar->setFormat(QString("%v %1").arg(unitLabel));
    
    double lastTime = m_timeBuffer.isEmpty() ? 0.0 : m_timeBuffer.last();
    std::cout << "[DEBUG] updateLiveDisplay: Before addSample, force=" << force << " time=" << lastTime << std::endl;
    m_forcePlot->addSample(force, lastTime);
    std::cout << "[DEBUG] updateLiveDisplay END" << std::endl;
}

void MeasurementTab::detectRepetitions()
{
    std::cout << "[DEBUG] detectRepetitions START: m_currentRepForces.size()=" 
              << m_currentRepForces.size() << std::endl;
    
    if (m_currentRepForces.isEmpty()) {
        std::cout << "[DEBUG] detectRepetitions END: m_currentRepForces is empty" << std::endl;
        return;
    }
    
    double currentForce = m_currentRepForces.last();
    double currentTime = m_currentRepTimes.last();
    
    std::cout << "[DEBUG] detectRepetitions: currentForce=" << currentForce 
              << " currentTime=" << currentTime
              << " m_inContraction=" << m_inContraction 
              << " m_contractionThreshold=" << m_contractionThreshold << std::endl;
    
    // Detekcja początku skurcza
    if (!m_inContraction && currentForce > m_contractionThreshold) {
        std::cout << "[DEBUG] detectRepetitions: Detected contraction START" << std::endl;
        m_inContraction = true;
        m_repStartTime = currentTime;
        m_repStartForce = currentForce;
        m_repPeakForce = currentForce;
        m_repPeakTime = currentTime;
        m_currentRepForces.clear();
        m_currentRepTimes.clear();
        m_currentRepForces.append(currentForce);
        m_currentRepTimes.append(currentTime);
        std::cout << "[DEBUG] detectRepetitions: Contraction started, repStartTime=" << m_repStartTime << std::endl;
    }
    // Śledzenie szczytu
    else if (m_inContraction && currentForce > m_repPeakForce) {
        std::cout << "[DEBUG] detectRepetitions: New peak force=" << currentForce << std::endl;
        m_repPeakForce = currentForce;
        m_repPeakTime = currentTime;
    }
    // Detekcja końca skurcza
    else if (m_inContraction && currentForce < m_contractionThreshold * 0.5) {
        std::cout << "[DEBUG] detectRepetitions: Detected contraction END, calling calculateRepetitionStats" << std::endl;
        // Koniec powtórzenia
        calculateRepetitionStats();
        
        m_inContraction = false;
        m_currentRepForces.clear();
        m_currentRepTimes.clear();
        std::cout << "[DEBUG] detectRepetitions: Contraction ended, reset state" << std::endl;
    }
    
    std::cout << "[DEBUG] detectRepetitions END" << std::endl;
}

void MeasurementTab::calculateRepetitionStats()
{
    std::cout << "[DEBUG] calculateRepetitionStats START: m_currentRepForces.size()=" 
              << m_currentRepForces.size() 
              << " m_completedSeriesStats.size()=" << m_completedSeriesStats.size() << std::endl;
    
    if (m_currentRepForces.size() < 2) {
        std::cout << "[DEBUG] calculateRepetitionStats END: m_currentRepForces.size() < 2" << std::endl;
        return;
    }
    
    if (m_completedSeriesStats.isEmpty()) {
        std::cerr << "[ERROR] calculateRepetitionStats: m_completedSeriesStats is EMPTY - this should not happen!" << std::endl;
        ensureCurrentSeriesInitialized();
    }
    
    std::cout << "[DEBUG] calculateRepetitionStats: Setting repNumber=" << (m_currentRepInSeries + 1) << std::endl;
    
    m_currentRepStats = RepetitionStats();
    m_currentRepStats.repNumber = m_currentRepInSeries + 1;
    m_currentRepStats.peakForce = m_repPeakForce;
    m_currentRepStats.timeToPeak = m_repPeakTime - m_repStartTime;
    
    // Rise rate (szybkość narastania)
    if (m_repPeakTime - m_repStartTime > 0) {
        m_currentRepStats.riseRate = (m_repPeakForce - m_repStartForce) / (m_repPeakTime - m_repStartTime);
    }
    
    // Average force decay rate from the peak to the first post-peak
    // sample at or below 10% of peak force.
    double fallEndTime = m_repPeakTime;
    double fallEndForce = m_repPeakForce;
    for (int i = 0; i < m_currentRepForces.size(); ++i) {
        if (m_currentRepTimes[i] > m_repPeakTime &&
            m_currentRepForces[i] <= m_repPeakForce * 0.1) {
            fallEndTime = m_currentRepTimes[i];
            fallEndForce = m_currentRepForces[i];
            break;
        }
    }
    if (fallEndTime > m_repPeakTime) {
        m_currentRepStats.fallRate =
            (m_repPeakForce - fallEndForce) / (fallEndTime - m_repPeakTime);
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
    
    std::cout << "[DEBUG] calculateRepetitionStats: Before accessing m_completedSeriesStats.last()" << std::endl;
    
    // Dodaj do bieżącej serii
    SeriesStats& currentSeries = m_completedSeriesStats.last();
    currentSeries.reps.append(m_currentRepStats);
    
    std::cout << "[DEBUG] calculateRepetitionStats: Before emit repetitionCompleted" << std::endl;
    
    // Emituj sygnał
    emit repetitionCompleted(m_currentSeries, m_currentRepStats.repNumber, m_currentRepStats);
    
    std::cout << "[DEBUG] calculateRepetitionStats: Before updateStatsTable" << std::endl;
    
    // Aktualizuj tabelę
    updateStatsTable();
    
    std::cout << "[DEBUG] calculateRepetitionStats: Before checking series completion" << std::endl;
    
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
        
        std::cout << "[DEBUG] calculateRepetitionStats: Before seriesDuration calculation, reps.size()=" 
                  << currentSeries.reps.size() << std::endl;
        
        // Szybkość serii - zabezpieczenie przed pustym wektorem
        double seriesDuration = 0;
        if (!currentSeries.reps.isEmpty()) {
            seriesDuration = currentSeries.reps.last().duration - currentSeries.reps.first().duration;
        }
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
        if (currentSeries.avgPeakForce > 0) {
            currentSeries.consistencyScore = std::max(0.0, 100.0 - (stdDev / currentSeries.avgPeakForce) * 100.0);
        }
        
        std::cout << "[DEBUG] calculateRepetitionStats: Before emit seriesCompleted" << std::endl;
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
    
    std::cout << "[DEBUG] calculateRepetitionStats: Before updateSeriesProgress" << std::endl;
    updateSeriesProgress();
}

void MeasurementTab::updateStatsTable()
{
    std::cout << "[DEBUG] updateStatsTable START: m_currentSeries=" << m_currentSeries 
              << " m_currentRepStats.repNumber=" << m_currentRepStats.repNumber << std::endl;
    
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
    
    std::cout << "[DEBUG] updateStatsTable END" << std::endl;
}

void MeasurementTab::updateSeriesProgress()
{
    std::cout << "[DEBUG] updateSeriesProgress: m_currentSeries=" << m_currentSeries 
              << " m_currentRepInSeries=" << m_currentRepInSeries << std::endl;
    
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
        int bin = qBound(0, static_cast<int>((v - minVal) / binSize), 9);
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
    // Manual capture reuses the latest frame from the shared asynchronous reader.
    if (!m_serialPort || !m_serialPort->isConnected()) {
        m_lblStatus->setText(tr("Status: BRAK CZUJNIKA"));
        m_lblStatus->setStyleSheet("color: red; font-weight: bold;");
        return;
    }
    if (!m_lastSensorData.isValid || (!m_showRawValues && !m_lastSensorData.hasCalibratedValue)) {
        m_lblStatus->setText(tr("Status: OCZEKIWANIE NA SKALIBROWANĄ PRÓBKĘ"));
        m_lblStatus->setStyleSheet("color: orange; font-weight: bold;");
        return;
    }

    const double value = m_showRawValues
        ? static_cast<double>(m_lastSensorData.value)
        : biofeedback::measurement::gramsToNewtons(m_lastSensorData.calibratedValue);
    if (!std::isfinite(value)) return;
    if (m_sessionStartTime <= 0.0) ensureSessionClockStarted();
    ensureCurrentSeriesInitialized();
    readSingleSample(value);
    m_lblStatus->setText(tr("Status: PRÓBKA ZAPISANA"));
    m_lblStatus->setStyleSheet("color: green; font-weight: bold;");
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
    const bool hasCompletedRepetition = std::any_of(
        m_completedSeriesStats.cbegin(), m_completedSeriesStats.cend(),
        [](const SeriesStats& series) { return !series.reps.isEmpty(); });
    if (!hasCompletedRepetition) {
        QMessageBox::warning(this, "Ostrzeżenie", "Brak zakończonych powtórzeń do zapisania.");
        return;
    }
    
    if (m_patientId.isEmpty()) {
        QMessageBox::warning(this, "Ostrzeżenie", "Najpierw ustaw ID pacjenta!");
        return;
    }
    
    // Utwórz katalog dla pacjenta jeśli nie istnieje
    QString dataDir = biofeedback::runtime::patientDirectory(m_patientId);
    QDir dir;
    if (!dir.exists(dataDir)) {
        dir.mkpath(dataDir);
    }
    
    // Generuj nazwę pliku z datą i numerem sesji
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    int sessionNum = m_patientSessions.size() + 1;
    QString filename = dataDir + "/pomiar_" + timestamp + "_sesja" + QString::number(sessionNum) + ".json";
    
    if (!exportToJSON(filename)) return;
    
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
    MeasurementSession session;
    session.sessionId = QUuid::createUuid().toString();
    session.patientId = m_patientId;
    session.date = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    session.series = m_completedSeriesStats;

    const QByteArray payload = serializeSessionToJSON(session).toUtf8();
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(payload) != payload.size() ||
        !file.commit()) {
        QMessageBox::critical(this, tr("Błąd zapisu"),
                              tr("Nie można atomowo zapisać sesji: %1").arg(file.errorString()));
        return false;
    }
    QFile::setPermissions(filename, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QMessageBox::information(this, tr("Sukces"), tr("Dane sesji zapisano do: %1").arg(filename));
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
    
    QString dataDir = biofeedback::runtime::patientDirectory(m_patientId);
    QDir dir(dataDir);
    if (!dir.exists()) {
        QMessageBox::information(this, "Informacja", "Brak zapisanych sesji dla tego pacjenta.");
        return;
    }
    
    m_patientSessions.clear();
    QStringList jsonFiles = dir.entryList(QStringList() << "pomiar_*.json", QDir::Files);
    
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

// ============================================================================
// Metody SerialCommunication - implementacja
// ============================================================================

void MeasurementTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    m_measurementAdapter.reset();
    m_lastSensorData = sensor::SensorData();
    m_serialPort = serial;
    if (m_serialPort && m_measurementEngine) {
        m_measurementAdapter = std::make_unique<sensor::MeasurementSerialAdapter>(
            *m_serialPort, m_measurementEngine, "desktop-hx711", "g");
    }
    if (m_serialPort) {
        m_dataCallbackToken = m_serialPort->addDataCallback([this](const sensor::SensorData& data) {
            QMetaObject::invokeMethod(this, [this, data]() {
                onSensorDataReceived(data);
            }, Qt::QueuedConnection);
        });
    }
}

bool MeasurementTab::isArduinoConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

void MeasurementTab::onSensorDataReceived(const sensor::SensorData& data)
{
    // Always replace the last frame, including rejected frames, so manual
    // capture can never reuse a stale calibrated sample.
    m_lastSensorData = data;
    if (!data.isValid) {
        addDebugMessage(tr("Otrzymano nieprawidłową ramkę z czujnika"), "WARNING");
        return;
    }
    if (!m_showRawValues && !data.hasCalibratedValue) {
        addDebugMessage(tr("Ramka zawiera tylko dane RAW; oczekuję na skalibrowaną wartość w gramach."), "WARNING");
        return;
    }
    addDebugMessage(QString("HX711: RAW=%1 | skalibrowana masa=%2 g")
        .arg(data.value).arg(data.calibratedValue, 0, 'f', 2), "ARDUINO");

    if (!m_isMeasuring) return;

    // All movement metrics are calculated in SI force. Unit selection affects
    // presentation only; raw ADC mode is a diagnostic capture, not biomechanics.
    const double value = m_showRawValues
        ? static_cast<double>(data.value)
        : biofeedback::measurement::gramsToNewtons(data.calibratedValue);
    if (!std::isfinite(value)) return;
    readSingleSample(value);
}

// ============================================================================
// MeasurementTab - Konwersja jednostek i aktualizacja wyświetlania
// ============================================================================

double MeasurementTab::convertForce(double newtons, ForceUnit targetUnit) const
{
    switch (targetUnit) {
        case ForceUnit::Kilograms:
            // 1 kgf = 9.80665 N.
            return newtons / 9.80665;  // kilogram-siła (kgf)
        case ForceUnit::Raw:
            // Wartość surowa nie jest konwertowana z Newtonów
            return newtons;
        case ForceUnit::Newtons:
        default:
            return newtons;
    }
}

void MeasurementTab::setCurrentUnit(ForceUnit unit)
{
    m_currentUnit = unit;
    m_showRawValues = false;
    if (m_btnToggleRaw && m_btnToggleRaw->isChecked())
        m_btnToggleRaw->setChecked(false);
    if (m_forcePlot)
        m_forcePlot->setUnit(unit);
    
    // Aktualizuj etykiety w tabelach
    updateUnitDisplay();
    updateStatsTableUnits();
    updateRawTableUnits();
    
    std::cout << "[MeasurementTab] Changed force unit to: " 
              << (unit == ForceUnit::Newtons ? "Newtons" : 
                  unit == ForceUnit::Kilograms ? "Kilograms" : "Raw") 
              << std::endl;
}

void MeasurementTab::updateUnitDisplay()
{
    QString unitLabel;
    if (m_showRawValues) {
        unitLabel = "ADC";
    } else {
        switch (m_currentUnit) {
            case ForceUnit::Kilograms: unitLabel = "kgf"; break;
            case ForceUnit::Raw: unitLabel = "ADC"; break;
            case ForceUnit::Newtons: default: unitLabel = "N"; break;
        }
    }

    // Aktualizuj etykietę aktualnej siły z nową jednostką
    if (m_lblCurrentForce && !m_rawForceBuffer.isEmpty()) {
        double currentForce = m_rawForceBuffer.last();
        double displayForce = currentForce;
        
        if (!m_showRawValues && m_currentUnit == ForceUnit::Kilograms) {
            displayForce = currentForce / 9.80665;
        }
        
        m_lblCurrentForce->setText(QString("Aktualna siła: %1 %2").arg(displayForce, 0, 'f', 1).arg(unitLabel));
    }
}

void MeasurementTab::updateStatsTableUnits()
{
    if (!m_statsTable) return;
    // Movement metrics are stored and reported in SI units. Changing the
    // live display unit must not relabel N values as kgf or ADC.
    if (m_statsTable->columnCount() == 12) {
        m_statsTable->setHorizontalHeaderLabels({
            "Seria", "Powt.", "Szczyt [N]", "Czas do szczytu [s]",
            "Rise Rate [N/s]", "Fall Rate [N/s]", "Top 8% [s]", "Bottom 8% [s]",
            "Impuls [N·s]", "Średnia [N]", "Mediana [N]", "Dominanta [N]"
        });
    }
}

void MeasurementTab::updateRawTableUnits()
{
    if (!m_rawTable) return;
    
    QString unitLabel;
    if (m_showRawValues) {
        unitLabel = "ADC";
    } else {
        switch (m_currentUnit) {
            case ForceUnit::Kilograms: unitLabel = "kgf"; break;
            case ForceUnit::Raw: unitLabel = "ADC"; break;
            case ForceUnit::Newtons: default: unitLabel = "N"; break;
        }
    }

    // Aktualizuj nagłówek kolumny siły
    if (m_rawTable->columnCount() > 1) {
        m_rawTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Siła [" + unitLabel + "]"));
        const int rows = m_rawTable->rowCount();
        const int offset = std::max(0, m_rawForceBuffer.size() - rows);
        for (int row = 0; row < rows; ++row) {
            const int sampleIndex = offset + row;
            if (sampleIndex >= m_rawForceBuffer.size()) break;
            const double sample = m_rawForceBuffer[sampleIndex];
            const double displayed = m_showRawValues ? sample
                : (m_currentUnit == ForceUnit::Kilograms ? sample / 9.80665 : sample);
            m_rawTable->setItem(row, 1,
                new QTableWidgetItem(QString::number(displayed, 'f', 2)));
        }
    }
}


void MeasurementTab::addDebugMessage(const QString& message, const QString& type)
{
    // Wysyłaj wiadomość do centralnego DebugManager, który przekaże ją do DebugTab
    core::DebugManager::instance().sendDebugMessage(message, type);
}

} // namespace tab
