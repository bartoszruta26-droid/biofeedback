#include "games/GalagaGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <utility>
#include <cmath>

namespace games {

GalagaGame::GalagaGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_keyboardDirection(0.0)
    , m_fireCooldown(0.0)
    , m_enemyDirection(1.0)
    , m_enemyFireCooldown(1.0)
    , m_hitCooldown(0.0)
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

void GalagaGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    m_serialPort = std::move(serial);
}

void GalagaGame::disconnectSerial()
{
    m_serialPort.reset();
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

void GalagaGame::initialize()
{
    m_force = 0.5;
    m_keyboardDirection = 0.0;
    m_fireCooldown = 0.0;
    m_enemyDirection = 1.0;
    m_enemyFireCooldown = 1.0;
    m_hitCooldown = 0.0;
    m_lives = 3;
    m_wave = 1;
    m_fireHeld = false;
    m_projectiles.clear();
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
            enemy.rect.moveLeft(qBound(8.0, enemy.rect.left(),
                qMax(8.0, static_cast<double>(newSize.width()) - 8.0 - enemy.rect.width())));
        }
        for (Projectile& projectile : m_projectiles)
            scaleRect(projectile.rect);
        m_player.moveLeft(qBound(8.0, m_player.left(),
            qMax(8.0, static_cast<double>(newSize.width()) - 8.0 - m_player.width())));
        m_player.moveTop(qBound(8.0, m_player.top(),
            qMax(8.0, static_cast<double>(newSize.height()) - 8.0 - m_player.height())));
    }
    GameEngine::resizeEvent(event);
}

void GalagaGame::createWave()
{
    m_enemies.clear();
    // Dopasuj formację do małych widoków: stałe osiem kolumn i trzy
    // wiersze powodowały nakładanie się przeciwników po zmianie rozmiaru.
    const int desiredRows = qBound(2, 3 + m_wave / 2, 6);
    const int maxRows = qMax(1, (height() - 60) / 34);
    const int rows = qMin(desiredRows, maxRows);
    const int columns = qBound(1, width() / 40, 8);
    const double margin = qMin(24.0, qMax(4.0, width() * 0.05));
    const double cellWidth = qMax(1.0, (width() - 2.0 * margin) / columns);
    const double enemyWidth = qMax(6.0, qMin(28.0, cellWidth - 6.0));
    const double enemyHeight = qMin(22.0, qMax(8.0, (height() - 60.0) / rows - 8.0));
    const double rowSpacing = qMax(enemyHeight + 2.0,
                                   qMin(34.0, (height() - 60.0) / rows));
    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Enemy enemy;
            enemy.rect = QRectF(margin + column * cellWidth + (cellWidth - enemyWidth) / 2.0,
                                48.0 + row * rowSpacing, enemyWidth, enemyHeight);
            enemy.hitPoints = (m_wave >= 3 && row == 0) ? 2 : 1;
            enemy.active = true;
            m_enemies.push_back(enemy);
        }
    }
    m_enemyDirection = 1.0;
    m_enemyFireCooldown = 1.0;
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

bool GalagaGame::loseLife()
{
    if (m_hitCooldown > 0.0)
        return false;

    --m_lives;
    if (m_lives <= 0) {
        stop();
        return true;
    }

    // Brief post-hit protection prevents overlapping hostile shots or a
    // descending formation from consuming several lives in adjacent frames.
    m_hitCooldown = 1.2;
    m_player.moveCenter(QPointF(width() * 0.5, height() - 46.0));
    // Do not erase from the vector here: loseLife() can be called while
    // update() is iterating it. Mark hostile shots for deferred removal.
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
    m_hitCooldown = qMax(0.0, m_hitCooldown - deltaTime);

    const double sensorX = 18.0 + m_force * qMax(0.0, width() - 36.0 - m_player.width());
    const double keyboardX = m_player.x() + m_keyboardDirection * 360.0 * deltaTime;
    const double targetX = m_keyboardDirection == 0.0 ? sensorX : keyboardX;
    m_player.moveLeft(qBound(8.0, targetX, qMax(8.0, width() - 8.0 - m_player.width())));

    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);
    if (m_fireHeld && m_fireCooldown <= 0.0) {
        firePlayerShot();
        m_fireCooldown = 0.24;
    }

    m_enemyFireCooldown -= deltaTime;
    if (m_enemyFireCooldown <= 0.0 && !m_enemies.isEmpty()) {
        // Deterministic shooter selection keeps replay and regression runs stable.
        const int startIndex = (m_wave * 7 + getScore()) % m_enemies.size();
        for (int offset = 0; offset < m_enemies.size(); ++offset) {
            const Enemy& shooter = m_enemies[(startIndex + offset) % m_enemies.size()];
            if (!shooter.active)
                continue;
            Projectile shot;
            shot.rect = QRectF(shooter.rect.center().x() - 2.0,
                               shooter.rect.bottom(), 4.0, 10.0);
            shot.velocityY = 250.0 + m_wave * 8.0;
            shot.fromPlayer = false;
            m_projectiles.push_back(shot);
            break;
        }
        m_enemyFireCooldown = qMax(0.45, 1.35 - m_wave * 0.06);
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
            const bool lostLife = loseLife();
            if (!isRunning())
                return;
            if (lostLife)
                createWave();
            return;
        }
    }

    for (Projectile& shot : m_projectiles) {
        if (shot.rect.isEmpty())
            continue;

        // Test the complete swept rectangle, not only the end position. A
        // fast shot can travel farther than an enemy's height in one frame.
        const QRectF previousRect = shot.rect;
        shot.rect.translate(0.0, shot.velocityY * deltaTime);
        const QRectF sweptRect = previousRect.united(shot.rect);

        if (shot.fromPlayer) {
            for (Enemy& enemy : m_enemies) {
                if (!enemy.active || !sweptRect.intersects(enemy.rect))
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
        } else if (sweptRect.intersects(m_player)) {
            shot.rect = QRectF();
            loseLife();
            if (!isRunning())
                return;
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
