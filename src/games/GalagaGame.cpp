#include "games/GalagaGame.hpp"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPolygonF>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>
#include <utility>

namespace games {

GalagaGame::GalagaGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.5)
    , m_enemyDirection(1.0)
    , m_enemySpeed(65.0)
    , m_playerFireInterval(0.34)
    , m_playerFireTimer(0.0)
    , m_enemyFireInterval(1.8)
    , m_enemyFireTimer(0.0)
    , m_enemyProjectileSpeed(150.0)
    , m_difficultyLevel(1)
    , m_level(1)
    , m_lives(3)
{
}

QString GalagaGame::gameName() const
{
    return QStringLiteral("Galaga Force Trainer");
}

void GalagaGame::initialize()
{
    m_level = 1;
    m_lives = 3;
    m_currentForce = 0.5;
    m_playerFireTimer = 0.0;
    m_enemyFireTimer = 0.0;
    m_enemyDirection = 1.0;

    setDifficultyLevel(m_difficultyLevel);
    m_ship = QRectF((width() - 38.0) / 2.0, height() - 52.0, 38.0, 26.0);
    spawnWave();
}

void GalagaGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    m_enemySpeed = 45.0 + m_difficultyLevel * 16.0;
    m_playerFireInterval = 0.42 - m_difficultyLevel * 0.035;
    m_enemyFireInterval = 2.2 - m_difficultyLevel * 0.22;
    m_enemyProjectileSpeed = 115.0 + m_difficultyLevel * 22.0;
    setScore(0);

    if (isRunning()) {
        m_level = 1;
        m_lives = 3;
        m_playerFireTimer = 0.0;
        m_enemyFireTimer = 0.0;
        spawnWave();
    }
}

int GalagaGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
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

bool GalagaGame::handleInput(double value)
{
    m_currentForce = qBound(MIN_FORCE, value / MAX_FORCE, 1.0);
    return true;
}

void GalagaGame::spawnWave()
{
    m_enemies.clear();
    m_projectiles.clear();
    m_enemyDirection = 1.0;

    if (width() <= 0 || height() <= 180)
        return;

    constexpr int columns = 8;
    const int rows = qBound(3, 3 + (m_level - 1) / 2, 5);
    const double enemyWidth = 28.0;
    const double enemyHeight = 20.0;
    const double spacingX = 48.0;
    const double spacingY = 36.0;
    const double formationWidth = (columns - 1) * spacingX + enemyWidth;
    const double startX = qMax(12.0, (width() - formationWidth) / 2.0);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Enemy enemy;
            enemy.rect = QRectF(startX + column * spacingX,
                                62.0 + row * spacingY,
                                enemyWidth, enemyHeight);
            enemy.row = row;
            enemy.active = true;
            m_enemies.push_back(enemy);
        }
    }
}

bool GalagaGame::hasActiveEnemies() const
{
    return std::any_of(m_enemies.cbegin(), m_enemies.cend(),
                       [](const Enemy& enemy) { return enemy.active; });
}

void GalagaGame::spawnEnemyProjectile()
{
    int activeCount = 0;
    for (const Enemy& enemy : m_enemies)
        if (enemy.active) ++activeCount;

    if (activeCount == 0)
        return;

    int selected = QRandomGenerator::global()->bounded(activeCount);
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        if (selected-- == 0) {
            m_projectiles.push_back({
                QPointF(enemy.rect.center().x(), enemy.rect.bottom()),
                QPointF(0.0, m_enemyProjectileSpeed),
                true
            });
            return;
        }
    }
}

void GalagaGame::updateFormation(double deltaTime)
{
    if (m_enemies.empty())
        return;

    double left = width();
    double right = 0.0;
    double bottom = 0.0;
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        left = qMin(left, enemy.rect.left());
        right = qMax(right, enemy.rect.right());
        bottom = qMax(bottom, enemy.rect.bottom());
    }

    const double movement = m_enemyDirection * m_enemySpeed * deltaTime;
    if ((right + movement >= width() - 12.0 && m_enemyDirection > 0.0) ||
        (left + movement <= 12.0 && m_enemyDirection < 0.0)) {
        m_enemyDirection = -m_enemyDirection;
        for (Enemy& enemy : m_enemies)
            if (enemy.active) enemy.rect.translate(0.0, 12.0);
    } else {
        for (Enemy& enemy : m_enemies)
            if (enemy.active) enemy.rect.translate(movement, 0.0);
    }

    double updatedBottom = 0.0;
    for (const Enemy& enemy : m_enemies)
        if (enemy.active) updatedBottom = qMax(updatedBottom, enemy.rect.bottom());

    if (updatedBottom >= m_ship.top() - 12.0)
        stop();
}

void GalagaGame::loseLife()
{
    --m_lives;
    if (m_lives <= 0) {
        stop();
        return;
    }

    m_ship.moveLeft((width() - m_ship.width()) / 2.0);
    m_playerFireTimer = 0.0;
}

