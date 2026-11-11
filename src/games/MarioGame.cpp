#include "games/MarioGame.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QPolygonF>
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
int MarioGame::activeEnemyCount() const { int n = 0; for (const Enemy& e : m_enemies) if (e.alive) ++n; return n; }
int MarioGame::goombaCount() const { int n = 0; for (const Enemy& e : m_enemies) if (e.alive && e.type == Enemy::GOOMBA) ++n; return n; }
int MarioGame::piranhaPlantCount() const { int n = 0; for (const Enemy& e : m_enemies) if (e.alive && e.type == Enemy::PIRANHA_PLANT) ++n; return n; }
int MarioGame::currentLevel() const { return m_currentLevel; }
int MarioGame::livesRemaining() const { return m_lives; }
int MarioGame::coinsCollected() const { return m_coinsCollected; }
bool MarioGame::doubleJumpAvailable() const { return m_player.doubleJumpAvailable; }
double MarioGame::invulnerabilityRemaining() const { return m_invulnerabilityRemaining; }
double MarioGame::worldScrollDistance() const { return m_worldDistance; }
double MarioGame::animationClock() const { return m_animationTime; }

void MarioGame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
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
                         QStringLiteral("Przytrzymaj tensometr lub SPACJĘ, aby naładować skok.\nPuść, aby skoczyć; w powietrzu naciśnij ponownie, aby wykonać podwójny skok.\nNaciśnij R, aby zagrać ponownie.  Wynik: %1").arg(getScore()));
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

        for (Enemy& enemy : m_enemies) {
            enemy.rect = QRectF(enemy.rect.x() * scaleX, enemy.rect.y() * scaleY,
                                enemy.rect.width() * scaleX, enemy.rect.height() * scaleY);
        }
        for (Collectible& item : m_collectibles) {
            item.rect = QRectF(item.rect.x() * scaleX, item.rect.y() * scaleY,
                               item.rect.width() * scaleX, item.rect.height() * scaleY);
        }
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
    m_keyboardJumpHeld = false;
    m_player.doubleJumpAvailable = true;
}

void MarioGame::resetLevel()
{
    m_obstacles.clear();
    m_collectibles.clear();
    m_enemies.clear();
    m_worldDistance = 0.0;
    m_animationTime = 0.0;
    m_enemySpawnTimer = 0.0;
    m_obstaclesPassed = 0;
    m_timeSinceLastObstacle = 0.0;
    m_obstacleSpawnInterval = 2.0;
    m_currentLevel = 1;
    m_obstaclesToNextLevel = 10;
    m_sensorPressed = false;
    m_keyboardJumpHeld = false;
    m_lives = STARTING_LIVES;
    m_coinsCollected = 0;
    m_coinStreak = 0;
    m_invulnerabilityRemaining = 0.0;
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

    // A single scene clock drives all animation and parallax from real gameplay travel.
    m_animationTime += deltaTime;
    m_worldDistance += PLAYER_SPEED * deltaTime;

    // Aktualizacja gracza
    updatePlayer(deltaTime);
    if (!isRunning()) return;
    
    // Generowanie i aktualizacja przeszkód
    generateObstacles(deltaTime);
    generateEnemies(deltaTime);
    updateObstacles(deltaTime);
    updateEnemies(deltaTime);
    if (!isRunning()) return;
    updateCollectibles(deltaTime);
    if (!isRunning()) return;
    if (m_invulnerabilityRemaining > 0.0)
        m_invulnerabilityRemaining = qMax(0.0, m_invulnerabilityRemaining - deltaTime);
    
    // Aktualizacja chmurek
    for (auto& cloud : m_clouds) {
        cloud.pos.setX(cloud.pos.x() - cloud.speed * deltaTime);
        if (cloud.pos.x() + cloud.size < 0) {
            cloud.pos.setX(width() + cloud.size);
            cloud.pos.setY(QRandomGenerator::global()->bounded(qMax(1, height() / 3)));
        }
    }
    
    // Falling into a pit costs one life, then respawns at the ground line.
    if (m_player.body.top() > height() || m_player.body.bottom() > height() + m_player.body.height()) {
        applyDamage();
        if (isRunning()) {
            m_player.body.moveTop(height() * GROUND_Y_RATIO - m_player.body.height());
            m_player.velocityY = -280.0;
            m_player.isOnGround = false;
            m_player.isJumping = true;
        }
    }
}

