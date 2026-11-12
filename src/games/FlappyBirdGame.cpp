#include "games/FlappyBirdGame.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QtMath>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QRectF>
#include <QRandomGenerator>
#include <cmath>

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
    , m_wasFlapping(false)
    , m_obstaclesPassed(0)
{
    setFocusPolicy(Qt::NoFocus);
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
    m_wasFlapping = false;
    m_flapRequested = false;
    m_obstaclesPassed = 0;
    
    // Zachowaj rozmiar logiczny także wtedy, gdy widget jest jeszcze ukryty.
    m_lastKnownSize = QSize(width(), height());

    // Inicjalizacja ptaka
    resetBird();
    
    // Czyszczenie przeszkód
    m_obstacles.clear();
    m_previousBirdPos = m_birdPos;
    m_previousObstacles.clear();
    m_obstacleSpawnTimer = 0.0f;
    
    // Sensor samples are routed by MainWindow or TrainingTab; games never own callbacks.

}

void FlappyBirdGame::setDifficultyLevel(int level) {
    m_difficultyLevel = qBound(1, level, 5);
    
    // Skalowanie trudności
    // Poziom 1: wolne przeszkody, duże luki, wolne generowanie
    // Poziom 5: szybkie przeszkody, małe luki, szybkie generowanie
    switch (m_difficultyLevel) {
        case 1:
            m_obstacleSpeed = 125.0f;
            m_minGapHeight = 190.0f;
            m_maxGapHeight = 240.0f;
            m_obstacleSpawnInterval = 2.9f;
            m_gravity = 330.0f;
            m_flapForce = -270.0f;
            break;
        case 2:
            m_obstacleSpeed = 145.0f;
            m_minGapHeight = 180.0f;
            m_maxGapHeight = 225.0f;
            m_obstacleSpawnInterval = 2.7f;
            m_gravity = 355.0f;
            m_flapForce = -285.0f;
            break;
        case 3:
            m_obstacleSpeed = 165.0f;
            m_minGapHeight = 170.0f;
            m_maxGapHeight = 215.0f;
            m_obstacleSpawnInterval = 2.5f;
            m_gravity = 380.0f;
            m_flapForce = -300.0f;
            break;
        case 4:
            m_obstacleSpeed = 190.0f;
            m_minGapHeight = 160.0f;
            m_maxGapHeight = 205.0f;
            m_obstacleSpawnInterval = 2.3f;
            m_gravity = 410.0f;
            m_flapForce = -315.0f;
            break;
        case 5:
            m_obstacleSpeed = 215.0f;
            m_minGapHeight = 150.0f;
            m_maxGapHeight = 195.0f;
            m_obstacleSpawnInterval = 2.1f;
            m_gravity = 440.0f;
            m_flapForce = -330.0f;
            break;
    }
    
    const double scaleY = height() > 0 ? height() / 480.0 : 1.0;
    m_minGapHeight = static_cast<float>(m_minGapHeight * scaleY);
    m_maxGapHeight = static_cast<float>(m_maxGapHeight * scaleY);
    m_obstaclesPassed = 0;
    setScore(0);
}

int FlappyBirdGame::getDifficultyLevel() const {
    return m_difficultyLevel;
}

void FlappyBirdGame::setGameSpeed(float speed) {
    if (std::isfinite(speed))
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
    
    handleSensorInput(data.calibratedValue);
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
    if (!std::isfinite(rawValue))
        return;
    const double normalizedForce = rawValue / m_maxForce;
    m_currentForce = qBound(0.0, normalizedForce, 1.0);
}

bool FlappyBirdGame::handleInput(double value) {
    if (!std::isfinite(value))
        return false;

    normalizeForceValue(value);
    // Hysteresis prevents noisy samples around the activation threshold from
    // generating repeated flaps. A new flap requires a real release first.
    const bool isPressing = m_wasFlapping
        ? m_currentForce > 0.08
        : m_currentForce >= 0.12;
    if (isRunning() && isPressing && !m_wasFlapping)
        m_flapRequested = true;
    m_wasFlapping = isPressing;
    return true;
}

double FlappyBirdGame::birdVerticalVelocity() const {
    return m_birdVelocity.y();
}

int FlappyBirdGame::activeObstacleCount() const {
    return static_cast<int>(m_obstacles.size());
}

void FlappyBirdGame::resetBird() {
    // Restore the same logical 640x480 proportions on every new session.
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    m_birdPos = QPointF(width() / 4.0f, height() / 2.0f);
    m_birdVelocity.setX(0.0f);
    m_birdVelocity.setY(0.0f);
    m_birdRadius = static_cast<float>(m_birdBaseRadius * qMin(scaleX, scaleY));
    m_obstacleWidth = static_cast<float>(m_obstacleBaseWidth * scaleX);
}

