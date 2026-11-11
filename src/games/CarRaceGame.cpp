#include "games/CarRaceGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {
namespace {
constexpr double kRoadWidthRatio = 0.72;
constexpr double kBoostDuration = 1.8;
constexpr double kBoostCooldown = 5.0;
constexpr double kCollisionProtection = 1.1;
}

CarRaceGame::CarRaceGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_spawnTimer(0.0)
    , m_spawnInterval(1.25)
    , m_roadSpeed(220.0)
    , m_distance(0.0)
    , m_boostTimer(0.0)
    , m_boostCooldown(0.0)
    , m_collisionTimer(0.0)
    , m_scoreDistanceAccumulator(0.0)
    , m_lane(1)
    , m_lives(3)
    , m_spawnCount(0)
{
    setFocusPolicy(Qt::StrongFocus);
}

QString CarRaceGame::gameName() const
{
    return QStringLiteral("Car Race Force Trainer");
}

void CarRaceGame::initialize()
{
    m_obstacles.clear();
    m_force = 0.5;
    m_spawnTimer = 0.0;
    m_spawnInterval = 1.25;
    m_roadSpeed = 220.0;
    m_distance = 0.0;
    m_boostTimer = 0.0;
    m_boostCooldown = 0.0;
    m_collisionTimer = 0.0;
    m_scoreDistanceAccumulator = 0.0;
    m_lane = 1;
    m_lives = 3;
    m_spawnCount = 0;
    setScore(0);
    m_player = QRectF(0.0, height() - 105.0, 38.0, 66.0);
    setLane(m_lane);
}

void CarRaceGame::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        for (Obstacle& obstacle : m_obstacles) {
            obstacle.rect = QRectF(obstacle.rect.x() * scaleX,
                                   obstacle.rect.y() * scaleY,
                                   obstacle.rect.width() * scaleX,
                                   obstacle.rect.height() * scaleY);
        }
    }
    GameEngine::resizeEvent(event);
    if (width() > 0 && height() > 0)
        setLane(m_lane);
}

QRectF CarRaceGame::laneRect(int lane, double top, double objectWidth, double objectHeight) const
{
    const double roadWidth = qMax(0.0, width() * kRoadWidthRatio);
    const double roadLeft = (width() - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;
    return QRectF(roadLeft + laneWidth * lane + (laneWidth - objectWidth) / 2.0,
                  top, objectWidth, objectHeight);
}

void CarRaceGame::setLane(int lane)
{
    m_lane = qBound(0, lane, 2);
    const double roadWidth = qMax(0.0, width() * kRoadWidthRatio);
    const double laneWidth = roadWidth / 3.0;
    const double playerWidth = qMin(42.0, qMax(1.0, laneWidth - 10.0));
    const double playerHeight = qMin(68.0, qMax(1.0, height() - 8.0));
    m_player.setSize(QSizeF(playerWidth, playerHeight));
    m_player.moveTop(qBound(0.0, static_cast<double>(height()) - playerHeight - 24.0,
                            qMax(0.0, static_cast<double>(height()) - playerHeight)));
    m_player.moveLeft(laneRect(m_lane, m_player.top(), playerWidth, playerHeight).left());
}

bool CarRaceGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    const double target = qBound(0.0, value / 100.0, 1.0);
    // Low-pass filtering removes sensor jitter without making steering sluggish.
    m_force += (target - m_force) * 0.65;

    // Hysteresis keeps the car in its lane when the signal fluctuates near a boundary.
    if (m_lane == 0 && m_force > 0.38)
        setLane(1);
    else if (m_lane == 1) {
        if (m_force < 0.30)
            setLane(0);
        else if (m_force > 0.70)
            setLane(2);
    } else if (m_lane == 2 && m_force < 0.62) {
        setLane(1);
    }
    return true;
}

void CarRaceGame::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        setLane(m_lane - 1);
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        setLane(m_lane + 1);
        break;
    case Qt::Key_Space:
    case Qt::Key_B:
        activateBoost();
        break;
    default:
        GameEngine::keyPressEvent(event);
        return;
    }
    event->accept();
}