void MarioGame::updatePlayer(double deltaTime)
{
    const double groundY = height() * GROUND_Y_RATIO;
    const QRectF previousBody = m_player.body;
    m_previousPlayerBody = previousBody;
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
        m_player.doubleJumpAvailable = true;
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

void MarioGame::generateEnemies(double deltaTime)
{
    m_enemySpawnTimer += qMax(0.0, deltaTime);
    const double groundY = height() * GROUND_Y_RATIO;
    const double interval = qMax(1.8, 3.8 - (m_currentLevel - 1) * 0.12);
    while (m_enemySpawnTimer >= interval) {
        m_enemySpawnTimer -= interval;
        Enemy enemy;
        enemy.type = Enemy::GOOMBA;
        enemy.rect = QRectF(width() + 12.0, groundY - 29.0, 29.0, 29.0);
        enemy.phase = QRandomGenerator::global()->generateDouble() * 6.283185307179586;
        enemy.alive = true;
        m_enemies.append(enemy);
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
        if (obstacle.type == Obstacle::PIPE && QRandomGenerator::global()->bounded(100) < 38) {
            Enemy plant;
            plant.type = Enemy::PIRANHA_PLANT;
            plant.rect = QRectF(obstacle.rect.center().x() - 13.0, obstacle.rect.top() - 42.0,
                                26.0, 42.0);
            plant.phase = QRandomGenerator::global()->generateDouble() * 6.283185307179586;
            plant.alive = true;
            m_enemies.append(plant);
        }

        // A short coin arc rewards a well-timed jump; rare stars grant temporary protection.
        if (QRandomGenerator::global()->bounded(100) < 72) {
            const int coinCount = 3 + QRandomGenerator::global()->bounded(3);
            for (int coin = 0; coin < coinCount; ++coin) {
                const double arc = std::sin((coin + 1) * 3.14159265358979323846 / (coinCount + 1));
                Collectible item;
                item.type = Collectible::COIN;
                item.rect = QRectF(width() + 35.0 + coin * 30.0,
                                   groundY - 48.0 - arc * 62.0, 18.0, 18.0);
                item.phase = QRandomGenerator::global()->generateDouble() * 6.283185307179586;
                m_collectibles.append(item);
            }
        }
        if (QRandomGenerator::global()->bounded(100) < 8) {
            Collectible star;
            star.type = Collectible::STAR;
            star.rect = QRectF(width() + 150.0, groundY - 112.0, 24.0, 24.0);
            star.phase = QRandomGenerator::global()->generateDouble() * 6.283185307179586;
            m_collectibles.append(star);
        }
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
            applyDamage();
            if (!isRunning()) return;
        }
    }
}

void MarioGame::updateEnemies(double deltaTime)
{
    const QRectF previous = m_previousPlayerBody;
    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy& enemy = m_enemies[i];
        enemy.phase += deltaTime * (enemy.type == Enemy::GOOMBA ? 5.0 : 2.8);
        enemy.rect.translate(-PLAYER_SPEED * deltaTime, 0.0);
        if (enemy.rect.right() < -4.0 || !enemy.alive) {
            m_enemies.removeAt(i);
            continue;
        }
        if (!m_player.body.adjusted(3.0, 2.0, -3.0, -2.0).intersects(enemy.rect))
            continue;
        const bool stomp = enemy.type == Enemy::GOOMBA && m_player.velocityY > 0.0 &&
            previous.bottom() <= enemy.rect.top() + 9.0 &&
            m_player.body.bottom() >= enemy.rect.top();
        if (stomp) {
            enemy.alive = false;
            m_player.body.moveBottom(enemy.rect.top());
            m_player.velocityY = -JUMP_FORCE * 0.48;
            m_player.isOnGround = false;
            m_player.isJumping = true;
            m_player.doubleJumpAvailable = true;
            addScore(200);
            m_coinStreak = 0;
        } else if (enemy.type == Enemy::PIRANHA_PLANT) {
            // Plants briefly retract, then become dangerous again.
            if (std::sin(enemy.phase) > -0.65) applyDamage();
        } else {
            applyDamage();
        }
        if (!isRunning()) return;
    }
}

