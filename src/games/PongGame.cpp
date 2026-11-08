#include "games/PongGame.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRectF>
#include <cmath>
#include <QRandomGenerator>

namespace games {

PongGame::PongGame(QWidget *parent)
    : GameEngine(parent)
    , m_leftPaddleY(0.0f)
    , m_rightPaddleY(0.0f)
    , m_ballPos(0.0f, 0.0f)
    , m_ballVel(0.0f, 0.0f)
    , m_ballRadius(8.0f)
    , m_playerScore(0)
    , m_aiScore(0)
    , m_difficultyLevel(1)
    , m_aiSpeed(200.0f)
    , m_ballSpeed(300.0f)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
    , m_currentForce(0.0)
{
}

PongGame::~PongGame() {
    disconnectSerial();
}

QString PongGame::gameName() const {
    return "Pong Force Trainer";
}

void PongGame::initialize() {
    setDifficultyLevel(1);
    m_currentForce = 0.0;
    m_playerScore = 0;
    m_aiScore = 0;
    
    // Inicjalizacja paletki gracza (lewa strona)
    m_leftPaddle.setWidth(m_paddleWidth);
    m_leftPaddle.setHeight(m_paddleHeight);
    m_leftPaddle.setX(m_paddleMargin);
    m_leftPaddleY = height() / 2.0f - m_paddleHeight / 2.0f;
    m_leftPaddle.setY(m_leftPaddleY);
    
    // Inicjalizacja paletki AI (prawa strona)
    m_rightPaddle.setWidth(m_paddleWidth);
    m_rightPaddle.setHeight(m_paddleHeight);
    m_rightPaddle.setX(width() - m_paddleMargin - m_paddleWidth);
    m_rightPaddleY = height() / 2.0f - m_paddleHeight / 2.0f;
    m_rightPaddle.setY(m_rightPaddleY);
    
    // Reset piłki
    resetBall();
    
    // Sensor samples are routed by MainWindow or TrainingTab; games never own callbacks.

}

void PongGame::setDifficultyLevel(int level) {
    m_difficultyLevel = qBound(1, level, 5);
    
    // Skalowanie trudności
    // Poziom 1: wolne AI, wolna piłka
    // Poziom 5: szybkie AI, szybka piłka
    m_aiSpeed = 150.0f + (m_difficultyLevel * 80.0f);
    m_ballSpeed = 250.0f + (m_difficultyLevel * 70.0f);
    
    m_playerScore = 0;
    m_aiScore = 0;
    setScore(0);
}

int PongGame::getDifficultyLevel() const {
    return m_difficultyLevel;
}

void PongGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial) {
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }
    
    m_serialPort = serial;
    m_ownSerial = false;
    
    if (m_serialPort && m_serialPort->isConnected()) {
        emit serialConnected();
    }
}

void PongGame::disconnectSerial() {
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
        m_serialPort = nullptr;
        m_ownSerial = false;
        emit serialDisconnected();
    } else if (m_serialPort) {
        m_serialPort = nullptr;
        emit serialDisconnected();
    }
}

bool PongGame::isSerialConnected() const {
    return m_serialPort && m_serialPort->isConnected();
}

void PongGame::onSensorDataReceived(const sensor::SensorData& data) {
    if (!data.isValid) {
        return;
    }
    
    normalizeForceValue(data.calibratedValue);
}

void PongGame::onConnectionStatusChanged(bool connected, const std::string& message) {
    if (connected) {
        qDebug() << "PongGame: Połączono z sensorem:" << QString::fromStdString(message);
        emit serialConnected();
    } else {
        qDebug() << "PongGame: Rozłączono z sensorem:" << QString::fromStdString(message);
        emit serialError(QString::fromStdString("Rozłączono: " + message));
        emit serialDisconnected();
    }
}

void PongGame::normalizeForceValue(double rawValue) {
    double normalizedForce = rawValue / m_maxForce;
    m_currentForce = qBound(0.0, normalizedForce, 1.0);
}

bool PongGame::handleInput(double value) {
    normalizeForceValue(value);
    return true;
}

