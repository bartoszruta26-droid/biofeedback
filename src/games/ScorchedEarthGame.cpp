#include "games/ScorchedEarthGame.hpp"
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {

ScorchedEarthGame::ScorchedEarthGame(QWidget *parent)
    : GameEngine(parent)
    , m_aimDegrees(45.0)
    , m_power(0.82)
    , m_wind(0.0)
    , m_enemyDelay(0.0)
    , m_playerShotDelay(2.0)
    , m_gravity(220.0)
    , m_playerHealth(100)
    , m_enemyHealth(100)
    , m_round(1)
    , m_playerTurn(true)
{
    setFocusPolicy(Qt::NoFocus);
}

QString ScorchedEarthGame::gameName() const
{
    return QStringLiteral("Scorched Earth Force Trainer");
}

void ScorchedEarthGame::initialize()
{
    m_projectiles.clear();
    m_aimDegrees = 45.0;
    m_power = 0.82;
    m_wind = 0.0;
    m_enemyDelay = 0.0;
    m_playerShotDelay = m_playerShotInterval;
    m_sensorClock.invalidate();
    m_hasSensorSample = false;
    const double scaleY = qMax(0.25, height() / 480.0);
    m_gravity = 220.0 * scaleY;
    m_playerHealth = 100;
    m_enemyHealth = 100;
    m_round = 1;
    m_playerTurn = true;
    buildTerrain();
    updateTankPositions();
}

void ScorchedEarthGame::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        for (QPointF& point : m_terrain)
            point = QPointF(point.x() * scaleX, point.y() * scaleY);
        auto scaleRect = [scaleX, scaleY](QRectF& rect) {
            rect = QRectF(rect.x() * scaleX, rect.y() * scaleY,
                          rect.width() * scaleX, rect.height() * scaleY);
        };
        scaleRect(m_playerTank);
        scaleRect(m_enemyTank);
        for (Projectile& projectile : m_projectiles) {
            projectile.position = QPointF(projectile.position.x() * scaleX,
                                           projectile.position.y() * scaleY);
            projectile.velocity = QPointF(projectile.velocity.x() * scaleX,
                                           projectile.velocity.y() * scaleY);
        }
    }
    GameEngine::resizeEvent(event);
    if (width() > 0 && height() > 0)
        m_gravity = 220.0 * qMax(0.25, height() / 480.0);
    if (m_terrain.isEmpty() && width() > 0 && height() > 0) {
        buildTerrain();
        updateTankPositions();
    }
}

void ScorchedEarthGame::buildTerrain()
{
    m_terrain.clear();
    const double scaleX = qMax(0.25, width() / 640.0);
    const int step = qMax(2, qRound(16.0 * scaleX));
    for (int x = 0; x <= width(); x += step) {
        const double logicalX = x / scaleX;
        const double y = height() * 0.70
            + std::sin(logicalX * 0.009) * height() * 0.075
            + std::sin(logicalX * 0.021 + 0.8) * height() * 0.025;
        m_terrain << QPointF(x, qBound(height() * 0.52, y, height() * 0.84));
    }
    if (m_terrain.isEmpty() || m_terrain.last().x() < width())
        m_terrain << QPointF(width(), height() * 0.70);
}

double ScorchedEarthGame::terrainY(double x) const
{
    if (m_terrain.isEmpty())
        return height() * 0.7;
    x = qBound(0.0, x, static_cast<double>(width()));
    for (int i = 1; i < m_terrain.size(); ++i) {
        const QPointF& a = m_terrain[i - 1];
        const QPointF& b = m_terrain[i];
        if (x <= b.x()) {
            const double span = qMax(1.0, b.x() - a.x());
            const double t = (x - a.x()) / span;
            return a.y() + (b.y() - a.y()) * t;
        }
    }
    return m_terrain.last().y();
}

