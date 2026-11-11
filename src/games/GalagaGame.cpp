#include "games/GalagaGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <QPolygonF>
#include <algorithm>
#include <utility>
#include <cmath>

namespace games {
namespace {
constexpr double kPlayerSpeed = 360.0;
constexpr double kPlayerShotSpeed = 520.0;
constexpr double kHostileShotSpeed = 230.0;
constexpr double kHitProtection = 1.25;
constexpr int kMaximumProjectiles = 100;
}

GalagaGame::GalagaGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_keyboardDirection(0.0)
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
    , m_fireHeld(false)
    , m_sensorInputValid(false)
{
    setFocusPolicy(Qt::StrongFocus);
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
    m_keyboardDirection = 0.0;
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
    m_fireHeld = false;
    m_sensorInputValid = false;
    m_projectiles.clear();
    m_powerUps.clear();
    m_player = QRectF(width() * 0.5 - 18.0, height() - 58.0, 36.0, 24.0);
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
    const int desiredRows = qBound(3, 3 + (m_wave - 1) / 2, 6);
    const int maxRows = qMax(1, (height() - 72) / 30);
    const int rows = qMin(desiredRows, maxRows);
    const int columns = qBound(1, width() / 40, 8);
    const double margin = qMin(24.0, qMax(4.0, width() * 0.05));
    const double cellWidth = qMax(1.0, (width() - 2.0 * margin) / columns);
    const double enemyWidth = qMax(6.0, qMin(28.0, cellWidth - 6.0));
    const double enemyHeight = qMin(22.0, qMax(8.0, (height() - 72.0) / rows - 7.0));
    const double rowSpacing = qMax(enemyHeight + 2.0,
                                   qMin(32.0, (height() - 72.0) / rows));
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Enemy enemy;
            enemy.rect = QRectF(margin + column * cellWidth + (cellWidth - enemyWidth) / 2.0,
                                48.0 + row * rowSpacing, enemyWidth, enemyHeight);
            enemy.hitPoints = (m_wave >= 2 && row == 0) ? 2 + (m_wave >= 6 ? 1 : 0) : 1;
            enemy.active = true;
            enemy.phase = (row * columns + column) * 0.37;
            m_enemies.push_back(enemy);
        }
    }
    m_enemyDirection = 1.0;
    m_enemyFireCooldown = qMax(0.45, 1.15 - m_wave * 0.035);
}

bool GalagaGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    m_force = qBound(0.0, value / 100.0, 1.0);
    m_sensorInputValid = true;
    return true;
}

void GalagaGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A)
        m_keyboardDirection = -1.0;
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D)
        m_keyboardDirection = 1.0;
    else if (event->key() == Qt::Key_Space)
        m_fireHeld = true;
    else if (event->key() == Qt::Key_X && !event->isAutoRepeat())
        activatePulse();
    else
        GameEngine::keyPressEvent(event);
}

void GalagaGame::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        if (m_keyboardDirection < 0.0)
            m_keyboardDirection = 0.0;
    } else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        if (m_keyboardDirection > 0.0)
            m_keyboardDirection = 0.0;
    } else if (event->key() == Qt::Key_Space) {
        m_fireHeld = false;
    } else {
        GameEngine::keyReleaseEvent(event);
    }
}

void GalagaGame::firePlayerShot()
{
    if (m_projectiles.size() >= kMaximumProjectiles)
        return;
    const double centerX = m_player.center().x();
    const int count = m_firePowerLevel >= 3 ? 3 : 1;
    for (int i = 0; i < count && m_projectiles.size() < kMaximumProjectiles; ++i) {
        const double spread = count == 1 ? 0.0 : (i - 1) * 105.0;
        Projectile shot;
        shot.rect = QRectF(centerX - 2.0, m_player.top() - 12.0, 4.0, 12.0);
        shot.velocityX = spread;
        shot.velocityY = -kPlayerShotSpeed;
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
    PowerUp item;
    item.rect = QRectF(position.x() - 8.0, position.y() - 8.0, 16.0, 16.0);
    item.type = ((m_enemiesDefeated / 5) - 1) % 3;
    item.velocity = 95.0 + qMin(45.0, m_wave * 3.0);
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
    m_player.moveCenter(QPointF(width() * 0.5, height() - 46.0));
    for (Projectile& shot : m_projectiles)
        if (!shot.fromPlayer)
            shot.rect = QRectF();
    return true;
}

void GalagaGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);
    m_elapsed += deltaTime;
    m_hitCooldown = qMax(0.0, m_hitCooldown - deltaTime);
    m_shieldRemaining = qMax(0.0, m_shieldRemaining - deltaTime);
    m_firePowerRemaining = qMax(0.0, m_firePowerRemaining - deltaTime);
    if (m_firePowerRemaining <= 0.0)
        m_firePowerLevel = 1;
    m_comboTimer = qMax(0.0, m_comboTimer - deltaTime);
    if (m_comboTimer <= 0.0)
        m_combo = 0;
    m_backgroundOffset = std::fmod(m_backgroundOffset + 24.0 * deltaTime, 40.0);

    const double sensorX = 8.0 + m_force * qMax(0.0, width() - 16.0 - m_player.width());
    const double keyboardX = m_player.x() + m_keyboardDirection * kPlayerSpeed * deltaTime;
    const double targetX = m_keyboardDirection == 0.0 ? sensorX : keyboardX;
    m_player.moveLeft(qBound(6.0, targetX, qMax(6.0, width() - 6.0 - m_player.width())));

    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);
    if (m_fireHeld && m_fireCooldown <= 0.0) {
        firePlayerShot();
        m_fireCooldown = m_firePowerLevel == 2 ? 0.13 : (m_firePowerLevel == 3 ? 0.19 : 0.24);
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
            shot.rect = QRectF(shooter.rect.center().x() - 2.5,
                               shooter.rect.bottom(), 5.0, 11.0);
            shot.velocityX = qBound(-105.0,
                (m_player.center().x() - shooter.rect.center().x()) * 0.2, 105.0);
            shot.velocityY = qMin(420.0, kHostileShotSpeed + m_wave * 13.0);
            shot.fromPlayer = false;
            m_projectiles.push_back(shot);
            break;
        }
        m_enemyFireCooldown = qMax(0.38, 1.3 - m_wave * 0.055);
    }

    const double enemySpeed = qMin(145.0, 25.0 + m_wave * 5.5);
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
                enemy.rect.translate(0.0, 9.0 + qMin(5.0, m_wave * 0.4));
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
                    remaining.rect.moveTop(qMax(42.0, remaining.rect.top() - qMax(24.0, height() * 0.08)));
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
        ? QStringLiteral("Czujnik/ruch  •  SPACJA: ogień  •  X: impuls")
        : QStringLiteral("Czujnik lub A/D, ←/→: ruch  •  SPACJA: ogień  •  X: impuls EMP");
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