void PongGame::resetBall() {
    // Reset pozycji piłki na środek
    m_ballPos.setX(width() / 2.0f);
    m_ballPos.setY(height() / 2.0f);
    
    // Losowy kierunek startu (w lewo lub w prawo)
    float dirX = (QRandomGenerator::global()->bounded(2) == 0) ? 1.0f : -1.0f;
    float dirY = (QRandomGenerator::global()->bounded(2) == 0) ? 0.5f : -0.5f;
    
    // Normalizacja wektora kierunku
    float length = std::sqrt(dirX * dirX + dirY * dirY);
    dirX /= length;
    dirY /= length;
    
    m_ballVel.setX(dirX * m_ballSpeed);
    m_ballVel.setY(dirY * m_ballSpeed);
}

void PongGame::updateAIPaddle(double deltaTime) {
    // AI próbuje podążać za piłką
    float targetY = m_ballPos.y() - m_paddleHeight / 2.0f;
    
    // Ograniczenie prędkości AI
    float deltaY = targetY - m_rightPaddleY;
    float maxMove = m_aiSpeed * deltaTime;
    
    if (std::abs(deltaY) > maxMove) {
        if (deltaY > 0) {
            m_rightPaddleY += maxMove;
        } else {
            m_rightPaddleY -= maxMove;
        }
    } else {
        m_rightPaddleY = targetY;
    }
    
    // Ograniczenie do obszaru gry
    m_rightPaddleY = qBound(0.0f, m_rightPaddleY, qMax(0.0f, static_cast<float>(height()) - m_paddleHeight));
    m_rightPaddle.setY(m_rightPaddleY);
}

void PongGame::checkCollisions() {
    // Aktualizacja pozycji paletki gracza na podstawie siły
    const float maxPaddleY = qMax(0.0f, static_cast<float>(height()) - m_paddleHeight);
    m_leftPaddleY = (1.0f - static_cast<float>(m_currentForce)) * maxPaddleY;
    m_leftPaddle.setY(m_leftPaddleY);
    
    // Kolizja piłki z górną i dolną krawędzią
    if (m_ballPos.y() - m_ballRadius <= 0 || m_ballPos.y() + m_ballRadius >= height()) {
        m_ballVel.setY(-m_ballVel.y());
        m_ballPos.setY(qMax(static_cast<qreal>(m_ballRadius), qMin(static_cast<qreal>(m_ballPos.y()), static_cast<qreal>(height() - m_ballRadius))));
    }
    
    // Kolizja z lewą paletką (gracz)
    if (m_ballPos.x() - m_ballRadius <= m_leftPaddle.right() &&
        m_ballPos.x() - m_ballRadius >= m_leftPaddle.left() &&
        m_ballPos.y() >= m_leftPaddle.top() &&
        m_ballPos.y() <= m_leftPaddle.bottom()) {
        
        // Odbicie piłki z lekkim przyspieszeniem
        // Zachowaj ograniczoną prędkość całkowitą i wyznacz oba składniki
        // wektora z jednego kąta odbicia (bez niekontrolowanego przyspieszania).
        const float hitPos = (m_ballPos.y() - m_leftPaddle.top()) / m_paddleHeight;
        const float angle = (hitPos - 0.5f) * 1.5f;
        const double speed = qMin(
            std::hypot(m_ballVel.x(), m_ballVel.y()) * 1.05,
            static_cast<double>(m_ballSpeed) * 2.0);
        m_ballVel.setX(std::cos(angle) * speed); // W prawo
        m_ballVel.setY(std::sin(angle) * speed);
        
        m_ballPos.setX(m_leftPaddle.right() + m_ballRadius);
    }
    
    // Kolizja z prawą paletką (AI)
    if (m_ballPos.x() + m_ballRadius >= m_rightPaddle.left() &&
        m_ballPos.x() + m_ballRadius <= m_rightPaddle.right() &&
        m_ballPos.y() >= m_rightPaddle.top() &&
        m_ballPos.y() <= m_rightPaddle.bottom()) {
        
        const float hitPos = (m_ballPos.y() - m_rightPaddle.top()) / m_paddleHeight;
        const float angle = (hitPos - 0.5f) * 1.5f;
        const double speed = qMin(
            std::hypot(m_ballVel.x(), m_ballVel.y()) * 1.05,
            static_cast<double>(m_ballSpeed) * 2.0);
        m_ballVel.setX(-std::cos(angle) * speed); // W lewo
        m_ballVel.setY(std::sin(angle) * speed);
        
        m_ballPos.setX(m_rightPaddle.left() - m_ballRadius);
    }
    
    // Punktacja - piłka poza ekranem
    if (m_ballPos.x() < 0) {
        // Punkt dla AI
        m_aiScore++;
        resetBall();
    } else if (m_ballPos.x() > width()) {
        // Punkt dla gracza
        m_playerScore++;
        addScore(10); // 10 punktów za każde trafienie
        resetBall();
    }
}

