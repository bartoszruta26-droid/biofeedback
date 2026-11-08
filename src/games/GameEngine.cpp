#include "games/GameEngine.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QThread>
#include <QMetaObject>
#include <QPointer>
#include <cmath>
#include <limits>

namespace games {

GameEngine::GameEngine(QWidget *parent)
    : QWidget(parent)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_score(0)
    , m_gameTimer(new QTimer(this))
    , m_lastFrameTime(0)
    , m_accumulatedSessionMs(0)
    , m_simulationAccumulator(0.0)
    , m_frameCount(0)
    , m_invalidInputCount(0)
    , m_sensorValue(0.0)
{
    // Konfiguracja timera gry - 60 FPS (ok. 16ms)
    m_gameTimer->setInterval(16);
    
    // Połączenie timera z pętlą gry
    connect(m_gameTimer, &QTimer::timeout, this, &GameEngine::gameLoop);
    
    // Ustawienie fokusa dla obsługi klawiatury
    setFocusPolicy(Qt::StrongFocus);
    
    // Tło widgetu
    setAutoFillBackground(true);
}

GameEngine::~GameEngine()
{
    // Zatrzymanie gry jeśli jest aktywna
    if (m_isRunning) {
        stop();
    }
}

void GameEngine::start()
{
    if (m_isRunning) {
        if (m_isPaused)
            resume();
        return;
    }

    // Reset stanu gry
    reset();
    
    // Inicjalizacja specyficzna dla gry
    initialize();
    
    // Aktualizacja stanu
    m_isRunning = true;
    m_isPaused = false;
    m_monotonicClock.start();
    m_lastFrameTime = 0;
    m_simulationAccumulator = 0.0;
    
    // Uruchomienie timera
    m_gameTimer->start();
    
    // Emisja sygnału
    emit gameStarted();
}

void GameEngine::stop()
{
    if (!m_isRunning) {
        return; // Gra nie jest aktywna
    }
    
    // Zatrzymanie timera
    m_gameTimer->stop();
    if (m_monotonicClock.isValid()) {
        m_accumulatedSessionMs += m_monotonicClock.elapsed();
        m_monotonicClock.invalidate();
    }
    
    // Aktualizacja stanu
    m_isRunning = false;
    m_isPaused = false;
    
    // Emisja sygnału z końcowym wynikiem
    emit gameEnded(m_score);
}

void GameEngine::pause()
{
    if (!m_isRunning || m_isPaused) {
        return; // Gra nie trwa lub jest już spauzowana
    }
    
    // Zatrzymanie timera
    m_gameTimer->stop();
    if (m_monotonicClock.isValid()) {
        m_accumulatedSessionMs += m_monotonicClock.elapsed();
        m_monotonicClock.invalidate();
    }
    
    // Aktualizacja stanu
    m_isPaused = true;
    
    // Emisja sygnału
    emit gamePaused();
}

void GameEngine::resume()
{
    if (!m_isRunning || !m_isPaused) {
        return; // Gra nie trwa lub nie jest spauzowana
    }
    
    // Aktualizacja czasu ostatniej klatki
    m_monotonicClock.start();
    m_lastFrameTime = 0;
    m_simulationAccumulator = 0.0;
    
    // Wznowienie timera
    m_gameTimer->start();
    
    // Aktualizacja stanu
    m_isPaused = false;
    
    // Emisja sygnału
    emit gameResumed();
}

bool GameEngine::isRunning() const
{
    return m_isRunning;
}

bool GameEngine::isPaused() const
{
    return m_isPaused;
}

int GameEngine::getScore() const
{
    return m_score;
}

QString GameEngine::getGameStats() const
{
    qint64 elapsed = m_accumulatedSessionMs;
    if (m_monotonicClock.isValid())
        elapsed += m_monotonicClock.elapsed();
    const QString state = !m_isRunning ? QStringLiteral("stopped")
        : (m_isPaused ? QStringLiteral("paused") : QStringLiteral("running"));
    return QStringLiteral("game=%1; state=%2; score=%3; elapsed_ms=%4; frames=%5; invalid_inputs=%6")
        .arg(gameName()).arg(state).arg(m_score).arg(elapsed)
        .arg(static_cast<qulonglong>(m_frameCount))
        .arg(static_cast<qulonglong>(m_invalidInputCount));
}

void GameEngine::resetGameStats()
{
    setScore(0);
    m_accumulatedSessionMs = 0;
    m_frameCount = 0;
    m_invalidInputCount = 0;
    if (m_isRunning && !m_isPaused) {
        m_monotonicClock.restart();
        m_lastFrameTime = 0;
        m_simulationAccumulator = 0.0;
    } else {
        m_monotonicClock.invalidate();
    }
}

void GameEngine::reset()
{
    // Zatrzymanie gry
    m_gameTimer->stop();
    
    // Reset stanu
    m_isRunning = false;
    m_isPaused = false;
    m_score = 0;
    m_sensorValue = 0.0;
    m_accumulatedSessionMs = 0;
    m_monotonicClock.invalidate();
    m_lastFrameTime = 0;
    m_simulationAccumulator = 0.0;
    m_frameCount = 0;
    m_invalidInputCount = 0;
    
    // Wymuszenie odświeżenia widoku
    QWidget::update();
}

void GameEngine::gameLoop()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }
    
    // Obliczenie delta time (czas w sekundach od ostatniej klatki)
    const qint64 currentTime = m_monotonicClock.elapsed();
    double deltaTime = static_cast<double>(currentTime - m_lastFrameTime) / 1000.0;
    m_lastFrameTime = currentTime;

    // Ignore negative/non-finite intervals and cap stalls to avoid tunnelling.
    if (!std::isfinite(deltaTime) || deltaTime < 0.0)
        deltaTime = 0.0;
    deltaTime = qMin(deltaTime, 0.1);
    
    // Fixed-step simulation keeps collision and movement behaviour independent
    // of GUI timer jitter. The frame delta is capped above to bound catch-up work.
    constexpr double fixedStep = 1.0 / 120.0;
    m_simulationAccumulator += deltaTime;
    int steps = 0;
    while (m_simulationAccumulator >= fixedStep && steps < 12 && m_isRunning && !m_isPaused) {
        update(fixedStep);
        m_simulationAccumulator -= fixedStep;
        ++steps;
    }
    m_frameCount += static_cast<quint64>(steps);
    if (steps == 12 && m_simulationAccumulator >= fixedStep)
        m_simulationAccumulator = 0.0;

    QWidget::update();
}

