#include "games/FlappyBirdGame.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRectF>
#include <QRandomGenerator>

namespace games {

FlappyBirdGame::FlappyBirdGame(QWidget *parent)
    : GameEngine(parent)
    , m_birdPos(0.0f, 0.0f)
    , m_birdVelocity(0.0f, 0.0f)
    , m_birdRadius(m_birdBaseRadius)
    , m_gravity(500.0f)
    , m_flapForce(-350.0f)
    , m_obstacleWidth(m_obstacleBaseWidth)
    , m_obstacleSpawnTimer(0.0f)
    , m_obstacleSpawnInterval(2.0f)
    , m_obstacleSpeed(200.0f)
    , m_minGapHeight(120.0f)
    , m_maxGapHeight(180.0f)
    , m_difficultyLevel(1)
    , m_gameSpeed(1.0f)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
    , m_currentForce(0.0)
    , m_obstaclesPassed(0)
{
}

FlappyBirdGame::~FlappyBirdGame() {
    disconnectSerial();
}

QString FlappyBirdGame::gameName() const {
    return "Flappy Bird Force Trainer";
}

void FlappyBirdGame::initialize() {
    setDifficultyLevel(1);
    m_gameSpeed = 1.0f;
    m_currentForce = 0.0;
    m_obstaclesPassed = 0;
    
    // Inicjalizacja ptaka
    resetBird();
    
    // Czyszczenie przeszkód
    m_obstacles.clear();
    m_obstacleSpawnTimer = 0.0f;
    
    // Jeśli mamy podłączonego seriala, ustaw callbacki
    if (m_serialPort) {
        m_serialPort->setDataCallback([this](const sensor::SensorData& data) {
            QMetaObject::invokeMethod(this, "onSensorDataReceived", 
                                     Qt::QueuedConnection,
                                     Q_ARG(sensor::SensorData, data));
        });
        
        m_serialPort->setConnectionCallback([this](bool connected, const std::string& message) {
            QMetaObject::invokeMethod(this, "onConnectionStatusChanged",
                                     Qt::QueuedConnection,
                                     Q_ARG(bool, connected),
                                     Q_ARG(std::string, message));
        });
    }
}

void FlappyBirdGame::setDifficultyLevel(int level) {
    m_difficultyLevel = qBound(1, level, 5);
    
    // Skalowanie trudności
    // Poziom 1: wolne przeszkody, duże luki, wolne generowanie
    // Poziom 5: szybkie przeszkody, małe luki, szybkie generowanie
    switch (m_difficultyLevel) {
        case 1:
            m_obstacleSpeed = 150.0f;
            m_minGapHeight = 160.0f;
            m_maxGapHeight = 220.0f;
            m_obstacleSpawnInterval = 2.5f;
            m_gravity = 400.0f;
            m_flapForce = -300.0f;
            break;
        case 2:
            m_obstacleSpeed = 180.0f;
            m_minGapHeight = 140.0f;
            m_maxGapHeight = 200.0f;
            m_obstacleSpawnInterval = 2.2f;
            m_gravity = 450.0f;
            m_flapForce = -320.0f;
            break;
        case 3:
            m_obstacleSpeed = 220.0f;
            m_minGapHeight = 120.0f;
            m_maxGapHeight = 180.0f;
            m_obstacleSpawnInterval = 2.0f;
            m_gravity = 500.0f;
            m_flapForce = -350.0f;
            break;
        case 4:
            m_obstacleSpeed = 270.0f;
            m_minGapHeight = 100.0f;
            m_maxGapHeight = 160.0f;
            m_obstacleSpawnInterval = 1.8f;
            m_gravity = 550.0f;
            m_flapForce = -380.0f;
            break;
        case 5:
            m_obstacleSpeed = 320.0f;
            m_minGapHeight = 90.0f;
            m_maxGapHeight = 140.0f;
            m_obstacleSpawnInterval = 1.5f;
            m_gravity = 600.0f;
            m_flapForce = -400.0f;
            break;
    }
    
    emit scoreChanged(0);
}

int FlappyBirdGame::getDifficultyLevel() const {
    return m_difficultyLevel;
}

void FlappyBirdGame::setGameSpeed(float speed) {
    m_gameSpeed = qBound(0.5f, speed, 2.0f);
}

float FlappyBirdGame::getGameSpeed() const {
    return m_gameSpeed;
}

void FlappyBirdGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial) {
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

void FlappyBirdGame::disconnectSerial() {
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

bool FlappyBirdGame::isSerialConnected() const {
    return m_serialPort && m_serialPort->isConnected();
}

void FlappyBirdGame::onSensorDataReceived(const sensor::SensorData& data) {
    if (!data.isValid) {
        return;
    }
    
    normalizeForceValue(data.calibratedValue);
}

void FlappyBirdGame::onConnectionStatusChanged(bool connected, const std::string& message) {
    if (connected) {
        qDebug() << "FlappyBirdGame: Połączono z sensorem:" << QString::fromStdString(message);
        emit serialConnected();
    } else {
        qDebug() << "FlappyBirdGame: Rozłączono z sensorem:" << QString::fromStdString(message);
        emit serialError(QString::fromStdString("Rozłączono: " + message));
        emit serialDisconnected();
    }
}

void FlappyBirdGame::normalizeForceValue(double rawValue) {
    double normalizedForce = rawValue / m_maxForce;
    m_currentForce = qBound(0.0, normalizedForce, 1.0);
}

bool FlappyBirdGame::handleInput(double value) {
    normalizeForceValue(value);
    return true;
}

void FlappyBirdGame::resetBird() {
    // Reset pozycji ptaka na lewą stronę ekranu
    m_birdPos.setX(width() / 4.0f);
    m_birdPos.setY(height() / 2.0f);
    m_birdVelocity.setX(0.0f);
    m_birdVelocity.setY(0.0f);
    m_birdRadius = m_birdBaseRadius;
}

void FlappyBirdGame::spawnObstacle() {
    Obstacle obs;
    obs.xPosition = static_cast<float>(width());
    obs.passed = false;
    
    // Losowa wysokość luki
    float availableHeight = static_cast<float>(height()) - m_minGapHeight;
    float minGapY = m_minGapHeight / 2.0f;
    float maxGapY = availableHeight - m_minGapHeight / 2.0f;
    
    obs.gapY = minGapY + QRandomGenerator::global()->generateReal() * (maxGapY - minGapY);
    obs.gapHeight = m_minGapHeight + QRandomGenerator::global()->generateReal() * (m_maxGapHeight - m_minGapHeight);
    
    m_obstacles.push_back(obs);
}

void FlappyBirdGame::updateBird(double deltaTime) {
    // Aplikowanie grawitacji
    m_birdVelocity.setY(m_birdVelocity.y() + m_gravity * deltaTime);
    
    // Aplikowanie siły z sensora (naciąg tensometru)
    if (m_currentForce > 0.1) {
        // Im większa siła, tym silniejsze odbicie w górę
        float flapStrength = static_cast<float>(m_currentForce) * m_flapForce;
        m_birdVelocity.setY(flapStrength);
    }
    
    // Aktualizacja pozycji
    m_birdPos.setY(m_birdPos.y() + m_birdVelocity.y() * deltaTime);
    
    // Ograniczenie do obszaru gry (góra/dół)
    if (m_birdPos.y() - m_birdRadius <= 0) {
        m_birdPos.setY(m_birdRadius);
        m_birdVelocity.setY(0);
    }
    
    // Sprawdzenie czy ptak nie spadł na dół (koniec gry)
    if (m_birdPos.y() + m_birdRadius >= height()) {
        m_birdPos.setY(height() - m_birdRadius);
        m_birdVelocity.setY(0);
        // Koniec gry - ptak dotknął ziemi
        stop();
    }
}

void FlappyBirdGame::updateObstacles(double deltaTime) {
    // Aktualizacja timera spawnowania
    m_obstacleSpawnTimer += deltaTime;
    
    // Spawnowanie nowych przeszkód
    if (m_obstacleSpawnTimer >= m_obstacleSpawnInterval / m_gameSpeed) {
        spawnObstacle();
        m_obstacleSpawnTimer = 0.0f;
    }
    
    // Aktualizacja pozycji przeszkód
    float moveDistance = m_obstacleSpeed * m_gameSpeed * deltaTime;
    
    for (auto it = m_obstacles.begin(); it != m_obstacles.end(); ) {
        it->xPosition -= moveDistance;
        
        // Sprawdzenie czy ptak minął przeszkodę
        if (!it->passed && it->xPosition + m_obstacleWidth < m_birdPos.x() - m_birdRadius) {
            it->passed = true;
            m_obstaclesPassed++;
            addScore(10); // 10 punktów za każdą miniętą przeszkodę
        }
        
        // Usuwanie przeszkód poza ekranem
        if (it->xPosition + m_obstacleWidth < 0) {
            it = m_obstacles.erase(it);
        } else {
            ++it;
        }
    }
}

void FlappyBirdGame::checkCollisions() {
    // Sprawdzenie kolizji z przeszkodami
    for (const auto& obs : m_obstacles) {
        // Sprawdzenie czy ptak jest w zakresie X przeszkody
        if (m_birdPos.x() + m_birdRadius > obs.xPosition && 
            m_birdPos.x() - m_birdRadius < obs.xPosition + m_obstacleWidth) {
            
            // Sprawdzenie kolizji z górną gałęzią
            if (m_birdPos.y() - m_birdRadius < obs.gapY - obs.gapHeight / 2.0f) {
                // Kolizja z górną gałęzią - koniec gry
                stop();
                return;
            }
            
            // Sprawdzenie kolizji z dolną gałęzią
            if (m_birdPos.y() + m_birdRadius > obs.gapY + obs.gapHeight / 2.0f) {
                // Kolizja z dolną gałęzią - koniec gry
                stop();
                return;
            }
        }
    }
}

void FlappyBirdGame::update(double deltaTime) {
    if (!isRunning()) return;
    
    // Aktualizacja ptaka
    updateBird(deltaTime);
    
    // Aktualizacja przeszkód
    updateObstacles(deltaTime);
    
    // Sprawdzenie kolizji
    checkCollisions();
}

void FlappyBirdGame::render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width();
    int h = height();
    
    // 1. Tło - gradient niebieski (niebo)
    QLinearGradient skyGradient(0, 0, 0, h);
    skyGradient.setColorAt(0, QColor(135, 206, 235));  // Jasnoniebieski
    skyGradient.setColorAt(1, QColor(70, 130, 180));   // Stalowy niebieski
    painter.fillRect(rect(), skyGradient);
    
    // 2. Chmury (dekoracyjne)
    painter.setBrush(QColor(255, 255, 255, 150));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(w * 0.2, h * 0.15, 60, 30);
    painter.drawEllipse(w * 0.25, h * 0.12, 50, 25);
    painter.drawEllipse(w * 0.7, h * 0.2, 70, 35);
    painter.drawEllipse(w * 0.75, h * 0.17, 55, 28);
    
    // 3. Przeszkody (gałęzie drzew)
    for (const auto& obs : m_obstacles) {
        // Kolor zależny od trudności
        QColor obstacleColor = getDifficultyColor().darker(110);
        painter.setBrush(QBrush(obstacleColor));
        painter.setPen(QPen(getDifficultyColor().darker(130), 2));
        
        // Górna gałąź
        QRectF topBranch(obs.xPosition, 0, m_obstacleWidth, obs.gapY - obs.gapHeight / 2.0f);
        painter.drawRect(topBranch);
        
        // Dolna gałąź
        QRectF bottomBranch(obs.xPosition, obs.gapY + obs.gapHeight / 2.0f, 
                           m_obstacleWidth, h - (obs.gapY + obs.gapHeight / 2.0f));
        painter.drawRect(bottomBranch);
        
        // Detale gałęzi (liście/dekoracje)
        painter.setBrush(QBrush(getDifficultyColor()));
        painter.drawEllipse(obs.xPosition + m_obstacleWidth/2 - 10, 
                           topBranch.bottom() - 15, 20, 20);
        painter.drawEllipse(obs.xPosition + m_obstacleWidth/2 + 15, 
                           topBranch.bottom() - 10, 15, 15);
        painter.drawEllipse(obs.xPosition + m_obstacleWidth/2 - 15, 
                           bottomBranch.top() + 5, 18, 18);
    }
    
    // 4. Ptak
    painter.setBrush(QBrush(Qt::yellow));
    painter.setPen(QPen(Qt::orange, 2));
    painter.drawEllipse(m_birdPos, m_birdRadius, m_birdRadius);
    
    // Oczy ptaka
    painter.setBrush(QBrush(Qt::black));
    QPointF eyePos(m_birdPos.x() + m_birdRadius/2, m_birdPos.y() - m_birdRadius/3);
    painter.drawEllipse(eyePos, m_birdRadius/4, m_birdRadius/4);
    
    // Dziób
    painter.setBrush(QBrush(Qt::red));
    QPointF beakPos(m_birdPos.x() + m_birdRadius, m_birdPos.y());
    painter.drawEllipse(beakPos, m_birdRadius/2, m_birdRadius/3);
    
    // 5. Wynik
    painter.setPen(QPen(Qt::white, 2));
    painter.setFont(QFont("Arial", 36, QFont::Bold));
    painter.drawText(w / 2, 50, Qt::AlignCenter, QString::number(m_score));
    
    // 6. Informacje
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, "Steruj siłą nacisku");
    painter.drawText(10, 40, QString("Siła: %1%").arg(static_cast<int>(m_currentForce * 100)));
    painter.drawText(10, 60, QString("Poziom: %1").arg(m_difficultyLevel));
    painter.drawText(10, 80, QString("Prędkość: x%1").arg(m_gameSpeed, 0, 'f', 1));
    painter.drawText(10, 100, QString("Minięto: %1").arg(m_obstaclesPassed));
    
    // 7. Pasek siły (prawa strona)
    int barW = 25;
    int barH = h - 40;
    int barX = w - 35;
    int barY = 20;
    
    painter.setPen(QPen(Qt::white, 2));
    painter.drawRect(barX, barY, barW, barH);
    
    int fillH = static_cast<int>(m_currentForce * barH);
    QLinearGradient barGradient(barX, barY + barH, barX, barY);
    barGradient.setColorAt(0, Qt::green);
    barGradient.setColorAt(0.5, Qt::yellow);
    barGradient.setColorAt(1, Qt::red);
    painter.setBrush(QBrush(barGradient));
    painter.drawRect(barX, barY + barH - fillH, barW, fillH);
    
    // 8. Linia ziemi
    painter.setPen(QPen(QColor(139, 90, 43), 4));
    painter.setBrush(QBrush(QColor(139, 90, 43)));
    painter.drawRect(0, h - 10, w, 10);
    
    // 9. Status połączenia serial
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(w/2 - 150, 30, "BRAK POŁĄCZENIA Z SENSOREM");
    }
}

QColor FlappyBirdGame::getDifficultyColor() const {
    switch (m_difficultyLevel) {
        case 1: return QColor(34, 139, 34);   // Forest Green
        case 2: return QColor(60, 179, 113);  // Medium Sea Green
        case 3: return QColor(218, 165, 32);  // Goldenrod
        case 4: return QColor(255, 140, 0);   // Dark Orange
        default: return QColor(178, 34, 34);  // Fire Brick Red
    }
}

} // namespace games

