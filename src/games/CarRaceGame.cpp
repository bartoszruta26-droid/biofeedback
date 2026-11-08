#include "games/CarRaceGame.hpp"
#include <QKeyEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {

CarRaceGame::CarRaceGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_spawnTimer(0.0)
    , m_spawnInterval(1.25)
    , m_roadSpeed(220.0)
    , m_distance(0.0)
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
    m_lane = 1;
    m_lives = 3;
    m_spawnCount = 0;
    m_player = QRectF(0.0, height() - 105.0, 38.0, 66.0);
    setLane(m_lane);
}

void CarRaceGame::setLane(int lane)
{
    m_lane = qBound(0, lane, 2);
    const double roadWidth = qMax(0.0, width() * 0.72);
    const double roadLeft = (width() - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;
    const double playerWidth = qMin(38.0, qMax(1.0, laneWidth - 8.0));
    const double playerHeight = qMin(66.0, qMax(1.0, height() - 8.0));
    m_player.setSize(QSizeF(playerWidth, playerHeight));
    m_player.moveTop(qBound(0.0, static_cast<double>(height()) - playerHeight - 24.0,
                            qMax(0.0, static_cast<double>(height()) - playerHeight)));
    m_player.moveLeft(roadLeft + laneWidth * m_lane + (laneWidth - m_player.width()) / 2.0);
}

bool CarRaceGame::handleInput(double value)
{
    m_force = qBound(0.0, value / 100.0, 1.0);
    setLane(qBound(0, static_cast<int>(m_force * 3.0), 2));
    return true;
}

void CarRaceGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A)
        setLane(m_lane - 1);
    else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D)
        setLane(m_lane + 1);
    else
        GameEngine::keyPressEvent(event);
}

void CarRaceGame::spawnObstacle()
{
    if (width() <= 0 || height() <= 0)
        return;
    const double roadWidth = width() * 0.72;
    const double roadLeft = (width() - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;
    const double obstacleWidth = qMin(42.0, qMax(1.0, laneWidth - 8.0));
    const double obstacleHeight = qMin(64.0, qMax(1.0, height() * 0.2));
    const int lane = (m_spawnCount * 2 + 1) % 3;
    ++m_spawnCount;
    Obstacle obstacle;
    obstacle.lane = lane;
    obstacle.passed = false;
    obstacle.rect = QRectF(roadLeft + laneWidth * lane + (laneWidth - obstacleWidth) / 2.0,
                           -obstacleHeight - 4.0, obstacleWidth, obstacleHeight);
    m_obstacles.push_back(obstacle);
}

void CarRaceGame::loseLife()
{
    --m_lives;
    if (m_lives <= 0)
        stop();
}

void CarRaceGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);

    m_distance += m_roadSpeed * deltaTime;
    m_roadSpeed = qMin(520.0, 220.0 + m_distance * 0.012);
    m_spawnInterval = qMax(0.58, 1.25 - m_distance / 12000.0);
    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer -= m_spawnInterval;
        spawnObstacle();
    }

    for (Obstacle& obstacle : m_obstacles) {
        obstacle.rect.translate(0.0, m_roadSpeed * deltaTime);
        if (!obstacle.passed && obstacle.rect.top() > m_player.bottom()) {
            obstacle.passed = true;
            addScore(5);
        }
        if (obstacle.rect.intersects(m_player)) {
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
    painter.fillRect(rect(), QColor(28, 105, 55));
    painter.setRenderHint(QPainter::Antialiasing);

    const double roadWidth = width() * 0.72;
    const double roadLeft = (width() - roadWidth) / 2.0;
    const double laneWidth = roadWidth / 3.0;
    painter.fillRect(QRectF(roadLeft, 0.0, roadWidth, height()), QColor(42, 45, 52));

    painter.setPen(QPen(QColor(235, 235, 220), 3, Qt::DashLine));
    for (int divider = 1; divider <= 2; ++divider) {
        const double x = roadLeft + laneWidth * divider;
        const double offset = std::fmod(m_distance * 0.8, 48.0);
        for (double y = -48.0 + offset; y < height(); y += 48.0)
            painter.drawLine(QPointF(x, y), QPointF(x, y + 24.0));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(70, 165, 245));
    painter.drawRoundedRect(m_player, 8.0, 8.0);
    painter.setBrush(QColor(170, 220, 250));
    painter.drawRoundedRect(QRectF(m_player.left() + 7.0, m_player.top() + 9.0,
                                   m_player.width() - 14.0, 20.0), 4.0, 4.0);

    painter.setBrush(QColor(225, 75, 65));
    for (const Obstacle& obstacle : m_obstacles)
        if (!obstacle.rect.isEmpty())
            painter.drawRoundedRect(obstacle.rect, 7.0, 7.0);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(14, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 46, QStringLiteral("Dystans: %1 m").arg(qRound(m_distance / 10.0)));
    painter.drawText(width() - 110, 24, QStringLiteral("Życia: %1").arg(m_lives));
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Czujnik: wybór pasa | Strzałki/A-D: zmiana pasa"));
}

} // namespace games