void FlappyBirdGame::adaptToSize(const QSize& oldSize, const QSize& newSize) {
    if (newSize.width() <= 0 || newSize.height() <= 0)
        return;

    if (oldSize.width() > 0 && oldSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        const double scaleRadius = qMin(scaleX, scaleY);

        m_birdPos.setX(m_birdPos.x() * scaleX);
        m_birdPos.setY(m_birdPos.y() * scaleY);
        m_birdVelocity.setY(m_birdVelocity.y() * scaleY);
        m_birdRadius = static_cast<float>(qMax(1.0, m_birdRadius * scaleRadius));
        m_obstacleWidth = static_cast<float>(qMax(1.0, m_obstacleWidth * scaleX));
        m_minGapHeight = static_cast<float>(m_minGapHeight * scaleY);
        m_maxGapHeight = static_cast<float>(m_maxGapHeight * scaleY);

        for (Obstacle& obstacle : m_obstacles) {
            obstacle.xPosition = static_cast<float>(obstacle.xPosition * scaleX);
            obstacle.gapY = static_cast<float>(obstacle.gapY * scaleY);
            obstacle.gapHeight = static_cast<float>(obstacle.gapHeight * scaleY);
        }
    } else {
        // A hidden widget may not receive resizeEvent until it is shown.
        // Restore the same logical 640x480 layout at the actual viewport size.
        const double scaleX = qMax(0.5, newSize.width() / 640.0);
        const double scaleY = qMax(0.5, newSize.height() / 480.0);
        const double scaleRadius = qMin(scaleX, scaleY);
        m_birdPos = QPointF(newSize.width() * 0.25, newSize.height() * 0.5);
        m_birdRadius = static_cast<float>(m_birdBaseRadius * scaleRadius);
        m_obstacleWidth = static_cast<float>(m_obstacleBaseWidth * scaleX);
        m_minGapHeight = static_cast<float>(m_minGapHeight * scaleY);
        m_maxGapHeight = static_cast<float>(m_maxGapHeight * scaleY);
    }

    m_birdRadius = static_cast<float>(qMin(
        static_cast<double>(m_birdRadius),
        qMin(newSize.width(), newSize.height()) / 2.0));
    const double radius = m_birdRadius;
    m_birdPos.setX(qBound(radius, m_birdPos.x(),
                          static_cast<double>(newSize.width()) - radius));
    m_birdPos.setY(qBound(radius, m_birdPos.y(),
                          static_cast<double>(newSize.height()) - radius));

    const double safeGapHeight = qMax(
        0.0, static_cast<double>(newSize.height()) - 2.0 * radius);
    for (Obstacle& obstacle : m_obstacles) {
        obstacle.gapHeight = static_cast<float>(
            qBound(0.0, static_cast<double>(obstacle.gapHeight), safeGapHeight));
        const double minGapCenter = radius + obstacle.gapHeight / 2.0;
        const double maxGapCenter = static_cast<double>(newSize.height()) -
                                    radius - obstacle.gapHeight / 2.0;
        obstacle.gapY = static_cast<float>(
            qBound(minGapCenter, static_cast<double>(obstacle.gapY), maxGapCenter));
        obstacle.xPosition = static_cast<float>(qBound(
            -static_cast<double>(m_obstacleWidth),
            static_cast<double>(obstacle.xPosition),
            static_cast<double>(newSize.width())));
    }
}

void FlappyBirdGame::paintEvent(QPaintEvent* event) {
    if (isRunning()) {
        GameEngine::paintEvent(event);
        return;
    }
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, false);
    painter.fillRect(rect(), Qt::black);
    render(painter);
}

void FlappyBirdGame::resizeEvent(QResizeEvent* event) {
    const QSize newSize = event->size();
    adaptToSize(m_lastKnownSize, newSize);
    m_lastKnownSize = newSize;
    m_previousBirdPos = m_birdPos;
    m_previousObstacles = m_obstacles;
    GameEngine::resizeEvent(event);
}

void FlappyBirdGame::spawnObstacle() {
    Obstacle obs;
    obs.xPosition = static_cast<float>(width());
    obs.passed = false;
    
    // Najpierw losuj wysokość luki, potem środek z marginesem na ptaka.
    const float safeHeight = qMax(0.0f, static_cast<float>(height()) - 2.0f * m_birdRadius);
    const float minGap = qMin(m_minGapHeight, safeHeight);
    const float maxGap = qMax(minGap, qMin(m_maxGapHeight, safeHeight));
    obs.gapHeight = minGap + static_cast<float>(QRandomGenerator::global()->generateDouble()) * (maxGap - minGap);
    const float minGapY = m_birdRadius + obs.gapHeight / 2.0f;
    const float maxGapY = static_cast<float>(height()) - m_birdRadius - obs.gapHeight / 2.0f;
    const float centerRange = qMax(0.0f, maxGapY - minGapY);
    obs.gapY = minGapY + static_cast<float>(QRandomGenerator::global()->generateDouble()) * centerRange;
    
    m_obstacles.push_back(obs);
}

