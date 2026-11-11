#include "games/MarioGame.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPaintEvent>
#include <QResizeEvent>
#include <cmath>

namespace games {

MarioGame::MarioGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentLevel(1)
    , m_obstaclesPassed(0)
    , m_obstaclesToNextLevel(10)
    , m_timeSinceLastObstacle(0.0)
    , m_obstacleSpawnInterval(2.0)
    , m_sensorThreshold(30.0)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
{
    // Inicjalizacja chmurek dekoracyjnych
    for (int i = 0; i < 5; ++i) {
        Cloud cloud;
        cloud.pos.setX(QRandomGenerator::global()->bounded(qMax(1, width())));
        cloud.pos.setY(QRandomGenerator::global()->bounded(qMax(1, height() / 3)));
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
        cloud.pos.setX(QRandomGenerator::global()->bounded(qMax(1, width())));
        cloud.pos.setY(QRandomGenerator::global()->bounded(qMax(1, height() / 3)));
        cloud.speed = 20.0 + QRandomGenerator::global()->bounded(30);
        cloud.size = 20 + QRandomGenerator::global()->bounded(30);
        m_clouds.append(cloud);
    }
}

double MarioGame::playerVerticalVelocity() const { return m_player.velocityY; }
bool MarioGame::playerOnGround() const { return m_player.isOnGround; }
bool MarioGame::playerCharging() const { return m_player.isCharging; }
double MarioGame::jumpCharge() const { return m_player.chargeLevel; }
int MarioGame::activeObstacleCount() const { return m_obstacles.size(); }
int MarioGame::currentLevel() const { return m_currentLevel; }

void MarioGame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    render(painter);

    if (!isRunning()) {
        const QRectF panel(width() * 0.12, height() * 0.34,
                           width() * 0.76, height() * 0.30);
        painter.fillRect(panel, QColor(0, 0, 0, 175));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", qMax(14, height() / 22), QFont::Bold));
        const QString heading = getScore() > 0 ? QStringLiteral("KONIEC GRY")
                                               : QStringLiteral("MARIO RUN");
        painter.drawText(QRectF(panel.left() + 8, panel.top() + 8,
                                panel.width() - 16, panel.height() * 0.40),
                         Qt::AlignCenter, heading);
        painter.setFont(QFont("Arial", qMax(10, height() / 38)));
        painter.drawText(QRectF(panel.left() + 8, panel.top() + panel.height() * 0.42,
                                panel.width() - 16, panel.height() * 0.48),
                         Qt::AlignCenter,
                         QStringLiteral("Przytrzymaj tensometr, aby naładować skok.\nPuść, aby wykonać skok.\nWynik: %1").arg(getScore()));
    }
}

void MarioGame::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        const double scaleDecor = qMin(scaleX, scaleY);

        m_player.body = QRectF(m_player.body.x() * scaleX,
                               m_player.body.y() * scaleY,
                               m_player.body.width() * scaleX,
                               m_player.body.height() * scaleY);
        m_player.velocityY *= scaleY;

        for (Obstacle& obstacle : m_obstacles) {
            obstacle.rect = QRectF(obstacle.rect.x() * scaleX,
                                   obstacle.rect.y() * scaleY,
                                   obstacle.rect.width() * scaleX,
                                   obstacle.rect.height() * scaleY);
        }

        for (Cloud& cloud : m_clouds) {
            cloud.pos.setX(cloud.pos.x() * scaleX);
            cloud.pos.setY(cloud.pos.y() * scaleY);
            cloud.size = qMax(1, static_cast<int>(std::lround(cloud.size * scaleDecor)));
        }

        const double groundY = newSize.height() * GROUND_Y_RATIO;
        const double maxPlayerY = qMax(0.0, static_cast<double>(newSize.height()) -
                                             m_player.body.height());
        m_player.body.moveTop(qBound(0.0, m_player.body.top(), maxPlayerY));
        if (m_player.isOnGround && m_obstacles.isEmpty())
            m_player.body.moveBottom(groundY);
        for (Cloud& cloud : m_clouds) {
            cloud.pos.setX(qBound(-static_cast<double>(cloud.size),
                                  cloud.pos.x(),
                                  static_cast<double>(newSize.width() + cloud.size)));
            cloud.pos.setY(qBound(0.0, cloud.pos.y(),
                                  static_cast<double>(newSize.height() / 3)));
        }
    }
    GameEngine::resizeEvent(event);
}

void MarioGame::initPlayer()
{
    const double viewWidth = qMax(1, width());
    const double viewHeight = qMax(1, height());
    const double playerWidth = qMin(32.0, viewWidth);
    const double playerHeight = qMin(40.0, viewHeight * 0.5);
    const double playerX = qBound(0.0, viewWidth * 0.08, viewWidth - playerWidth);
    m_player.body = QRectF(playerX, qMax(0.0, viewHeight * GROUND_Y_RATIO - playerHeight),
                           playerWidth, playerHeight);
    m_player.velocityY = 0.0;
    m_player.isJumping = false;
    m_player.isOnGround = true;
    m_player.chargeLevel = 0.0;
    m_player.isCharging = false;
    m_player.frame = 0;
    m_player.frameTimer = 0.0;
    m_sensorPressed = false;
}

