#include "tab/TrainingTab.hpp"
#include "tab/TrainingAnalyticsWidget.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QStackedWidget>
#include <QDir>
#include <cmath>
#include "games/PongGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/MarioGame.hpp"
#include "games/SinGame.hpp"
#include "measurement/Units.hpp"

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
    setMinimumHeight(150);
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
    m_currentForce = force;
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
    
    // Sekcja 1: Ćwiczenie
    QRect exerciseRect(margin, margin, sectionWidth - margin, height() - 2 * margin);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(exerciseRect, Qt::AlignTop | Qt::AlignHCenter, 
                    QString("Ćwiczenie %1/%2").arg(m_currentExercise + 1).arg(m_totalExercises));
    
    // Sekcja 2: Seria
    QRect seriesRect(margin + sectionWidth, margin, sectionWidth - margin, height() - 2 * margin);
    painter.drawText(seriesRect, Qt::AlignTop | Qt::AlignHCenter,
                    QString("Seria %1/%2").arg(m_currentSeries + 1).arg(m_totalSeries));
    
    // Sekcja 3: Powtórzenie
    QRect repRect(margin + 2 * sectionWidth, margin, sectionWidth - margin, height() - 2 * margin);
    painter.drawText(repRect, Qt::AlignTop | Qt::AlignHCenter,
                    QString("Powtórzenie %1/%2").arg(m_currentRep + 1).arg(m_totalReps));
    
    // Pasek siły
    int barHeight = 20;
    int barY = height() - barHeight - margin;
    int barWidth = width() - 2 * margin;
    
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
    , m_activeGameType("none")
    , m_statsBox(nullptr)
    , m_statsContainer(nullptr)
    , m_statsSummaryLabel(nullptr)
    , m_analyticsWidget(nullptr)
    , m_timer(nullptr)
    , m_restTimer(nullptr)
    , m_isTraining(false)
    , m_isPaused(false)
    , m_currentExerciseIndex(0)
    , m_currentSeriesIndex(0)
    , m_currentRepIndex(0)
    , m_sessionTime(0)
    , m_restTimeRemaining(0)
    , m_inRestPeriod(false)
    , m_restAdvancesExercise(false)
    , m_repetitionDetector()
    , m_currentSeriesRepetitions()
    , m_currentForce(0.0)
    , m_peakForceInRep(0.0)
    , m_serialPort(nullptr)
{
    setupUI();
}

TrainingTab::~TrainingTab()
{
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    stopTraining();
}

void TrainingTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Panel konfiguracji
    m_configBox = new QGroupBox("Konfiguracja ćwiczenia", this);
    m_configBox->setFont(QFont("Arial", 11, QFont::Bold));
    QHBoxLayout* configLayout = new QHBoxLayout(m_configBox);
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
    m_gameTypeCombo->addItem("Flappy Bird", "flappy");
    m_gameTypeCombo->addItem("Mario", "mario");
    m_gameTypeCombo->addItem("Sinusoidalna", "sin");
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
    
    m_mainLayout->addWidget(m_configBox);
    
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
    
    m_mainLayout->addLayout(m_controlLayout);
    
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
    
    m_mainLayout->addWidget(m_progressBox);
    
    // Widget gry - miejsce na silnik gier
    m_gameBox = new QGroupBox("Gra treningowa", this);
    QVBoxLayout* gameLayout = new QVBoxLayout(m_gameBox);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameLayout->setSpacing(0);
    
    // Placeholder dla gry - będzie zastąpiony przez rzeczywisty widget gry
    QLabel* gamePlaceholder = new QLabel("Wybierz grę i rozpocznij trening", m_gameBox);
    gamePlaceholder->setAlignment(Qt::AlignCenter);
    gamePlaceholder->setMinimumHeight(400);
    gamePlaceholder->setStyleSheet("background-color: #1a1a1a; color: #888; font-size: 18px; border: 2px solid #333;");
    gameLayout->addWidget(gamePlaceholder);
    
    m_mainLayout->addWidget(m_gameBox, 1);  // Stretch factor 1 to give more space
    
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
    
    m_mainLayout->addWidget(m_statsBox);
    
    // TrainingTab nie posiada własnego terminala debugowego.
    // Wszystkie wiadomości debugowe są wysyłane do DebugManager,
    // który przekazuje je do centralnej zakładki DebugTab.
    
    // Timer
    m_timer = new QTimer(this);
    m_timer->setInterval(100);  // 10 Hz
    
    m_restTimer = new QTimer(this);
    m_restTimer->setInterval(1000);  // 1 Hz
    
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
        m_restTimeRemaining--;
        updateRestDisplay();
        if (m_restTimeRemaining <= 0) {
            m_restTimer->stop();
            m_inRestPeriod = false;
            m_lblRestTimer->setText("");
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
    if (m_exercisePlan.isEmpty()) {
        QMessageBox::warning(this, "Brak planu", "Dodaj ćwiczenia przed rozpoczęciem treningu.");
        return;
    }
    if (!m_serialPort || !m_serialPort->isConnected()) {
        QMessageBox::warning(this, tr("Brak urządzenia"),
                             tr("Podłącz czujnik HX711 przed rozpoczęciem treningu."));
        return;
    }

    m_isTraining = true;
    m_isPaused = false;
    m_sessionTime = 0;
    m_currentExerciseIndex = 0;
    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    m_inRestPeriod = false;
    m_restAdvancesExercise = false;
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
    
    emit trainingStarted();
}

void TrainingTab::stopTraining()
{
    if (!m_isTraining) return;
    
    m_timer->stop();
    m_restTimer->stop();
    
    m_isTraining = false;
    m_isPaused = false;
    m_inRestPeriod = false;
    m_restAdvancesExercise = false;
    m_repetitionDetector.reset();
    
    // Zatrzymanie gry jeśli aktywna
    if (m_currentGame && m_currentGame->isRunning()) {
        m_currentGame->stop();
    }
    
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
    if (!m_isTraining || m_isPaused) return;
    
    m_isPaused = true;
    m_timer->stop();
    m_restTimer->stop();
    
    if (m_currentGame && m_currentGame->isRunning()) {
        m_currentGame->pause();
    }
    
    m_btnPauseResume->setText("Wznów");
    
    emit trainingPaused();
}

void TrainingTab::resumeTraining()
{
    if (!m_isTraining || !m_isPaused) return;
    
    m_isPaused = false;
    m_timer->start();
    if (m_inRestPeriod) {
        m_restTimer->start();
    }
    
    if (m_currentGame && m_currentGame->isPaused()) {
        m_currentGame->resume();
    }
    
    m_btnPauseResume->setText("Pauza");
    
    emit trainingResumed();
}

bool TrainingTab::isTrainingActive() const
{
    return m_isTraining && !m_isPaused;
}

bool TrainingTab::isTrainingPaused() const
{
    return m_isPaused;
}

void TrainingTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    m_serialPort = serial;

    if (m_serialPort) {
        m_dataCallbackToken = m_serialPort->addDataCallback([this](const sensor::SensorData& data) {
            QMetaObject::invokeMethod(this, [this, data]() {
                onSensorDataReceived(data);
            }, Qt::QueuedConnection);
        });
    }
}

void TrainingTab::setCurrentPatientContext(const QString& patientPesel,
                                             const QString& patientDataPath)
{
    if (m_isTraining) return;
    m_currentPatientPesel = patientPesel;
    m_currentPatientDataPath = patientDataPath;
    if (m_analyticsWidget) {
        const QString historyPath = patientDataPath.isEmpty()
            ? QString()
            : QDir(patientDataPath).filePath(QStringLiteral("training_history.json"));
        m_analyticsWidget->setHistoryFilePath(historyPath);
    }
}