void GameEngine::handleSensorInput(double value)
{
    // Sensor callbacks may originate on the serial worker thread. QWidget and
    // every derived game state must only be touched on the GUI thread.
    if (QThread::currentThread() != thread()) {
        // Sensor streams can be much faster than the GUI event loop. Keep only
        // the newest sample and schedule at most one pending GUI-thread callback.
        m_pendingSensorValue.store(value, std::memory_order_relaxed);
        if (m_sensorDispatchQueued.exchange(true, std::memory_order_acq_rel))
            return;

        QPointer<GameEngine> guard(this);
        const bool queued = QMetaObject::invokeMethod(this, [guard] {
            if (!guard)
                return;

            // Clear the flag before reading the latest value. A concurrent
            // producer either gets included in this read or schedules another
            // dispatch, so the newest sample cannot be stranded.
            guard->m_sensorDispatchQueued.store(false, std::memory_order_release);
            const double pending = guard->m_pendingSensorValue.load(std::memory_order_acquire);
            guard->handleSensorInput(pending);
        }, Qt::QueuedConnection);

        if (!queued)
            m_sensorDispatchQueued.store(false, std::memory_order_release);
        return;
    }
    if (!std::isfinite(value)) {
        ++m_invalidInputCount;
        return;
    }
    m_sensorValue = value;
    handleInput(value);
    QWidget::update();
}

void GameEngine::setScore(int score)
{
    if (m_score != score) {
        m_score = score;
        emit scoreChanged(m_score);
    }
}

void GameEngine::addScore(int points)
{
    if (points == 0)
        return;
    const qint64 total = static_cast<qint64>(m_score) + points;
    setScore(static_cast<int>(qBound<qint64>(
        std::numeric_limits<int>::min(), total, std::numeric_limits<int>::max())));
}

void GameEngine::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    // Tworzenie obiektu malującego
    QPainter painter(this);
    
    // Włączenie antyaliasingu dla lepszej jakości grafiki
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Tło - czarne dla gier
    painter.fillRect(rect(), Qt::black);
    
    // Renderowanie gry jeśli jest uruchomiona
    if (m_isRunning) {
        render(painter);
    } else {
        // Ekran startowy/końcowy
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 24, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, 
                        m_score > 0 ? QString("Koniec Gry!\nWynik: %1").arg(m_score) 
                                    : "Naciśnij Start aby rozpocząć");
    }
}

void GameEngine::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // Odświeżenie widoku po zmianie rozmiaru
    if (m_isRunning) {
        QWidget::update();
    }
}

} // namespace games