void MarioGame::resetLevel()
{
    m_obstacles.clear();
    m_obstaclesPassed = 0;
    m_timeSinceLastObstacle = 0.0;
    m_obstacleSpawnInterval = 2.0;
    m_currentLevel = 1;
    m_obstaclesToNextLevel = 10;
    m_sensorPressed = false;
}

void MarioGame::increaseDifficulty()
{
    m_currentLevel++;
    m_obstaclesPassed = 0;
    m_obstaclesToNextLevel = 10 + (m_currentLevel * 5);
    
    // Zmniejsz interwał spawnowania przeszkód
    m_obstacleSpawnInterval = qMax(1.0, 2.0 - ((m_currentLevel - 1) * 0.12));
}

void MarioGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    // Ogranicz bezpośrednie wywołania, tak jak w GameEngine.
    deltaTime = qMin(deltaTime, 0.1);

    // Aktualizacja gracza
    updatePlayer(deltaTime);
    if (!isRunning()) return;
    
    // Generowanie i aktualizacja przeszkód
    generateObstacles(deltaTime);
    updateObstacles(deltaTime);
    if (!isRunning()) return;
    
    // Aktualizacja chmurek
    for (auto& cloud : m_clouds) {
        cloud.pos.setX(cloud.pos.x() - cloud.speed * deltaTime);
        if (cloud.pos.x() + cloud.size < 0) {
            cloud.pos.setX(width() + cloud.size);
            cloud.pos.setY(QRandomGenerator::global()->bounded(qMax(1, height() / 3)));
        }
    }
    
    // Sprawdzenie czy gracz spadł w dziurę
    if (m_player.body.bottom() > height()) {
        stop();
    }
}

void MarioGame::updatePlayer(double deltaTime)
{
    const double groundY = height() * GROUND_Y_RATIO;
    const QRectF previousBody = m_player.body;
    const bool wasGrounded = m_player.isOnGround;

    if (!m_player.isOnGround)
        m_player.velocityY += GRAVITY * deltaTime;

    m_player.body.translate(0.0, m_player.velocityY * deltaTime);

    // Floating platforms are one-way surfaces: land only while descending
    // through their top face. They are not fatal side-wall obstacles.
    bool landedOnPlatform = false;
    if (m_player.velocityY >= 0.0) {
        for (const auto& obstacle : m_obstacles) {
            if (obstacle.type != Obstacle::PLATFORM)
                continue;
            const bool horizontalOverlap =
                m_player.body.right() > obstacle.rect.left() &&
                m_player.body.left() < obstacle.rect.right();
            const bool crossedTop = previousBody.bottom() <= obstacle.rect.top() + 2.0 &&
                                    m_player.body.bottom() >= obstacle.rect.top();
            if (horizontalOverlap && crossedTop) {
                m_player.body.moveBottom(obstacle.rect.top());
                m_player.velocityY = 0.0;
                landedOnPlatform = true;
                break;
            }
        }
    }

    bool overGap = false;
    for (const auto& obstacle : m_obstacles) {
        if (obstacle.type == Obstacle::GAP &&
            m_player.body.right() > obstacle.rect.left() &&
            m_player.body.left() < obstacle.rect.right()) {
            overGap = true;
            break;
        }
    }

    const bool crossedGroundTop = previousBody.bottom() <= groundY &&
                                  m_player.body.bottom() >= groundY;
    const bool supportedByGround = !overGap &&
        ((wasGrounded && previousBody.bottom() >= groundY - 1.0 &&
          previousBody.bottom() <= groundY + 1.0) || crossedGroundTop);
    const bool landed = !wasGrounded && (landedOnPlatform || supportedByGround);
    if (landedOnPlatform) {
        m_player.isOnGround = true;
        m_player.isJumping = false;
    } else if (supportedByGround) {
        m_player.body.setBottom(groundY);
        m_player.velocityY = 0.0;
        m_player.isJumping = false;
        m_player.isOnGround = true;
    } else {
        m_player.isOnGround = false;
    }

    if (landed) {
        m_player.chargeLevel = 0.0;
        m_player.isCharging = false;
    }

    if (m_player.isCharging && m_player.isOnGround)
        m_player.chargeLevel = qMin(1.0, m_player.chargeLevel + deltaTime / MAX_CHARGE_TIME);

    if (m_player.isOnGround && !m_player.isCharging) {
        m_player.frameTimer += deltaTime;
        while (m_player.frameTimer >= 0.1) {
            m_player.frame = (m_player.frame + 1) % 4;
            m_player.frameTimer -= 0.1;
        }
    }
}

