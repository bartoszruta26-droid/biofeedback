#include "games/GameEngine.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTime>

namespace games {

GameEngine::GameEngine(QWidget *parent)
    : QWidget(parent)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_score(0)
    , m_gameTimer(new QTimer(this))
    , m_lastFrameTime(0)
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
    if (m_isRunning && !m_isPaused) {
        return; // Gra już trwa
    }
    
    // Reset stanu gry
    reset();
    
    // Inicjalizacja specyficzna dla gry
    initialize();
    
    // Aktualizacja stanu
    m_isRunning = true;
    m_isPaused = false;
    m_lastFrameTime = QTime::currentTime().msecsSinceStartOfDay();
    
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
    m_lastFrameTime = QTime::currentTime().msecsSinceStartOfDay();
    
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
    
    // Wymuszenie odświeżenia widoku
    update();
}

void GameEngine::gameLoop()
{
    if (!m_isRunning || m_isPaused) {
        return;
    }
    
    // Obliczenie delta time (czas w sekundach od ostatniej klatki)
    qint64 currentTime = QTime::currentTime().msecsSinceStartOfDay();
    double deltaTime = (currentTime - m_lastFrameTime) / 1000.0;
    m_lastFrameTime = currentTime;
    
    // Ograniczenie delta time aby uniknąć skoków przy lagach
    if (deltaTime > 0.1) {
        deltaTime = 0.1;
    }
    
    // Aktualizacja logiki gry
    update(deltaTime);
    
    // Odświeżenie widoku (wywoła paintEvent)
    update();
}

void GameEngine::handleSensorInput(double value)
{
    // Zapisanie wartości sensora
    m_sensorValue = value;
    
    // Przekazanie do specyficznej obsługi w grze
    handleInput(value);
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
    if (points != 0) {
        setScore(m_score + points);
    }
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
        update();
    }
}

} // namespace games
