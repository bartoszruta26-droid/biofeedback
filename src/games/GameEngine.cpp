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
    , m_gameElapsedMs(0)
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
        return;
    }
    
    // Reset stanu gry
    reset();
    
    // Inicjalizacja specyficzna dla gry
    initialize();
    
    // Aktualizacja stanu
    m_isRunning = true;
    m_isPaused = false;
    m_frameClock.start();
    
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
    m_frameClock.restart();
    
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

void GameEngine::reset()
{
    // Zatrzymanie gry
    m_gameTimer->stop();
    
    // Reset stanu
    m_isRunning = false;
    m_isPaused = false;
    m_score = 0;
    m_sensorValue = 0.0;
    resetGameStats();
    m_frameClock.invalidate();
    
    // Wymuszenie odświeżenia widoku
    QWidget::update();
}

void GameEngine::gameLoop()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }
    
    // Zegar monotoniczny jest odporny na zmianę daty i strefy czasowej.
    const qint64 elapsedMs = m_frameClock.isValid() ? m_frameClock.restart() : 0;
    m_gameElapsedMs += qMax<qint64>(0, elapsedMs);
    ++m_frameCount;
    const double deltaTime = qBound(0.0, elapsedMs / 1000.0, 0.1);
    update(deltaTime);
    
    // Odświeżenie widoku (wywoła paintEvent)
    QWidget::update();
}

void GameEngine::handleSensorInput(double value)
{
    if (QThread::currentThread() != thread()) {
        QPointer<GameEngine> guard(this);
        QMetaObject::invokeMethod(this, [guard, value]() {
            if (guard) guard->handleSensorInput(value);
        }, Qt::QueuedConnection);
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
    if (points == 0) return;
    const qint64 total = static_cast<qint64>(m_score) + points;
    setScore(static_cast<int>(qBound<qint64>(
        std::numeric_limits<int>::min(), total, std::numeric_limits<int>::max())));
}

QString GameEngine::getGameStats() const
{
    const QString state = !m_isRunning ? QStringLiteral("stopped")
        : (m_isPaused ? QStringLiteral("paused") : QStringLiteral("running"));
    return QStringLiteral("game=%1; state=%2; score=%3; elapsed_ms=%4; frames=%5; invalid_inputs=%6")
        .arg(gameName()).arg(state).arg(m_score).arg(m_gameElapsedMs)
        .arg(m_frameCount).arg(m_invalidInputCount);
}

void GameEngine::resetGameStats()
{
    m_gameElapsedMs = 0;
    m_frameCount = 0;
    m_invalidInputCount = 0;
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