void CarRaceGame::activateBoost()
{
    if (m_boostCooldown <= 0.0 && isRunning() && !isPaused()) {
        m_boostTimer = kBoostDuration;
        m_boostCooldown = kBoostCooldown;
    }
}

void CarRaceGame::spawnObstacle()
{
    if (width() <= 0 || height() <= 0)
        return;

    // Deterministic three-lane cycle is reproducible in regression tests and
    // guarantees that successive traffic does not occupy the same lane.
    const int lane = (m_spawnCount * 2 + 1) % 3;
    ++m_spawnCount;

    const double roadWidth = width() * kRoadWidthRatio;
    const double laneWidth = roadWidth / 3.0;
    const double obstacleWidth = qMin(44.0, qMax(1.0, laneWidth - 10.0));
    const double obstacleHeight = qMin(70.0, qMax(1.0, height() * 0.21));
    Obstacle obstacle;
    obstacle.lane = lane;
    obstacle.passed = false;
    obstacle.rect = laneRect(lane, -obstacleHeight - 6.0, obstacleWidth, obstacleHeight);
    m_obstacles.push_back(obstacle);
}

void CarRaceGame::loseLife()
{
    if (m_collisionTimer > 0.0 || m_lives <= 0)
        return;

    --m_lives;
    m_collisionTimer = kCollisionProtection;
    if (m_lives <= 0)
        stop();
}

void CarRaceGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);

    m_boostTimer = qMax(0.0, m_boostTimer - deltaTime);
    m_boostCooldown = qMax(0.0, m_boostCooldown - deltaTime);
    m_collisionTimer = qMax(0.0, m_collisionTimer - deltaTime);

    const double baseSpeed = qMin(560.0, 220.0 + m_distance * 0.014);
    m_roadSpeed = baseSpeed * (m_boostTimer > 0.0 ? 1.55 : 1.0);
    m_distance += m_roadSpeed * deltaTime;
    m_spawnInterval = qMax(0.58, 1.25 - m_distance / 14000.0);
    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer -= m_spawnInterval;
        spawnObstacle();
    }

    m_scoreDistanceAccumulator += m_roadSpeed * deltaTime;
    while (m_scoreDistanceAccumulator >= 250.0) {
        m_scoreDistanceAccumulator -= 250.0;
        addScore(1);
    }

    for (Obstacle& obstacle : m_obstacles) {
        obstacle.rect.translate(0.0, m_roadSpeed * deltaTime);
        if (!obstacle.passed && obstacle.rect.top() > m_player.bottom()) {
            obstacle.passed = true;
            addScore(5);
        }
        if (m_collisionTimer <= 0.0 && obstacle.rect.intersects(m_player)) {
            obstacle.rect = QRectF();
            loseLife();
            if (!isRunning())
                return;
        }
    }

    m_obstacles.erase(std::remove_if(m_obstacles.begin(), m_obstacles.end(),
        [this](const Obstacle& obstacle) {
            return obstacle.rect.isEmpty() || obstacle.rect.top() > height() + 80.0;
        }), m_obstacles.end());
}