void FlappyBirdGame::updateBird(double deltaTime) {
    // Gravity is continuous; sensor, keyboard and pointer input request discrete
    // impulses. Holding a key or sensor force cannot pin the bird in the air.
    if (m_flapRequested) {
        const float strength = static_cast<float>(qMax(0.55, m_currentForce));
        const double scaleY = qMax(0.5, height() / 480.0);
        m_birdVelocity.setY(m_flapForce * strength * scaleY);
        m_flapRequested = false;
    }
    const double scaleY = qMax(0.5, height() / 480.0);
    m_birdVelocity.setY(m_birdVelocity.y() + m_gravity * scaleY * deltaTime);

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
        m_obstacleSpawnTimer -= m_obstacleSpawnInterval / m_gameSpeed;
    }
    
    // Aktualizacja pozycji przeszkód
    const double scaleX = qMax(0.5, width() / 640.0);
    float moveDistance = static_cast<float>(m_obstacleSpeed * scaleX * m_gameSpeed * deltaTime);
    
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
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    const QSize currentSize(width(), height());
    if (currentSize != m_lastKnownSize) {
        adaptToSize(m_lastKnownSize, currentSize);
        m_lastKnownSize = currentSize;
    }
    m_previousBirdPos = m_birdPos;
    m_previousObstacles = m_obstacles;

    // Ochrona wywołań bezpośrednich: nie wykonuj nieograniczonego skoku czasu.
    deltaTime = qMin(deltaTime, 0.1);

    // Aktualizacja ptaka
    updateBird(deltaTime);
    if (!isRunning()) return;
    
    // Aktualizacja przeszkód
    updateObstacles(deltaTime);
    
    // Sprawdzenie kolizji
    checkCollisions();
}

void FlappyBirdGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const QPointF currentBird = m_birdPos;
    const auto currentObstacles = m_obstacles;
    m_birdPos = m_previousBirdPos + (currentBird - m_previousBirdPos) * alpha;
    if (m_previousObstacles.size() == m_obstacles.size()) {
        for (std::size_t i = 0; i < m_obstacles.size(); ++i) {
            m_obstacles[i].xPosition = static_cast<float>(
                m_previousObstacles[i].xPosition +
                (currentObstacles[i].xPosition - m_previousObstacles[i].xPosition) * alpha);
            m_obstacles[i].gapY = static_cast<float>(
                m_previousObstacles[i].gapY +
                (currentObstacles[i].gapY - m_previousObstacles[i].gapY) * alpha);
        }
    }
    render(painter);
    m_birdPos = currentBird;
    m_obstacles = currentObstacles;
}

void FlappyBirdGame::render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    int w = width();
    int h = height();
    
    // 1. Tło - gradient niebieski (niebo)
    painter.fillRect(rect(), QColor(105, 174, 215));
    
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
    painter.setBrush(QBrush(QColor(240, 205, 70)));
    painter.setPen(QPen(QColor(180, 115, 35), 2));
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
    painter.setFont(QFont("Arial", 18, QFont::Bold));
    QRectF textRect(0, 0, w, 100);
    painter.drawText(textRect, Qt::AlignCenter, QString::number(m_score));
    
    // 6. Informacje
    painter.setFont(QFont("Arial", 8));
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
    const QColor forceColor = m_currentForce < 0.45f ? QColor(75, 205, 125)
        : (m_currentForce < 0.75f ? QColor(235, 195, 65) : QColor(225, 85, 75));
    painter.setBrush(forceColor);
    painter.drawRect(barX, barY + barH - fillH, barW, fillH);
    
    // 8. Linia ziemi
    painter.setPen(QPen(QColor(139, 90, 43), 4));
    painter.setBrush(QBrush(QColor(139, 90, 43)));
    painter.drawRect(0, h - 10, w, 10);
    
    if (!isRunning() && !isPaused()) {
        painter.fillRect(QRectF(w * 0.15, h * 0.38, w * 0.70, h * 0.22),
                         QColor(0, 0, 0, 155));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", qMax(12, h / 24), QFont::Bold));
        painter.drawText(QRectF(w * 0.17, h * 0.40, w * 0.66, h * 0.09),
                         Qt::AlignCenter, "KONIEC GRY");
        painter.setFont(QFont("Arial", qMax(9, h / 38)));
        painter.drawText(QRectF(w * 0.17, h * 0.49, w * 0.66, h * 0.08),
                         Qt::AlignCenter, "Spacja / kliknięcie — zagraj ponownie");
    }

    // 9. Status połączenia serial
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setFont(QFont("Arial", 9, QFont::Bold));
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

