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
    , m_renderInterpolationAlpha(0.0)
    , m_frameCount(0)
    , m_renderFrameCount(0)
    , m_timerPumpCount(0)
    , m_droppedSimulationSteps(0)
    , m_renderTimeNsTotal(0)
    , m_renderTimeNsMax(0)
    , m_simulationTimeNsTotal(0)
    , m_simulationTimeNsMax(0)
    , m_invalidInputCount(0)
    , m_sensorValue(0.0)
{
    // Pump the fixed-step simulation at approximately 120 Hz. The accumulator
    // uses an exact 1/120 s step; Qt timer jitter is absorbed without changing
    // game physics or tying simulation to the display refresh rate.
    // Every game paints an opaque full-frame background. Let Qt skip the
    // implicit background erase and compositor blending for this widget.
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAutoFillBackground(false);

    m_gameTimer->setInterval(8);
    m_gameTimer->setTimerType(Qt::PreciseTimer);
    
    // Połączenie timera z pętlą gry
    connect(m_gameTimer, &QTimer::timeout, this, &GameEngine::gameLoop);
    
    // Ustawienie fokusa dla obsługi klawiatury
    setFocusPolicy(Qt::NoFocus);
    
    // The game paints every pixel itself; avoid an extra QWidget background pass.
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(false);
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
    
    // reset() initializes the derived game state after clearing the base state.
    // Keep initialization in one place to avoid running it twice on start().
    
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

    // Invalidate callbacks already queued for this session before clearing
    // the mailbox. The GUI lambda checks this generation before consuming it.
    m_sensorDispatchGeneration.fetch_add(1, std::memory_order_acq_rel);
    m_sensorDispatchQueued.store(false, std::memory_order_release);
    m_pendingSensorValue.store(0.0, std::memory_order_relaxed);
    m_pendingSensorValueAvailable.store(false, std::memory_order_relaxed);
    m_pendingInvalidInputCount.store(0, std::memory_order_relaxed);
    
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
    m_renderInterpolationAlpha = 0.0;
    
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
    const double renderAverageUs = m_renderFrameCount > 0
        ? static_cast<double>(m_renderTimeNsTotal) / static_cast<double>(m_renderFrameCount) / 1000.0
        : 0.0;
    const double simulationAverageUs = m_timerPumpCount > 0
        ? static_cast<double>(m_simulationTimeNsTotal) / static_cast<double>(m_timerPumpCount) / 1000.0
        : 0.0;
    const double elapsedSeconds = static_cast<double>(elapsed) / 1000.0;
    const double paintFpsEstimate = elapsedSeconds > 0.0
        ? static_cast<double>(m_renderFrameCount) / elapsedSeconds
        : 0.0;
    return QStringLiteral("game=%1; state=%2; score=%3; elapsed_ms=%4; "
                          "frames=%5; simulation_steps=%5; render_frames=%6; timer_pumps=%7; "
                          "dropped_steps=%8; render_avg_us=%9; render_max_us=%10; "
                          "simulation_avg_us=%11; simulation_max_us=%12; paint_fps_estimate=%13; invalid_inputs=%14")
        .arg(gameName()).arg(state).arg(m_score).arg(elapsed)
        .arg(static_cast<qulonglong>(m_frameCount))
        .arg(static_cast<qulonglong>(m_renderFrameCount))
        .arg(static_cast<qulonglong>(m_timerPumpCount))
        .arg(static_cast<qulonglong>(m_droppedSimulationSteps))
        .arg(renderAverageUs, 0, 'f', 2)
        .arg(static_cast<double>(m_renderTimeNsMax) / 1000.0, 0, 'f', 2)
        .arg(simulationAverageUs, 0, 'f', 2)
        .arg(static_cast<double>(m_simulationTimeNsMax) / 1000.0, 0, 'f', 2)
        .arg(paintFpsEstimate, 0, 'f', 1)
        .arg(static_cast<qulonglong>(m_invalidInputCount));
}

