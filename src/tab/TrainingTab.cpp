#include "tab/TrainingTab.hpp"
#include <algorithm>
#include <vector>
#include "tab/TrainingAnalyticsWidget.hpp"
#include "measurement/Units.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QStackedWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QDir>
#include <QGuiApplication>
#include <QScreen>
#include <QShortcut>
#include <QKeySequence>
#include <QElapsedTimer>
#include <QPainterPath>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <cmath>
#include <limits>
#include <utility>
#include "games/PongGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/MarioGame.hpp"
#include "games/SinGame.hpp"
#include "games/TanGame.hpp"
#include "games/ArkanoidGame.hpp"
#include "games/GalagaGame.hpp"
#include "games/GameFactory.hpp"

namespace tab {

// ==================== TrainingProgressWidget ====================

TrainingProgressWidget::TrainingProgressWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentExercise(0)
    , m_totalExercises(0)
    , m_currentSeries(0)
    , m_totalSeries(0)
    , m_currentRep(0)
    , m_totalReps(0)
    , m_targetForce(0.0)
    , m_currentForce(0.0)
    , m_sessionTime(0)
{
    setMinimumHeight(280);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void TrainingProgressWidget::setCurrentExercise(int current, int total)
{
    m_currentExercise = current;
    m_totalExercises = total;
    update();
}

void TrainingProgressWidget::setCurrentSeries(int current, int total)
{
    m_currentSeries = current;
    m_totalSeries = total;
    update();
}

void TrainingProgressWidget::setCurrentRep(int current, int total)
{
    m_currentRep = current;
    m_totalReps = total;
    update();
}

void TrainingProgressWidget::setTargetForce(double force)
{
    m_targetForce = force;
    update();
}

void TrainingProgressWidget::setCurrentForce(double force)
{
    if (!std::isfinite(force))
        return;
    m_currentForce = force;
    constexpr std::size_t MaximumTraceSamples = 600; // 7.5 s at 80 SPS
    m_forceHistory.push_back(force);
    while (m_forceHistory.size() > MaximumTraceSamples)
        m_forceHistory.pop_front();
    update();
}

void TrainingProgressWidget::clearForceHistory()
{
    m_forceHistory.clear();
    m_currentForce = 0.0;
    update();
}

void TrainingProgressWidget::setSessionTime(int seconds)
{
    m_sessionTime = seconds;
    update();
}

void TrainingProgressWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Tło
    painter.fillRect(rect(), QColor(30, 30, 30));
    
    int margin = 10;
    int sectionWidth = (width() - 3 * margin) / 3;
    
    // Empty plans and idle sessions are represented explicitly; never display
    // a phantom "1/0" progress state. Clamp stale indices after plan edits.
    const auto progressLabel = [](const QString& name, int current, int total) {
        if (total <= 0) {
            return QString("%1 —/0").arg(name);
        }
        const int safeCurrent = qBound(0, current, total - 1);
        return QString("%1 %2/%3").arg(name).arg(safeCurrent + 1).arg(total);
    };

    // Sekcja 1: Ćwiczenie
    QRect exerciseRect(margin, margin, sectionWidth - margin, height() - 2 * margin);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(exerciseRect, Qt::AlignTop | Qt::AlignHCenter,
                    progressLabel(QStringLiteral("Ćwiczenie"), m_currentExercise, m_totalExercises));

    // Sekcja 2: Seria
    QRect seriesRect(margin + sectionWidth, margin, sectionWidth - margin, height() - 2 * margin);
    painter.drawText(seriesRect, Qt::AlignTop | Qt::AlignHCenter,
                    progressLabel(QStringLiteral("Seria"), m_currentSeries, m_totalSeries));

    // Sekcja 3: Powtórzenie
    QRect repRect(margin + 2 * sectionWidth, margin, sectionWidth - margin, height() - 2 * margin);
    painter.drawText(repRect, Qt::AlignTop | Qt::AlignHCenter,
                    progressLabel(QStringLiteral("Powtórzenie"), m_currentRep, m_totalReps));
    
    // Live force trace: bounded six-second window, updated on every sensor
    // sample and painted at the display cadence with sub-pixel coordinates.
    const int barHeight = 20;
    const int barY = height() - barHeight - margin;
    const int barWidth = width() - 2 * margin;
    const QRectF traceRect(margin, 42, barWidth, qMax(8, barY - 50));
    if (m_forceHistory.size() >= 2 && traceRect.width() > 1.0 && traceRect.height() > 1.0) {
        double scaleMax = qMax(1.0, m_targetForce * 1.2);
        for (double value : m_forceHistory)
            scaleMax = qMax(scaleMax, value * 1.05);

        painter.save();
        painter.setClipRect(traceRect);
        painter.setPen(QPen(QColor(90, 110, 130, 110), 1, Qt::DashLine));
        for (int grid = 1; grid <= 2; ++grid) {
            const qreal y = traceRect.top() + traceRect.height() * grid / 3.0;
            painter.drawLine(QPointF(traceRect.left(), y), QPointF(traceRect.right(), y));
        }

        QVector<QPointF> tracePoints;
        tracePoints.reserve(static_cast<int>(m_forceHistory.size()));
        const double denominator = static_cast<double>(m_forceHistory.size() - 1);
        for (std::size_t index = 0; index < m_forceHistory.size(); ++index) {
            const qreal x = traceRect.left() +
                traceRect.width() * static_cast<double>(index) / denominator;
            const qreal y = traceRect.bottom() -
                qBound(0.0, m_forceHistory[index] / scaleMax, 1.0) * traceRect.height();
            tracePoints.append(QPointF(x, y));
        }

        QPainterPath trace;
        trace.moveTo(tracePoints.front());
        for (int index = 0; index + 1 < tracePoints.size(); ++index) {
            const QPointF& p1 = tracePoints[index];
            const QPointF& p2 = tracePoints[index + 1];
            const QPointF& p0 = tracePoints[qMax(0, index - 1)];
            const QPointF& p3 = tracePoints[qMin(index + 2, tracePoints.size() - 1)];
            const qreal lowY = qMin(p1.y(), p2.y());
            const qreal highY = qMax(p1.y(), p2.y());
            QPointF control1 = p1 + (p2 - p0) / 6.0;
            QPointF control2 = p2 - (p3 - p1) / 6.0;
            control1.setY(qBound(lowY, control1.y(), highY));
            control2.setY(qBound(lowY, control2.y(), highY));
            trace.cubicTo(control1, control2, p2);
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(70, 190, 240), 2.2, Qt::SolidLine,
                            Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(trace);
        painter.restore();
    }

    // Pasek siły
    
    // Tło paska
    painter.setBrush(QColor(60, 60, 60));
    painter.setPen(QColor(80, 80, 80));
    painter.drawRoundedRect(margin, barY, barWidth, barHeight, 5, 5);
    
    // Wypełnienie paska
    double forceRatio = m_targetForce > 0 ? m_currentForce / m_targetForce : 0;
    forceRatio = qBound(0.0, forceRatio, 1.0);
    
    QColor forceColor = getForceColorStatic(m_currentForce, m_targetForce);
    painter.setBrush(forceColor);
    painter.setPen(forceColor);
    int fillWidth = static_cast<int>(barWidth * forceRatio);
    painter.drawRoundedRect(margin, barY, fillWidth, barHeight, 5, 5);
    
    // Tekst na pasku
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 9, QFont::Bold));
    painter.drawText(margin, barY, barWidth, barHeight, Qt::AlignCenter,
                    QString("%1 N").arg(m_currentForce, 0, 'f', 1));
}

QColor TrainingProgressWidget::getForceColorStatic(double current, double target)
{
    if (target <= 0) return Qt::gray;
    
    double ratio = current / target;
    if (ratio < 0.5) return QColor(255, 100, 100);  // Czerwony
    if (ratio < 0.8) return QColor(255, 200, 100);  // Pomarańczowy
    if (ratio < 1.0) return QColor(100, 255, 100);  // Jasnozielony
    return QColor(50, 200, 50);  // Zielony
}

// ==================== TrainingTab ====================