void CarRaceGame::render(QPainter& painter)
{
    painter.fillRect(rect(), QColor(30, 112, 59));
    painter.setRenderHint(QPainter::Antialiasing);

    const double roadWidth = width() * kRoadWidthRatio;
    const double roadLeft = (width() - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;
    painter.fillRect(QRectF(roadLeft, 0.0, roadWidth, height()), QColor(39, 43, 51));

    // Road shoulders and animated lane markings.
    painter.fillRect(QRectF(roadLeft - 5.0, 0.0, 5.0, height()), QColor(224, 65, 55));
    painter.fillRect(QRectF(roadLeft + roadWidth, 0.0, 5.0, height()), QColor(224, 65, 55));
    painter.setPen(QPen(QColor(225, 229, 218), 2, Qt::DashLine));
    const double offset = std::fmod(m_distance * 0.8, 54.0);
    for (int divider = 1; divider <= 2; ++divider) {
        const double x = roadLeft + laneWidth * divider;
        for (double y = -54.0 + offset; y < height(); y += 54.0)
            painter.drawLine(QPointF(x, y), QPointF(x, y + 27.0));
    }

    // Traffic cars: dark windows, bumpers and lights make obstacles readable.
    for (const Obstacle& obstacle : m_obstacles) {
        if (obstacle.rect.isEmpty())
            continue;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(205, 70, 62));
        painter.drawRoundedRect(obstacle.rect, 7.0, 7.0);
        const QRectF window(obstacle.rect.left() + obstacle.rect.width() * 0.18,
                            obstacle.rect.top() + obstacle.rect.height() * 0.16,
                            obstacle.rect.width() * 0.64,
                            obstacle.rect.height() * 0.28);
        painter.setBrush(QColor(65, 87, 104));
        painter.drawRoundedRect(window, 3.0, 3.0);
        painter.setBrush(QColor(255, 220, 120));
        painter.drawRect(QRectF(obstacle.rect.left() + 4.0, obstacle.rect.bottom() - 7.0,
                                qMax(1.0, obstacle.rect.width() * 0.2), 3.0));
        painter.drawRect(QRectF(obstacle.rect.right() - obstacle.rect.width() * 0.2 - 4.0,
                                obstacle.rect.bottom() - 7.0,
                                qMax(1.0, obstacle.rect.width() * 0.2), 3.0));
    }

    // Player vehicle with a clear cockpit and wheel details.
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_collisionTimer > 0.0 && (static_cast<int>(m_collisionTimer * 12.0) % 2 == 0)
                         ? QColor(255, 255, 255, 130) : QColor(55, 157, 245));
    painter.drawRoundedRect(m_player, 9.0, 9.0);
    painter.setBrush(QColor(170, 224, 250));
    painter.drawRoundedRect(QRectF(m_player.left() + m_player.width() * 0.17,
                                   m_player.top() + m_player.height() * 0.16,
                                   m_player.width() * 0.66, m_player.height() * 0.27), 4.0, 4.0);
    painter.setBrush(QColor(22, 27, 33));
    const double wheelWidth = qMax(2.0, m_player.width() * 0.12);
    painter.drawRoundedRect(QRectF(m_player.left() - 2.0, m_player.top() + 12.0,
                                   wheelWidth, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(m_player.right() - wheelWidth + 2.0, m_player.top() + 12.0,
                                   wheelWidth, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(m_player.left() - 2.0, m_player.bottom() - 27.0,
                                   wheelWidth, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(m_player.right() - wheelWidth + 2.0, m_player.bottom() - 27.0,
                                   wheelWidth, 15.0), 2.0, 2.0);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
    painter.drawText(12, 23, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(12, 43, QStringLiteral("Dystans: %1 m").arg(qRound(m_distance / 10.0)));
    painter.drawText(width() - 112, 23, QStringLiteral("Życia: %1").arg(m_lives));
    painter.drawText(width() - 112, 43, QStringLiteral("Prędkość: %1").arg(qRound(m_roadSpeed / 4.0)));

    const QRectF boostTrack(12.0, 55.0, 112.0, 9.0);
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1));
    painter.setBrush(QColor(20, 25, 30, 170));
    painter.drawRoundedRect(boostTrack, 4.0, 4.0);
    const double boostReady = m_boostCooldown <= 0.0 ? 1.0 :
        1.0 - qBound(0.0, m_boostCooldown / kBoostCooldown, 1.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_boostTimer > 0.0 ? QColor(255, 194, 55) : QColor(70, 205, 130));
    painter.drawRoundedRect(QRectF(boostTrack.left() + 1.0, boostTrack.top() + 1.0,
                                   (boostTrack.width() - 2.0) * boostReady,
                                   boostTrack.height() - 2.0), 3.0, 3.0);

    painter.setFont(QFont(QStringLiteral("Arial"), 9));
    painter.setPen(QColor(255, 255, 255, 220));
    painter.drawText(12, height() - 12,
        QStringLiteral("Czujnik / ← A → D: pas | SPACJA lub B: dopalacz"));

    if (!isRunning() && m_lives <= 0) {
        painter.fillRect(rect(), QColor(0, 0, 0, 175));
        painter.setPen(Qt::white);
        painter.setFont(QFont(QStringLiteral("Arial"), 25, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter,
            QStringLiteral("KONIEC GRY\nWynik: %1").arg(getScore()));
    }
}

} // namespace games
