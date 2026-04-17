#include "games/MarioGame.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPaintEvent>
#include <QTime>
#include <cmath>

namespace games {

MarioGame::MarioGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentLevel(1)
    , m_obstaclesPassed(0)
    , m_obstaclesToNextLevel(10)
    , m_timeSinceLastObstacle(0.0)
    , m_obstacleSpawnInterval(2.0)
    , m_sensorThreshold(0.3)
    , m_chargeStartTime(0.0)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
{
    // Inicjalizacja chmurek dekoracyjnych
    for (int i = 0; i < 5; ++i) {
        Cloud cloud;
        cloud.pos.setX(QRandomGenerator::global()->bounded(width()));
        cloud.pos.setY(QRandomGenerator::global()->bounded(height() / 3));
        cloud.speed = 20.0 + QRandomGenerator::global()->bounded(30);
        cloud.size = 20 + QRandomGenerator::global()->bounded(30);
        m_clouds.append(cloud);
    }
}

MarioGame::~MarioGame()
{
    cleanupSensorConnection();
}

QString MarioGame::gameName() const
{
    return "Mario Run";
}

void MarioGame::initialize()
{
    initPlayer();
    resetLevel();
    
    // Reset chmurek
    m_clouds.clear();
    for (int i = 0; i < 5; ++i) {
        Cloud cloud;
        cloud.pos.setX(QRandomGenerator::global()->bounded(width()));
        cloud.pos.setY(QRandomGenerator::global()->bounded(height() / 3));
        cloud.speed = 20.0 + QRandomGenerator::global()->bounded(30);
        cloud.size = 20 + QRandomGenerator::global()->bounded(30);
        m_clouds.append(cloud);
    }
}

void MarioGame::initPlayer()
{
    m_player.body = QRectF(50, height() * GROUND_Y_RATIO - 40, 32, 40);
    m_player.velocityY = 0.0;
    m_player.isJumping = false;
    m_player.isOnGround = true;
    m_player.chargeLevel = 0.0;
    m_player.isCharging = false;
    m_player.frame = 0;
    m_player.frameTimer = 0.0;
}

void MarioGame::resetLevel()
{
    m_obstacles.clear();
    m_obstaclesPassed = 0;
    m_timeSinceLastObstacle = 0.0;
    m_obstacleSpawnInterval = 2.0;
    m_currentLevel = 1;
    m_obstaclesToNextLevel = 10;
}

void MarioGame::increaseDifficulty()
{
    m_currentLevel++;
    m_obstaclesPassed = 0;
    m_obstaclesToNextLevel = 10 + (m_currentLevel * 5);
    
    // Zmniejsz interwał spawnowania przeszkód
    m_obstacleSpawnInterval = qMax(0.8, 2.0 - (m_currentLevel * 0.15));
}

void MarioGame::update(double deltaTime)
{
    // Aktualizacja gracza
    updatePlayer(deltaTime);
    
    // Generowanie i aktualizacja przeszkód
    generateObstacles();
    updateObstacles(deltaTime);
    
    // Aktualizacja chmurek
    for (auto& cloud : m_clouds) {
        cloud.pos.setX(cloud.pos.x() - cloud.speed * deltaTime);
        if (cloud.pos.x() + cloud.size < 0) {
            cloud.pos.setX(width() + cloud.size);
            cloud.pos.setY(QRandomGenerator::global()->bounded(height() / 3));
        }
    }
    
    // Sprawdzenie czy gracz spadł w dziurę
    if (m_player.body.bottom() > height()) {
        stop();
    }
}

void MarioGame::updatePlayer(double deltaTime)
{
    double groundY = height() * GROUND_Y_RATIO;
    
    // Grawitacja
    if (!m_player.isOnGround) {
        m_player.velocityY += GRAVITY * deltaTime;
    }
    
    // Aktualizacja pozycji
    m_player.body.setY(m_player.body.y() + m_player.velocityY * deltaTime);
    
    // Kolizja z ziemią
    if (m_player.body.bottom() >= groundY) {
        m_player.body.setBottom(groundY);
        m_player.velocityY = 0.0;
        m_player.isJumping = false;
        m_player.isOnGround = true;
        m_player.chargeLevel = 0.0;
        m_player.isCharging = false;
    } else {
        m_player.isOnGround = false;
    }
    
    // Animacja biegu
    if (m_player.isOnGround && !m_player.isCharging) {
        m_player.frameTimer += deltaTime;
        if (m_player.frameTimer > 0.1) {
            m_player.frame = (m_player.frame + 1) % 4;
            m_player.frameTimer = 0.0;
        }
    }
}

void MarioGame::generateObstacles()
{
    m_timeSinceLastObstacle += 0.016; // Zakładamy ~60 FPS
    
    if (m_timeSinceLastObstacle >= m_obstacleSpawnInterval) {
        m_timeSinceLastObstacle = 0.0;
        
        Obstacle obstacle;
        obstacle.passed = false;
        
        // Losowanie typu przeszkody z uwzględnieniem poziomu trudności
        int rand = QRandomGenerator::global()->bounded(100);
        double groundY = height() * GROUND_Y_RATIO;
        
        if (rand < 20) {
            // Kolce (20%)
            obstacle.type = Obstacle::SPIKE;
            obstacle.rect = QRectF(width(), groundY - 30, 30, 30);
        } else if (rand < 45) {
            // Rurka (25%)
            obstacle.type = Obstacle::PIPE;
            int pipeHeight = 40 + (m_currentLevel * 10);
            obstacle.rect = QRectF(width(), groundY - pipeHeight, 50, pipeHeight);
        } else if (rand < 60 && m_currentLevel >= 2) {
            // Platforma (15%, od poziomu 2)
            obstacle.type = Obstacle::PLATFORM;
            obstacle.rect = QRectF(width(), groundY - 70 - (QRandomGenerator::global()->bounded(50)), 80, 20);
        } else if (rand < 75 && m_currentLevel >= 3) {
            // Dziura (15%, od poziomu 3)
            obstacle.type = Obstacle::GAP;
            obstacle.rect = QRectF(width(), groundY, 60 + (m_currentLevel * 10), height() - groundY);
        } else {
            // Domyślnie kolce
            obstacle.type = Obstacle::SPIKE;
            obstacle.rect = QRectF(width(), groundY - 30, 30, 30);
        }
        
        m_obstacles.append(obstacle);
    }
}

void MarioGame::updateObstacles(double deltaTime)
{
    double groundY = height() * GROUND_Y_RATIO;
    
    for (int i = m_obstacles.size() - 1; i >= 0; --i) {
        auto& obs = m_obstacles[i];
        
        // Przesuwanie przeszkody w lewo
        obs.rect.setX(obs.rect.x() - PLAYER_SPEED * deltaTime);
        
        // Usunięcie jeśli poza ekranem
        if (obs.rect.right() < 0) {
            m_obstacles.removeAt(i);
            continue;
        }
        
        // Sprawdzenie czy gracz minął przeszkodę
        if (!obs.passed && obs.rect.right() < m_player.body.left()) {
            obs.passed = true;
            m_obstaclesPassed++;
            addScore(10);
            
            // Sprawdzenie awansu na kolejny poziom
            if (m_obstaclesPassed >= m_obstaclesToNextLevel) {
                increaseDifficulty();
            }
        }
        
        // Wykrywanie kolizji
        if (checkCollision(m_player, obs)) {
            stop();
        }
    }
}

bool MarioGame::checkCollision(const Player& player, const Obstacle& obstacle)
{
    // Dla dziur sprawdzamy czy gracz jest nad dziurą i nie skacze
    if (obstacle.type == Obstacle::GAP) {
        double playerCenterX = player.body.center().x();
        if (playerCenterX > obstacle.rect.left() && 
            playerCenterX < obstacle.rect.right() &&
            player.isOnGround) {
            return true;
        }
        return false;
    }
    
    // Dla innych przeszkód sprawdzamy prostokąty
    return player.body.intersects(obstacle.rect);
}

bool MarioGame::handleInput(double value)
{
    // Tensometr: wartość > threshold oznacza naciskanie
    bool isPressing = value > m_sensorThreshold;
    
    if (isPressing && !m_player.isCharging && m_player.isOnGround) {
        // Rozpoczęcie ładowania skoku
        m_player.isCharging = true;
        m_player.frame = 0; // Zatrzymanie animacji biegu
        m_chargeStartTime = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
    } else if (!isPressing && m_player.isCharging && m_player.isOnGround) {
        // Zwolnienie tensometru - wykonanie skoku
        double currentTime = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
        double chargeDuration = currentTime - m_chargeStartTime;
        
        // Obliczenie siły skoku na podstawie czasu ładowania
        double chargeRatio = qMin(1.0, chargeDuration / MAX_CHARGE_TIME);
        double jumpForce = JUMP_FORCE * (0.3 + 0.7 * chargeRatio);
        
        m_player.velocityY = -jumpForce;
        m_player.isJumping = true;
        m_player.isOnGround = false;
        m_player.isCharging = false;
        m_player.chargeLevel = 0.0;
        
        return true;
    } else if (isPressing && m_player.isCharging) {
        // Kontynuacja ładowania
        double currentTime = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
        double chargeDuration = currentTime - m_chargeStartTime;
        m_player.chargeLevel = qMin(1.0, chargeDuration / MAX_CHARGE_TIME);
    }
    
    return false;
}

void MarioGame::render(QPainter& painter)
{
    renderBackground(painter);
    renderObstacles(painter);
    renderPlayer(painter, m_player);
    renderUI(painter);
}

void MarioGame::renderPlayer(QPainter& painter, const Player& player)
{
    double x = player.body.x();
    double y = player.body.y();
    double w = player.body.width();
    double h = player.body.height();
    
    // Styl 4-bitowy - proste kształty geometryczne
    
    // Ciało (czerwone)
    painter.fillRect(QRectF(x, y + h * 0.4, w, h * 0.4), COLOR_MARIO_RED);
    
    // Głowa (kolor skóry)
    painter.fillRect(QRectF(x + w * 0.1, y, w * 0.8, h * 0.4), COLOR_MARIO_SKIN);
    
    // Czapka (czerwona)
    painter.fillRect(QRectF(x, y, w, h * 0.15), COLOR_MARIO_RED);
    painter.fillRect(QRectF(x + w * 0.5, y + h * 0.1, w * 0.6, h * 0.05), COLOR_MARIO_RED);
    
    // Włosy/brody (brązowe)
    painter.fillRect(QRectF(x + w * 0.1, y + h * 0.25, w * 0.2, h * 0.1), COLOR_MARIO_BROWN);
    
    // Buty (brązowe)
    if (player.isJumping) {
        // Nogi podciągnięte podczas skoku
        painter.fillRect(QRectF(x + w * 0.1, y + h * 0.75, w * 0.35, h * 0.2), COLOR_MARIO_BROWN);
        painter.fillRect(QRectF(x + w * 0.55, y + h * 0.75, w * 0.35, h * 0.2), COLOR_MARIO_BROWN);
    } else if (player.isCharging) {
        // Pozycja przygotowawcza do skoku
        painter.fillRect(QRectF(x + w * 0.1, y + h * 0.8, w * 0.35, h * 0.15), COLOR_MARIO_BROWN);
        painter.fillRect(QRectF(x + w * 0.55, y + h * 0.8, w * 0.35, h * 0.15), COLOR_MARIO_BROWN);
    } else {
        // Animacja biegu
        int frame = player.frame;
        if (frame == 0 || frame == 2) {
            // Noga w przód
            painter.fillRect(QRectF(x + w * 0.1, y + h * 0.8, w * 0.35, h * 0.15), COLOR_MARIO_BROWN);
            painter.fillRect(QRectF(x + w * 0.55, y + h * 0.75, w * 0.35, h * 0.2), COLOR_MARIO_BROWN);
        } else {
            // Druga noga w przód
            painter.fillRect(QRectF(x + w * 0.1, y + h * 0.75, w * 0.35, h * 0.2), COLOR_MARIO_BROWN);
            painter.fillRect(QRectF(x + w * 0.55, y + h * 0.8, w * 0.35, h * 0.15), COLOR_MARIO_BROWN);
        }
    }
}

void MarioGame::renderObstacles(QPainter& painter)
{
    double groundY = height() * GROUND_Y_RATIO;
    
    for (const auto& obs : m_obstacles) {
        switch (obs.type) {
            case Obstacle::SPIKE:
                // Trójkątne kolce (styl 4-bitowy)
                {
                    QVector<QPoint> spike = {
                        QPoint(obs.rect.left(), obs.rect.bottom()),
                        QPoint(obs.rect.center().x(), obs.rect.top()),
                        QPoint(obs.rect.right(), obs.rect.bottom())
                    };
                    painter.setBrush(COLOR_SPIKE);
                    painter.setPen(COLOR_SPIKE);
                    painter.drawPolygon(spike);
                }
                break;
                
            case Obstacle::PIPE:
                // Zielona rurka z obwódką
                painter.fillRect(obs.rect, COLOR_PIPE);
                painter.setPen(Qt::black);
                painter.drawRect(obs.rect);
                // Górna krawędź rurki
                {
                    QRectF rim(obs.rect.left() - 5, obs.rect.top(), obs.rect.width() + 10, 10);
                    painter.fillRect(rim, COLOR_PIPE);
                    painter.drawRect(rim);
                }
                break;
                
            case Obstacle::PLATFORM:
                // Platforma z ceglanym wzorem
                painter.fillRect(obs.rect, COLOR_GROUND);
                painter.setPen(Qt::black);
                painter.drawRect(obs.rect);
                // Prosty wzór cegieł
                for (double x = obs.rect.left(); x < obs.rect.right(); x += 20) {
                    painter.drawLine(QPointF(x, obs.rect.top()), QPointF(x, obs.rect.bottom()));
                }
                break;
                
            case Obstacle::GAP:
                // Dziura - nie rysujemy niczego, to brak ziemi
                // Można dodać czarne tło dla lepszej widoczności
                painter.fillRect(obs.rect, Qt::black);
                break;
        }
    }
    
    // Rysowanie ziemi (z przerwami na dziury)
    painter.fillRect(QRectF(0, groundY, width(), height() - groundY), COLOR_GROUND);
    
    // Detale ziemi (prosty wzór)
    painter.setPen(QColor(0xA0, 0x30, 0x00));
    for (double x = 0; x < width(); x += 40) {
        for (double y = groundY + 10; y < height(); y += 30) {
            bool inGap = false;
            for (const auto& obs : m_obstacles) {
                if (obs.type == Obstacle::GAP && 
                    x > obs.rect.left() && x < obs.rect.right()) {
                    inGap = true;
                    break;
                }
            }
            if (!inGap) {
                painter.fillRect(QRectF(x, y, 30, 20), QColor(0xA0, 0x30, 0x00));
            }
        }
    }
}

void MarioGame::renderBackground(QPainter& painter)
{
    // Niebo (gradient w stylu 4-bitowym - jednolity kolor)
    painter.fillRect(rect(), COLOR_SKY);
    
    // Chmurki (proste okręgi/owale)
    painter.setBrush(COLOR_CLOUD);
    painter.setPen(COLOR_CLOUD);
    for (const auto& cloud : m_clouds) {
        int r = cloud.size / 2;
        painter.drawEllipse(QPoint(cloud.pos.toPoint()), r, static_cast<int>(r * 0.6));
        painter.drawEllipse(QPoint(cloud.pos.x() - r * 0.5, cloud.pos.y() + r * 0.2), static_cast<int>(r * 0.7), static_cast<int>(r * 0.5));
        painter.drawEllipse(QPoint(cloud.pos.x() + r * 0.5, cloud.pos.y() + r * 0.2), static_cast<int>(r * 0.7), static_cast<int>(r * 0.5));
    }
    
    // Wzgórza w tle (proste trójkąty)
    painter.setBrush(QColor(0x00, 0xA8, 0x00));
    painter.setPen(QColor(0x00, 0xA8, 0x00));
    for (int i = 0; i < 3; ++i) {
        double x = fmod(-QTime::currentTime().msecsSinceStartOfDay() / 50.0 + i * 300, width() + 200) - 100;
        QVector<QPoint> hill = {
            QPoint(x, height() * GROUND_Y_RATIO),
            QPoint(x + 100, height() * GROUND_Y_RATIO - 60),
            QPoint(x + 200, height() * GROUND_Y_RATIO)
        };
        painter.drawPolygon(hill);
    }
}

void MarioGame::renderUI(QPainter& painter)
{
    // Pasek ładowania skoku
    if (m_player.isCharging) {
        int barWidth = 100;
        int barHeight = 15;
        int x = m_player.body.x();
        int y = m_player.body.top() - 25;
        
        // Tło paska
        painter.fillRect(QRectF(x, y, barWidth, barHeight), COLOR_CHARGE_EMPTY);
        
        // Poziom naładowania
        int fillWidth = static_cast<int>(barWidth * m_player.chargeLevel);
        QColor chargeColor = m_player.chargeLevel < 0.5 ? COLOR_CHARGE_EMPTY : 
                            (m_player.chargeLevel < 0.8 ? QColor(0xFC, 0xC8, 0x00) : COLOR_CHARGE_FULL);
        painter.fillRect(QRectF(x, y, fillWidth, barHeight), chargeColor);
        
        // Obramowanie
        painter.setPen(Qt::white);
        painter.drawRect(QRectF(x, y, barWidth, barHeight));
    }
    
    // Wynik
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(20, 30, QString("Wynik: %1").arg(m_score));
    
    // Poziom
    painter.drawText(20, 55, QString("Poziom: %1").arg(m_currentLevel));
    
    // Postęp do następnego poziomu
    int progressWidth = 150;
    int progressHeight = 10;
    double progress = static_cast<double>(m_obstaclesPassed) / m_obstaclesToNextLevel;
    painter.fillRect(QRectF(20, 65, progressWidth, progressHeight), Qt::darkGray);
    painter.fillRect(QRectF(20, 65, progressWidth * progress, progressHeight), Qt::green);
    painter.setPen(Qt::white);
    painter.drawRect(QRectF(20, 65, progressWidth, progressHeight));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(20, 90, QString("%1/%2").arg(m_obstaclesPassed).arg(m_obstaclesToNextLevel));
    
    // Instrukcja
    if (!m_isRunning) {
        painter.setFont(QFont("Arial", 14));
        painter.drawText(rect(), Qt::AlignBottom | Qt::AlignHCenter, 
                        "Przytrzymaj tensometr aby naładować skok, puść aby skoczyć!");
    }
    
    // Komunikat o braku połączenia z Arduino
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QColor(255, 100, 100));
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter,
                        "BRAK POŁĄCZENIA Z ARDUINO NANO HX711");
    }
}

// ==================== Metody SerialCommunication ====================

void MarioGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }
    
    m_serialPort = serial;
    
    if (m_serialPort && m_serialPort->isConnected()) {
        setupSensorListening();
    }
    
    m_ownSerial = false;
}

bool MarioGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

void MarioGame::setupSensorListening()
{
    if (!m_serialPort) return;
    
    m_serialPort->setDataCallback([this](const sensor::SensorData& data) {
        QMetaObject::invokeMethod(this, [this, data]() {
            handleSensorInput(data.force);
        }, Qt::QueuedConnection);
    });
    
    m_serialPort->setConnectionCallback([this](bool connected, const std::string& message) {
        QMetaObject::invokeMethod(this, [this, connected, message]() {
            if (!connected) {
                // Można wyświetlić komunikat o utracie połączenia
            }
        }, Qt::QueuedConnection);
    });
    
    m_serialPort->startAsyncReading();
}

void MarioGame::cleanupSensorConnection()
{
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
        m_serialPort = nullptr;
        m_ownSerial = false;
    } else if (m_serialPort) {
        m_serialPort = nullptr;
    }
}

} // namespace games