TrainingTab::TrainingTab(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_configBox(nullptr)
    , m_exerciseSelector(nullptr)
    , m_btnAddExercise(nullptr)
    , m_btnRemoveExercise(nullptr)
    , m_targetForceSpin(nullptr)
    , m_seriesSpin(nullptr)
    , m_repsSpin(nullptr)
    , m_restTimeSpin(nullptr)
    , m_gameTypeCombo(nullptr)
    , m_controlLayout(nullptr)
    , m_btnStartStop(nullptr)
    , m_btnPauseResume(nullptr)
    , m_btnSave(nullptr)
    , m_btnLoad(nullptr)
    , m_btnExport(nullptr)
    , m_progressBox(nullptr)
    , m_progressWidget(nullptr)
    , m_lblCurrentExercise(nullptr)
    , m_lblCurrentSeries(nullptr)
    , m_lblCurrentRep(nullptr)
    , m_lblTimer(nullptr)
    , m_forceBar(nullptr)
    , m_lblCurrentForce(nullptr)
    , m_lblTargetForce(nullptr)
    , m_lblRestTimer(nullptr)
    , m_gameBox(nullptr)
    , m_currentGame(nullptr)
    , m_gameWindow(nullptr)
    , m_btnOpenGame(nullptr)
    , m_activeGameType("none")
    , m_statsBox(nullptr)
    , m_statsContainer(nullptr)
    , m_statsSummaryLabel(nullptr)
    , m_analyticsWidget(nullptr)
    , m_timer(nullptr)
    , m_restTimer(nullptr)
    , m_currentExerciseIndex(0)
    , m_currentSeriesIndex(0)
    , m_currentRepIndex(0)
    , m_sessionTime(0)
    , m_restTimeRemaining(0)
    , m_repetitionDetector()
    , m_currentSeriesRepetitions()
    , m_currentForce(0.0)
    , m_peakForceInRep(0.0)
    , m_serialPort(nullptr)
    , m_measurementEngine(nullptr)
    , m_liveUiTimer(new QTimer(this))
{
    m_liveUiTimer->setSingleShot(true);
    m_liveUiTimer->setTimerType(Qt::PreciseTimer);
    m_liveUiTimer->setInterval(16);
    connect(m_liveUiTimer, &QTimer::timeout, this, &TrainingTab::refreshLiveUi);
    setupUI();
}

TrainingTab::~TrainingTab()
{
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    stopTraining();
    if (m_gameWindow) {
        m_gameWindow->close();
        delete m_gameWindow;
        m_gameWindow = nullptr;
        m_currentGame = nullptr;
    }
}

void TrainingTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(4);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);

    // Keep the training display, progress and analytics on the left (75%).
    // The exercise plan and its parameters live in a scrollable right column.
    auto* workspaceSplitter = new QSplitter(Qt::Horizontal, this);
    workspaceSplitter->setChildrenCollapsible(false);
    workspaceSplitter->setHandleWidth(5);

    auto* visualScroll = new QScrollArea(workspaceSplitter);
    visualScroll->setWidgetResizable(true);
    visualScroll->setFrameShape(QFrame::NoFrame);
    auto* visualPanel = new QWidget(visualScroll);
    auto* visualLayout = new QVBoxLayout(visualPanel);
    visualLayout->setSpacing(8);
    visualLayout->setContentsMargins(4, 4, 4, 4);

    auto* optionsScroll = new QScrollArea(workspaceSplitter);
    optionsScroll->setWidgetResizable(true);
    optionsScroll->setFrameShape(QFrame::NoFrame);
    optionsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* optionsPanel = new QWidget(optionsScroll);
    auto* optionsLayout = new QVBoxLayout(optionsPanel);
    optionsLayout->setSpacing(8);
    optionsLayout->setContentsMargins(8, 8, 8, 8);
    auto* optionsHeading = new QLabel(tr("KONFIGURACJA TRENINGU"), optionsPanel);
    optionsHeading->setStyleSheet("font-weight:700;color:#40516a;");
    optionsLayout->addWidget(optionsHeading);

    visualScroll->setWidget(visualPanel);
    optionsScroll->setWidget(optionsPanel);
    workspaceSplitter->addWidget(visualScroll);
    workspaceSplitter->addWidget(optionsScroll);
    workspaceSplitter->setStretchFactor(0, 3);
    workspaceSplitter->setStretchFactor(1, 1);
    workspaceSplitter->setSizes({900, 300});
    // Keep manual sidebar resizing available while enforcing a 25% maximum.
    // QSplitter preserves the relative sizes as the window changes resolution.
    connect(workspaceSplitter, &QSplitter::splitterMoved, workspaceSplitter,
            [workspaceSplitter](int position, int) {
        const int availableWidth = workspaceSplitter->width() - workspaceSplitter->handleWidth();
        const int maxVisualWidth = availableWidth * 3 / 4;
        if (position < maxVisualWidth) {
            workspaceSplitter->setSizes({maxVisualWidth, availableWidth - maxVisualWidth});
        }
    });
    m_mainLayout->addWidget(workspaceSplitter, 1);

    // Panel konfiguracji w prawej kolumnie.
    m_configBox = new QGroupBox("Konfiguracja ćwiczenia", optionsPanel);
    m_configBox->setFont(QFont("Arial", 11, QFont::Bold));
    QVBoxLayout* configLayout = new QVBoxLayout(m_configBox);
    configLayout->setSpacing(10);
    
    m_exerciseSelector = new QComboBox(this);
    m_exerciseSelector->setMinimumWidth(200);
    m_exerciseSelector->setMinimumHeight(35);
    m_exerciseSelector->setFont(QFont("Arial", 10));
    configLayout->addWidget(new QLabel("Ćwiczenie:", this));
    configLayout->addWidget(m_exerciseSelector);
    
    configLayout->addWidget(new QLabel("Siła celu [N]:", this));
    m_targetForceSpin = new QDoubleSpinBox(this);
    m_targetForceSpin->setRange(0, 500);
    m_targetForceSpin->setValue(50.0);
    m_targetForceSpin->setDecimals(1);
    m_targetForceSpin->setMinimumHeight(35);
    m_targetForceSpin->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_targetForceSpin);
    
    configLayout->addWidget(new QLabel("Serie:", this));
    m_seriesSpin = new QSpinBox(this);
    m_seriesSpin->setRange(1, 20);
    m_seriesSpin->setValue(3);
    m_seriesSpin->setMinimumHeight(35);
    m_seriesSpin->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_seriesSpin);
    
    configLayout->addWidget(new QLabel("Powtórzenia:", this));
    m_repsSpin = new QSpinBox(this);
    m_repsSpin->setRange(1, 50);
    m_repsSpin->setValue(8);
    m_repsSpin->setMinimumHeight(35);
    m_repsSpin->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_repsSpin);
    
    configLayout->addWidget(new QLabel("Odpoczynek [s]:", this));
    m_restTimeSpin = new QSpinBox(this);
    m_restTimeSpin->setRange(0, 300);
    m_restTimeSpin->setValue(60);
    m_restTimeSpin->setMinimumHeight(35);
    m_restTimeSpin->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_restTimeSpin);
    
    configLayout->addWidget(new QLabel("Gra:", this));
    m_gameTypeCombo = new QComboBox(this);
    m_gameTypeCombo->addItem("Pong", "pong");
    m_gameTypeCombo->addItem("Arkanoid", "arkanoid");
    m_gameTypeCombo->addItem("Flappy Bird", "flappy");
    m_gameTypeCombo->addItem("Mario", "mario");
    m_gameTypeCombo->addItem("Sinusoidalna", "sin");
    m_gameTypeCombo->addItem("Tangensoidalna", "tan");
    m_gameTypeCombo->addItem("Galaga", "galaga");
    m_gameTypeCombo->addItem("Archer", "archer");
    m_gameTypeCombo->addItem("Scorched Earth", "scorched_earth");
    m_gameTypeCombo->addItem("Car Race", "car_race");
    m_gameTypeCombo->setMinimumHeight(35);
    m_gameTypeCombo->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_gameTypeCombo);
    
    m_btnAddExercise = new QPushButton("Dodaj", this);
    m_btnAddExercise->setMaximumWidth(80);
    m_btnAddExercise->setMinimumHeight(35);
    m_btnAddExercise->setFont(QFont("Arial", 10, QFont::Bold));
    configLayout->addWidget(m_btnAddExercise);
    
    m_btnRemoveExercise = new QPushButton("Usuń", this);
    m_btnRemoveExercise->setMaximumWidth(80);
    m_btnRemoveExercise->setMinimumHeight(35);
    m_btnRemoveExercise->setFont(QFont("Arial", 10));
    configLayout->addWidget(m_btnRemoveExercise);
    
    optionsLayout->addWidget(m_configBox);
    
    // Panel sterowania
    m_controlLayout = new QHBoxLayout();
    m_controlLayout->setSpacing(12);
    
    m_btnStartStop = new QPushButton("Start", this);
    m_btnStartStop->setMinimumHeight(45);
    m_btnStartStop->setFont(QFont("Arial", 12, QFont::Bold));
    m_controlLayout->addWidget(m_btnStartStop);
    
    m_btnPauseResume = new QPushButton("Pauza", this);
    m_btnPauseResume->setMinimumHeight(40);
    m_btnPauseResume->setFont(QFont("Arial", 12));
    m_btnPauseResume->setEnabled(false);
    m_controlLayout->addWidget(m_btnPauseResume);
    
    m_btnSave = new QPushButton("Zapisz wyniki", this);
    m_btnSave->setMinimumHeight(35);
    m_btnSave->setFont(QFont("Arial", 10));
    m_controlLayout->addWidget(m_btnSave);
    
    m_btnLoad = new QPushButton("Wczytaj plan", this);
    m_btnLoad->setMinimumHeight(35);
    m_btnLoad->setFont(QFont("Arial", 10));
    m_controlLayout->addWidget(m_btnLoad);
    
    m_btnExport = new QPushButton("Eksportuj plan", this);
    m_btnExport->setMinimumHeight(35);
    m_btnExport->setFont(QFont("Arial", 10));
    m_controlLayout->addWidget(m_btnExport);
    
    visualLayout->addLayout(m_controlLayout);
    
    // Panel postępu
    m_progressBox = new QGroupBox("Postęp treningu", this);
    QVBoxLayout* progressLayout = new QVBoxLayout(m_progressBox);
    
    m_progressWidget = new TrainingProgressWidget(this);
    progressLayout->addWidget(m_progressWidget);
    
    QHBoxLayout* infoLayout = new QHBoxLayout();
    
    m_lblCurrentExercise = new QLabel("Ćwiczenie: -", this);
    infoLayout->addWidget(m_lblCurrentExercise);
    
    m_lblCurrentSeries = new QLabel("Seria: -", this);
    infoLayout->addWidget(m_lblCurrentSeries);
    
    m_lblCurrentRep = new QLabel("Powtórzenie: -", this);
    infoLayout->addWidget(m_lblCurrentRep);
    
    m_lblTimer = new QLabel("Czas: 00:00", this);
    m_lblTimer->setFont(QFont("Arial", 14, QFont::Bold));
    infoLayout->addWidget(m_lblTimer);
    
    progressLayout->addLayout(infoLayout);
    
    QHBoxLayout* forceLayout = new QHBoxLayout();
    forceLayout->addWidget(new QLabel("Aktualna siła:", this));
    
    m_lblCurrentForce = new QLabel("0.0 N", this);
    m_lblCurrentForce->setFont(QFont("Arial", 12, QFont::Bold));
    forceLayout->addWidget(m_lblCurrentForce);
    
    forceLayout->addWidget(new QLabel("Cel:", this));
    m_lblTargetForce = new QLabel("50.0 N", this);
    forceLayout->addWidget(m_lblTargetForce);
    
    m_forceBar = new QProgressBar(this);
    m_forceBar->setRange(0, 100);
    m_forceBar->setValue(0);
    m_forceBar->setFormat("%v%");
    forceLayout->addWidget(m_forceBar);
    
    m_lblRestTimer = new QLabel("", this);
    m_lblRestTimer->setFont(QFont("Arial", 12, QFont::Bold));
    m_lblRestTimer->setStyleSheet("color: yellow;");
    forceLayout->addWidget(m_lblRestTimer);
    
    progressLayout->addLayout(forceLayout);
    
    visualLayout->addWidget(m_progressBox, 2);
    
    // The game is hosted in a separate, non-modal fullscreen QDialog.
    // Keep a reopen control in the training workspace instead of embedding it.
    m_gameBox = new QGroupBox("Gra treningowa", this);
    QVBoxLayout* gameLayout = new QVBoxLayout(m_gameBox);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->setSpacing(0);

    m_btnOpenGame = new QPushButton(
        tr("Otwórz grę w osobnym oknie pełnoekranowym"), m_gameBox);
    m_btnOpenGame->setMinimumHeight(54);
    m_btnOpenGame->setEnabled(false);
    m_btnOpenGame->setToolTip(
        tr("Gra działa w niezależnym oknie. Klawisz Esc przywraca okno z paskiem tytułu."));
    gameLayout->addWidget(m_btnOpenGame);
    connect(m_btnOpenGame, &QPushButton::clicked, this, &TrainingTab::showGameWindow);

    visualLayout->addWidget(m_gameBox, 0);
    
    // Tabela statystyk
    m_statsBox = new QGroupBox("Statystyki sesji", this);
    QVBoxLayout* statsLayout = new QVBoxLayout(m_statsBox);
    
    m_statsContainer = new QWidget(this);
    QVBoxLayout* statsContentLayout = new QVBoxLayout(m_statsContainer);
    statsContentLayout->setContentsMargins(4, 4, 4, 4);
    m_statsSummaryLabel = new QLabel("Brak zarejestrowanych powtórzeń.", m_statsContainer);
    m_statsSummaryLabel->setWordWrap(true);
    statsContentLayout->addWidget(m_statsSummaryLabel);
    m_analyticsWidget = new TrainingAnalyticsWidget(m_statsContainer);
    statsContentLayout->addWidget(m_analyticsWidget, 1);
    statsLayout->addWidget(m_statsContainer);
    
    visualLayout->addWidget(m_statsBox, 3);
    visualLayout->addStretch(1);
    optionsLayout->addStretch(1);
    
    // TrainingTab nie posiada własnego terminala debugowego.
    // Wszystkie wiadomości debugowe są wysyłane do DebugManager,
    // który przekazuje je do centralnej zakładki DebugTab.
    
    // Timer
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);  // 1 Hz — licznik sesji zwiększa się raz na sekundę
    
    m_restTimer = new QTimer(this);
    m_restTimer->setInterval(100);  // 10 Hz; remaining time is measured monotonically
    
    // Połączenia sygnałów
    connect(m_btnStartStop, &QPushButton::clicked, this, &TrainingTab::onStartStopClicked);
    connect(m_btnPauseResume, &QPushButton::clicked, this, &TrainingTab::onPauseResumeClicked);
    connect(m_btnSave, &QPushButton::clicked, this, &TrainingTab::onSaveResults);
    connect(m_btnLoad, &QPushButton::clicked, this, &TrainingTab::onLoadPlan);
    connect(m_btnExport, &QPushButton::clicked, this, &TrainingTab::onExportPlan);
    connect(m_btnAddExercise, &QPushButton::clicked, this, [this]() {
        TrainingExercise ex;
        ex.name = QString("Ćwiczenie %1").arg(m_exercisePlan.size() + 1);
        ex.targetForce = m_targetForceSpin->value();
        ex.seriesCount = m_seriesSpin->value();
        ex.repsPerSeries = m_repsSpin->value();
        ex.restTime = m_restTimeSpin->value();
        ex.gameType = m_gameTypeCombo->currentData().toString();
        addExercise(ex);
    });
    connect(m_btnRemoveExercise, &QPushButton::clicked, this, [this]() {
        int idx = m_exerciseSelector->currentIndex();
        if (idx >= 0) {
            removeExercise(idx);
        }
    });
    connect(m_exerciseSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TrainingTab::onExerciseSelected);
    connect(m_timer, &QTimer::timeout, this, &TrainingTab::onTimerTick);
    connect(m_restTimer, &QTimer::timeout, this, [this]() {
        updateRestDisplay();
        if (m_restTimeRemaining <= 0) {
            m_restTimer->stop();
            advanceAfterRest();
        }
    });
}