void PongGame::update(double deltaTime) {
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0) return;

    // Match GameEngine's stall policy before converting the step count to int.
    // The normal GameEngine path already supplies fixed 1/120 s steps; this
    // subdivision is a defensive guard for direct callers with larger deltas.
    deltaTime = qMin(deltaTime, 0.1);
    constexpr double maxPhysicsStep = 1.0 / 120.0;
    const int steps = qMax(1, static_cast<int>(std::ceil(deltaTime / maxPhysicsStep)));
    const double stepTime = deltaTime / steps;

    for (int step = 0; step < steps && isRunning(); ++step) {
        m_ballPos.setX(m_ballPos.x() + m_ballVel.x() * stepTime);
        m_ballPos.setY(m_ballPos.y() + m_ballVel.y() * stepTime);
        updateAIPaddle(stepTime);
        checkCollisions();
    }
}

void PongGame::render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width();
    int h = height();
    
    // 1. Tło
    painter.fillRect(rect(), QColor(10, 10, 30));
    
    // 2. Linia środkowa
    painter.setPen(QPen(Qt::white, 2, Qt::DashLine));
    painter.drawLine(w / 2, 0, w / 2, h);
    
    // 3. Paletka gracza (lewa) - kolor zależny od trudności
    painter.setBrush(QBrush(getDifficultyColor()));
    painter.setPen(Qt::NoPen);
    painter.drawRect(m_leftPaddle.toRect());
    
    // 4. Paletka AI (prawa)
    painter.setBrush(QBrush(Qt::red));
    painter.drawRect(m_rightPaddle.toRect());
    
    // 5. Piłka
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(m_ballPos, m_ballRadius, m_ballRadius);
    
    // 6. Wyniki
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 36, QFont::Bold));
    painter.drawText(w / 4, 50, QString::number(m_playerScore));
    painter.drawText(3 * w / 4, 50, QString::number(m_aiScore));
    
    // 7. Informacje
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, "Steruj siłą nacisku");
    painter.drawText(10, 40, QString("Siła: %1%").arg(static_cast<int>(m_currentForce * 100)));
    painter.drawText(10, 60, QString("Poziom: %1").arg(m_difficultyLevel));
    
    // 8. Pasek siły
    int barW = 20;
    int barH = h - 40;
    int barX = w - 30;
    int barY = 20;
    
    painter.setPen(Qt::white);
    painter.drawRect(barX, barY, barW, barH);
    
    int fillH = static_cast<int>(m_currentForce * barH);
    painter.setBrush(QBrush(Qt::yellow));
    painter.drawRect(barX, barY + barH - fillH, barW, fillH);
    
    // 9. Status połączenia serial
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(w/2 - 100, 30, "BRAK POŁĄCZENIA Z SENSOREM");
    }
}

QColor PongGame::getDifficultyColor() const {
    switch (m_difficultyLevel) {
        case 1: return Qt::green;
        case 2: return Qt::cyan;
        case 3: return Qt::yellow;
        case 4: return QColorConstants::Svg::orange;
        default: return Qt::red;
    }
}

} // namespace games