void MarioGame::renderEnemies(QPainter& painter)
{
    painter.save();
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.alive) continue;
        const QRectF r = enemy.rect;
        if (enemy.type == Enemy::GOOMBA) {
            const double squash = 1.0 + 0.08 * std::sin(enemy.phase);
            const QRectF body(r.left(), r.top() + (1.0 - squash) * 8.0,
                              r.width(), r.height() * squash);
            painter.setPen(QPen(QColor(0x4B, 0x22, 0x12), 2.0));
            painter.setBrush(QColor(0xA9, 0x58, 0x24));
            painter.drawRoundedRect(body.adjusted(1, 4, -1, 0), 8, 8);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0xF8, 0xE8, 0xC8));
            painter.drawEllipse(QRectF(r.left() + 6, r.top() + 10, 6, 8));
            painter.drawEllipse(QRectF(r.left() + 17, r.top() + 10, 6, 8));
            painter.setBrush(QColor(0x20, 0x18, 0x12));
            painter.drawEllipse(QRectF(r.left() + 8, r.top() + 13, 3, 4));
            painter.drawEllipse(QRectF(r.left() + 19, r.top() + 13, 3, 4));
            painter.setPen(QPen(QColor(0x3A, 0x1B, 0x10), 3.0));
            painter.drawLine(QPointF(r.left() + 3, r.bottom() - 2), QPointF(r.left() + 12, r.bottom() - 2));
            painter.drawLine(QPointF(r.left() + 17, r.bottom() - 2), QPointF(r.right() - 3, r.bottom() - 2));
        } else {
            const double lift = (std::sin(enemy.phase) > -0.15) ? 0.0 : 17.0;
            const QRectF stem(r.center().x() - 6, r.top() + lift + 12, 12, r.height() - 12 - lift);
            painter.setPen(QPen(QColor(0x16, 0x79, 0x27), 2.0));
            painter.setBrush(QColor(0x35, 0xB9, 0x43));
            painter.drawRoundedRect(stem, 5, 5);
            const QRectF head(r.left(), r.top() + lift, r.width(), 22);
            painter.setPen(QPen(QColor(0x9B, 0x1F, 0x24), 2.0));
            painter.setBrush(QColor(0xE9, 0x43, 0x45));
            painter.drawEllipse(head);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0xFF, 0xF4, 0xD8));
            for (int dot = 0; dot < 5; ++dot) {
                const double x = head.left() + 4 + dot * 4.5;
                painter.drawEllipse(QRectF(x, head.top() + 5 + (dot % 2) * 7, 3, 3));
            }
            painter.setBrush(QColor(0x22, 0x18, 0x18));
            painter.drawEllipse(QRectF(head.left() + 5, head.top() + 8, 4, 5));
            painter.drawEllipse(QRectF(head.right() - 9, head.top() + 8, 4, 5));
        }
    }
    painter.restore();
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

void MarioGame::launchJump(double force)
{
    const bool launchedFromGround = m_player.isOnGround;
    m_player.velocityY = -qBound(180.0, force, JUMP_FORCE);
    m_player.isJumping = true;
    m_player.isOnGround = false;
    m_player.isCharging = false;
    m_player.chargeLevel = 0.0;
    m_player.doubleJumpAvailable = launchedFromGround;
}

void MarioGame::applyDamage()
{
    if (m_invulnerabilityRemaining > 0.0 || !isRunning())
        return;
    --m_lives;
    m_coinStreak = 0;
    if (m_lives <= 0) {
        stop();
        return;
    }
    m_invulnerabilityRemaining = HIT_PROTECTION_TIME;
    m_player.isCharging = false;
    m_player.chargeLevel = 0.0;
    m_player.velocityY = -280.0;
    m_player.isJumping = true;
    m_player.isOnGround = false;
    m_player.doubleJumpAvailable = false;
}

