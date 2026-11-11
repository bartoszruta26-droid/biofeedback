#include "games/GalagaGame.hpp"
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <QPolygonF>
#include <algorithm>
#include <utility>
#include <cmath>

namespace games {
namespace {
constexpr double kPlayerShotSpeed = 440.0;
constexpr double kHostileShotSpeed = 190.0;
constexpr double kHitProtection = 1.25;
constexpr int kMaximumProjectiles = 64;
}

GalagaGame::GalagaGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_sensorInputAge(1.0)
    , m_forceBeforeSpecialGesture(0.5)
    , m_specialGestureSeconds(0.0)
    , m_fireCooldown(0.0)
    , m_enemyDirection(1.0)
    , m_enemyFireCooldown(1.0)
    , m_hitCooldown(0.0)
    , m_shieldRemaining(0.0)
    , m_firePowerRemaining(0.0)
    , m_backgroundOffset(0.0)
    , m_elapsed(0.0)
    , m_comboTimer(0.0)
    , m_lives(3)
    , m_wave(1)
    , m_specialCharges(2)
    , m_firePowerLevel(1)
    , m_enemiesDefeated(0)
    , m_combo(0)
    , m_specialGestureActive(false)
    , m_sensorInputValid(false)
{
    setFocusPolicy(Qt::NoFocus);
}

QString GalagaGame::gameName() const
{
    return QStringLiteral("Galaga Force Trainer");
}

void GalagaGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    m_serialPort = std::move(serial);
}

void GalagaGame::disconnectSerial()
{
    m_serialPort.reset();
    m_sensorInputValid = false;
    m_sensorInputAge = 1.0;
}

bool GalagaGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

int GalagaGame::activeEnemyCount() const
{
    return static_cast<int>(std::count_if(m_enemies.cbegin(), m_enemies.cend(),
        [](const Enemy& enemy) { return enemy.active; }));
}

int GalagaGame::activeProjectileCount() const
{
    return static_cast<int>(std::count_if(m_projectiles.cbegin(), m_projectiles.cend(),
        [](const Projectile& shot) { return !shot.rect.isEmpty(); }));
}

int GalagaGame::activePowerUpCount() const
{
    return m_powerUps.size();
}

void GalagaGame::initialize()
{
    m_force = 0.5;
    m_sensorInputAge = 1.0;
    m_forceBeforeSpecialGesture = 0.5;
    m_specialGestureSeconds = 0.0;
    m_fireCooldown = 0.0;
    m_enemyDirection = 1.0;
    m_enemyFireCooldown = 1.0;
    m_hitCooldown = 0.0;
    m_shieldRemaining = 0.0;
    m_firePowerRemaining = 0.0;
    m_backgroundOffset = 0.0;
    m_elapsed = 0.0;
    m_comboTimer = 0.0;
    m_lives = 3;
    m_wave = 1;
    m_specialCharges = 2;
    m_firePowerLevel = 1;
    m_enemiesDefeated = 0;
    m_combo = 0;
    m_specialGestureActive = false;
    m_sensorInputValid = false;
    m_projectiles.clear();
    m_powerUps.clear();
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const double playerWidth = 36.0 * scaleX;
    const double playerHeight = 24.0 * scaleY;
    m_player = QRectF(width() * 0.5 - playerWidth * 0.5,
                      height() - 58.0 * scaleY, playerWidth, playerHeight);
    createWave();
}

void GalagaGame::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        auto scaleRect = [scaleX, scaleY](QRectF& rect) {
            rect = QRectF(rect.x() * scaleX, rect.y() * scaleY,
                          rect.width() * scaleX, rect.height() * scaleY);
        };
        scaleRect(m_player);
        for (Enemy& enemy : m_enemies) {
            scaleRect(enemy.rect);
            enemy.rect.moveLeft(qBound(6.0, enemy.rect.left(),
                qMax(6.0, static_cast<double>(newSize.width()) - 6.0 - enemy.rect.width())));
        }
        for (Projectile& projectile : m_projectiles)
            scaleRect(projectile.rect);
        for (PowerUp& powerUp : m_powerUps)
            scaleRect(powerUp.rect);
        m_player.moveLeft(qBound(6.0, m_player.left(),
            qMax(6.0, static_cast<double>(newSize.width()) - 6.0 - m_player.width())));
        m_player.moveTop(qBound(8.0, m_player.top(),
            qMax(8.0, static_cast<double>(newSize.height()) - 8.0 - m_player.height())));
    }
    GameEngine::resizeEvent(event);
}