void TrainingTab::addExercise(const TrainingExercise& exercise)
{
    m_exercisePlan.append(exercise);
    ExerciseStats stats;
    stats.exerciseName = exercise.name;
    m_exerciseStats.append(stats);
    
    m_exerciseSelector->addItem(exercise.name);
    updateDisplay();
}

void TrainingTab::removeExercise(int index)
{
    if (index < 0 || index >= m_exercisePlan.size()) return;
    
    m_exercisePlan.removeAt(index);
    m_exerciseStats.removeAt(index);
    m_exerciseSelector->removeItem(index);
    updateDisplay();
}

void TrainingTab::clearPlan()
{
    m_exercisePlan.clear();
    m_exerciseStats.clear();
    m_exerciseSelector->clear();
    m_currentExerciseIndex = 0;
    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    updateDisplay();
}

QVector<TrainingExercise> TrainingTab::getExercisePlan() const
{
    return m_exercisePlan;
}

void TrainingTab::setCurrentExerciseIndex(int index)
{
    if (index >= 0 && index < m_exercisePlan.size()) {
        m_currentExerciseIndex = index;
        m_exerciseSelector->setCurrentIndex(index);
        updateDisplay();
    }
}

void TrainingTab::startTraining()
{
    if (!m_stateMachine.canStart()) return;
    if (m_exercisePlan.isEmpty()) {
        QMessageBox::warning(this, "Brak planu", "Dodaj ćwiczenia przed rozpoczęciem treningu.");
        return;
    }
    if (!m_serialPort || !m_serialPort->isConnected()) {
        QMessageBox::warning(this, tr("Brak urządzenia"),
                             tr("Podłącz czujnik HX711 przed rozpoczęciem treningu."));
        return;
    }

    if (!m_sensorCalibrationReady) {
        QMessageBox::warning(this, tr("Brak kalibracji"),
                             tr("Zapisz i wczytaj profil kalibracji HX711 przed rozpoczęciem treningu."));
        return;
    }

    if (m_analyticsWidget && !m_analyticsWidget->beginSession()) {
        QMessageBox::warning(this, tr("Niezapisana sesja"),
                             tr("Zapisz poprzednią sesję treningową przed rozpoczęciem kolejnej."));
        return;
    }
    if (!m_stateMachine.start()) return;
    m_sessionTime = 0;
    m_restTimeRemaining = 0;
    m_restDurationMs = 0;
    m_restClock.invalidate();
    m_sessionElapsedOffsetMs = 0;
    m_lastCheckpointSeconds = -1;
    m_trainingClock.reset();
    m_sessionClock.start();
    m_progressWidget->clearForceHistory();
    m_activeSessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_sessionStartedAtUtc = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    m_trainingClock.update(0, true);
    m_currentExerciseIndex = 0;
    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    m_currentSeriesRepetitions.clear();
    m_repetitionDetector.reset();
    for (int i = 0; i < m_exerciseStats.size(); ++i) {
        ExerciseStats fresh;
        fresh.exerciseName = m_exercisePlan[i].name;
        m_exerciseStats[i] = fresh;
    }
    refreshAnalytics();
    
    // Inicjalizacja pierwszego ćwiczenia
    loadNextExercise();
    
    // Uruchomienie timera
    m_timer->start();
    
    // Aktualizacja UI
    m_btnStartStop->setText("Stop");
    m_btnStartStop->setStyleSheet("background-color: #cc4444; color: white;");
    m_btnPauseResume->setEnabled(true);
    m_btnPauseResume->setText("Pauza");
    m_configBox->setEnabled(false);
    
    if (!saveExecutionCheckpoint())
        addDebugMessage("Nie udało się utworzyć początkowego punktu kontrolnego sesji.", "ERROR");
    emit trainingStarted();
}

void TrainingTab::stopTraining(bool completed)
{
    if (!m_stateMachine.running()) return;

    updateSessionClocks();
    if (completed) m_stateMachine.complete();
    else m_stateMachine.interrupt();
    m_timer->stop();
    m_restTimer->stop();

    // Stop the game before snapshotting analytics: gameEnded may add final points.
    if (m_currentGame && m_currentGame->isRunning())
        m_currentGame->stop();
    if (m_gameWindow) m_gameWindow->hide();

    refreshAnalytics();
    if (m_analyticsWidget) {
        m_analyticsWidget->setSessionCompleted(completed);
        if (!m_analyticsWidget->saveSessionAutomatically())
            addDebugMessage("Nie udało się automatycznie zapisać sesji treningowej; sesja pozostaje do ponownego zapisu.", "ERROR");
    }

    m_repetitionDetector.reset();
    if (m_liveUiTimer) m_liveUiTimer->stop();
    m_pendingUiSamples = 0;
    
    // Aktualizacja UI
    m_btnStartStop->setText("Start");
    m_btnStartStop->setStyleSheet("");
    m_btnPauseResume->setEnabled(false);
    m_btnPauseResume->setText("Pauza");
    m_configBox->setEnabled(true);
    
    // Reset wyświetlaczy
    m_progressWidget->setCurrentExercise(0, 0);
    m_progressWidget->setCurrentSeries(0, 0);
    m_progressWidget->setCurrentRep(0, 0);
    m_lblTimer->setText("Czas: 00:00");
    m_lblRestTimer->setText("");
    
    emit trainingStopped();
}

void TrainingTab::pauseTraining()
{
    if (!m_stateMachine.running() || !m_stateMachine.pause()) return;

    updateSessionClocks();
    if (m_stateMachine.resting() && m_restClock.isValid()) {
        // Preserve the exact millisecond remainder; the label is rounded up
        // for display only and must not extend the actual rest interval.
        m_restDurationMs = qMax<qint64>(0, m_restDurationMs - m_restClock.elapsed());
        m_restTimeRemaining = static_cast<int>((m_restDurationMs + 999) / 1000);
        m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
    }
    m_restTimer->stop();
    
    if (m_currentGame && m_currentGame->isRunning()) {
        m_currentGame->pause();
    }
    
    m_btnPauseResume->setText("Wznów");
    
    if (!saveExecutionCheckpoint())
        addDebugMessage("Nie udało się zapisać punktu kontrolnego pauzy.", "ERROR");
    emit trainingPaused();
}

void TrainingTab::resumeTraining()
{
    if (!m_stateMachine.running() || !m_stateMachine.resume()) return;

    if (m_stateMachine.resting()) {
        // m_restDurationMs already contains the exact frozen remainder.
        m_restClock.restart();
        m_restTimer->start();
    }
    
    if (m_currentGame && m_currentGame->isPaused()) {
        m_currentGame->resume();
    }
    
    m_btnPauseResume->setText("Pauza");
    
    if (!saveExecutionCheckpoint())
        addDebugMessage("Nie udało się zapisać punktu kontrolnego wznowienia.", "ERROR");
    emit trainingResumed();
}

bool TrainingTab::isTrainingActive() const
{
    return m_stateMachine.running() && !m_stateMachine.paused();
}

bool TrainingTab::isTrainingPaused() const
{
    return m_stateMachine.paused();
}

void TrainingTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    // Serial ownership and acquisition stay in MainWindow/MeasurementTab.
    // Training receives calibrated samples only through MeasurementEngine.
    m_serialPort = std::move(serial);
}

void TrainingTab::setMeasurementEngine(
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine)
{
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    {
        std::lock_guard<std::mutex> lock(m_measurementQueueMutex);
        m_measurementQueue.clear();
        m_droppedMeasurementSamples = 0;
    }
    m_measurementEngine = std::move(engine);
    if (!m_measurementEngine) return;

    m_measurementCallbackToken = m_measurementEngine->subscribe(
        [this](const biofeedback::measurement::MeasurementSample& sample) {
            enqueueMeasurementSample(sample);
        });
}

void TrainingTab::setSensorCalibrationReady(bool ready)
{
    m_sensorCalibrationReady = ready;
}

