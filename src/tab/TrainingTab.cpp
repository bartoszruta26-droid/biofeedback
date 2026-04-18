#include "tab/TrainingTab.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QStackedWidget>
#include "games/PongGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/MarioGame.hpp"
#include "games/SinGame.hpp"

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
    , m_statsBox(nullptr)
    , m_statsContainer(nullptr)
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
    , m_currentForce(0.0)
    , m_peakForceInRep(0.0)
    , m_serialPort(nullptr)
{
    setupUI();
}

TrainingTab::~TrainingTab()
{
    stopTraining();
}

void TrainingTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Panel konfiguracji
    m_configBox = new QGroupBox("Konfiguracja ćwiczenia", this);
    QHBoxLayout* configLayout = new QHBoxLayout(m_configBox);
    
    m_exerciseSelector = new QComboBox(this);
    m_exerciseSelector->setMinimumWidth(200);
    configLayout->addWidget(new QLabel("Ćwiczenie:", this));
    configLayout->addWidget(m_exerciseSelector);
    
    configLayout->addWidget(new QLabel("Siła celu [N]:", this));
    m_targetForceSpin = new QDoubleSpinBox(this);
    m_targetForceSpin->setRange(0, 500);
    m_targetForceSpin->setValue(50.0);
    m_targetForceSpin->setDecimals(1);
    configLayout->addWidget(m_targetForceSpin);
    
    configLayout->addWidget(new QLabel("Serie:", this));
    m_seriesSpin = new QSpinBox(this);
    m_seriesSpin->setRange(1, 20);
    m_seriesSpin->setValue(3);
    configLayout->addWidget(m_seriesSpin);
    
    configLayout->addWidget(new QLabel("Powtórzenia:", this));
    m_repsSpin = new QSpinBox(this);
    m_repsSpin->setRange(1, 50);
    m_repsSpin->setValue(8);
    configLayout->addWidget(m_repsSpin);
    
    configLayout->addWidget(new QLabel("Odpoczynek [s]:", this));
    m_restTimeSpin = new QSpinBox(this);
    m_restTimeSpin->setRange(0, 300);
    m_restTimeSpin->setValue(60);
    configLayout->addWidget(m_restTimeSpin);
    
    configLayout->addWidget(new QLabel("Gra:", this));
    m_gameTypeCombo = new QComboBox(this);
    m_gameTypeCombo->addItem("Pong", "pong");
    m_gameTypeCombo->addItem("Flappy Bird", "flappy");
    m_gameTypeCombo->addItem("Mario", "mario");
    configLayout->addWidget(m_gameTypeCombo);
    
    m_btnAddExercise = new QPushButton("Dodaj", this);
    m_btnAddExercise->setMaximumWidth(80);
    configLayout->addWidget(m_btnAddExercise);
    
    m_btnRemoveExercise = new QPushButton("Usuń", this);
    m_btnRemoveExercise->setMaximumWidth(80);
    configLayout->addWidget(m_btnRemoveExercise);
    
    m_mainLayout->addWidget(m_configBox);
    
    // Panel sterowania
    m_controlLayout = new QHBoxLayout();
    
    m_btnStartStop = new QPushButton("Start", this);
    m_btnStartStop->setMinimumHeight(40);
    m_btnStartStop->setFont(QFont("Arial", 12, QFont::Bold));
    m_controlLayout->addWidget(m_btnStartStop);
    
    m_btnPauseResume = new QPushButton("Pauza", this);
    m_btnPauseResume->setMinimumHeight(40);
    m_btnPauseResume->setFont(QFont("Arial", 12));
    m_btnPauseResume->setEnabled(false);
    m_controlLayout->addWidget(m_btnPauseResume);
    
    m_btnSave = new QPushButton("Zapisz wyniki", this);
    m_controlLayout->addWidget(m_btnSave);
    
    m_btnLoad = new QPushButton("Wczytaj plan", this);
    m_controlLayout->addWidget(m_btnLoad);
    
    m_btnExport = new QPushButton("Eksportuj plan", this);
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
    
    // Widget gry
    m_gameBox = new QGroupBox("Gra treningowa", this);
    QVBoxLayout* gameLayout = new QVBoxLayout(m_gameBox);
    gameLayout->setContentsMargins(5, 5, 5, 5);
    
    // Placeholder dla gry - będzie zastąpiony przez rzeczywisty widget gry
    QLabel* gamePlaceholder = new QLabel("Wybierz grę i rozpocznij trening", this);
    gamePlaceholder->setAlignment(Qt::AlignCenter);
    gamePlaceholder->setMinimumHeight(200);
    gamePlaceholder->setStyleSheet("background-color: #1a1a1a; color: #888; font-size: 16px;");
    gameLayout->addWidget(gamePlaceholder);
    
    m_mainLayout->addWidget(m_gameBox);
    
    // Tabela statystyk
    m_statsBox = new QGroupBox("Statystyki sesji", this);
    m_statsBox->setMaximumHeight(200);
    QVBoxLayout* statsLayout = new QVBoxLayout(m_statsBox);
    
    m_statsContainer = new QWidget(this);
    statsLayout->addWidget(m_statsContainer);
    
    m_mainLayout->addWidget(m_statsBox);
    
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
            loadNextSeries();
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
    
    m_isTraining = true;
    m_isPaused = false;
    m_sessionTime = 0;
    m_currentExerciseIndex = 0;
    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    
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
    m_serialPort = serial;
    
    if (m_serialPort) {
        m_serialPort->setDataCallback([this](const sensor::SensorData& data) {
            // Callback w wątku sensora - używamy QMetaObject dla bezpieczeństwa
            QMetaObject::invokeMethod(this, "onSensorDataReceived", 
                                     Qt::QueuedConnection,
                                     Q_ARG(sensor::SensorData, data));
        });
    }
}