void GalagaGame::createWave()
{
    m_enemies.clear();
    m_powerUps.clear();
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const int desiredRows = qBound(3, 3 + (m_wave - 1) / 3, 5);
    const int maxRows = qMax(1, qRound((height() - 72.0 * scaleY) / (30.0 * scaleY)));
    const int rows = qMin(desiredRows, maxRows);
    const int columns = qBound(1, width() / qMax(1, qRound(40.0 * scaleX)), 8);
    const double margin = qMin(24.0 * scaleX, qMax(4.0 * scaleX, width() * 0.05));
    const double cellWidth = qMax(1.0, (width() - 2.0 * margin) / columns);
    const double enemyWidth = qMax(6.0 * scaleX, qMin(28.0 * scaleX, cellWidth - 6.0 * scaleX));
    const double enemyHeight = qMin(22.0 * scaleY,
        qMax(8.0 * scaleY, (height() - 72.0 * scaleY) / rows - 7.0 * scaleY));
    const double rowSpacing = qMax(enemyHeight + 2.0 * scaleY,
                                   qMin(32.0 * scaleY, (height() - 72.0 * scaleY) / rows));
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Enemy enemy;
            enemy.rect = QRectF(margin + column * cellWidth + (cellWidth - enemyWidth) / 2.0,
                                48.0 * scaleY + row * rowSpacing, enemyWidth, enemyHeight);
            enemy.hitPoints = (m_wave >= 2 && row == 0) ? 2 + (m_wave >= 6 ? 1 : 0) : 1;
            enemy.active = true;
            enemy.phase = (row * columns + column) * 0.37;
            m_enemies.push_back(enemy);
        }
    }
    m_enemyDirection = 1.0;
    m_enemyFireCooldown = qMax(0.62, 1.45 - m_wave * 0.025);
}

bool GalagaGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    const double target = qBound(0.0, value / 100.0, 1.0);
    m_sensorInputAge = 0.0;
    m_sensorInputValid = true;
    if (m_specialGestureActive) {
        if (target <= 0.30) {
            const double duration = m_specialGestureSeconds;
            m_specialGestureActive = false;
            m_specialGestureSeconds = 0.0;
            m_force = m_forceBeforeSpecialGesture;
            if (duration >= 0.35 && duration <= 1.2)
                activatePulse();
        }
        return true;
    }
    if (target >= 0.95) {
        m_specialGestureActive = true;
        m_specialGestureSeconds = 0.0;
        m_forceBeforeSpecialGesture = m_force;
        return true;
    }
    m_force += (target - m_force) * 0.45;
    return true;
}

void GalagaGame::firePlayerShot()
{
    if (m_projectiles.size() >= kMaximumProjectiles)
        return;
    const double centerX = m_player.center().x();
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const int count = m_firePowerLevel >= 3 ? 3 : 1;
    for (int i = 0; i < count && m_projectiles.size() < kMaximumProjectiles; ++i) {
        const double spread = count == 1 ? 0.0 : (i - 1) * 80.0 * scaleX;
        Projectile shot;
        shot.rect = QRectF(centerX - 2.0 * scaleX, m_player.top() - 12.0 * scaleY,
                           4.0 * scaleX, 12.0 * scaleY);
        shot.velocityX = spread;
        shot.velocityY = -kPlayerShotSpeed * scaleY;
        shot.fromPlayer = true;
        m_projectiles.push_back(shot);
    }
}

void GalagaGame::spawnPowerUp(const QPointF& position)
{
    // Predictable drops make training sessions reproducible: every fifth
    // defeated enemy releases a rotating upgrade.
    if (m_enemiesDefeated == 0 || m_enemiesDefeated % 5 != 0 || m_powerUps.size() >= 4)
        return;
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    PowerUp item;
    item.rect = QRectF(position.x() - 8.0 * scaleX, position.y() - 8.0 * scaleY,
                       16.0 * scaleX, 16.0 * scaleY);
    item.type = ((m_enemiesDefeated / 5) - 1) % 3;
    item.velocity = (80.0 + qMin(35.0, m_wave * 2.5)) * scaleY;
    m_powerUps.push_back(item);
}