void MarioGame::updateCollectibles(double deltaTime)
{
    for (int i = m_collectibles.size() - 1; i >= 0; --i) {
        Collectible& item = m_collectibles[i];
        item.rect.translate(-PLAYER_SPEED * deltaTime, 0.0);
        item.phase += deltaTime * 4.0;
        if (item.rect.right() < 0.0) {
            m_collectibles.removeAt(i);
            continue;
        }
        if (!m_player.body.adjusted(2.0, 2.0, -2.0, -2.0).intersects(item.rect))
            continue;
        if (item.type == Collectible::COIN) {
            ++m_coinsCollected;
            ++m_coinStreak;
            addScore(50 + qMin(100, (m_coinStreak - 1) * 10));
        } else {
            m_invulnerabilityRemaining = qMax(m_invulnerabilityRemaining, STAR_POWER_TIME);
            addScore(100);
            m_coinStreak = 0;
        }
        m_collectibles.removeAt(i);
    }
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

    if (!wasPressed && m_sensorPressed) {
        if (m_player.isOnGround) {
            m_player.isCharging = true;
            m_player.frame = 0;
            m_player.chargeLevel = 0.0;
        } else if (m_player.doubleJumpAvailable) {
            launchJump(JUMP_FORCE * 0.78);
        }
        return true;
    }

    if (wasPressed && !m_sensorPressed && m_player.isCharging && m_player.isOnGround) {
        const double chargeRatio = qBound(0.0, m_player.chargeLevel, 1.0);
        const double jumpForce = JUMP_FORCE * (0.43 + 0.57 * chargeRatio);
        launchJump(jumpForce);
        return true;
    }

    return m_sensorPressed && m_player.isCharging;
}

void MarioGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_R && !isRunning() && !event->isAutoRepeat()) {
        reset();
        start();
        event->accept();
        return;
    }
    const bool jumpKey = event->key() == Qt::Key_Space ||
                         event->key() == Qt::Key_Up ||
                         event->key() == Qt::Key_W;
    if (jumpKey && !event->isAutoRepeat()) {
        if (!m_keyboardJumpHeld) {
            m_keyboardJumpHeld = true;
            if (isRunning() && m_player.isOnGround) {
                m_player.isCharging = true;
                m_player.chargeLevel = 0.0;
                m_player.frame = 0;
            } else if (isRunning() && m_player.doubleJumpAvailable) {
                launchJump(JUMP_FORCE * 0.78);
            }
        }
        event->accept();
        return;
    }
    GameEngine::keyPressEvent(event);
}

void MarioGame::keyReleaseEvent(QKeyEvent* event)
{
    const bool jumpKey = event->key() == Qt::Key_Space ||
                         event->key() == Qt::Key_Up ||
                         event->key() == Qt::Key_W;
    if (jumpKey && !event->isAutoRepeat()) {
        if (m_keyboardJumpHeld && m_player.isCharging && m_player.isOnGround) {
            const double chargeRatio = qBound(0.0, m_player.chargeLevel, 1.0);
            launchJump(JUMP_FORCE * (0.43 + 0.57 * chargeRatio));
        }
        m_keyboardJumpHeld = false;
        event->accept();
        return;
    }
    GameEngine::keyReleaseEvent(event);
}

void MarioGame::render(QPainter& painter)
{
    renderBackground(painter);
    renderObstacles(painter);
    renderEnemies(painter);
    renderCollectibles(painter);
    if (m_invulnerabilityRemaining <= 0.0 ||
        (static_cast<int>(m_invulnerabilityRemaining * 12.0) % 2 == 0))
        renderPlayer(painter, m_player);
    renderUI(painter);
}