void MarioGame::generateObstacles(double deltaTime)
{
    m_timeSinceLastObstacle += qMax(0.0, deltaTime);
    
    while (m_timeSinceLastObstacle >= m_obstacleSpawnInterval) {
        m_timeSinceLastObstacle -= m_obstacleSpawnInterval;
        
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
            const int maximumPipeHeight = qMax(40, static_cast<int>(height() * 0.45));
            int pipeHeight = qMin(maximumPipeHeight, 40 + (m_currentLevel * 8));
            obstacle.rect = QRectF(width(), groundY - pipeHeight, 50, pipeHeight);
        } else if (rand < 60 && m_currentLevel >= 2) {
            // Platforma (15%, od poziomu 2)
            obstacle.type = Obstacle::PLATFORM;
            obstacle.rect = QRectF(width(), groundY - 70 - (QRandomGenerator::global()->bounded(50)), 80, 20);
        } else if (rand < 75 && m_currentLevel >= 3) {
            // Dziura (15%, od poziomu 3)
            obstacle.type = Obstacle::GAP;
            const double maximumGapWidth = qMax(40.0, width() * 0.25);
            const double gapWidth = qMin(maximumGapWidth, 60.0 + (m_currentLevel * 8.0));
            obstacle.rect = QRectF(width(), groundY, gapWidth, height() - groundY);
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
            return;
        }
    }
}

bool MarioGame::checkCollision(const Player& player, const Obstacle& obstacle)
{
    // Dziura staje się śmiertelna, gdy postać opadnie o własną
    // wysokość poniżej poziomu podłoża, pozostając nad otworem.
    if (obstacle.type == Obstacle::GAP) {
        const double groundY = height() * GROUND_Y_RATIO;
        const bool overlapsGap = player.body.right() > obstacle.rect.left() &&
                                 player.body.left() < obstacle.rect.right();
        return overlapsGap && player.body.bottom() > groundY + player.body.height();
    }
    
    // Platforms are handled as one-way landing surfaces in updatePlayer().
    if (obstacle.type == Obstacle::PLATFORM)
        return false;

    // Slightly inset the player hitbox so decorative sprite edges do not
    // cause unfair collisions; narrow spike bounds to match their triangle.
    const QRectF hitbox = player.body.adjusted(4.0, 3.0, -4.0, -2.0);
    if (obstacle.type == Obstacle::SPIKE)
        return hitbox.intersects(obstacle.rect.adjusted(6.0, 0.0, -6.0, 0.0));
    return hitbox.intersects(obstacle.rect);
}

bool MarioGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    // Schmitt trigger: 30% to press, 20% to release. This avoids repeated
    // charge/release transitions when HX711 readings fluctuate near threshold.
    const bool wasPressed = m_sensorPressed;
    m_sensorPressed = wasPressed ? value > (m_sensorThreshold * 0.67)
                                 : value >= m_sensorThreshold;

    if (!wasPressed && m_sensorPressed && m_player.isOnGround) {
        m_player.isCharging = true;
        m_player.frame = 0;
        m_player.chargeLevel = 0.0;
        return true;
    }

    if (wasPressed && !m_sensorPressed && m_player.isCharging && m_player.isOnGround) {
        const double chargeRatio = qBound(0.0, m_player.chargeLevel, 1.0);
        const double jumpForce = JUMP_FORCE * (0.43 + 0.57 * chargeRatio);
        m_player.velocityY = -jumpForce;
        m_player.isJumping = true;
        m_player.isOnGround = false;
        m_player.isCharging = false;
        m_player.chargeLevel = 0.0;
        return true;
    }

    return m_sensorPressed && m_player.isCharging;
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
    // Narysuj otwory ponownie po podłożu, aby pozostały widoczne.
    for (const auto& obs : m_obstacles) {
        if (obs.type == Obstacle::GAP)
            painter.fillRect(obs.rect, Qt::black);
    }

    // Detale ziemi (prosty wzór)
    painter.setPen(QColor(0xA0, 0x30, 0x00));
    for (double x = 0; x < width(); x += 40) {
        for (double y = groundY + 10; y < height(); y += 30) {
            const QRectF tile(x, y, 30, 20);
            bool intersectsGap = false;
            for (const auto& obs : m_obstacles) {
                if (obs.type == Obstacle::GAP && tile.intersects(obs.rect)) {
                    intersectsGap = true;
                    break;
                }
            }
            if (!intersectsGap)
                painter.fillRect(tile, QColor(0xA0, 0x30, 0x00));
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
        const qint64 elapsedMs = m_accumulatedSessionMs + (m_monotonicClock.isValid() ? m_monotonicClock.elapsed() : 0);
        double x = fmod(-static_cast<double>(elapsedMs) / 50.0 + i * 300, width() + 200) - 100;
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
    painter.setFont(QFont("Arial", 10, QFont::Bold));
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
    painter.setFont(QFont("Arial", 7));
    painter.drawText(20, 90, QString("%1/%2").arg(m_obstaclesPassed).arg(m_obstaclesToNextLevel));
    
    // Instrukcja

    // Komunikat o braku połączenia z Arduino
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QColor(255, 100, 100));
        painter.setFont(QFont("Arial", 8, QFont::Bold));
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
    
    m_ownSerial = false;
}

bool MarioGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
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