void ScorchedEarthGame::updateTankPositions()
{
    const double scaleX = qMax(0.25, width() / 640.0);
    const double scaleY = qMax(0.25, height() / 480.0);
    const double tankWidth = 36.0 * scaleX;
    const double tankHeight = 20.0 * scaleY;
    const double playerX = qMin(72.0 * scaleX, width() * 0.18);
    const double enemyX = qMax(playerX + 80.0 * scaleX, width() - 108.0 * scaleX);
    m_playerTank = QRectF(playerX, terrainY(playerX + tankWidth * 0.5) - tankHeight,
                          tankWidth, tankHeight);
    m_enemyTank = QRectF(enemyX, terrainY(enemyX + tankWidth * 0.5) - tankHeight,
                         tankWidth, tankHeight);
}

bool ScorchedEarthGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    const double normalized = qBound(0.0, value / 100.0, 1.0);
    m_sensorClock.restart();
    m_hasSensorSample = true;
    m_aimDegrees = 15.0 + normalized * 65.0;
    return true;
}

void ScorchedEarthGame::fireProjectile(bool playerOwned)
{
    if (!isRunning() || !m_projectiles.isEmpty())
        return;
    const QRectF& tank = playerOwned ? m_playerTank : m_enemyTank;
    const double angle = (playerOwned ? m_aimDegrees : 42.0) * 3.14159265358979323846 / 180.0;
    const double scaleX = qMax(0.25, width() / 640.0);
    const double scaleY = qMax(0.25, height() / 480.0);
    const double speed = 420.0 * (playerOwned ? m_power : 0.78);
    const double direction = playerOwned ? 1.0 : -1.0;
    Projectile projectile;
    projectile.position = QPointF(playerOwned ? tank.right() : tank.left(),
                                  tank.top() + 2.0);
    projectile.velocity = QPointF(direction * std::cos(angle) * speed * scaleX,
                                  -std::sin(angle) * speed * scaleY);
    projectile.playerOwned = playerOwned;
    projectile.active = true;
    m_projectiles.push_back(projectile);
    if (playerOwned)
        m_playerTurn = false;
}

void ScorchedEarthGame::render(QPainter& painter)
{
    painter.fillRect(rect(), QColor(55, 88, 125));
    painter.fillRect(QRect(0, height() / 2, width(), height() / 2),
                     QColor(202, 133, 83));
    painter.setRenderHint(QPainter::Antialiasing, false);

    QPolygonF ground = m_terrain;
    ground << QPointF(width(), height()) << QPointF(0.0, height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(110, 82, 55));
    painter.drawPolygon(ground);

    painter.setBrush(QColor(65, 185, 95));
    painter.drawRoundedRect(m_playerTank, 5.0, 5.0);
    painter.setBrush(QColor(205, 90, 70));
    painter.drawRoundedRect(m_enemyTank, 5.0, 5.0);

    painter.setPen(QPen(QColor(245, 235, 190), 4));
    const double scaleX = qMax(0.25, width() / 640.0);
    const double scaleY = qMax(0.25, height() / 480.0);
    painter.drawLine(m_playerTank.center(),
        m_playerTank.center() + QPointF(std::cos(m_aimDegrees * 3.14159265358979323846 / 180.0) * 28.0 * scaleX,
                                        -std::sin(m_aimDegrees * 3.14159265358979323846 / 180.0) * 28.0 * scaleY));

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    for (const Projectile& projectile : m_projectiles)
        painter.drawEllipse(projectile.position, 4.0, 4.0);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(9, qRound(13.0 * scaleY)), QFont::Bold));
    painter.drawText(14, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 46, QStringLiteral("Twoje HP: %1").arg(m_playerHealth));
    painter.drawText(width() - 150, 24, QStringLiteral("Wróg HP: %1").arg(m_enemyHealth));
    painter.drawText(width() - 150, 46, QStringLiteral("Runda: %1").arg(m_round));
    painter.drawText(width() / 2 - 65, 24, QStringLiteral("Moc: %1%").arg(qRound(m_power * 100.0)));
    painter.drawText(width() / 2 - 65, 46, m_playerTurn ? QStringLiteral("TWOJA TURA") : QStringLiteral("RUCH PRZECIWNIKA"));
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Tensometr: kąt | Strzał automatyczny za: %1 s | Wiatr: %2")
            .arg(qMax(0.0, m_playerShotDelay), 0, 'f', 1).arg(qRound(m_wind)));
}

} // namespace games