void MarioGame::renderCollectibles(QPainter& painter)
{
    painter.save();
    for (const Collectible& item : m_collectibles) {
        const double bob = std::sin(item.phase) * 3.0;
        const double spin = 0.78 + 0.22 * std::abs(std::sin(item.phase * 1.7));
        const QRectF r = item.rect.translated(0.0, bob);
        if (item.type == Collectible::COIN) {
            painter.setPen(QPen(QColor(0x8C, 0x55, 0x00), 2.0));
            painter.setBrush(QColor(0xFF, 0xD8, 0x32));
            painter.drawEllipse(QRectF(r.center().x() - r.width() * spin * 0.5, r.top(),
                                       r.width() * spin, r.height()));
            painter.setPen(QPen(QColor(0xFF, 0xF3, 0xA0), 2.0));
            painter.drawLine(QPointF(r.center().x(), r.top() + 4.0),
                             QPointF(r.center().x(), r.bottom() - 4.0));
        } else {
            const QPointF c = r.center();
            painter.save();
            painter.translate(c);
            painter.rotate(std::sin(item.phase) * 8.0);
            painter.translate(-c);
            QPolygonF star;
            for (int point = 0; point < 10; ++point) {
                const double angle = -1.5707963267948966 + point * 3.14159265358979323846 / 5.0;
                const double radius = (point % 2 == 0) ? r.width() * 0.5 : r.width() * 0.22;
                star << QPointF(c.x() + std::cos(angle) * radius,
                                c.y() + std::sin(angle) * radius);
            }
            painter.setPen(QPen(QColor(0x1A, 0x66, 0xB3), 1.5));
            painter.setBrush(QColor(0x6D, 0xE8, 0xFF));
            painter.drawPolygon(star);
            painter.restore();
        }
    }
    painter.restore();
}