void TrainingTab::setCurrentPatientContext(const QString& patientPesel,
                                             const QString& patientDataPath)
{
    if (m_stateMachine.running()) return;
    m_currentPatientPesel = patientPesel;
    m_currentPatientDataPath = patientDataPath;
    if (m_analyticsWidget) {
        const QString historyPath = patientDataPath.isEmpty()
            ? QString()
            : QDir(patientDataPath).filePath(QStringLiteral("training_history.json"));
        m_analyticsWidget->setHistoryFilePath(historyPath);
        QJsonObject recovered;
        if (m_analyticsWidget->recoverPendingSession(recovered)) {
            const auto answer = QMessageBox::question(this, tr("Odzyskiwanie treningu"),
                tr("Znaleziono niedokończoną sesję treningową. Czy wznowić ją od zapisanego miejsca?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (answer == QMessageBox::Yes && restoreExecutionCheckpoint(recovered.value("execution_state").toObject())) {
                addDebugMessage("Odzyskano sesję treningową. Trening jest wstrzymany do chwili wybrania Wznów.", "INFO");
            } else {
                m_analyticsWidget->setSessionCompleted(false);
                m_analyticsWidget->saveSessionAutomatically();
                addDebugMessage("Odzyskana sesja została oznaczona jako przerwana.", "WARNING");
            }
        }
    }
}

void TrainingTab::setGameEngine(games::GameEngine* game)
{
    m_currentGame = game;
    if (!m_currentGame) return;

    if (!m_gameWindow) {
        m_gameWindow = new QDialog(this, Qt::Window);
        m_gameWindow->setWindowTitle(tr("Biofeedback — gra treningowa"));
        m_gameWindow->setModal(false);
        m_gameWindow->setAttribute(Qt::WA_DeleteOnClose, false);
        m_gameWindow->setMinimumSize(640, 400);
        auto* gameLayout = new QVBoxLayout(m_gameWindow);
        gameLayout->setContentsMargins(0, 0, 0, 0);
        gameLayout->setSpacing(0);
        auto* leaveFullscreen = new QShortcut(QKeySequence(Qt::Key_Escape), m_gameWindow);
        connect(leaveFullscreen, &QShortcut::activated, m_gameWindow, [this]() {
            if (!m_gameWindow) return;
            m_gameWindow->showNormal();
            m_gameWindow->resize(1280, 800);
            m_gameWindow->raise();
            m_gameWindow->activateWindow();
        });
    }

    m_currentGame->setParent(m_gameWindow);
    m_currentGame->setMinimumSize(600, 360);
    m_currentGame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (m_gameWindow->layout())
        m_gameWindow->layout()->addWidget(m_currentGame);

    connect(m_currentGame, &games::GameEngine::gameEnded, this, &TrainingTab::onGameEnded);
    connect(m_currentGame, &games::GameEngine::gameStarted, this, &TrainingTab::onGameStarted);
    connect(m_currentGame, &games::GameEngine::scoreChanged, this, &TrainingTab::onScoreChanged);

    if (m_btnOpenGame) m_btnOpenGame->setEnabled(true);
    showGameWindow();
}

void TrainingTab::showGameWindow()
{
    if (!m_gameWindow || !m_currentGame) return;
    const QRect available = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry() : QRect(0, 0, 1280, 800);
    m_gameWindow->setGeometry(available);
    m_gameWindow->showFullScreen();
    m_gameWindow->raise();
    m_gameWindow->activateWindow();
    m_currentGame->setFocus(Qt::OtherFocusReason);
}

void TrainingTab::configureGameForExercise()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size())
        return;

    QString type = m_exercisePlan[m_currentExerciseIndex].gameType.trimmed().toLower();
    if (type == "sinusoidal" || type == "sin_game" || type == "sinus")
        type = "sin";
    if (type.isEmpty()) type = "none";
    if (type == m_activeGameType && (type == "none" || m_currentGame)) return;

    if (m_currentGame && m_currentGame->isRunning())
        m_currentGame->stop();
    m_currentGame = nullptr;
    if (m_gameWindow) {
        m_gameWindow->close();
        delete m_gameWindow;
        m_gameWindow = nullptr;
    }

    m_activeGameType = type;
    if (m_btnOpenGame) m_btnOpenGame->setEnabled(false);
    if (type == "none") return;

    games::GameEngine* selectedGame = games::createGame(type, nullptr);
    if (!selectedGame) {
        addDebugMessage(QString("Nieznany typ gry w planie treningowym: %1").arg(type), "WARNING");
        return;
    }
    setGameEngine(selectedGame);
}

QVector<ExerciseStats> TrainingTab::getTrainingStats() const
{
    return m_exerciseStats;
}

QJsonObject TrainingTab::buildSharedTrainingRecord(const QString& patientUuid) const
{
    if (patientUuid.isEmpty() || m_activeSessionId.isEmpty() || m_exercisePlan.isEmpty())
        return {};

    QJsonArray plan;
    for (const TrainingExercise& exercise : m_exercisePlan) {
        plan.append(QJsonObject{
            {QStringLiteral("name"), exercise.name},
            {QStringLiteral("series_count"), exercise.seriesCount},
            {QStringLiteral("repetitions_per_series"), exercise.repsPerSeries},
            {QStringLiteral("target_force_n"), exercise.targetForce},
            {QStringLiteral("rest_seconds"), exercise.restTime},
            {QStringLiteral("game_id"), exercise.gameType}
        });
    }

    QJsonArray exerciseResults;
    QJsonArray gameRecords;
    const int count = qMin(m_exerciseStats.size(), m_exercisePlan.size());
    for (int index = 0; index < count; ++index) {
        const ExerciseStats& stats = m_exerciseStats.at(index);
        const TrainingExercise& exercise = m_exercisePlan.at(index);
        QJsonArray repetitions;
        for (const TrainingRepetitionRecord& repetition : stats.repetitions) {
            const auto& measured = repetition.measurement;
            repetitions.append(QJsonObject{
                {QStringLiteral("series_index"), repetition.seriesIndex},
                {QStringLiteral("repetition_index"), repetition.repetitionIndex},
                {QStringLiteral("start_timestamp_ms"), static_cast<qint64>(measured.startTimestampMs)},
                {QStringLiteral("end_timestamp_ms"), static_cast<qint64>(measured.endTimestampMs)},
                {QStringLiteral("duration_ms"), static_cast<qint64>(measured.durationMs)},
                {QStringLiteral("time_to_peak_ms"), static_cast<qint64>(measured.timeToPeakMs)},
                {QStringLiteral("peak_force_n"), measured.peakForceN},
                {QStringLiteral("mean_force_n"), measured.meanForceN},
                {QStringLiteral("sample_count"), static_cast<qint64>(measured.sampleCount)}
            });
        }

        QJsonArray series;
        for (const biofeedback::training::SeriesSummary& summary : stats.series) {
            series.append(QJsonObject{
                {QStringLiteral("repetition_count"), static_cast<qint64>(summary.repetitionCount)},
                {QStringLiteral("average_peak_force_n"), summary.averagePeakForceN},
                {QStringLiteral("maximum_peak_force_n"), summary.maximumPeakForceN},
                {QStringLiteral("average_duration_ms"), summary.averageDurationMs},
                {QStringLiteral("average_time_to_peak_ms"), summary.averageTimeToPeakMs},
                {QStringLiteral("peak_force_cv_percent"), summary.peakForceCoefficientOfVariationPercent},
                {QStringLiteral("target_achievement_percent"), summary.targetAchievementPercent}
            });
        }

        exerciseResults.append(QJsonObject{
            {QStringLiteral("exercise_index"), index},
            {QStringLiteral("exercise_name"), stats.exerciseName},
            {QStringLiteral("completed_series"), stats.completedSeries},
            {QStringLiteral("completed_repetitions"), stats.completedReps},
            {QStringLiteral("average_peak_force_n"), stats.avgPeakForce},
            {QStringLiteral("maximum_peak_force_n"), stats.maxPeakForce},
            {QStringLiteral("average_reaction_time_ms"), stats.avgReactionTime},
            {QStringLiteral("consistency_score_percent"), stats.consistencyScore},
            {QStringLiteral("completed"), stats.isCompleted},
            {QStringLiteral("game_id"), exercise.gameType},
            {QStringLiteral("game_score"), stats.gameScore},
            {QStringLiteral("repetitions"), repetitions},
            {QStringLiteral("series"), series}
        });

        if (!exercise.gameType.isEmpty() && exercise.gameType != QStringLiteral("none") &&
            stats.gameScore > 0) {
            gameRecords.append(QJsonObject{
                {QStringLiteral("exercise_index"), index},
                {QStringLiteral("game_id"), exercise.gameType},
                {QStringLiteral("score"), stats.gameScore},
                {QStringLiteral("exercise_name"), stats.exerciseName}
            });
        }
    }

    const qint64 elapsedMs = m_sessionElapsedOffsetMs +
        (m_sessionClock.isValid() ? m_sessionClock.elapsed() : 0);
    const bool completed = m_stateMachine.state() == biofeedback::training::SessionState::Completed;
    return QJsonObject{
        {QStringLiteral("session_id"), m_activeSessionId},
        {QStringLiteral("patient_id"), patientUuid},
        {QStringLiteral("started_at"), m_sessionStartedAtUtc},
        {QStringLiteral("finished_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("status"), completed ? QStringLiteral("completed") : QStringLiteral("interrupted")},
        {QStringLiteral("elapsed_ms"), elapsedMs},
        {QStringLiteral("active_training_seconds"), static_cast<double>(m_trainingClock.activeSeconds())},
        {QStringLiteral("plan"), plan},
        {QStringLiteral("exercise_results"), exerciseResults},
        {QStringLiteral("game_results"), gameRecords},
        {QStringLiteral("source_client"), QStringLiteral("desktop")}
    };
}

void TrainingTab::refreshAnalytics()
{
    if (m_analyticsWidget) {
        m_analyticsWidget->setStats(m_exerciseStats, m_sessionTime,
            static_cast<int>(m_trainingClock.activeSeconds()));
    }
}

bool TrainingTab::exportResultsToCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "Ćwiczenie,Seria,Powtórzenie,Czas_trwania_ms,Czas_do_szczytu_ms,"
           "Siła_średnia_N,Siła_szczytowa_N,Próbki,Cel_osiągnięty,"
           "Średni_szczyt_serii_N,Maks_szczyt_serii_N,Cel_serii_pct,CV_serii_pct,Punkty_gry\n";

    for (int exerciseIndex = 0; exerciseIndex < m_exerciseStats.size(); ++exerciseIndex) {
        const auto& stats = m_exerciseStats[exerciseIndex];
        const double targetForce = exerciseIndex < m_exercisePlan.size()
            ? m_exercisePlan[exerciseIndex].targetForce : 0.0;
        QString escapedName = stats.exerciseName;
        escapedName.replace("\"", "\"\"");
        for (const auto& item : stats.repetitions) {
            const auto& rep = item.measurement;
            const bool targetReached = targetForce > 0.0 && rep.peakForceN >= targetForce;
            const int seriesOffset = item.seriesIndex - 1;
            const bool hasSeriesSummary = seriesOffset >= 0 && seriesOffset < stats.series.size();
            const auto seriesSummary = hasSeriesSummary
                ? stats.series[seriesOffset] : biofeedback::training::SeriesSummary{};
            out << "\"" << escapedName << "\"" << ","
                << item.seriesIndex << "," << item.repetitionIndex << ","
                << rep.durationMs << "," << rep.timeToPeakMs << ","
                << QString::number(rep.meanForceN, 'f', 4) << ","
                << QString::number(rep.peakForceN, 'f', 4) << ","
                << rep.sampleCount << "," << (targetReached ? "tak" : "nie") << ","
                << (hasSeriesSummary ? QString::number(seriesSummary.averagePeakForceN, 'f', 4) : QString()) << ","
                << (hasSeriesSummary ? QString::number(seriesSummary.maximumPeakForceN, 'f', 4) : QString()) << ","
                << (hasSeriesSummary ? QString::number(seriesSummary.targetAchievementPercent, 'f', 2) : QString()) << ","
                << (hasSeriesSummary ? QString::number(seriesSummary.peakForceCoefficientOfVariationPercent, 'f', 2) : QString()) << ","
                << stats.gameScore << "\n";
        }
    }

    file.close();
    return file.error() == QFile::NoError;
}

bool TrainingTab::importTrainingPlan(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    clearPlan();
    
    QTextStream in(&file);
    QString line = in.readLine();  // Nagłówek
    
    while (!in.atEnd()) {
        line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() >= 5) {
            TrainingExercise ex;
            ex.name = parts[0];
            ex.seriesCount = parts[1].toInt();
            ex.repsPerSeries = parts[2].toInt();
            ex.targetForce = parts[3].toDouble();
            ex.restTime = parts[4].toDouble();
            if (parts.size() > 5) {
                ex.gameType = parts[5];
                if (ex.gameType == QStringLiteral("scored_earth"))
                    ex.gameType = QStringLiteral("scorched_earth");
            }
            addExercise(ex);
        }
    }
    
    file.close();
    return true;
}