void TrainingTab::setGameEngine(games::GameEngine* game)
{
    m_currentGame = game;
    
    if (m_currentGame) {
        // Podłączenie sygnałów gry
        connect(m_currentGame, &games::GameEngine::gameEnded, this, &TrainingTab::onGameEnded);
        connect(m_currentGame, &games::GameEngine::gameStarted, this, &TrainingTab::onGameStarted);
        connect(m_currentGame, &games::GameEngine::scoreChanged, this, &TrainingTab::onScoreChanged);
        
        // Jeśli mamy SerialCommunication, podłącz go do gry
        if (m_serialPort) {
            // Spróbuj rzutować na konkretny typ gry i podłącz SerialCommunication
            auto pongGame = dynamic_cast<games::PongGame*>(m_currentGame);
            if (pongGame) {
                pongGame->setSerialConnection(m_serialPort);
            }
            
            auto flappyGame = dynamic_cast<games::FlappyBirdGame*>(m_currentGame);
            if (flappyGame) {
                flappyGame->setSerialConnection(m_serialPort);
            }
            
            auto marioGame = dynamic_cast<games::MarioGame*>(m_currentGame);
            if (marioGame) {
                marioGame->setSerialConnection(m_serialPort);
            }
            
            auto sinGame = dynamic_cast<games::SinGame*>(m_currentGame);
            if (sinGame) {
                sinGame->setSerialConnection(m_serialPort);
            }
        }
    }
}

QVector<ExerciseStats> TrainingTab::getTrainingStats() const
{
    return m_exerciseStats;
}

bool TrainingTab::exportResultsToCSV(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Nagłówek
    out << "Ćwiczenie,Seria,Powtórzenie,Siła szczytowa,Czas do szczytu,Wynik\n";
    
    for (const auto& stats : m_exerciseStats) {
        out << stats.exerciseName << ","
            << stats.completedSeries << ","
            << stats.completedReps << ","
            << stats.avgPeakForce << ","
            << stats.avgReactionTime << ","
            << stats.consistencyScore << "\n";
    }
    
    file.close();
    return true;
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
    if (!m_isTraining || m_isPaused) return;
    
    double force = data.calibratedValue;
    m_currentForce = force;
    
    // Śledzenie siły szczytowej w powtórzeniu
    if (force > m_peakForceInRep) {
        m_peakForceInRep = force;
    }
    
    // Aktualizacja UI
    m_lblCurrentForce->setText(QString("%1 N").arg(force, 0, 'f', 1));
    m_progressWidget->setCurrentForce(force);
    
    double targetForce = 0.0;
    if (!m_exercisePlan.isEmpty() && m_currentExerciseIndex < m_exercisePlan.size()) {
        targetForce = m_exercisePlan[m_currentExerciseIndex].targetForce;
    }
    int barValue = targetForce > 0 ? static_cast<int>((force / targetForce) * 100) : 0;
    m_forceBar->setValue(qBound(0, barValue, 100));
    
    // Przekazanie danych do gry
    if (m_currentGame && m_currentGame->isRunning()) {
        m_currentGame->handleSensorInput(force);
    }
    
    emit newForceSample(force);
}