void GalagaGame::updateProjectiles(double deltaTime)
{
    bool lifeLostThisFrame = false;
    for (auto it = m_projectiles.begin(); it != m_projectiles.end();) {
        it->position += it->velocity * deltaTime;
        const QRectF projectileBounds(it->position.x() - 2.5,
                                      it->position.y() - 7.0,
                                      5.0, 14.0);
        bool remove = projectileBounds.bottom() < 0.0 ||
                      projectileBounds.top() > height();

        if (it->fromEnemy && !remove && !lifeLostThisFrame &&
            projectileBounds.intersects(m_ship)) {
            remove = true;
            lifeLostThisFrame = true;
            loseLife();
            if (!isRunning())
                return;
        } else if (!it->fromEnemy && !remove) {
            for (Enemy& enemy : m_enemies) {
                if (enemy.active && projectileBounds.intersects(enemy.rect)) {
                    enemy.active = false;
                    addScore(10 + (4 - enemy.row) * 5);
                    remove = true;
                    break;
                }
            }
        }

        if (remove)
            it = m_projectiles.erase(it);
        else
            ++it;
    }

    if (!isRunning())
        return;

    if (lifeLostThisFrame) {
        m_projectiles.erase(
            std::remove_if(m_projectiles.begin(), m_projectiles.end(),
                           [](const Projectile& p) { return p.fromEnemy; }),
            m_projectiles.end());
    }

    if (!hasActiveEnemies())
        advanceLevel();
}

void GalagaGame::advanceLevel()
{
    ++m_level;
    m_enemySpeed = qMin(m_enemySpeed * 1.08, 240.0);
    m_enemyProjectileSpeed = qMin(m_enemyProjectileSpeed * 1.06, 420.0);
    addScore(100);
    spawnWave();
}

void GalagaGame::update(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0)
        return;

    const double maxShipX = qMax(0.0, static_cast<double>(width()) - m_ship.width());
    m_ship.moveLeft(qBound(0.0,
        m_currentForce * maxShipX, maxShipX));

    m_playerFireTimer += deltaTime;
    while (m_playerFireTimer >= m_playerFireInterval) {
        m_playerFireTimer -= m_playerFireInterval;
        m_projectiles.push_back({
            QPointF(m_ship.center().x(), m_ship.top()),
            QPointF(0.0, -390.0),
            false
        });
    }

    updateFormation(deltaTime);
    if (!isRunning())
        return;

    m_enemyFireTimer += deltaTime;
    if (m_enemyFireTimer >= m_enemyFireInterval) {
        m_enemyFireTimer = 0.0;
        spawnEnemyProjectile();
    }

    updateProjectiles(deltaTime);
}

void GalagaGame::render(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(8, 10, 28));

    // Gwiazdy tła
    painter.setPen(QPen(QColor(95, 105, 145), 1));
    for (int i = 0; i < 36; ++i) {
        const int x = (i * 97 + 31) % qMax(1, width());
        const int y = (i * 53 + 19) % qMax(1, height());
        painter.drawPoint(x, y);
    }

    // Formacja przeciwników
    for (const Enemy& enemy : m_enemies) {
        if (!enemy.active)
            continue;
        const QColor color = enemy.row == 0 ? QColor(245, 95, 115)
            : (enemy.row == 1 ? QColor(245, 175, 80) : QColor(105, 190, 235));
        painter.setBrush(color);
        painter.setPen(QPen(color.darker(160), 1));
        painter.drawRoundedRect(enemy.rect, 5.0, 5.0);
        painter.setBrush(QColor(25, 25, 45));
        painter.drawEllipse(QPointF(enemy.rect.center().x() - 6, enemy.rect.center().y()), 2.0, 2.0);
        painter.drawEllipse(QPointF(enemy.rect.center().x() + 6, enemy.rect.center().y()), 2.0, 2.0);
    }

    // Pociski gracza i przeciwników
    for (const Projectile& projectile : m_projectiles) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(projectile.fromEnemy ? QColor(255, 95, 95) : QColor(110, 245, 255));
        painter.drawRoundedRect(QRectF(projectile.position.x() - 2.5,
                                       projectile.position.y() - 7.0,
                                       5.0, 14.0), 2.0, 2.0);
    }

    // Statek gracza
    QPolygonF shipShape;
    shipShape << QPointF(m_ship.center().x(), m_ship.top())
              << QPointF(m_ship.right(), m_ship.bottom())
              << QPointF(m_ship.center().x(), m_ship.bottom() - 6.0)
              << QPointF(m_ship.left(), m_ship.bottom());
    painter.setBrush(QColor(100, 225, 235));
    painter.setPen(QPen(QColor(30, 100, 150), 2));
    painter.drawPolygon(shipShape);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(14, 26, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 48, QStringLiteral("Poziom: %1").arg(m_level));
    painter.drawText(width() - 120, 26, QStringLiteral("Życia: %1").arg(m_lives));

    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
                     QStringLiteral("Steruj statkiem siłą nacisku — strzał automatyczny"));
    painter.drawText(width() - 145, height() - 12,
                     QStringLiteral("Trudność: %1").arg(m_difficultyLevel));

    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(Qt::red);
        painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter,
                         QStringLiteral("BRAK POŁĄCZENIA Z CZUJNIKIEM"));
    }
}

} // namespace games