bool TrainingTab::exportTrainingPlan(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Nagłówek
    out << "Nazwa,Serie,Powtórzenia,Siła_celu,Odpoczynek,Typ_gry\n";
    
    for (const auto& ex : m_exercisePlan) {
        out << ex.name << ","
            << ex.seriesCount << ","
            << ex.repsPerSeries << ","
            << ex.targetForce << ","
            << ex.restTime << ","
            << ex.gameType << "\n";
    }
    
    file.close();
    return true;
}

void TrainingTab::onSaveResults()
{
    QString filename = QFileDialog::getSaveFileName(this, "Zapisz wyniki", 
                                                     "", "CSV Files (*.csv)");
    if (!filename.isEmpty()) {
        if (exportResultsToCSV(filename)) {
            QMessageBox::information(this, "Sukces", "Wyniki zapisane pomyślnie.");
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się zapisać wyników.");
        }
    }
}

void TrainingTab::onLoadPlan()
{
    QString filename = QFileDialog::getOpenFileName(this, "Wczytaj plan treningowy",
                                                     "", "CSV Files (*.csv)");
    if (!filename.isEmpty()) {
        if (importTrainingPlan(filename)) {
            QMessageBox::information(this, "Sukces", "Plan wczytany pomyślnie.");
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się wczytać planu.");
        }
    }
}

void TrainingTab::onExportPlan()
{
    QString filename = QFileDialog::getSaveFileName(this, "Eksportuj plan",
                                                     "", "CSV Files (*.csv)");
    if (!filename.isEmpty()) {
        if (exportTrainingPlan(filename)) {
            QMessageBox::information(this, "Sukces", "Plan eksportowany pomyślnie.");
        } else {
            QMessageBox::critical(this, "Błąd", "Nie udało się eksportować planu.");
        }
    }
}

void TrainingTab::onStartStopClicked()
{
    if (m_stateMachine.running()) {
        stopTraining();
    } else {
        startTraining();
    }
}

void TrainingTab::onPauseResumeClicked()
{
    if (m_stateMachine.paused()) {
        resumeTraining();
    } else {
        pauseTraining();
    }
}

void TrainingTab::onExerciseSelected(int index)
{
    if (index >= 0 && index < m_exercisePlan.size()) {
        const auto& ex = m_exercisePlan[index];
        m_targetForceSpin->setValue(ex.targetForce);
        m_seriesSpin->setValue(ex.seriesCount);
        m_repsSpin->setValue(ex.repsPerSeries);
        m_restTimeSpin->setValue(ex.restTime);
        
        int gameIdx = m_gameTypeCombo->findData(ex.gameType);
        if (gameIdx >= 0) {
            m_gameTypeCombo->setCurrentIndex(gameIdx);
        }
    }
}

void TrainingTab::onTimerTick()
{
    if (!m_stateMachine.running()) return;
    updateSessionClocks();
    if (m_stateMachine.resting() && !m_stateMachine.paused()) updateRestDisplay();

    const int activeSeconds = static_cast<int>(m_trainingClock.activeSeconds());
    const int activeMinutes = activeSeconds / 60;
    const int activeRemainder = activeSeconds % 60;
    const int minutes = m_sessionTime / 60;
    const int seconds = m_sessionTime % 60;
    m_lblTimer->setText(QString("Sesja: %1:%2 | Aktywnie: %3:%4")
                        .arg(minutes, 2, 10, QChar('0'))
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(activeMinutes, 2, 10, QChar('0'))
                        .arg(activeRemainder, 2, 10, QChar('0')));
    updateProgressWidgets();
    // Atomically keep a recent snapshot of current progress, including rest
    // milliseconds and the in-flight repetition detector.
    if (m_stateMachine.running() && !m_stateMachine.paused() &&
        !saveExecutionCheckpoint()) {
        addDebugMessage("Nie udało się zapisać bieżącego punktu kontrolnego.", "ERROR");
    }
    if (m_sessionTime > 0 && m_sessionTime % 5 == 0 && m_sessionTime != m_lastCheckpointSeconds) {
        m_lastCheckpointSeconds = m_sessionTime;
        if (!saveExecutionCheckpoint())
            addDebugMessage("Nie udało się zapisać okresowego punktu kontrolnego.", "ERROR");
    }
}

void TrainingTab::updateSessionClocks()
{
    if (!m_stateMachine.running() || !m_sessionClock.isValid()) return;
    const qint64 elapsedMs = m_sessionClock.elapsed();
    m_trainingClock.update(m_sessionElapsedOffsetMs + elapsedMs, !m_stateMachine.paused() && !m_stateMachine.resting());
    m_sessionTime = static_cast<int>(m_trainingClock.sessionSeconds());
}

void TrainingTab::enqueueMeasurementSample(
    const biofeedback::measurement::MeasurementSample& sample)
{
    bool scheduleDrain = false;
    {
        std::lock_guard<std::mutex> lock(m_measurementQueueMutex);
        constexpr std::size_t MaximumQueuedSamples = 4096;
        if (m_measurementQueue.size() >= MaximumQueuedSamples) {
            m_droppedMeasurementSamples += m_measurementQueue.size() + 1;
            m_measurementQueue.clear();
        }
        m_measurementQueue.push_back(sample);
        if (!m_measurementDrainScheduled) {
            m_measurementDrainScheduled = true;
            scheduleDrain = true;
        }
    }
    if (scheduleDrain) {
        const bool queued = QMetaObject::invokeMethod(this, [this]() {
            drainMeasurementSamples();
        }, Qt::QueuedConnection);
        if (!queued) {
            std::lock_guard<std::mutex> lock(m_measurementQueueMutex);
            m_droppedMeasurementSamples += m_measurementQueue.size();
            m_measurementQueue.clear();
            m_measurementDrainScheduled = false;
        }
    }
}

void TrainingTab::drainMeasurementSamples()
{
    constexpr std::size_t MaximumBatch = 128;
    std::vector<biofeedback::measurement::MeasurementSample> batch;
    std::size_t dropped = 0;
    bool scheduleAgain = false;
    {
        std::lock_guard<std::mutex> lock(m_measurementQueueMutex);
        dropped = std::exchange(m_droppedMeasurementSamples, 0);
        const std::size_t count = std::min(MaximumBatch, m_measurementQueue.size());
        batch.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            batch.push_back(std::move(m_measurementQueue.front()));
            m_measurementQueue.pop_front();
        }
        if (m_measurementQueue.empty()) {
            m_measurementDrainScheduled = false;
        } else {
            scheduleAgain = true;
        }
    }
    if (dropped != 0) {
        m_repetitionDetector.reset();
        addDebugMessage(QString("Przepełnienie kolejki treningowej: pominięto %1 próbek; detektor powtórzeń zresetowany.")
                            .arg(static_cast<qulonglong>(dropped)), "WARNING");
    }
    for (const auto& sample : batch)
        onMeasurementSample(sample);
    if (scheduleAgain) {
        const bool queued = QMetaObject::invokeMethod(this, [this]() {
            drainMeasurementSamples();
        }, Qt::QueuedConnection);
        if (!queued) {
            std::lock_guard<std::mutex> lock(m_measurementQueueMutex);
            m_droppedMeasurementSamples += m_measurementQueue.size();
            m_measurementQueue.clear();
            m_measurementDrainScheduled = false;
        }
    }
}

void TrainingTab::scheduleLiveUiRefresh()
{
    if (m_liveUiTimer && !m_liveUiTimer->isActive())
        m_liveUiTimer->start();
}

void TrainingTab::refreshLiveUi()
{
    if (m_pendingUiSamples == 0 || !m_stateMachine.running())
        return;

    m_pendingUiSamples = 0;
    const double force = m_currentForce;
    const double targetForce = (m_currentExerciseIndex >= 0 &&
                                m_currentExerciseIndex < m_exercisePlan.size())
        ? m_exercisePlan[m_currentExerciseIndex].targetForce : 0.0;

    // A GUI frame consumes the newest state only. The repetition detector
    // still receives every acquired sample in onMeasurementSample().
    m_progressWidget->setCurrentForce(force);
    m_lblCurrentForce->setText(QString("%1 N").arg(force, 0, 'f', 2));
    const int barValue = targetForce > 0.0
        ? static_cast<int>((force / targetForce) * 100.0) : 0;
    m_forceBar->setValue(qBound(0, barValue, 100));

    if (m_currentGame && m_currentGame->isRunning() && !m_currentGame->isPaused()) {
        const double gameInput = targetForce > 0.0
            ? qBound(0.0, force / targetForce * 100.0, 100.0) : 0.0;
        m_currentGame->handleSensorInput(gameInput);
    }
}

void TrainingTab::onMeasurementSample(
    const biofeedback::measurement::MeasurementSample& sample)
{
    if (!m_stateMachine.running()) return;

    // Training consumes only the calibrated SI force stream published by
    // MeasurementTab. Raw ADC counts are never treated as force values.
    if (m_stateMachine.paused() || m_stateMachine.resting() || !sample.isValid() || sample.unit != "N" ||
        m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size()) {
        biofeedback::training::RepetitionRecord discarded;
        m_repetitionDetector.process(sample.deviceTimestampMs, 0.0, false, discarded);
        return;
    }

    const double force = std::abs(sample.value); // Tensile magnitude in newtons; calibration may encode bridge polarity.
    if (!std::isfinite(force)) {
        biofeedback::training::RepetitionRecord discarded;
        m_repetitionDetector.process(sample.deviceTimestampMs, 0.0, false, discarded);
        return;
    }
    // Preserve every sample for analysis; coalesce only presentation and
    // game-control work onto a 60 Hz newest-value refresh.
    m_currentForce = force;
    if (m_pendingUiSamples < std::numeric_limits<quint64>::max())
        ++m_pendingUiSamples;
    scheduleLiveUiRefresh();

    biofeedback::training::RepetitionRecord repetition;
    if (m_repetitionDetector.process(sample.deviceTimestampMs, force, true, repetition))
        onRepetitionDetected(repetition);

    emit newForceSample(force);
}