void GalagaGame::collectPowerUp(const PowerUp& powerUp)
{
    switch (powerUp.type) {
    case 0:
        m_firePowerLevel = 3;
        m_firePowerRemaining = 12.0;
        break;
    case 1:
        m_shieldRemaining = qMax(m_shieldRemaining, 8.0);
        break;
    default:
        m_firePowerRemaining = qMax(m_firePowerRemaining, 10.0);
        m_firePowerLevel = 2;
        break;
    }
    addScore(25);
}

void GalagaGame::activatePulse()
{
    if (!isRunning() || m_specialCharges <= 0)
        return;
    --m_specialCharges;
    // EMP pulse removes every hostile projectile and damages the whole
    // formation once. It is deliberately scarce and cannot be auto-repeated.
    for (Projectile& shot : m_projectiles)
        if (!shot.fromPlayer)
            shot.rect = QRectF();
    for (Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        --enemy.hitPoints;
        if (enemy.hitPoints <= 0) {
            enemy.active = false;
            ++m_enemiesDefeated;
            addScore(10);
            spawnPowerUp(enemy.rect.center());
        } else {
            addScore(3);
        }
    }
    m_combo = 0;
    m_comboTimer = 0.0;
}

bool GalagaGame::loseLife()
{
    if (m_hitCooldown > 0.0)
        return false;
    if (m_shieldRemaining > 0.0) {
        m_shieldRemaining = 0.0;
        m_hitCooldown = 0.55;
        return false;
    }
    --m_lives;
    m_combo = 0;
    m_comboTimer = 0.0;
    if (m_lives <= 0) {
        stop();
        return true;
    }
    m_hitCooldown = kHitProtection;
    m_player.moveCenter(QPointF(width() * 0.5,
        height() - 46.0 * qMax(0.5, height() / 480.0)));
    for (Projectile& shot : m_projectiles)
        if (!shot.fromPlayer)
            shot.rect = QRectF();
    return true;
}

void GalagaGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    m_previousPlayer = m_player;
    m_previousProjectiles = m_projectiles;
    m_previousEnemies = m_enemies;
    m_previousPowerUps = m_powerUps;
    m_previousBackgroundOffset = m_backgroundOffset;
    m_previousElapsed = m_elapsed;
    deltaTime = qMin(deltaTime, 0.1);
    m_elapsed += deltaTime;
    m_sensorInputAge += deltaTime;
    if (m_sensorInputAge > 0.35) {
        m_sensorInputValid = false;
        if (m_specialGestureActive) {
            m_specialGestureActive = false;
            m_specialGestureSeconds = 0.0;
            m_force = m_forceBeforeSpecialGesture;
        }
    } else if (m_specialGestureActive) {
        m_specialGestureSeconds += deltaTime;
    }
    m_hitCooldown = qMax(0.0, m_hitCooldown - deltaTime);
    m_shieldRemaining = qMax(0.0, m_shieldRemaining - deltaTime);
    m_firePowerRemaining = qMax(0.0, m_firePowerRemaining - deltaTime);
    if (m_firePowerRemaining <= 0.0)
        m_firePowerLevel = 1;
    m_comboTimer = qMax(0.0, m_comboTimer - deltaTime);
    if (m_comboTimer <= 0.0)
        m_combo = 0;
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    m_backgroundOffset = std::fmod(m_backgroundOffset + 24.0 * scaleY * deltaTime,
                                   40.0 * scaleY);

    if (m_sensorInputValid) {
        const double sensorX = 8.0 * scaleX +
            m_force * qMax(0.0, width() - 16.0 * scaleX - m_player.width());
        m_player.moveLeft(qBound(6.0 * scaleX, sensorX,
            qMax(6.0 * scaleX, width() - 6.0 * scaleX - m_player.width())));
    }

    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);
    if (m_sensorInputValid && !m_specialGestureActive && m_fireCooldown <= 0.0) {
        firePlayerShot();
        m_fireCooldown = m_firePowerLevel == 2 ? 0.17 : (m_firePowerLevel == 3 ? 0.24 : 0.30);
    }

    m_enemyFireCooldown -= deltaTime;
    if (m_enemyFireCooldown <= 0.0 && !m_enemies.isEmpty() &&
        m_projectiles.size() < kMaximumProjectiles) {
        const int startIndex = (m_wave * 7 + getScore()) % m_enemies.size();
        for (int offset = 0; offset < m_enemies.size(); ++offset) {
            const Enemy& shooter = m_enemies[(startIndex + offset) % m_enemies.size()];
            if (!shooter.active)
                continue;
            Projectile shot;
            shot.rect = QRectF(shooter.rect.center().x() - 2.5 * scaleX,
                               shooter.rect.bottom(), 5.0 * scaleX, 11.0 * scaleY);
            shot.velocityY = qMin(340.0 * scaleY,
                (kHostileShotSpeed + m_wave * 8.0) * scaleY);
            // Lead the stationary/current sensor-controlled player instead of
            // using a weak fixed fraction that makes shots miss at wide layouts.
            const double travelTime = qMax(0.12,
                (m_player.top() - shot.rect.bottom()) / shot.velocityY);
            shot.velocityX = qBound(-260.0 * scaleX,
                (m_player.center().x() - shooter.rect.center().x()) / travelTime,
                260.0 * scaleX);
            shot.fromPlayer = false;
            m_projectiles.push_back(shot);
            break;
        }
        m_enemyFireCooldown = qMax(0.55, 1.45 - m_wave * 0.035);
    }

    const double enemySpeed = qMin(95.0, 20.0 + m_wave * 3.5) * scaleX;
    bool hitEdge = false;
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        const double nextX = enemy.rect.x() + m_enemyDirection * enemySpeed * deltaTime;
        if (nextX < 6.0 || nextX + enemy.rect.width() > width() - 6.0) {
            hitEdge = true;
            break;
        }
    }
    if (hitEdge) {
        m_enemyDirection = -m_enemyDirection;
        for (Enemy& enemy : m_enemies)
            if (enemy.active)
                enemy.rect.translate(0.0, (7.0 + qMin(4.0, m_wave * 0.25)) * scaleY);
    }
    for (Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        enemy.rect.translate(m_enemyDirection * enemySpeed * deltaTime, 0.0);
        if (enemy.rect.bottom() >= m_player.top()) {
            loseLife();
            if (!isRunning())
                return;
            for (Enemy& remaining : m_enemies)
                if (remaining.active)
                    remaining.rect.moveTop(qMax(42.0 * scaleY,
                        remaining.rect.top() - qMax(24.0 * scaleY, height() * 0.08)));
            break;
        }
    }

    for (Projectile& shot : m_projectiles) {
        if (shot.rect.isEmpty())
            continue;
        const QRectF previousRect = shot.rect;
        shot.rect.translate(shot.velocityX * deltaTime, shot.velocityY * deltaTime);
        const QRectF sweptRect = previousRect.united(shot.rect);
        if (shot.fromPlayer) {
            for (Enemy& enemy : m_enemies) {
                if (!enemy.active || !sweptRect.intersects(enemy.rect))
                    continue;
                shot.rect = QRectF();
                --enemy.hitPoints;
                if (enemy.hitPoints <= 0) {
                    enemy.active = false;
                    ++m_enemiesDefeated;
                    ++m_combo;
                    m_comboTimer = 2.2;
                    addScore(10 + qMin(20, m_combo * 2));
                    spawnPowerUp(enemy.rect.center());
                } else {
                    addScore(3);
                }
                break;
            }
        } else if (sweptRect.intersects(m_player)) {
            shot.rect = QRectF();
            loseLife();
            if (!isRunning())
                return;
        }
    }

    for (PowerUp& item : m_powerUps) {
        const QRectF previousRect = item.rect;
        item.rect.translate(0.0, item.velocity * deltaTime);
        if (previousRect.united(item.rect).intersects(m_player)) {
            collectPowerUp(item);
            item.rect = QRectF();
        }
    }

    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [this](const Projectile& shot) {
            return shot.rect.isEmpty() || shot.rect.right() < 0.0 ||
                   shot.rect.left() > width() || shot.rect.bottom() < 0.0 ||
                   shot.rect.top() > height();
        }), m_projectiles.end());
    m_powerUps.erase(std::remove_if(m_powerUps.begin(), m_powerUps.end(),
        [this](const PowerUp& item) {
            return item.rect.isEmpty() || item.rect.top() > height();
        }), m_powerUps.end());

    if (activeEnemyCount() == 0) {
        ++m_wave;
        m_specialCharges = qMin(4, m_specialCharges + 1);
        addScore(100 + m_wave * 10);
        createWave();
    }
}

void GalagaGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const QRectF player = m_player;
    const auto projectiles = m_projectiles;
    const auto enemies = m_enemies;
    const auto powerUps = m_powerUps;
    const double background = m_backgroundOffset;
    const double elapsed = m_elapsed;

    const auto interpolateRect = [alpha](QRectF& current, const QRectF& previous) {
        current = QRectF(
            previous.x() + (current.x() - previous.x()) * alpha,
            previous.y() + (current.y() - previous.y()) * alpha,
            current.width(), current.height());
    };
    interpolateRect(m_player, m_previousPlayer);
    if (m_projectiles.size() == m_previousProjectiles.size())
        for (int i = 0; i < m_projectiles.size(); ++i)
            interpolateRect(m_projectiles[i].rect, m_previousProjectiles[i].rect);
    if (m_enemies.size() == m_previousEnemies.size())
        for (int i = 0; i < m_enemies.size(); ++i)
            interpolateRect(m_enemies[i].rect, m_previousEnemies[i].rect);
    if (m_powerUps.size() == m_previousPowerUps.size())
        for (int i = 0; i < m_powerUps.size(); ++i)
            interpolateRect(m_powerUps[i].rect, m_previousPowerUps[i].rect);
    m_backgroundOffset = m_previousBackgroundOffset +
        (background - m_previousBackgroundOffset) * alpha;
    m_elapsed = m_previousElapsed + (elapsed - m_previousElapsed) * alpha;

    render(painter);

    m_player = player;
    m_projectiles = projectiles;
    m_enemies = enemies;
    m_powerUps = powerUps;
    m_backgroundOffset = background;
    m_elapsed = elapsed;
}