void GameEngine::resetGameStats()
{
    setScore(0);
    m_accumulatedSessionMs = 0;
    m_frameCount = 0;
    m_renderFrameCount = 0;
    m_timerPumpCount = 0;
    m_droppedSimulationSteps = 0;
    m_renderTimeNsTotal = 0;
    m_renderTimeNsMax = 0;
    m_simulationTimeNsTotal = 0;
    m_simulationTimeNsMax = 0;
    m_invalidInputCount = 0;
    m_pendingInvalidInputCount.store(0, std::memory_order_relaxed);
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
    // Invalidate queued sensor callbacks from any previous game instance.
    m_sensorDispatchGeneration.fetch_add(1, std::memory_order_acq_rel);
    m_sensorDispatchQueued.store(false, std::memory_order_release);
    m_pendingSensorValue.store(0.0, std::memory_order_relaxed);
    m_pendingSensorValueAvailable.store(false, std::memory_order_relaxed);
    m_pendingInvalidInputCount.store(0, std::memory_order_relaxed);

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
    m_renderInterpolationAlpha = 0.0;
    m_frameCount = 0;
    m_renderFrameCount = 0;
    m_timerPumpCount = 0;
    m_droppedSimulationSteps = 0;
    m_renderTimeNsTotal = 0;
    m_renderTimeNsMax = 0;
    m_simulationTimeNsTotal = 0;
    m_simulationTimeNsMax = 0;
    m_invalidInputCount = 0;
    m_pendingInvalidInputCount.store(0, std::memory_order_relaxed);
    
    // Rebuild the derived game's state as part of the public reset contract.
    // start() delegates here too, so each new session is initialized exactly once.
    initialize();

    // Wymuszenie odświeżenia widoku
    QWidget::update();
}

void GameEngine::gameLoop()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }
    
    // Use nanosecond monotonic time: millisecond quantization causes visible
    // cadence jitter at 90/120 Hz, especially when 8 ms timer ticks alternate.
    const qint64 currentTime = m_monotonicClock.nsecsElapsed();
    double deltaTime = static_cast<double>(currentTime - m_lastFrameTime) / 1.0e9;
    m_lastFrameTime = currentTime;

    // Ignore negative/non-finite intervals and cap stalls to avoid tunnelling.
    if (!std::isfinite(deltaTime) || deltaTime < 0.0)
        deltaTime = 0.0;
    if (deltaTime > 0.1) {
        const double discardedSeconds = deltaTime - 0.1;
        m_droppedSimulationSteps += static_cast<quint64>(
            discardedSeconds / kFixedSimulationStepSeconds);
        deltaTime = 0.1;
    }
    
    // Fixed-step simulation keeps collision and movement behaviour independent
    // of GUI timer jitter. The frame delta is capped above to bound catch-up work.
    constexpr double fixedStep = kFixedSimulationStepSeconds;
    m_simulationAccumulator += deltaTime;
    QElapsedTimer simulationTimer;
    simulationTimer.start();
    int steps = 0;
    while (m_simulationAccumulator >= fixedStep && steps < 12 && m_isRunning && !m_isPaused) {
        update(fixedStep);
        m_simulationAccumulator -= fixedStep;
        ++steps;
    }
    const qint64 simulationElapsedNs = simulationTimer.nsecsElapsed();
    ++m_timerPumpCount;
    m_simulationTimeNsTotal += simulationElapsedNs;
    m_simulationTimeNsMax = qMax(m_simulationTimeNsMax, simulationElapsedNs);
    m_frameCount += static_cast<quint64>(steps);
    if (steps == 12 && m_simulationAccumulator >= fixedStep) {
        const quint64 dropped = static_cast<quint64>(m_simulationAccumulator / fixedStep);
        m_droppedSimulationSteps += dropped;
        m_simulationAccumulator = std::fmod(m_simulationAccumulator, fixedStep);
    }

    // Preserve the full accumulator fraction. Each rendered frame blends the
    // previous and current 120 Hz fixed-step states, including on 60/90/120 Hz
    // displays, without advancing the simulation from the paint callback.
    m_renderInterpolationAlpha = qBound(0.0, m_simulationAccumulator / fixedStep, 1.0);
    QWidget::update();
}