void TrainingTab::onGameEnded(int score)
{
    if (m_currentExerciseIndex >= 0 && m_currentExerciseIndex < m_exerciseStats.size()) {
        m_exerciseStats[m_currentExerciseIndex].gameScore += score;
        refreshAnalytics();
    }
}

void TrainingTab::onGameStarted()
{
    // Gra rozpoczęta
}

void TrainingTab::onScoreChanged(int /*score*/)
{
    // Wynik zmieniony
}

void TrainingTab::updateDisplay()
{
    updateProgressWidgets();
    
    if (!m_exercisePlan.isEmpty() && m_currentExerciseIndex < m_exercisePlan.size()) {
        const auto& ex = m_exercisePlan[m_currentExerciseIndex];
        m_lblTargetForce->setText(QString("%1 N").arg(ex.targetForce, 0, 'f', 1));
    }
}

void TrainingTab::updateProgressWidgets()
{
    m_progressWidget->setCurrentExercise(m_currentExerciseIndex, m_exercisePlan.size());
    m_progressWidget->setSessionTime(m_sessionTime);
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size()) {
        m_progressWidget->setCurrentSeries(0, 0);
        m_progressWidget->setCurrentRep(0, 0);
        m_lblCurrentExercise->setText(QString("Ćwiczenie: %1/%2")
                                      .arg(m_exercisePlan.size()).arg(m_exercisePlan.size()));
        m_lblCurrentSeries->setText("Seria: -");
        m_lblCurrentRep->setText("Powtórzenie: -");
        return;
    }
    const auto& exercise = m_exercisePlan[m_currentExerciseIndex];
    m_progressWidget->setCurrentSeries(m_currentSeriesIndex, exercise.seriesCount);
    m_progressWidget->setCurrentRep(m_currentRepIndex, exercise.repsPerSeries);
    m_lblCurrentExercise->setText(QString("Ćwiczenie: %1/%2")
                                  .arg(m_currentExerciseIndex + 1).arg(m_exercisePlan.size()));
    m_lblCurrentSeries->setText(QString("Seria: %1/%2")
                                .arg(qMin(m_currentSeriesIndex + 1, exercise.seriesCount))
                                .arg(exercise.seriesCount));
    m_lblCurrentRep->setText(QString("Powtórzenie: %1/%2")
                             .arg(qMin(m_currentRepIndex + 1, exercise.repsPerSeries))
                             .arg(exercise.repsPerSeries));
}

void TrainingTab::loadNextExercise()
{
    if (m_currentExerciseIndex >= m_exercisePlan.size()) {
        const QVector<ExerciseStats> completed = m_exerciseStats;
        stopTraining(true);
        emit trainingFinished(completed);
        return;
    }

    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    m_currentSeriesRepetitions.clear();
    const auto& exercise = m_exercisePlan[m_currentExerciseIndex];
    m_repetitionDetector.setTargetForce(exercise.targetForce);
    m_progressWidget->setTargetForce(exercise.targetForce);
    m_lblTargetForce->setText(QString("%1 N").arg(exercise.targetForce, 0, 'f', 1));
    const int gameIndex = m_gameTypeCombo->findData(exercise.gameType);
    if (gameIndex >= 0) m_gameTypeCombo->setCurrentIndex(gameIndex);
    configureGameForExercise();
    loadNextSeries();
    saveExecutionCheckpoint();
}

void TrainingTab::loadNextSeries()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size()) {
        loadNextExercise();
        return;
    }
    const auto& exercise = m_exercisePlan[m_currentExerciseIndex];
    if (m_currentSeriesIndex >= exercise.seriesCount) {
        calculateExerciseStats();
        ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
        stats.isCompleted = true;
        emit exerciseCompleted(m_currentExerciseIndex, stats);
        ++m_currentExerciseIndex;
        if (m_currentExerciseIndex >= m_exercisePlan.size()) {
            const QVector<ExerciseStats> completed = m_exerciseStats;
            stopTraining(true);
            emit trainingFinished(completed);
        } else {
            // The last repetition normally stages this rest in finishCurrentSeries.
            if (!m_stateMachine.resting()) {
                m_restTimeRemaining = 120;
                m_restDurationMs = 120000;
                if (!m_stateMachine.beginRest(biofeedback::training::RestTarget::NextExercise))
                    return;
                m_restClock.start();
            }
            if (m_stateMachine.restTarget() != biofeedback::training::RestTarget::NextExercise)
                return;
            m_lblRestTimer->setText(QString("Odpoczynek przed kolejnym ćwiczeniem: %1 s")
                                    .arg(m_restTimeRemaining));
            m_restTimer->start();
            saveExecutionCheckpoint();
        }
        return;
    }

    m_currentRepIndex = 0;
    m_peakForceInRep = 0.0;
    m_currentSeriesRepetitions.clear();
    m_repetitionDetector.setTargetForce(exercise.targetForce);
    if (m_currentGame) {
        m_currentGame->reset();
        m_currentGame->start();
        showGameWindow();
    }
    updateProgressWidgets();
    saveExecutionCheckpoint();
}

void TrainingTab::loadNextRep()
{
    m_peakForceInRep = 0.0;
    updateProgressWidgets();
    saveExecutionCheckpoint();
}

void TrainingTab::onRepetitionDetected(
    const biofeedback::training::RepetitionRecord& repetition)
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exerciseStats.size())
        return;

    m_peakForceInRep = repetition.peakForceN;
    m_currentSeriesRepetitions.push_back(repetition);

    ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
    TrainingRepetitionRecord record;
    record.seriesIndex = m_currentSeriesIndex + 1;
    record.repetitionIndex = m_currentRepIndex + 1;
    record.measurement = repetition;
    stats.repetitions.append(record);
    stats.completedReps = stats.repetitions.size();
    stats.maxPeakForce = qMax(stats.maxPeakForce, repetition.peakForceN);
    stats.avgPeakForce = 0.0;
    for (const auto& item : stats.repetitions)
        stats.avgPeakForce += item.measurement.peakForceN;
    stats.avgPeakForce /= static_cast<double>(stats.repetitions.size());
    stats.avgReactionTime = 0.0;
    for (const auto& item : stats.repetitions)
        stats.avgReactionTime += item.measurement.timeToPeakMs / 1000.0;
    stats.avgReactionTime /= static_cast<double>(stats.repetitions.size());
    refreshAnalytics();

    const auto seriesSummary = biofeedback::training::summarizeSeries(
        m_currentSeriesRepetitions, m_exercisePlan[m_currentExerciseIndex].targetForce);
    m_statsSummaryLabel->setText(
        QString("Powtórzenia: %1 | Serie ukończone: %2/%3\n"
                "Ostatnie: szczyt %4 N, średnia %5 N, czas %6 ms, czas do szczytu %7 ms\n"
                "Seria: średni szczyt %8 N, maksymalny %9 N")
            .arg(stats.completedReps).arg(stats.completedSeries)
            .arg(m_exercisePlan[m_currentExerciseIndex].seriesCount)
            .arg(repetition.peakForceN, 0, 'f', 2)
            .arg(repetition.meanForceN, 0, 'f', 2)
            .arg(repetition.durationMs).arg(repetition.timeToPeakMs)
            .arg(seriesSummary.averagePeakForceN, 0, 'f', 2)
            .arg(seriesSummary.maximumPeakForceN, 0, 'f', 2));
    emit repetitionCompleted(m_currentExerciseIndex, m_currentSeriesIndex, m_currentRepIndex);
    ++m_currentRepIndex;
    if (m_currentRepIndex >= m_exercisePlan[m_currentExerciseIndex].repsPerSeries) {
        finishCurrentSeries();
    } else {
        loadNextRep();
    }
    if (!saveExecutionCheckpoint())
        addDebugMessage("Nie udało się zapisać punktu kontrolnego powtórzenia.", "ERROR");
}

void TrainingTab::finishCurrentSeries()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size())
        return;

    const int completedSeriesIndex = m_currentSeriesIndex;
    const int completedExerciseIndex = m_currentExerciseIndex;
    const auto& exercise = m_exercisePlan[m_currentExerciseIndex];
    const auto summary = biofeedback::training::summarizeSeries(
        m_currentSeriesRepetitions, exercise.targetForce);
    ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
    stats.series.append(summary);
    stats.completedSeries = stats.series.size();
    refreshAnalytics();
    const double cv = biofeedback::training::summarizeSeries(
        stats.repetitions.isEmpty()
            ? std::vector<biofeedback::training::RepetitionRecord>{}
            : [&stats]() {
                std::vector<biofeedback::training::RepetitionRecord> all;
                all.reserve(static_cast<std::size_t>(stats.repetitions.size()));
                for (const auto& item : stats.repetitions) all.push_back(item.measurement);
                return all;
              }(), exercise.targetForce).peakForceCoefficientOfVariationPercent;
    stats.consistencyScore = qMax(0.0, 100.0 - cv);
    m_statsSummaryLabel->setText(
        QString("Powtórzenia: %1 | Serie ukończone: %2/%3\n"
                "Ostatnia seria: średni szczyt %4 N, maksymalny %5 N\n"
                "Czas powtórzenia: %6 ms | czas do szczytu: %7 ms\n"
                "Trafienia w cel: %8% | zmienność szczytu (CV): %9%")
            .arg(stats.completedReps).arg(stats.completedSeries)
            .arg(exercise.seriesCount)
            .arg(summary.averagePeakForceN, 0, 'f', 2)
            .arg(summary.maximumPeakForceN, 0, 'f', 2)
            .arg(summary.averageDurationMs, 0, 'f', 0)
            .arg(summary.averageTimeToPeakMs, 0, 'f', 0)
            .arg(summary.targetAchievementPercent, 0, 'f', 0)
            .arg(summary.peakForceCoefficientOfVariationPercent, 0, 'f', 1));
    emit seriesCompleted(m_currentExerciseIndex, completedSeriesIndex);

    if (m_currentGame && m_currentGame->isRunning()) m_currentGame->stop();
    // Compute the next cursor but do not publish it until all statistics
    // and the rest target have been staged.
    const bool exerciseWillFinish = (m_currentSeriesIndex + 1 >= exercise.seriesCount);
    const int nextSeriesIndex = exerciseWillFinish ? 0 : m_currentSeriesIndex + 1;
    m_currentSeriesIndex = nextSeriesIndex;
    m_currentRepIndex = 0;
    m_repetitionDetector.reset();

    if (exerciseWillFinish) {
        calculateExerciseStats();
        stats.isCompleted = true;
        emit exerciseCompleted(m_currentExerciseIndex, stats);
        ++m_currentExerciseIndex;
        if (m_currentExerciseIndex >= m_exercisePlan.size()) {
            const QVector<ExerciseStats> completed = m_exerciseStats;
            stopTraining(true);
            emit trainingFinished(completed);
            return;
        }
        m_currentSeriesIndex = 0;
        m_currentRepIndex = 0;
        m_restTimeRemaining = 120;
        m_restDurationMs = 120000;
    } else {
        m_restTimeRemaining = qMax(0, static_cast<int>(exercise.restTime));
        m_restDurationMs = static_cast<qint64>(m_restTimeRemaining) * 1000;
    }

    updateProgressWidgets();
    if (m_restTimeRemaining > 0) {
        updateSessionClocks();
        const auto target = m_currentExerciseIndex != completedExerciseIndex
            ? biofeedback::training::RestTarget::NextExercise
            : biofeedback::training::RestTarget::NextSeries;
        if (!m_stateMachine.beginRest(target)) {
            addDebugMessage("Nie można rozpocząć odpoczynku z bieżącego stanu.", "ERROR");
            return;
        }
        m_restClock.start();
        m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
        m_restTimer->start();
    } else {
        // No rest: advance directly without introducing a Resting transition.
        if (m_currentExerciseIndex != completedExerciseIndex)
            loadNextExercise();
        else
            loadNextSeries();
    }
    if (!saveExecutionCheckpoint())
        addDebugMessage("Nie udało się zapisać przejścia na następną serię.", "ERROR");
}