void MarioGame::renderPlayer(QPainter& painter, const Player& player)
{
    const double x = player.body.x();
    const double y = player.body.y();
    const double w = player.body.width();
    const double h = player.body.height();
    const double u = qMin(w / 16.0, h / 20.0); // Pixel-art unit, scales with widget size.
    const double ox = x + (w - 16.0 * u) * 0.5;
    const double oy = y + (h - 20.0 * u) * 0.5;
    const auto px = [&](double gx, double gy, double gw, double gh) {
        return QRectF(ox + gx * u, oy + gy * u, gw * u, gh * u);
    };

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Ground shadow gives the sprite a clear sense of height while jumping.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, player.isOnGround ? 65 : 28));
    const double groundY = height() * GROUND_Y_RATIO;
    painter.drawEllipse(QRectF(x - 3 * u, groundY - 2 * u, w + 6 * u, 3 * u));

    // Back arm and glove.
    painter.setPen(QPen(QColor(0x6E, 0x25, 0x1B), 0.7 * u));
    painter.setBrush(COLOR_MARIO_RED);
    painter.drawRect(px(2, 10, 3, 4));
    painter.setBrush(Qt::white);
    painter.drawRect(px(1, 13, 3, 2));

    // Overalls and shirt.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xD9, 0x32, 0x27));
    painter.drawRect(px(5, 9, 7, 5));
    painter.setBrush(QColor(0x20, 0x55, 0xB8));
    painter.drawRect(px(4, 12, 8, 5));
    painter.drawRect(px(5, 10, 2, 4));
    painter.drawRect(px(9, 10, 2, 4));
    painter.setBrush(QColor(0xFF, 0xD9, 0x43));
    painter.drawRect(px(5, 12, 1, 1));
    painter.drawRect(px(10, 12, 1, 1));

    // Head, ear, hair and face.
    painter.setBrush(COLOR_MARIO_SKIN);
    painter.drawRect(px(5, 4, 8, 6));
    painter.drawRect(px(4, 6, 2, 3));
    painter.setBrush(COLOR_MARIO_BROWN);
    painter.drawRect(px(4, 5, 2, 3));
    painter.drawRect(px(5, 8, 3, 2));
    painter.drawRect(px(9, 8, 4, 1));
    painter.setBrush(QColor(0x20, 0x18, 0x18));
    painter.drawRect(px(10, 6, 1, 2));
    painter.setBrush(QColor(0xFF, 0xD1, 0xA0));
    painter.drawRect(px(11, 7, 2, 1));

    // Cap with a visible brim and badge.
    painter.setBrush(COLOR_MARIO_RED);
    painter.drawRect(px(4, 3, 10, 2));
    painter.drawRect(px(6, 2, 7, 2));
    painter.drawRect(px(12, 4, 3, 1));
    painter.setBrush(Qt::white);
    painter.drawRect(px(8, 3, 2, 1));

    // Front arm and glove.
    painter.setBrush(COLOR_MARIO_RED);
    painter.drawRect(px(11, 10, 3, 3));
    painter.setBrush(Qt::white);
    painter.drawRect(px(13, 12, 2, 2));

    // Animated legs and sturdy brown shoes.
    const bool stride = !player.isJumping && !player.isCharging && (player.frame % 2 == 0);
    if (player.isJumping) {
        painter.setBrush(COLOR_MARIO_BROWN);
        painter.drawRect(px(4, 16, 4, 2));
        painter.drawRect(px(9, 16, 4, 2));
        painter.setBrush(QColor(0xB5, 0x72, 0x32));
        painter.drawRect(px(3, 17, 4, 2));
        painter.drawRect(px(10, 17, 4, 2));
    } else if (player.isCharging) {
        painter.setBrush(COLOR_MARIO_BROWN);
        painter.drawRect(px(3, 17, 5, 2));
        painter.drawRect(px(9, 17, 5, 2));
    } else if (stride) {
        painter.setBrush(COLOR_MARIO_BROWN);
        painter.drawRect(px(4, 16, 3, 2));
        painter.drawRect(px(9, 15, 3, 3));
        painter.setBrush(QColor(0xB5, 0x72, 0x32));
        painter.drawRect(px(2, 18, 5, 2));
        painter.drawRect(px(10, 17, 5, 2));
    } else {
        painter.setBrush(COLOR_MARIO_BROWN);
        painter.drawRect(px(4, 15, 3, 3));
        painter.drawRect(px(9, 16, 3, 2));
        painter.setBrush(QColor(0xB5, 0x72, 0x32));
        painter.drawRect(px(3, 17, 5, 2));
        painter.drawRect(px(9, 18, 5, 2));
    }

    // Dark pixel outline around the silhouette.
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(0x4A, 0x20, 0x18), qMax(1.0, 0.65 * u)));
    painter.drawRect(px(4, 2, 10, 8));
    painter.drawRect(px(4, 9, 9, 8));
    painter.restore();
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

    // Bright grass lip and layered dirt bands improve ground readability.
    painter.fillRect(QRectF(0, groundY, width(), 7), QColor(0x4C, 0xC6, 0x3A));
    painter.fillRect(QRectF(0, groundY + 7, width(), 5), QColor(0x2E, 0x8B, 0x2E));
    // Ground texture scrolls at full camera speed; tile scale increases toward the foreground.
    const double tileWidth = 40.0;
    const double groundOffset = std::fmod(m_worldDistance, tileWidth);
    painter.setPen(QColor(0xA0, 0x30, 0x00));
    for (double x = -tileWidth - groundOffset; x < width() + tileWidth; x += tileWidth) {
        for (double y = groundY + 10; y < height(); y += 30) {
            const double depth = qBound(0.0, (y - groundY) / qMax(1.0, height() - groundY), 1.0);
            const double tileScale = 0.68 + depth * 0.32;
            const QRectF tile(x, y, 30.0 * tileScale, 20.0 * tileScale);
            bool intersectsGap = false;
            for (const auto& obs : m_obstacles) {
                if (obs.type == Obstacle::GAP && tile.intersects(obs.rect)) {
                    intersectsGap = true;
                    break;
                }
            }
            if (!intersectsGap)
                painter.fillRect(tile, depth > 0.55 ? QColor(0x8A, 0x37, 0x12) : QColor(0xA0, 0x30, 0x00));
        }
    }
    // Keep the entire opening clear after grass and texture have been painted.
    for (const auto& obs : m_obstacles) {
        if (obs.type == Obstacle::GAP)
            painter.fillRect(obs.rect, QColor(0x12, 0x20, 0x32));
    }
}

