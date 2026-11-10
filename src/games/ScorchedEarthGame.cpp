#include "games/ScorchedEarthGame.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QLinearGradient>
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
    , m_gravity(220.0)
    , m_playerHealth(100)
    , m_enemyHealth(100)
    , m_round(1)
    , m_playerTurn(true)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
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
    if (m_terrain.isEmpty() && width() > 0 && height() > 0) {
        buildTerrain();
        updateTankPositions();
    }
}

void ScorchedEarthGame::buildTerrain()
{
    m_terrain.clear();
    const int step = 16;
    for (int x = 0; x <= width(); x += step) {
        const double y = height() * 0.70
            + std::sin(x * 0.009) * height() * 0.075
            + std::sin(x * 0.021 + 0.8) * height() * 0.025;
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
    const double playerX = qMin(72.0, width() * 0.18);
    const double enemyX = qMax(playerX + 80.0, width() - 108.0);
    m_playerTank = QRectF(playerX, terrainY(playerX + 18.0) - 20.0, 36.0, 20.0);
    m_enemyTank = QRectF(enemyX, terrainY(enemyX + 18.0) - 20.0, 36.0, 20.0);
}

bool ScorchedEarthGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    const double normalized = qBound(0.0, value / 100.0, 1.0);
    m_aimDegrees = 15.0 + normalized * 65.0;
    return true;
}

void ScorchedEarthGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W)
        m_aimDegrees = qMin(85.0, m_aimDegrees + 2.0);
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S)
        m_aimDegrees = qMax(10.0, m_aimDegrees - 2.0);
    else if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal)
        m_power = qMin(1.0, m_power + 0.03);
    else if (event->key() == Qt::Key_Minus)
        m_power = qMax(0.35, m_power - 0.03);
    else if (event->key() == Qt::Key_Space && !event->isAutoRepeat() && m_playerTurn)
        fireProjectile(true);
    else
        GameEngine::keyPressEvent(event);
}

void ScorchedEarthGame::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF origin(m_playerTank.right(), m_playerTank.top() + 2.0);
    const QPointF cursor = event->localPos();
    const double dx = cursor.x() - origin.x();
    const double dy = origin.y() - cursor.y();
    if (dx > 1.0)
        m_aimDegrees = qBound(10.0,
            std::atan2(dy, dx) * 180.0 / 3.14159265358979323846, 85.0);
    event->accept();
}

void ScorchedEarthGame::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setFocus(Qt::MouseFocusReason);
        if (m_playerTurn)
            fireProjectile(true);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void ScorchedEarthGame::fireProjectile(bool playerOwned)
{
    if (!isRunning() || !m_projectiles.isEmpty())
        return;
    const QRectF& tank = playerOwned ? m_playerTank : m_enemyTank;
    const double angle = (playerOwned ? m_aimDegrees : 42.0) * 3.14159265358979323846 / 180.0;
    const double speed = 420.0 * (playerOwned ? m_power : 0.78);
    const double direction = playerOwned ? 1.0 : -1.0;
    Projectile projectile;
    projectile.position = QPointF(playerOwned ? tank.right() : tank.left(),
                                  tank.top() + 2.0);
    projectile.velocity = QPointF(direction * std::cos(angle) * speed,
                                  -std::sin(angle) * speed);
    projectile.playerOwned = playerOwned;
    projectile.active = true;
    m_projectiles.push_back(projectile);
    if (playerOwned)
        m_playerTurn = false;
}

void ScorchedEarthGame::applyCrater(double x)
{
    constexpr double radius = 38.0;
    for (QPointF& point : m_terrain) {
        const double distance = std::abs(point.x() - x);
        if (distance >= radius)
            continue;
        const double depth = 22.0 * (1.0 - distance / radius);
        point.setY(qMin(static_cast<double>(height() - 12), point.y() + depth));
    }
    updateTankPositions();
}