void TrainingTab::advanceAfterRest()
{
    if (!m_stateMachine.running() || m_stateMachine.paused()) return;
    updateRestDisplay();
    if (m_restTimeRemaining > 0) return;
    updateSessionClocks();
    m_restTimer->stop();
    const auto target = m_stateMachine.restTarget();
    m_restDurationMs = 0;
    m_restTimeRemaining = 0;
    m_restClock.invalidate();
    if (m_stateMachine.resting() && !m_stateMachine.paused()) {
        if (!m_stateMachine.endRest()) return;
    }
    m_lblRestTimer->clear();
    if (target == biofeedback::training::RestTarget::NextExercise) {
        loadNextExercise();
    } else {
        loadNextSeries();
    }
    saveExecutionCheckpoint();
}

void TrainingTab::calculateExerciseStats()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exerciseStats.size())
        return;
    ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
    std::vector<biofeedback::training::RepetitionRecord> all;
    all.reserve(static_cast<std::size_t>(stats.repetitions.size()));
    for (const auto& item : stats.repetitions) all.push_back(item.measurement);
    const double target = m_currentExerciseIndex < m_exercisePlan.size()
        ? m_exercisePlan[m_currentExerciseIndex].targetForce : 0.0;
    const auto summary = biofeedback::training::summarizeSeries(all, target);
    stats.completedReps = static_cast<int>(summary.repetitionCount);
    stats.maxPeakForce = summary.maximumPeakForceN;
    stats.avgPeakForce = summary.averagePeakForceN;
    stats.avgReactionTime = summary.averageTimeToPeakMs / 1000.0;
    stats.consistencyScore = qMax(0.0, 100.0 -
        summary.peakForceCoefficientOfVariationPercent);
    stats.completedSeries = stats.series.size();
    if (m_currentExerciseIndex < m_exercisePlan.size())
        stats.isCompleted = stats.completedSeries >= m_exercisePlan[m_currentExerciseIndex].seriesCount;
}

void TrainingTab::startRestTimer()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size())
        return;
    updateSessionClocks();
    m_restTimeRemaining = qMax(0, static_cast<int>(m_exercisePlan[m_currentExerciseIndex].restTime));
    m_restDurationMs = static_cast<qint64>(m_restTimeRemaining) * 1000;
    if (m_restTimeRemaining <= 0) {
        advanceAfterRest();
        return;
    }
    m_stateMachine.beginRest();
    m_restClock.start();
    m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
    if (!m_stateMachine.paused()) m_restTimer->start();
    saveExecutionCheckpoint();
}


QJsonObject TrainingTab::buildExecutionCheckpoint() const
{
    QJsonObject state;
    state.insert("format_version", 2);
    state.insert("state", static_cast<int>(m_stateMachine.state()));
    state.insert("resume_state", static_cast<int>(m_stateMachine.resumeState()));
    state.insert("current_exercise_index", m_currentExerciseIndex);
    state.insert("current_series_index", m_currentSeriesIndex);
    state.insert("current_repetition_index", m_currentRepIndex);
    state.insert("rest_target", static_cast<int>(m_stateMachine.restTarget()));
    state.insert("session_elapsed_ms", static_cast<double>(m_trainingClock.sessionMilliseconds()));
    state.insert("active_elapsed_ms", static_cast<double>(m_trainingClock.activeMilliseconds()));
    qint64 remainingRestMs = m_restDurationMs;
    if (m_stateMachine.resting() && !m_stateMachine.paused() && m_restClock.isValid())
        remainingRestMs = qMax<qint64>(0, m_restDurationMs - m_restClock.elapsed());
    state.insert("rest_remaining_ms", static_cast<double>(remainingRestMs));

    const auto detectorSnapshot = m_repetitionDetector.snapshot();
    QJsonObject detector;
    detector.insert("version", static_cast<int>(detectorSnapshot.version));
    detector.insert("active", detectorSnapshot.active);
    detector.insert("waiting_for_release", detectorSnapshot.waitingForRelease);
    detector.insert("start_timestamp_ms", static_cast<double>(detectorSnapshot.startTimestampMs));
    detector.insert("last_timestamp_ms", static_cast<double>(detectorSnapshot.lastTimestampMs));
    detector.insert("peak_timestamp_ms", static_cast<double>(detectorSnapshot.peakTimestampMs));
    detector.insert("peak_force_n", detectorSnapshot.peakForceN);
    detector.insert("area_force_ms", detectorSnapshot.areaForceMs);
    detector.insert("previous_force_n", detectorSnapshot.previousForceN);
    detector.insert("sample_count", static_cast<double>(detectorSnapshot.sampleCount));
    QJsonArray inFlightSamples;
    for (const auto& sample : detectorSnapshot.samples) {
        QJsonObject value;
        value.insert("timestamp_ms", static_cast<double>(sample.timestampMs));
        value.insert("force_n", sample.forceN);
        inFlightSamples.append(value);
    }
    detector.insert("samples", inFlightSamples);
    state.insert("repetition_detector", detector);

    QJsonArray plan;
    for (const auto& exercise : m_exercisePlan) {
        QJsonObject item;
        item.insert("name", exercise.name);
        item.insert("series_count", exercise.seriesCount);
        item.insert("repetitions_per_series", exercise.repsPerSeries);
        item.insert("target_force_n", exercise.targetForce);
        item.insert("rest_seconds", exercise.restTime);
        item.insert("game_type", exercise.gameType);
        plan.append(item);
    }
    state.insert("exercise_plan", plan);
    return state;
}

bool TrainingTab::saveExecutionCheckpoint()
{
    if (!m_analyticsWidget) return false;
    if (!m_stateMachine.running()) return true;
    updateSessionClocks();
    m_analyticsWidget->setExecutionState(buildExecutionCheckpoint());
    return m_analyticsWidget->saveSessionCheckpoint();
}

