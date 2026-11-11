#include "games/CarRaceGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QLinearGradient>
#include <QPolygonF>
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
    , m_animationTime(0.0)
    , m_visualLane(1.0)
    , m_cameraShakeTimer(0.0)
    , m_passStreak(0)
    , m_collectiblesCollected(0)
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
    m_animationTime = 0.0;
    m_visualLane = 1.0;
    m_cameraShakeTimer = 0.0;
    m_passStreak = 0;
    m_collectiblesCollected = 0;
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
    obstacle.boostCell = (m_spawnCount % 5 == 0);
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

    m_animationTime += deltaTime;
    m_visualLane += (static_cast<double>(m_lane) - m_visualLane) * qMin(1.0, deltaTime * 11.0);
    m_cameraShakeTimer = qMax(0.0, m_cameraShakeTimer - deltaTime);
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
        if (obstacle.boostCell && !obstacle.passed && obstacle.rect.intersects(m_player)) {
            obstacle.passed = true;
            obstacle.rect = QRectF();
            ++m_collectiblesCollected;
            addScore(25);
            m_boostCooldown = qMax(0.0, m_boostCooldown - 2.0);
            continue;
        }
        if (!obstacle.passed && obstacle.rect.top() > m_player.bottom()) {
            obstacle.passed = true;
            if (!obstacle.boostCell) {
                ++m_passStreak;
                addScore(5 + qMin(m_passStreak - 1, 5) * 2);
            }
        }
        if (!obstacle.boostCell && m_collisionTimer <= 0.0 && obstacle.rect.intersects(m_player)) {
            obstacle.rect = QRectF();
            m_passStreak = 0;
            m_cameraShakeTimer = 0.32;
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
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    const double w = width(), h = height();
    const double roadWidth = w * kRoadWidthRatio;
    const double roadLeft = (w - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;

    QLinearGradient skyGradient(0.0, 0.0, 0.0, h);
    skyGradient.setColorAt(0.0, QColor(100, 174, 220));
    skyGradient.setColorAt(1.0, QColor(220, 235, 244));
    painter.fillRect(rect(), skyGradient);
    painter.fillRect(QRectF(0.0, h * 0.34, w, h * 0.66), QColor(55, 132, 72));

    // Dalekie wzgórza i bliższa roślinność poruszają się z różną prędkością.
    const double farShift = std::fmod(m_distance * 0.035, w * 0.55);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(91, 143, 157));
    for (int i = -1; i < 4; ++i) {
        const double x = i * w * 0.42 - farShift;
        QPolygonF ridge;
        ridge << QPointF(x - w * 0.16, h * 0.39) << QPointF(x + w * 0.04, h * 0.19)
              << QPointF(x + w * 0.20, h * 0.38) << QPointF(x + w * 0.34, h * 0.22)
              << QPointF(x + w * 0.52, h * 0.40) << QPointF(x + w * 0.52, h * 0.52)
              << QPointF(x - w * 0.16, h * 0.52);
        painter.drawPolygon(ridge);
    }
    const double nearShift = std::fmod(m_distance * 0.11, w * 0.38);
    painter.setBrush(QColor(43, 112, 62));
    for (int i = -1; i < 6; ++i) {
        const double x = i * w * 0.27 - nearShift;
        painter.drawEllipse(QRectF(x, h * 0.31, w * 0.23, h * 0.20));
        painter.drawEllipse(QRectF(x + w * 0.11, h * 0.29, w * 0.19, h * 0.22));
    }

    painter.fillRect(QRectF(roadLeft - 12.0, 0.0, roadWidth + 24.0, h), QColor(40, 45, 48));
    painter.fillRect(QRectF(roadLeft, 0.0, roadWidth, h), QColor(42, 47, 55));
    painter.fillRect(QRectF(roadLeft - 6.0, 0.0, 6.0, h), QColor(226, 67, 57));
    painter.fillRect(QRectF(roadLeft + roadWidth, 0.0, 6.0, h), QColor(226, 67, 57));

    const double markerOffset = std::fmod(m_distance * 0.95, 72.0);
    for (double y = -72.0 + markerOffset; y < h + 72.0; y += 72.0) {
        const double depth = qBound(0.0, (y + 72.0) / (h + 72.0), 1.0);
        const double size = 3.0 + depth * 7.0;
        painter.setBrush(QColor(235, 239, 224));
        painter.drawRoundedRect(QRectF(roadLeft - 15.0 - size, y, size, size * 2.6), 1.5, 1.5);
        painter.drawRoundedRect(QRectF(roadLeft + roadWidth + 15.0, y, size, size * 2.6), 1.5, 1.5);
    }

    // Linie pasów, drzewa i znaczniki pobocza wzmacniają efekt głębi.
    for (int divider = 1; divider <= 2; ++divider) {
        const double x = roadLeft + laneWidth * divider;
        for (double y = -66.0 + std::fmod(m_distance * 0.88, 66.0); y < h + 66.0; y += 66.0) {
            const double depth = qBound(0.0, (y + 66.0) / (h + 66.0), 1.0);
            const double dashH = 9.0 + depth * 25.0, dashW = 1.0 + depth * 1.8;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(224, 229, 221, 210));
            painter.drawRoundedRect(QRectF(x - dashW / 2.0, y, dashW, dashH), dashW / 2.0, dashW / 2.0);
        }
    }
    const double treeShift = std::fmod(m_distance * 0.19, 118.0);
    for (int side : {-1, 1}) for (int i = -1; i < 5; ++i) {
        const double y = i * 118.0 + treeShift - 25.0;
        const double depth = qBound(0.0, (y + 25.0) / (h + 50.0), 1.0);
        const double size = 8.0 + depth * 18.0;
        const double x = side < 0 ? roadLeft - 30.0 - size : roadLeft + roadWidth + 30.0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(91, 66, 45));
        painter.drawRect(QRectF(x + size * 0.42, y + size * 0.55, size * 0.16, size * 0.65));
        painter.setBrush(QColor(35, 103, 53));
        painter.drawEllipse(QRectF(x, y, size, size));
    }

    for (const Obstacle& obstacle : m_obstacles) {
        if (obstacle.rect.isEmpty()) continue;
        const double depth = qBound(0.0, (obstacle.rect.center().y() + 70.0) / (h + 70.0), 1.0);
        const double pulse = 0.5 + 0.5 * std::sin(m_animationTime * 7.0 + obstacle.rect.y() * 0.02);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(18, 22, 28, 75));
        painter.drawEllipse(QRectF(obstacle.rect.left() - 3.0, obstacle.rect.bottom() - 3.0,
                                   obstacle.rect.width() + 6.0, 9.0 + depth * 5.0));
        if (obstacle.boostCell) {
            const double bob = std::sin(m_animationTime * 5.0 + obstacle.rect.y() * 0.03) * 3.0;
            const QRectF token(obstacle.rect.center().x() - obstacle.rect.width() * 0.30,
                               obstacle.rect.center().y() - obstacle.rect.height() * 0.30 + bob,
                               obstacle.rect.width() * 0.60, obstacle.rect.height() * 0.60);
            painter.setPen(QPen(QColor(255, 245, 180), 2.0));
            painter.setBrush(QColor(255, 183, 42, 220));
            painter.drawEllipse(token);
            painter.setPen(QPen(QColor(255, 255, 225), 2.0));
            painter.drawLine(QPointF(token.center().x(), token.top() + 5.0),
                             QPointF(token.center().x(), token.bottom() - 5.0));
            continue;
        }
        painter.setBrush(QColor(190 + static_cast<int>(pulse * 35.0), 61, 58));
        painter.drawRoundedRect(obstacle.rect, 7.0, 7.0);
        const QRectF window(obstacle.rect.left() + obstacle.rect.width() * 0.18,
                            obstacle.rect.top() + obstacle.rect.height() * 0.16,
                            obstacle.rect.width() * 0.64, obstacle.rect.height() * 0.28);
        painter.setBrush(QColor(65, 87, 104));
        painter.drawRoundedRect(window, 3.0, 3.0);
        painter.setBrush(QColor(255, 222, 132, 180 + static_cast<int>(pulse * 75.0)));
        painter.drawRoundedRect(QRectF(obstacle.rect.left() + 4.0, obstacle.rect.bottom() - 8.0,
                                       qMax(1.0, obstacle.rect.width() * 0.2), 4.0), 1.5, 1.5);
        painter.drawRoundedRect(QRectF(obstacle.rect.right() - obstacle.rect.width() * 0.2 - 4.0,
                                       obstacle.rect.bottom() - 8.0, qMax(1.0, obstacle.rect.width() * 0.2),
                                       4.0), 1.5, 1.5);
    }

    const double carLeft = roadLeft + laneWidth * m_visualLane + (laneWidth - m_player.width()) / 2.0;
    QRectF car(carLeft, m_player.top(), m_player.width(), m_player.height());
    car.translate(0.0, std::sin(m_animationTime * 13.0) * 1.4);
    if (m_cameraShakeTimer > 0.0) {
        const double shake = m_cameraShakeTimer * 18.0;
        painter.translate(std::sin(m_animationTime * 65.0) * shake, std::cos(m_animationTime * 53.0) * shake * 0.45);
    }
    if (m_boostTimer > 0.0) {
        const double flame = 7.0 + 5.0 * (0.5 + 0.5 * std::sin(m_animationTime * 32.0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 151, 35, 220));
        painter.drawEllipse(QRectF(car.center().x() - car.width() * 0.22, car.bottom() - 1.0,
                                   car.width() * 0.44, flame));
        painter.setBrush(QColor(255, 225, 100, 230));
        painter.drawEllipse(QRectF(car.center().x() - car.width() * 0.10, car.bottom(),
                                   car.width() * 0.20, flame * 0.62));
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_collisionTimer > 0.0 && (static_cast<int>(m_collisionTimer * 12.0) % 2 == 0)
                         ? QColor(255, 255, 255, 130) : QColor(55, 157, 245));
    painter.drawRoundedRect(car, 9.0, 9.0);
    painter.setBrush(QColor(170, 224, 250));
    painter.drawRoundedRect(QRectF(car.left() + car.width() * 0.17, car.top() + car.height() * 0.16,
                                   car.width() * 0.66, car.height() * 0.27), 4.0, 4.0);
    painter.setBrush(QColor(22, 27, 33));
    const double wheelW = qMax(2.0, car.width() * 0.12);
    const double wheelOffset = std::fmod(m_animationTime * 22.0, 4.0) < 2.0 ? 0.0 : 1.0;
    painter.drawRoundedRect(QRectF(car.left() - 2.0, car.top() + 12.0 + wheelOffset, wheelW, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(car.right() - wheelW + 2.0, car.top() + 12.0 + wheelOffset, wheelW, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(car.left() - 2.0, car.bottom() - 27.0 - wheelOffset, wheelW, 15.0), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(car.right() - wheelW + 2.0, car.bottom() - 27.0 - wheelOffset, wheelW, 15.0), 2.0, 2.0);
    painter.restore();

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
    painter.drawText(12, 23, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(12, 43, QStringLiteral("Dystans: %1 m").arg(qRound(m_distance / 10.0)));
    painter.drawText(width() - 112, 23, QStringLiteral("Życia: %1").arg(m_lives));
    painter.drawText(width() - 112, 43, QStringLiteral("Prędkość: %1").arg(qRound(m_roadSpeed / 4.0)));
    if (m_passStreak > 1) painter.drawText(12, 80, QStringLiteral("SERIA x%1").arg(m_passStreak));
    painter.drawText(width() - 150, 64, QStringLiteral("Poziom: %1").arg(difficultyLevel()));
    painter.drawText(width() - 150, 84, QStringLiteral("Dopalacze: %1").arg(m_collectiblesCollected));
    const QRectF track(12.0, 55.0, 112.0, 9.0);
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1));
    painter.setBrush(QColor(20, 25, 30, 170));
    painter.drawRoundedRect(track, 4.0, 4.0);
    const double ready = m_boostCooldown <= 0.0 ? 1.0 : 1.0 - qBound(0.0, m_boostCooldown / kBoostCooldown, 1.0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_boostTimer > 0.0 ? QColor(255, 194, 55) : QColor(70, 205, 130));
    painter.drawRoundedRect(QRectF(track.left() + 1.0, track.top() + 1.0,
                                   (track.width() - 2.0) * ready, track.height() - 2.0), 3.0, 3.0);
    painter.setFont(QFont(QStringLiteral("Arial"), 9));
    painter.setPen(QColor(255, 255, 255, 220));
    painter.drawText(12, height() - 12, QStringLiteral("Czujnik / ← A → D: pas | SPACJA lub B: dopalacz"));
    if (!isRunning() && m_lives <= 0) {
        painter.fillRect(rect(), QColor(0, 0, 0, 175));
        painter.setPen(Qt::white);
        painter.setFont(QFont(QStringLiteral("Arial"), 25, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("KONIEC GRY\nWynik: %1").arg(getScore()));
    }
}

} // namespace games
