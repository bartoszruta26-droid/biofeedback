#include "games/GalagaGame.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {

GalagaGame::GalagaGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_keyboardDirection(0.0)
    , m_fireCooldown(0.0)
    , m_enemyDirection(1.0)
    , m_enemyStepTimer(0.0)
    , m_lives(3)
    , m_wave(1)
    , m_fireHeld(false)
{
    setFocusPolicy(Qt::StrongFocus);
}

QString GalagaGame::gameName() const
{
    return QStringLiteral("Galaga Force Trainer");
}

void GalagaGame::initialize()
{
    m_force = 0.5;
    m_keyboardDirection = 0.0;
    m_fireCooldown = 0.0;
    m_enemyDirection = 1.0;
    m_enemyStepTimer = 0.0;
    m_lives = 3;
    m_wave = 1;
    m_fireHeld = false;
    m_projectiles.clear();
    m_player = QRectF(width() * 0.5 - 18.0, height() - 58.0, 36.0, 24.0);
    createWave();
}

void GalagaGame::createWave()
{
    m_enemies.clear();
    const int rows = qBound(2, 3 + m_wave / 2, 6);
    const int columns = 8;
    const double cellWidth = qMax(24.0, (width() - 48.0) / columns);
    const double enemyWidth = qMin(28.0, cellWidth - 6.0);
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Enemy enemy;
            enemy.rect = QRectF(24.0 + column * cellWidth + (cellWidth - enemyWidth) / 2.0,
                                48.0 + row * 34.0, enemyWidth, 22.0);
            enemy.hitPoints = (m_wave >= 3 && row == 0) ? 2 : 1;
            enemy.active = true;
            m_enemies.push_back(enemy);
        }
    }
    m_enemyDirection = 1.0;
    m_enemyStepTimer = 0.0;
}

bool GalagaGame::handleInput(double value)
{
    m_force = qBound(0.0, value / 100.0, 1.0);
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
    Projectile shot;
    shot.rect = QRectF(m_player.center().x() - 2.0, m_player.top() - 12.0, 4.0, 12.0);
    shot.velocityY = -460.0;
    shot.fromPlayer = true;
    m_projectiles.push_back(shot);
}

void GalagaGame::loseLife()
{
    --m_lives;
    if (m_lives <= 0) {
        stop();
        return;
    }
    m_player.moveCenter(QPointF(width() * 0.5, height() - 46.0));
}

void GalagaGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);

    const double sensorX = 18.0 + m_force * qMax(0.0, width() - 36.0 - m_player.width());
    const double keyboardX = m_player.x() + m_keyboardDirection * 360.0 * deltaTime;
    const double targetX = m_keyboardDirection == 0.0 ? sensorX : keyboardX;
    m_player.moveLeft(qBound(8.0, targetX, qMax(8.0, width() - 8.0 - m_player.width())));

    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);
    if (m_fireHeld && m_fireCooldown <= 0.0) {
        firePlayerShot();
        m_fireCooldown = 0.24;
    }

    const double enemySpeed = 24.0 + m_wave * 4.0;
    bool hitEdge = false;
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        const double nextX = enemy.rect.x() + m_enemyDirection * enemySpeed * deltaTime;
        if (nextX < 8.0 || nextX + enemy.rect.width() > width() - 8.0) {
            hitEdge = true;
            break;
        }
    }
    if (hitEdge) {
        m_enemyDirection = -m_enemyDirection;
        for (Enemy& enemy : m_enemies)
            if (enemy.active)
                enemy.rect.translate(0.0, 12.0);
    }
    for (Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        enemy.rect.translate(m_enemyDirection * enemySpeed * deltaTime, 0.0);
        if (enemy.rect.bottom() >= m_player.top()) {
            loseLife();
            if (!isRunning())
                return;
            createWave();
            return;
        }
    }

    for (Projectile& shot : m_projectiles)
        shot.rect.translate(0.0, shot.velocityY * deltaTime);

    for (Projectile& shot : m_projectiles) {
        if (!shot.fromPlayer || shot.rect.isEmpty())
            continue;
        for (Enemy& enemy : m_enemies) {
            if (!enemy.active || !shot.rect.intersects(enemy.rect))
                continue;
            shot.rect = QRectF();
            --enemy.hitPoints;
            if (enemy.hitPoints <= 0) {
                enemy.active = false;
                addScore(10);
            } else {
                addScore(3);
            }
            break;
        }
    }

    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [this](const Projectile& shot) {
            return shot.rect.isEmpty() || shot.rect.bottom() < 0.0 ||
                   shot.rect.top() > height();
        }), m_projectiles.end());

    const bool anyEnemy = std::any_of(m_enemies.cbegin(), m_enemies.cend(),
        [](const Enemy& enemy) { return enemy.active; });
    if (!anyEnemy) {
        ++m_wave;
        addScore(50);
        createWave();
    }
}

void GalagaGame::render(QPainter& painter)
{
    painter.fillRect(rect(), QColor(5, 8, 24));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(80, 210, 255));
    painter.drawRoundedRect(m_player, 6.0, 6.0);
    painter.setBrush(QColor(180, 240, 255));
    painter.drawRect(QRectF(m_player.center().x() - 3.0, m_player.top() - 8.0, 6.0, 9.0));

    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        painter.setBrush(enemy.hitPoints > 1 ? QColor(255, 190, 70) : QColor(235, 75, 105));
        painter.drawRoundedRect(enemy.rect, 5.0, 5.0);
    }
    painter.setBrush(QColor(255, 245, 150));
    for (const Projectile& shot : m_projectiles)
        painter.drawRoundedRect(shot.rect, 2.0, 2.0);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(14, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 46, QStringLiteral("Fala: %1").arg(m_wave));
    painter.drawText(width() - 110, 24, QStringLiteral("Życia: %1").arg(m_lives));
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Czujnik: pozycja statku | Strzał: przytrzymaj SPACJĘ"));
}

} // namespace games