bool TrainingTab::restoreExecutionCheckpoint(const QJsonObject& checkpoint)
{
    const auto reject = [](const QString& reason) {
        qWarning().noquote() << "Checkpoint restore rejected at" << reason;
        return false;
    };
    qWarning() << "Checkpoint restore started; version =" << checkpoint.value("format_version").toInt()
               << "keys =" << checkpoint.keys();
    // Phase 1: parse and validate everything into temporary values. No live
    // session, analytics, timer, or widget state is changed in this phase.
    const int checkpointVersion = checkpoint.value("format_version").toInt();
    if ((checkpointVersion != 1 && checkpointVersion != 2) ||
        !checkpoint.value("exercise_plan").isArray() || !m_analyticsWidget)
        return reject(QStringLiteral("validation-1"));

    const auto isFiniteNonNegative = [](double value) {
        return std::isfinite(value) && value >= 0.0;
    };
    const auto readUnsignedTimestamp = [&reject](const QJsonValue& value, std::uint32_t& result) {
        if (!value.isDouble()) return reject(QStringLiteral("validation-2"));
        const double number = value.toDouble();
        if (!std::isfinite(number) || number < 0.0 ||
            number > static_cast<double>(std::numeric_limits<std::uint32_t>::max()) ||
            std::floor(number) != number) return reject(QStringLiteral("validation-3"));
        result = static_cast<std::uint32_t>(number);
        return true;
    };

    QVector<TrainingExercise> stagedPlan;
    const QJsonArray planValues = checkpoint.value("exercise_plan").toArray();
    if (planValues.isEmpty() || planValues.size() > 256) return reject(QStringLiteral("validation-4"));
    for (const QJsonValue& value : planValues) {
        if (!value.isObject()) return reject(QStringLiteral("validation-5"));
        const QJsonObject item = value.toObject();
        TrainingExercise exercise;
        exercise.name = item.value("name").toString();
        exercise.seriesCount = item.value("series_count").toInt(-1);
        exercise.repsPerSeries = item.value("repetitions_per_series").toInt(-1);
        exercise.targetForce = item.value("target_force_n").toDouble(-1.0);
        exercise.restTime = item.value("rest_seconds").toDouble(-1.0);
        exercise.gameType = item.value("game_type").toString();
        if (exercise.gameType == QStringLiteral("scored_earth"))
            exercise.gameType = QStringLiteral("scorched_earth");
        if (exercise.name.trimmed().isEmpty() || exercise.seriesCount < 1 ||
            exercise.seriesCount > 100 || exercise.repsPerSeries < 1 ||
            exercise.repsPerSeries > 1000 ||
            !isFiniteNonNegative(exercise.targetForce) || exercise.targetForce > 100000.0 ||
            !isFiniteNonNegative(exercise.restTime) || exercise.restTime > 86400.0)
            return reject(QStringLiteral("validation-6"));
        stagedPlan.append(exercise);
    }

    const int exerciseIndex = checkpoint.value("current_exercise_index").toInt(-1);
    const int seriesIndex = checkpoint.value("current_series_index").toInt(-1);
    const int repetitionIndex = checkpoint.value("current_repetition_index").toInt(-1);
    if (exerciseIndex < 0 || exerciseIndex >= stagedPlan.size() ||
        seriesIndex < 0 || seriesIndex >= stagedPlan[exerciseIndex].seriesCount ||
        repetitionIndex < 0 || repetitionIndex >= stagedPlan[exerciseIndex].repsPerSeries)
        return reject(QStringLiteral("validation-7"));

    const int savedStateValue = checkpoint.value("state").toInt(
        static_cast<int>(biofeedback::training::SessionState::Active));
    const int resumeStateValue = checkpoint.value("resume_state").toInt(
        static_cast<int>(biofeedback::training::SessionState::Active));
    if (savedStateValue < static_cast<int>(biofeedback::training::SessionState::Active) ||
        savedStateValue > static_cast<int>(biofeedback::training::SessionState::Resting) ||
        resumeStateValue < static_cast<int>(biofeedback::training::SessionState::Active) ||
        resumeStateValue > static_cast<int>(biofeedback::training::SessionState::Resting))
        return reject(QStringLiteral("validation-8"));

    const auto savedState = static_cast<biofeedback::training::SessionState>(savedStateValue);
    auto resumeState = static_cast<biofeedback::training::SessionState>(resumeStateValue);
    if (savedState == biofeedback::training::SessionState::Resting)
        resumeState = biofeedback::training::SessionState::Resting;
    else if (savedState != biofeedback::training::SessionState::Paused)
        resumeState = biofeedback::training::SessionState::Active;
    if (savedState != biofeedback::training::SessionState::Active &&
        savedState != biofeedback::training::SessionState::Paused &&
        savedState != biofeedback::training::SessionState::Resting)
        return reject(QStringLiteral("validation-9"));

    const int savedRestTarget = checkpoint.value("rest_target").toInt(
        static_cast<int>(biofeedback::training::RestTarget::NextSeries));
    if (savedRestTarget < static_cast<int>(biofeedback::training::RestTarget::NextSeries) ||
        savedRestTarget > static_cast<int>(biofeedback::training::RestTarget::NextExercise))
        return reject(QStringLiteral("validation-10"));
    const auto restTarget = static_cast<biofeedback::training::RestTarget>(savedRestTarget);

    const double restMsValue = checkpoint.value("rest_remaining_ms").toDouble(0.0);
    const double sessionMsValue = checkpoint.value("session_elapsed_ms").toDouble(-1.0);
    const double activeMsValue = checkpoint.value("active_elapsed_ms").toDouble(-1.0);
    if (!isFiniteNonNegative(restMsValue) || restMsValue > 86400000.0 ||
        !isFiniteNonNegative(sessionMsValue) || sessionMsValue > 9.0e15 ||
        !isFiniteNonNegative(activeMsValue) || activeMsValue > sessionMsValue)
        return reject(QStringLiteral("validation-11"));
    const qint64 stagedRestMs = static_cast<qint64>(std::floor(restMsValue));
    const qint64 sessionMs = static_cast<qint64>(std::floor(sessionMsValue));
    const qint64 activeMs = static_cast<qint64>(std::floor(activeMsValue));
    if (resumeState == biofeedback::training::SessionState::Resting && stagedRestMs <= 0)
        return reject(QStringLiteral("validation-12"));
    const qint64 effectiveRestMs = resumeState == biofeedback::training::SessionState::Resting
        ? stagedRestMs : 0;

    biofeedback::training::SessionStateMachine stagedStateMachine;
    if (!stagedStateMachine.restore(biofeedback::training::SessionState::Paused,
                                    resumeState, restTarget)) { addDebugMessage("Punkt kontrolny odrzucony: nieprawidłowy stan automatu sesji.", "ERROR"); return reject(QStringLiteral("validation-13")); }

    // Parse and validate the detector independently of the live detector.
    biofeedback::training::RepetitionDetector stagedDetector;
    stagedDetector.setTargetForce(stagedPlan[exerciseIndex].targetForce);
    const QJsonObject detectorObject = checkpoint.value("repetition_detector").toObject();
    const bool hasDetectorSnapshot = !detectorObject.isEmpty();
    if (checkpointVersion >= 2 && !hasDetectorSnapshot) return reject(QStringLiteral("validation-14"));
    if (hasDetectorSnapshot && resumeState == biofeedback::training::SessionState::Active) {
        biofeedback::training::RepetitionDetectorSnapshot detectorSnapshot;
        const int detectorVersion = detectorObject.value("version").toInt(-1);
        if (detectorVersion != 1 || !detectorObject.value("samples").isArray())
            return reject(QStringLiteral("validation-15"));
        detectorSnapshot.version = static_cast<std::uint32_t>(detectorVersion);
        detectorSnapshot.active = detectorObject.value("active").toBool(false);
        detectorSnapshot.waitingForRelease = detectorObject.value("waiting_for_release").toBool(false);
        if (!readUnsignedTimestamp(detectorObject.value("start_timestamp_ms"), detectorSnapshot.startTimestampMs) ||
            !readUnsignedTimestamp(detectorObject.value("last_timestamp_ms"), detectorSnapshot.lastTimestampMs) ||
            !readUnsignedTimestamp(detectorObject.value("peak_timestamp_ms"), detectorSnapshot.peakTimestampMs))
            return reject(QStringLiteral("validation-16"));
        detectorSnapshot.peakForceN = detectorObject.value("peak_force_n").toDouble(
            std::numeric_limits<double>::quiet_NaN());
        detectorSnapshot.areaForceMs = detectorObject.value("area_force_ms").toDouble(
            std::numeric_limits<double>::quiet_NaN());
        detectorSnapshot.previousForceN = detectorObject.value("previous_force_n").toDouble(
            std::numeric_limits<double>::quiet_NaN());
        const double sampleCount = detectorObject.value("sample_count").toDouble(-1.0);
        const QJsonArray samples = detectorObject.value("samples").toArray();
        if (!std::isfinite(sampleCount) || sampleCount < 0.0 || sampleCount > 100000.0 ||
            std::floor(sampleCount) != sampleCount || samples.size() != static_cast<int>(sampleCount))
            return reject(QStringLiteral("validation-17"));
        detectorSnapshot.sampleCount = static_cast<std::size_t>(sampleCount);
        detectorSnapshot.samples.reserve(static_cast<std::size_t>(samples.size()));
        for (const QJsonValue& value : samples) {
            if (!value.isObject()) return reject(QStringLiteral("validation-18"));
            const QJsonObject sample = value.toObject();
            std::uint32_t timestamp = 0;
            const double force = sample.value("force_n").toDouble(
                std::numeric_limits<double>::quiet_NaN());
            if (!readUnsignedTimestamp(sample.value("timestamp_ms"), timestamp) ||
                !isFiniteNonNegative(force)) return reject(QStringLiteral("validation-19"));
            detectorSnapshot.samples.push_back({timestamp, force});
        }
        if (!stagedDetector.restore(detectorSnapshot)) { addDebugMessage("Punkt kontrolny odrzucony: nieprawidłowy stan detektora.", "ERROR"); return reject(QStringLiteral("validation-20")); }
    }

    // The statistics are already loaded from the recovered history entry.
    // Stage their shape before committing any of the execution state.
    const QVector<ExerciseStats> stagedStats = m_analyticsWidget->currentStats();
    if (stagedStats.size() != stagedPlan.size()) { addDebugMessage(QString("Punkt kontrolny odrzucony: liczba zestawów statystyk (%1) różni się od planu (%2).").arg(stagedStats.size()).arg(stagedPlan.size()), "ERROR"); return reject(QStringLiteral("validation-21")); }
    for (int index = 0; index < stagedPlan.size(); ++index) {
        if (stagedStats[index].exerciseName != stagedPlan[index].name) { addDebugMessage(QString("Punkt kontrolny odrzucony: nazwa ćwiczenia %1 nie zgadza się z planem %2.").arg(stagedStats[index].exerciseName, stagedPlan[index].name), "ERROR"); return reject(QStringLiteral("validation-22")); }
    }

    std::vector<biofeedback::training::RepetitionRecord> stagedSeriesRepetitions;
    for (const auto& record : stagedStats[exerciseIndex].repetitions) {
        if (record.seriesIndex == seriesIndex + 1)
            stagedSeriesRepetitions.push_back(record.measurement);
    }

    // Phase 2: commit. All rejectable input has been checked above.
    m_exercisePlan = std::move(stagedPlan);
    m_exerciseStats = std::move(stagedStats);
    m_analyticsWidget->setExecutionState(checkpoint);
    m_currentExerciseIndex = exerciseIndex;
    m_currentSeriesIndex = seriesIndex;
    m_currentRepIndex = repetitionIndex;
    m_restDurationMs = effectiveRestMs;
    m_restTimeRemaining = static_cast<int>((effectiveRestMs + 999) / 1000);
    m_restClock.invalidate();
    m_sessionElapsedOffsetMs = sessionMs;
    m_trainingClock.restore(sessionMs, activeMs);
    m_sessionTime = static_cast<int>(sessionMs / 1000);
    m_sessionClock.start();
    m_stateMachine = stagedStateMachine;
    m_repetitionDetector = stagedDetector;
    m_currentSeriesRepetitions = std::move(stagedSeriesRepetitions);

    m_exerciseSelector->clear();
    for (const auto& exercise : m_exercisePlan) m_exerciseSelector->addItem(exercise.name);
    m_exerciseSelector->setCurrentIndex(m_currentExerciseIndex);
    m_configBox->setEnabled(false);
    m_btnStartStop->setText("Stop");
    m_btnStartStop->setStyleSheet("background-color: #cc4444; color: white;");
    m_btnPauseResume->setEnabled(true);
    m_btnPauseResume->setText("Wznów");
    configureGameForExercise();
    if (m_currentGame) {
        m_currentGame->reset();
        m_currentGame->start();
        m_currentGame->pause();
    }
    updateProgressWidgets();
    updateDisplay();
    if (m_stateMachine.resting()) {
        m_lblRestTimer->setText(QString("Odpoczynek: %1 s — sesja odzyskana, wstrzymana").arg(m_restTimeRemaining));
        m_restTimer->stop();
    } else {
        m_lblRestTimer->clear();
    }
    m_timer->start();
    return true;
}

void TrainingTab::updateRestDisplay()
{
    if (!m_restClock.isValid()) return;
    const qint64 remainingMs = qMax<qint64>(
        0, m_restDurationMs - m_restClock.elapsed());
    m_restTimeRemaining = static_cast<int>((remainingMs + 999) / 1000);
    m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
}

QString TrainingTab::getExerciseDisplayName(int index) const
{
    if (index >= 0 && index < m_exercisePlan.size()) {
        return m_exercisePlan[index].name;
    }
    return "";
}

QColor TrainingTab::getForceColor(double current, double target) const
{
    if (target <= 0) return Qt::gray;
    
    double ratio = current / target;
    if (ratio < 0.5) return QColor(255, 100, 100);
    if (ratio < 0.8) return QColor(255, 200, 100);
    if (ratio < 1.0) return QColor(100, 255, 100);
    return QColor(50, 200, 50);
}

// ==================== Debug Terminal Methods ====================

void TrainingTab::addDebugMessage(const QString& message, const QString& type)
{
    // Wysyłaj wiadomość do centralnego DebugManager, który przekaże ją do DebugTab
    core::DebugManager::instance().sendDebugMessage(message, type);
}

} // namespace tab