void MarioGame::renderBackground(QPainter& painter)
{
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(0x36, 0x76, 0xD8));
    sky.setColorAt(0.68, QColor(0x8C, 0xD8, 0xF4));
    sky.setColorAt(1.0, QColor(0xD5, 0xF2, 0xFF));
    painter.fillRect(rect(), sky);

    // Sun glow and pixel-style sun give the scene depth without external assets.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xFF, 0xF0, 0xA0, 80));
    painter.drawEllipse(QRectF(width() * 0.76, height() * 0.08, 100, 100));
    const double sunPulse = 1.0 + 0.045 * std::sin(m_animationTime * 1.4);
    const QRectF sunRect(width() * 0.80 - 17.0 * (sunPulse - 1.0),
                         height() * 0.12 - 17.0 * (sunPulse - 1.0),
                         34.0 * sunPulse, 34.0 * sunPulse);
    painter.setBrush(QColor(0xFF, 0xE6, 0x75));
    painter.drawRect(sunRect);

    renderParallaxBackground(painter);
    renderClouds(painter);
}

void MarioGame::renderClouds(QPainter& painter)
{
    painter.save();
    for (int i = 0; i < m_clouds.size(); ++i) {
        const Cloud& cloud = m_clouds[i];
        const double scale = qMax(0.55, cloud.size / 42.0);
        const double bob = std::sin(m_animationTime * (0.7 + scale * 0.18) + i * 1.7) * 2.8 * scale;
        const double x = cloud.pos.x();
        const double y = cloud.pos.y() + bob;
        const QRectF shadow(x - 27 * scale, y - 3 * scale, 58 * scale, 25 * scale);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0x65, 0xA8, 0xD4, 80));
        painter.drawRoundedRect(shadow, 12 * scale, 12 * scale);
        painter.setBrush(QColor(0xFF, 0xFF, 0xFF));
        painter.drawEllipse(QRectF(x - 23 * scale, y - 9 * scale, 28 * scale, 26 * scale));
        painter.drawEllipse(QRectF(x - 7 * scale, y - 19 * scale, 35 * scale, 34 * scale));
        painter.drawEllipse(QRectF(x + 10 * scale, y - 7 * scale, 27 * scale, 24 * scale));
        painter.fillRect(QRectF(x - 20 * scale, y + 2 * scale, 48 * scale, 10 * scale), Qt::white);
    }
    painter.restore();
}