void TrainingTab::onGameEnded(int score)
{
    // Gra zakończona - można zarejestrować wynik
    calculateExerciseStats();
}

void TrainingTab::onGameStarted()
{
    // Gra rozpoczęta
}

void TrainingTab::onScoreChanged(int score)
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
    m_progressWidget->setCurrentSeries(m_currentSeriesIndex, 
                                       m_exercisePlan[m_currentExerciseIndex].seriesCount);
    m_progressWidget->setCurrentRep(m_currentRepIndex,
                                    m_exercisePlan[m_currentExerciseIndex].repsPerSeries);
    m_progressWidget->setSessionTime(m_sessionTime);
    
    m_lblCurrentExercise->setText(QString("Ćwiczenie: %1/%2")
                                  .arg(m_currentExerciseIndex + 1)
                                  .arg(m_exercisePlan.size()));
    m_lblCurrentSeries->setText(QString("Seria: %1/%2")
                                .arg(m_currentSeriesIndex + 1)
                                .arg(m_exercisePlan[m_currentExerciseIndex].seriesCount));
    m_lblCurrentRep->setText(QString("Powtórzenie: %1/%2")
                             .arg(m_currentRepIndex + 1)
                             .arg(m_exercisePlan[m_currentExerciseIndex].repsPerSeries));
}

void TrainingTab::loadNextExercise()
{
    if (m_currentExerciseIndex >= m_exercisePlan.size()) {
        // Wszystkie ćwiczenia zakończone
        stopTraining();
        emit trainingFinished(m_exerciseStats);
        return;
    }
    
    m_currentSeriesIndex = 0;
    m_currentRepIndex = 0;
    
    const auto& ex = m_exercisePlan[m_currentExerciseIndex];
    
    // Konfiguracja gry dla tego ćwiczenia
    if (m_gameTypeCombo->currentData().toString() != ex.gameType) {
        // TODO: Przełączenie typu gry
    }
    
    loadNextSeries();
}

void TrainingTab::loadNextSeries()
{
    const auto& ex = m_exercisePlan[m_currentExerciseIndex];
    
    if (m_currentSeriesIndex >= ex.seriesCount) {
        // Seria zakończona, następne ćwiczenie
        m_currentExerciseIndex++;
        loadNextExercise();
        return;
    }
    
    m_currentRepIndex = 0;
    m_peakForceInRep = 0;
    
    // Start gry dla tej serii
    if (m_currentGame) {
        m_currentGame->reset();
        m_currentGame->start();
    }
    
    updateProgressWidgets();
}

void TrainingTab::loadNextRep()
{
    const auto& ex = m_exercisePlan[m_currentExerciseIndex];
    
    if (m_currentRepIndex >= ex.repsPerSeries) {
        // Powtórzenia w serii zakończone
        checkExerciseCompletion();
        return;
    }
    
    m_peakForceInRep = 0;
    updateProgressWidgets();
}

void TrainingTab::checkExerciseCompletion()
{
    const auto& ex = m_exercisePlan[m_currentExerciseIndex];
    
    if (m_currentSeriesIndex >= ex.seriesCount) {
        // Ćwiczenie zakończone
        calculateExerciseStats();
        
        ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
        stats.isCompleted = true;
        
        emit exerciseCompleted(m_currentExerciseIndex, stats);
        
        // Następne ćwiczenie
        m_currentExerciseIndex++;
        
        // Odpoczynek między ćwiczeniami
        if (m_currentExerciseIndex < m_exercisePlan.size()) {
            m_inRestPeriod = true;
            m_restTimeRemaining = 120;  // 2 minuty odpoczynku między ćwiczeniami
            m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
            m_restTimer->start();
        } else {
            stopTraining();
            emit trainingFinished(m_exerciseStats);
        }
    } else {
        // Odpoczynek między seriami
        m_inRestPeriod = true;
        m_restTimeRemaining = static_cast<int>(ex.restTime);
        m_lblRestTimer->setText(QString("Odpoczynek: %1 s").arg(m_restTimeRemaining));
        m_restTimer->start();
    }
}

void TrainingTab::calculateExerciseStats()
{
    ExerciseStats& stats = m_exerciseStats[m_currentExerciseIndex];
    stats.completedSeries = m_currentSeriesIndex;
    stats.completedReps = m_currentRepIndex;
    // TODO: Obliczenie pełnych statystyk
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

} // namespace tab