void ScorchedEarthGame::resolveImpact(const QPointF& point, bool playerOwned)
{
    if (playerOwned && m_enemyTank.adjusted(-5, -5, 5, 5).contains(point)) {
        m_enemyHealth -= 25;
        addScore(25);
        if (m_enemyHealth <= 0) {
            addScore(100);
            ++m_round;
            m_enemyHealth = 100;
            m_playerHealth = qMin(100, m_playerHealth + 20);
            m_wind = std::sin(m_round * 0.7) * 34.0;
        }
    } else if (!playerOwned && m_playerTank.adjusted(-5, -5, 5, 5).contains(point)) {
        m_playerHealth -= 25;
        if (m_playerHealth <= 0) {
            stop();
            return;
        }
    }
    applyCrater(point.x());
    if (playerOwned) {
        m_enemyDelay = 0.75;
    } else {
        m_playerTurn = true;
    }
}

void ScorchedEarthGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);

    if (m_projectiles.isEmpty() && !m_playerTurn) {
        m_enemyDelay -= deltaTime;
        if (m_enemyDelay <= 0.0)
            fireProjectile(false);
    }

    constexpr double maxSegmentLength = 4.0;
    for (Projectile& projectile : m_projectiles) {
        if (!projectile.active)
            continue;

        // Integrate a bounded number of micro-steps and test the whole swept
        // segment so fast shells cannot tunnel through tanks or terrain.
        const QPointF oldPosition = projectile.position;
        const QPointF oldVelocity = projectile.velocity;
        projectile.velocity.ry() += m_gravity * deltaTime;
        projectile.velocity.rx() += m_wind * 0.08 * deltaTime;
        const QPointF displacement = projectile.velocity * deltaTime;
        const int samples = qBound(1, static_cast<int>(std::ceil(
            std::hypot(displacement.x(), displacement.y()) / maxSegmentLength)), 256);
        bool impacted = false;

        for (int sample = 1; sample <= samples; ++sample) {
            const double fraction = static_cast<double>(sample) / samples;
            const QPointF current = oldPosition + oldVelocity * (deltaTime * fraction) +
                QPointF(0.5 * m_wind * 0.08 * deltaTime * deltaTime * fraction * fraction,
                        0.5 * m_gravity * deltaTime * deltaTime * fraction * fraction);
            const bool outside = current.x() < 0.0 || current.x() > width() ||
                current.y() > height() || current.y() < -height();
            const bool hitTerrain = !outside &&
                current.y() >= terrainY(current.x());
            const bool hitTank = !outside &&
                ((projectile.playerOwned &&
                  m_enemyTank.adjusted(-5, -5, 5, 5).contains(current)) ||
                 (!projectile.playerOwned &&
                  m_playerTank.adjusted(-5, -5, 5, 5).contains(current)));

            if (outside) {
                projectile.active = false;
                if (projectile.playerOwned)
                    m_enemyDelay = 0.75;
                else
                    m_playerTurn = true;
                impacted = true;
                break;
            }
            if (hitTerrain || hitTank) {
                projectile.position = current;
                projectile.active = false;
                resolveImpact(current, projectile.playerOwned);
                if (!isRunning())
                    return;
                impacted = true;
                break;
            }
        }

        if (!impacted)
            projectile.position = oldPosition + displacement;
    }

    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [](const Projectile& projectile) { return !projectile.active; }), m_projectiles.end());
}

void ScorchedEarthGame::render(QPainter& painter)
{
    QLinearGradient sky(0.0, 0.0, 0.0, height());
    sky.setColorAt(0.0, QColor(20, 35, 72));
    sky.setColorAt(1.0, QColor(220, 130, 80));
    painter.fillRect(rect(), sky);
    painter.setRenderHint(QPainter::Antialiasing);

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
    painter.drawLine(m_playerTank.center(),
        m_playerTank.center() + QPointF(std::cos(m_aimDegrees * 3.14159265358979323846 / 180.0) * 28.0,
                                        -std::sin(m_aimDegrees * 3.14159265358979323846 / 180.0) * 28.0));

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    for (const Projectile& projectile : m_projectiles)
        painter.drawEllipse(projectile.position, 4.0, 4.0);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 13, QFont::Bold));
    painter.drawText(14, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 46, QStringLiteral("Twoje HP: %1").arg(m_playerHealth));
    painter.drawText(width() - 150, 24, QStringLiteral("Wróg HP: %1").arg(m_enemyHealth));
    painter.drawText(width() - 150, 46, QStringLiteral("Runda: %1").arg(m_round));
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Czujnik/kursor: kąt | +/-: siła | SPACJA: strzał | Wiatr: %1")
            .arg(qRound(m_wind)));
}

} // namespace games