void GalagaGame::render(QPainter& painter)
{
    painter.save();
    painter.fillRect(rect(), QColor(4, 7, 22));
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);

    // Layered star field with different parallax speeds.
    for (int i = 0; i < 46; ++i) {
        const double x = std::fmod(i * 97.0 + 31.0, qMax(1, width()));
        const double speed = 0.55 + (i % 4) * 0.45;
        const double y = std::fmod(i * 53.0 + m_backgroundOffset * speed, qMax(1, height()));
        const double radius = i % 9 == 0 ? 2.0 : (i % 3 == 0 ? 1.25 : 0.7);
        painter.setBrush(i % 5 == 0 ? QColor(120, 205, 255, 190) : QColor(115, 135, 185, 145));
        painter.drawEllipse(QPointF(x, y), radius, radius);
    }

    // Ship silhouette, with a visible shield ring when protected.
    if (m_hitCooldown <= 0.0 || static_cast<int>(m_hitCooldown * 12.0) % 2 == 0) {
        QPolygonF ship;
        ship << QPointF(m_player.center().x(), m_player.top() - 8.0)
             << QPointF(m_player.left(), m_player.bottom())
             << QPointF(m_player.center().x(), m_player.bottom() - 5.0)
             << QPointF(m_player.right(), m_player.bottom());
        painter.setBrush(QColor(55, 195, 245));
        painter.setPen(QPen(QColor(190, 245, 255), 1.5));
        painter.drawPolygon(ship);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 175, 75));
        painter.drawEllipse(QPointF(m_player.center().x(), m_player.bottom() + 2.0), 4.0, 7.0);
    }
    if (m_shieldRemaining > 0.0) {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(QColor(90, 235, 255, 190), 2.0));
        painter.drawEllipse(m_player.adjusted(-7.0, -8.0, 7.0, 8.0));
    }

    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        const QColor body = enemy.hitPoints >= 3 ? QColor(185, 105, 255)
            : (enemy.hitPoints == 2 ? QColor(255, 185, 65) : QColor(235, 70, 105));
        painter.setBrush(body);
        painter.setPen(QPen(body.lighter(150), 1.0));
        painter.drawRoundedRect(enemy.rect, 5.0, 5.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 235, 245));
        painter.drawEllipse(QPointF(enemy.rect.center().x() - enemy.rect.width() * 0.2,
                                    enemy.rect.center().y()), 1.5, 1.5);
        painter.drawEllipse(QPointF(enemy.rect.center().x() + enemy.rect.width() * 0.2,
                                    enemy.rect.center().y()), 1.5, 1.5);
    }

    for (const Projectile& shot : m_projectiles) {
        if (shot.rect.isEmpty())
            continue;
        painter.setBrush(shot.fromPlayer ? QColor(90, 240, 255) : QColor(255, 90, 115));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(shot.rect, 2.5, 2.5);
        if (shot.fromPlayer) {
            painter.setPen(QPen(QColor(90, 240, 255, 75), 2.0));
            painter.drawLine(QPointF(shot.rect.center().x(), shot.rect.bottom()),
                             QPointF(shot.rect.center().x(), shot.rect.bottom() + 9.0));
        }
    }

    for (const PowerUp& item : m_powerUps) {
        const QColor color = item.type == 0 ? QColor(90, 235, 255)
            : (item.type == 1 ? QColor(100, 245, 155) : QColor(255, 205, 80));
        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 1.0));
        painter.drawEllipse(item.rect);
        painter.setPen(QPen(QColor(15, 25, 40), 2.0));
        painter.drawLine(item.rect.center().x(), item.rect.top() + 4.0,
                         item.rect.center().x(), item.rect.bottom() - 4.0);
        painter.drawLine(item.rect.left() + 4.0, item.rect.center().y(),
                         item.rect.right() - 4.0, item.rect.center().y());
    }

    painter.setPen(Qt::white);
    const int hudSize = qBound(9, height() / 30, 14);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), hudSize, QFont::DemiBold));
    painter.drawText(QRect(8, 5, width() / 3, hudSize + 10), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(QRect(width() / 3, 5, width() / 3, hudSize + 10), Qt::AlignHCenter | Qt::AlignVCenter,
                     QStringLiteral("Fala: %1").arg(m_wave));
    painter.drawText(QRect(2 * width() / 3, 5, width() / 3 - 8, hudSize + 10),
                     Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("Życia: %1").arg(m_lives));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(7, hudSize - 2, 10)));
    const QString controls = width() < 300
        ? QStringLiteral("Tensometr: ruch  •  ogień automatyczny  •  gest 95%: EMP")
        : QStringLiteral("Tensometr: ruch statku  •  ogień automatyczny  •  gest 95%: EMP");
    painter.drawText(QRect(6, height() - hudSize - 7, width() - 12, hudSize + 4),
                     Qt::AlignHCenter | Qt::AlignVCenter, controls);
    painter.drawText(QRect(8, hudSize + 8, width() - 16, hudSize + 4), Qt::AlignLeft,
        QStringLiteral("EMP: %1   Seria: x%2   %3")
            .arg(m_specialCharges).arg(m_combo)
            .arg(m_shieldRemaining > 0.0 ? QStringLiteral("OSŁONA")
                 : (m_firePowerLevel == 3 ? QStringLiteral("POTRÓJNY STRZAŁ")
                    : (m_firePowerLevel == 2 ? QStringLiteral("SZYBKI OGIEŃ") : QString()))));
    if (m_hitCooldown > 0.0) {
        painter.setPen(QColor(255, 225, 120));
        painter.drawText(QRect(0, height() / 2 - 18, width(), 36), Qt::AlignHCenter | Qt::AlignVCenter,
                         QStringLiteral("OSŁONA PO TRAFIENIU"));
    }
    painter.restore();
}

} // namespace games