void MarioGame::renderParallaxBackground(QPainter& painter)
{
    const double groundY = height() * GROUND_Y_RATIO;
    const double world = m_worldDistance;
    painter.save();
    painter.setPen(Qt::NoPen);

    // Distant mountains: 9% of camera speed, muted and close to the horizon.
    const double farSpacing = 330.0;
    const double farOffset = std::fmod(world * 0.09, farSpacing);
    for (int i = -1; i < width() / farSpacing + 3; ++i) {
        const double x = i * farSpacing - farOffset;
        const double peak = groundY - height() * 0.25;
        QPolygonF mountain;
        mountain << QPointF(x, groundY)
                 << QPointF(x + farSpacing * 0.26, peak + 28.0)
                 << QPointF(x + farSpacing * 0.48, peak - 14.0)
                 << QPointF(x + farSpacing * 0.72, peak + 34.0)
                 << QPointF(x + farSpacing, groundY);
        painter.setBrush(QColor(0x83, 0xB9, 0xB0));
        painter.drawPolygon(mountain);
        QPolygonF snow;
        snow << QPointF(x + farSpacing * 0.38, peak + 8.0)
             << QPointF(x + farSpacing * 0.48, peak - 14.0)
             << QPointF(x + farSpacing * 0.58, peak + 10.0)
             << QPointF(x + farSpacing * 0.50, peak + 3.0);
        painter.setBrush(QColor(0xD8, 0xEF, 0xE4, 180));
        painter.drawPolygon(snow);
    }

    // Midground hills: 26% speed and larger apparent shapes.
    const double midSpacing = 250.0;
    const double midOffset = std::fmod(world * 0.26, midSpacing);
    for (int i = -1; i < width() / midSpacing + 3; ++i) {
        const double x = i * midSpacing - midOffset;
        const double hillHeight = height() * (0.105 + ((i & 1) ? 0.018 : 0.0));
        painter.setBrush(QColor(0x69, 0xC2, 0x75));
        painter.drawEllipse(QRectF(x, groundY - hillHeight * 0.72,
                                   midSpacing * 0.92, hillHeight * 1.45));
        painter.setBrush(QColor(0xA4, 0xDF, 0x8B, 170));
        painter.drawEllipse(QRectF(x + midSpacing * 0.18, groundY - hillHeight * 0.54,
                                   midSpacing * 0.22, hillHeight * 0.16));
    }

    // Foreground shrubs: 58% speed, larger and higher contrast.
    const double nearSpacing = 190.0;
    const double nearOffset = std::fmod(world * 0.58, nearSpacing);
    for (int i = -1; i < width() / nearSpacing + 3; ++i) {
        const double x = i * nearSpacing - nearOffset;
        const double base = groundY + 1.0;
        const double scale = 0.72 + ((i & 1) ? 0.16 : 0.0);
        painter.setBrush(QColor(0x2B, 0x91, 0x48));
        painter.drawEllipse(QRectF(x + 16, base - 48 * scale, 42 * scale, 58 * scale));
        painter.drawEllipse(QRectF(x + 39, base - 62 * scale, 48 * scale, 72 * scale));
        painter.drawEllipse(QRectF(x + 66, base - 42 * scale, 38 * scale, 52 * scale));
        painter.setBrush(QColor(0x1C, 0x6F, 0x3A));
        painter.drawRect(QRectF(x + 51, base - 12 * scale, 9 * scale, 14 * scale));
    }

    // Three flocks of small birds cross the sky with animated wing beats.
    painter.setPen(QPen(QColor(0x3D, 0x72, 0x8D, 170), 1.8));
    for (int bird = 0; bird < 3; ++bird) {
        const double cycle = std::fmod(m_animationTime * (18.0 + bird * 3.0) +
                                       bird * width() * 0.42, width() + 70.0);
        const double bx = cycle - 35.0;
        const double by = height() * (0.20 + bird * 0.055) +
                          std::sin(m_animationTime * 2.1 + bird) * 4.0;
        const double wing = 3.0 + 2.0 * std::sin(m_animationTime * 8.0 + bird);
        painter.drawArc(QRectF(bx - 8, by - wing, 9, wing * 2), 0, 150 * 16);
        painter.drawArc(QRectF(bx, by - wing, 9, wing * 2), 30 * 16, 150 * 16);
    }
    painter.restore();
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
    
    // Wynik i status postaci: translucent panel keeps text readable over scenery.
    painter.fillRect(QRectF(10, 8, 190, 116), QColor(0, 0, 0, 105));
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(20, 30, QString("Wynik: %1").arg(m_score));
    painter.drawText(20, 48, QString("Życia: %1   Monety: %2").arg(m_lives).arg(m_coinsCollected));
    if (m_invulnerabilityRemaining > 0.0) {
        painter.setPen(QColor(0x90, 0xED, 0xFF));
        painter.drawText(20, 66, QString("Ochrona: %1 s").arg(m_invulnerabilityRemaining, 0, 'f', 1));
        painter.setPen(Qt::white);
    }
    
    // Poziom
    painter.drawText(20, 84, QString("Poziom: %1").arg(m_currentLevel));
    
    // Postęp do następnego poziomu
    int progressWidth = 150;
    int progressHeight = 10;
    double progress = static_cast<double>(m_obstaclesPassed) / m_obstaclesToNextLevel;
    painter.fillRect(QRectF(20, 94, progressWidth, progressHeight), Qt::darkGray);
    painter.fillRect(QRectF(20, 94, progressWidth * progress, progressHeight), Qt::green);
    painter.setPen(Qt::white);
    painter.drawRect(QRectF(20, 94, progressWidth, progressHeight));
    painter.setFont(QFont("Arial", 7));
    painter.drawText(20, 119, QString("%1/%2").arg(m_obstaclesPassed).arg(m_obstaclesToNextLevel));
    
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