void TrainingTab::setGameEngine(games::GameEngine* game)
{
    m_currentGame = game;
    if (!m_currentGame) return;

    m_currentGame->setParent(m_gameBox);
    m_currentGame->setMinimumHeight(400);
    if (m_gameBox->layout() && m_gameBox->layout()->indexOf(m_currentGame) < 0) {
        while (QLayoutItem* item = m_gameBox->layout()->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
        m_gameBox->layout()->addWidget(m_currentGame);
    }

    disconnect(m_currentGame, nullptr, this, nullptr);
    connect(m_currentGame, &games::GameEngine::gameEnded, this, &TrainingTab::onGameEnded);
    connect(m_currentGame, &games::GameEngine::gameStarted, this, &TrainingTab::onGameStarted);
    connect(m_currentGame, &games::GameEngine::scoreChanged, this, &TrainingTab::onScoreChanged);

    if (m_serialPort) {
        if (auto* game = dynamic_cast<games::PongGame*>(m_currentGame))
            game->setSerialConnection(m_serialPort);
        if (auto* game = dynamic_cast<games::FlappyBirdGame*>(m_currentGame))
            game->setSerialConnection(m_serialPort);
        if (auto* game = dynamic_cast<games::MarioGame*>(m_currentGame))
            game->setSerialConnection(m_serialPort);
        if (auto* game = dynamic_cast<games::SinGame*>(m_currentGame))
            game->setSerialConnection(m_serialPort);
    }
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

    if (m_currentGame) {
        if (m_currentGame->isRunning()) m_currentGame->stop();
        if (m_gameBox->layout()) m_gameBox->layout()->removeWidget(m_currentGame);
        delete m_currentGame;
        m_currentGame = nullptr;
    }
    if (m_gameBox->layout()) {
        while (QLayoutItem* item = m_gameBox->layout()->takeAt(0)) {
            if (QWidget* widget = item->widget()) delete widget;
            delete item;
        }
    }

    m_activeGameType = type;
    games::GameEngine* selectedGame = nullptr;
    if (type == "pong") selectedGame = new games::PongGame(m_gameBox);
    else if (type == "flappy" || type == "flappy_bird")
        selectedGame = new games::FlappyBirdGame(m_gameBox);
    else if (type == "mario" || type == "mario_run")
        selectedGame = new games::MarioGame(m_gameBox);
    else if (type == "sin")
        selectedGame = new games::SinGame(m_gameBox);

    if (selectedGame) {
        m_gameBox->layout()->addWidget(selectedGame);
        setGameEngine(selectedGame);
    } else {
        auto* placeholder = new QLabel(
            type == "none" ? tr("Trening bez gry") : tr("Nieznany typ gry: %1").arg(type),
            m_gameBox);
        placeholder->setAlignment(Qt::AlignCenter);
        placeholder->setMinimumHeight(400);
        placeholder->setStyleSheet("background-color:#1a1a1a;color:#888;font-size:18px;");
        m_gameBox->layout()->addWidget(placeholder);
    }
}

QVector<ExerciseStats> TrainingTab::getTrainingStats() const
{
    return m_exerciseStats;
}

void TrainingTab::refreshAnalytics()
{
    if (m_analyticsWidget) m_analyticsWidget->setStats(m_exerciseStats, m_sessionTime);
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
    if (m_isTraining) {
        stopTraining();
    } else {
        startTraining();
    }
}

void TrainingTab::onPauseResumeClicked()
{
    if (m_isPaused) {
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
    if (!m_isTraining || m_isPaused) return;
    
    m_sessionTime++;
    
    // Aktualizacja wyświetlacza czasu
    int minutes = m_sessionTime / 60;
    int seconds = m_sessionTime % 60;
    m_lblTimer->setText(QString("Czas: %1:%2")
                        .arg(minutes, 2, 10, QChar('0'))
                        .arg(seconds, 2, 10, QChar('0')));
    
    updateProgressWidgets();
}

void TrainingTab::onSensorDataReceived(const sensor::SensorData& data)
{
    if (!m_isTraining || m_isPaused || m_inRestPeriod ||
        !data.isValid || m_currentExerciseIndex < 0 ||
        m_currentExerciseIndex >= m_exercisePlan.size()) return;

    // Firmware reports calibrated mass-equivalent values in grams. Training
    // targets are expressed in newtons (SI), so convert exactly once here.
    const double force = biofeedback::measurement::gramsToNewtons(data.calibratedValue);
    if (!std::isfinite(force) || force < 0.0) return;
    m_currentForce = force;

    m_lblCurrentForce->setText(QString("%1 N").arg(force, 0, 'f', 2));
    m_progressWidget->setCurrentForce(force);

    const double targetForce = m_exercisePlan[m_currentExerciseIndex].targetForce;
    const int barValue = targetForce > 0.0
        ? static_cast<int>((force / targetForce) * 100.0) : 0;
    m_forceBar->setValue(qBound(0, barValue, 100));

    if (m_currentGame && m_currentGame->isRunning()) {
        const double gameInput = targetForce > 0.0
            ? qBound(0.0, force / targetForce * 100.0, 100.0) : 0.0;
        m_currentGame->handleSensorInput(gameInput);
    }

    biofeedback::training::RepetitionRecord repetition;
    if (m_repetitionDetector.process(data.timestamp, force, true, repetition))
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
        stopTraining();
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
            stopTraining();
            emit trainingFinished(completed);
        } else {
            m_inRestPeriod = true;
            m_restAdvancesExercise = true;
            m_restTimeRemaining = 120;
            m_lblRestTimer->setText(QString("Odpoczynek przed kolejnym ćwiczeniem: %1 s")
                                    .arg(m_restTimeRemaining));
            if (m_restTimeRemaining == 0) advanceAfterRest();
            else m_restTimer->start();
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
    }
    updateProgressWidgets();
}

void TrainingTab::loadNextRep()
{
    m_peakForceInRep = 0.0;
    updateProgressWidgets();
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
}

void TrainingTab::finishCurrentSeries()
{
    if (m_currentExerciseIndex < 0 || m_currentExerciseIndex >= m_exercisePlan.size())
        return;

    const int completedSeriesIndex = m_currentSeriesIndex;
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
    ++m_currentSeriesIndex;
    m_currentRepIndex = 0;
    m_repetitionDetector.reset();

    if (m_currentSeriesIndex >= exercise.seriesCount) {
        calculateExerciseStats();
        stats.isCompleted = true;
        emit exerciseCompleted(m_currentExerciseIndex, stats);
        ++m_currentExerciseIndex;
        if (m_currentExerciseIndex >= m_exercisePlan.size()) {
            const QVector<ExerciseStats> completed = m_exerciseStats;
            stopTraining();
            emit trainingFinished(completed);
            return;
        }
        m_currentSeriesIndex = 0;
        m_currentRepIndex = 0;
        m_restAdvancesExercise = true;
        m_restTimeRemaining = 120;
    } else {
        m_restAdvancesExercise = false;
        m_restTimeRemaining = static_cast<int>(exercise.restTime);
    }

    updateProgressWidgets();
    if (m_restTimeRemaining > 0) {
        m_inRestPeriod = true;
        m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
        m_restTimer->start();
    } else {
        advanceAfterRest();
    }
}

void TrainingTab::advanceAfterRest()
{
    if (!m_isTraining || m_isPaused) return;
    m_inRestPeriod = false;
    m_lblRestTimer->clear();
    if (m_restAdvancesExercise) {
        m_restAdvancesExercise = false;
        loadNextExercise();
    } else {
        loadNextSeries();
    }
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
    const auto& ex = m_exercisePlan[m_currentExerciseIndex];
    m_restTimeRemaining = static_cast<int>(ex.restTime);
    m_inRestPeriod = true;
    m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
    m_restTimer->start();
}

void TrainingTab::updateRestDisplay()
{
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