void GameEngine::handleSensorInput(double value)
{
    // Validate before publishing into the cross-thread mailbox. Otherwise a
    // trailing NaN/Inf can overwrite a valid sample before the GUI consumes it.
    if (!std::isfinite(value)) {
        if (QThread::currentThread() == thread()) {
            ++m_invalidInputCount;
        } else {
            m_pendingInvalidInputCount.fetch_add(1, std::memory_order_relaxed);
            const quint64 generation =
                m_sensorDispatchGeneration.load(std::memory_order_acquire);
            if (!m_sensorDispatchQueued.exchange(true, std::memory_order_acq_rel)) {
                QPointer<GameEngine> guard(this);
                const bool queued = QMetaObject::invokeMethod(this, [guard, generation] {
                    if (!guard ||
                        guard->m_sensorDispatchGeneration.load(std::memory_order_acquire) != generation)
                        return;
                    guard->m_sensorDispatchQueued.store(false, std::memory_order_release);
                    guard->m_invalidInputCount += guard->m_pendingInvalidInputCount.exchange(
                        0, std::memory_order_acq_rel);
                    if (guard->m_pendingSensorValueAvailable.exchange(
                            false, std::memory_order_acq_rel)) {
                        const double pending = guard->m_pendingSensorValue.load(
                            std::memory_order_acquire);
                        guard->handleSensorInput(pending);
                    }
                }, Qt::QueuedConnection);
                if (!queued)
                    m_sensorDispatchQueued.store(false, std::memory_order_release);
            }
        }
        return;
    }

    // Sensor callbacks may originate on the serial worker thread. QWidget and
    // every derived game state must only be touched on the GUI thread.
    if (QThread::currentThread() != thread()) {
        // Sensor streams can be much faster than the GUI event loop. Keep only
        // the newest valid sample and schedule at most one pending callback.
        m_pendingSensorValue.store(value, std::memory_order_relaxed);
        m_pendingSensorValueAvailable.store(true, std::memory_order_release);
        const quint64 generation =
            m_sensorDispatchGeneration.load(std::memory_order_acquire);
        if (m_sensorDispatchQueued.exchange(true, std::memory_order_acq_rel))
            return;

        QPointer<GameEngine> guard(this);
        const bool queued = QMetaObject::invokeMethod(this, [guard, generation] {
            if (!guard)
                return;
            if (guard->m_sensorDispatchGeneration.load(std::memory_order_acquire) != generation)
                return;

            // Clear the flag before reading the latest value. A concurrent
            // producer either gets included in this read or schedules another
            // dispatch, so the newest sample cannot be stranded.
            guard->m_sensorDispatchQueued.store(false, std::memory_order_release);
            guard->m_invalidInputCount += guard->m_pendingInvalidInputCount.exchange(
                0, std::memory_order_acq_rel);
            if (guard->m_pendingSensorValueAvailable.exchange(
                    false, std::memory_order_acq_rel)) {
                const double pending = guard->m_pendingSensorValue.load(std::memory_order_acquire);
                guard->handleSensorInput(pending);
            }
        }, Qt::QueuedConnection);

        if (!queued)
            m_sensorDispatchQueued.store(false, std::memory_order_release);
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

void GameEngine::renderInterpolated(QPainter& painter, double alpha)
{
    Q_UNUSED(alpha);
    render(painter);
}

void GameEngine::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    // Tworzenie obiektu malującego
    QPainter painter(this);
    
    // Pixel-art rendering is the low-cost default. Individual games may
    // explicitly enable smoothing only where it materially improves clarity.
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);

    // Running games render an opaque background themselves. Avoid a second
    // full-window fill on every presentation frame (up to 120 Hz requested).
    if (m_isRunning) {
        QElapsedTimer renderTimer;
        renderTimer.start();
        renderInterpolated(painter, renderInterpolationAlpha());
        const qint64 renderElapsedNs = renderTimer.nsecsElapsed();
        ++m_renderFrameCount;
        m_renderTimeNsTotal += renderElapsedNs;
        m_renderTimeNsMax = qMax(m_renderTimeNsMax, renderElapsedNs);
    } else {
        painter.fillRect(rect(), Qt::black);
        // Ekran startowy/końcowy
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
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
