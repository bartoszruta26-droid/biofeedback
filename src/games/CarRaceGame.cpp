#include "games/CarRaceGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <QPaintEvent>
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
    , m_collisionCooldown(0.0)
    , m_sensorInitialized(false)
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
    m_collisionCooldown = 0.0;
    m_sensorInitialized = false;
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
    if (!std::isfinite(value))
        return false;

    m_force = qBound(0.0, value / 100.0, 1.0);
    const double forcePercent = m_force * 100.0;
    int nextLane = m_lane;

    if (!m_sensorInitialized) {
        nextLane = forcePercent < 33.333 ? 0 : (forcePercent >= 66.667 ? 2 : 1);
        m_sensorInitialized = true;
    } else if (m_lane == 0) {
        if (forcePercent >= 70.0) nextLane = 2;
        else if (forcePercent >= 38.0) nextLane = 1;
    } else if (m_lane == 1) {
        if (forcePercent <= 30.0) nextLane = 0;
        else if (forcePercent >= 70.0) nextLane = 2;
    } else {
        if (forcePercent <= 30.0) nextLane = 0;
        else if (forcePercent <= 62.0) nextLane = 1;
    }

    setLane(nextLane);
    return true;
}

void CarRaceGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_A) {
        setLane(m_lane - 1);
        m_force = (m_lane + 0.5) / 3.0;
        m_sensorInitialized = false;
    } else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_D) {
        setLane(m_lane + 1);
        m_force = (m_lane + 0.5) / 3.0;
        m_sensorInitialized = false;
    }
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
    while (m_spawnTimer >= m_spawnInterval) {
        m_spawnTimer -= m_spawnInterval;
        spawnObstacle();
    }
    m_collisionCooldown = qMax(0.0, m_collisionCooldown - deltaTime);

    for (Obstacle& obstacle : m_obstacles) {
        obstacle.rect.translate(0.0, m_roadSpeed * deltaTime);
        if (!obstacle.passed && obstacle.rect.top() > m_player.bottom()) {
            obstacle.passed = true;
            addScore(5);
        }
        if (obstacle.rect.intersects(m_player)) {
            obstacle.rect = QRectF();
            if (m_collisionCooldown <= 0.0) {
                m_collisionCooldown = 0.85;
                loseLife();
                if (!isRunning()) return;
            }
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
    const double roadRight = roadLeft + roadWidth;
    painter.fillRect(QRectF(roadLeft, 0.0, roadWidth, height()), QColor(38, 42, 49));
    painter.fillRect(QRectF(roadLeft, 0.0, 5.0, height()), QColor(245, 205, 80));
    painter.fillRect(QRectF(roadRight - 5.0, 0.0, 5.0, height()), QColor(245, 205, 80));
    painter.fillRect(QRectF(roadLeft + laneWidth * m_lane + 3.0, 0.0,
                            qMax(0.0, laneWidth - 6.0), height()),
                     QColor(255, 255, 255, 15));

    painter.setPen(QPen(QColor(225, 228, 218), 2, Qt::DashLine));
    for (int divider = 1; divider <= 2; ++divider) {
        const double x = roadLeft + laneWidth * divider;
        const double offset = std::fmod(m_distance * 0.8, 48.0);
        for (double y = -48.0 + offset; y < height(); y += 48.0)
            painter.drawLine(QPointF(x, y), QPointF(x, y + 24.0));
    }

    // Traffic vehicles have wheels, bodywork and a windscreen.
    for (const Obstacle& obstacle : m_obstacles) {
        if (obstacle.rect.isEmpty())
            continue;
        const QRectF car = obstacle.rect;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(20, 22, 27));
        painter.drawRoundedRect(QRectF(car.left() - 3.0, car.top() + 9.0,
                                       5.0, qMax(1.0, car.height() - 18.0)), 2.0, 2.0);
        painter.drawRoundedRect(QRectF(car.right() - 2.0, car.top() + 9.0,
                                       5.0, qMax(1.0, car.height() - 18.0)), 2.0, 2.0);
        painter.setBrush(QColor(205, 68, 62));
        painter.drawRoundedRect(car, 7.0, 7.0);
        painter.setBrush(QColor(245, 185, 165));
        painter.drawRoundedRect(QRectF(car.left() + car.width() * 0.18,
                                       car.top() + car.height() * 0.16,
                                       car.width() * 0.64, car.height() * 0.25), 3.0, 3.0);
        painter.setBrush(QColor(255, 225, 115));
        painter.drawRect(QRectF(car.left() + 4.0, car.bottom() - 5.0,
                                qMax(1.0, car.width() - 8.0), 3.0));
    }

    // Player vehicle with a visible hit/recovery state.
    const QRectF car = m_player;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(15, 18, 22));
    painter.drawRoundedRect(QRectF(car.left() - 4.0, car.top() + 10.0,
                                   6.0, qMax(1.0, car.height() - 20.0)), 2.0, 2.0);
    painter.drawRoundedRect(QRectF(car.right() - 2.0, car.top() + 10.0,
                                   6.0, qMax(1.0, car.height() - 20.0)), 2.0, 2.0);
    painter.setBrush(m_collisionCooldown > 0.0 ? QColor(255, 190, 65) : QColor(48, 142, 230));
    painter.drawRoundedRect(car, 8.0, 8.0);
    painter.setBrush(QColor(155, 218, 250));
    painter.drawRoundedRect(QRectF(car.left() + car.width() * 0.17,
                                   car.top() + car.height() * 0.17,
                                   car.width() * 0.66, car.height() * 0.23), 4.0, 4.0);
    painter.setBrush(QColor(255, 242, 170));
    painter.drawRoundedRect(QRectF(car.left() + 4.0, car.top() + 3.0,
                                   qMax(1.0, car.width() - 8.0), 4.0), 2.0, 2.0);
    painter.setBrush(QColor(245, 75, 65));
    painter.drawRoundedRect(QRectF(car.left() + 4.0, car.bottom() - 7.0,
                                   qMax(1.0, car.width() - 8.0), 4.0), 2.0, 2.0);

    // HUD: score, distance, lives, force gauge and selected lane.
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(10, height() / 34), QFont::Bold));
    painter.drawText(12, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(12, 46, QStringLiteral("Dystans: %1 m").arg(qRound(m_distance / 10.0)));
    painter.drawText(width() - 112, 24, QStringLiteral("Życia: %1").arg(m_lives));

    const double gaugeWidth = qMin(150.0, qMax(72.0, width() * 0.28));
    const double gaugeX = qMax(10.0, width() - gaugeWidth - 12.0);
    const double gaugeY = 38.0;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 110));
    painter.drawRoundedRect(QRectF(gaugeX, gaugeY, gaugeWidth, 12.0), 5.0, 5.0);
    painter.setBrush(QColor(90, 205, 130));
    painter.drawRoundedRect(QRectF(gaugeX, gaugeY, gaugeWidth * m_force, 12.0), 5.0, 5.0);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(8, height() / 52)));
    painter.drawText(QRectF(gaugeX, gaugeY + 14.0, gaugeWidth, 16.0),
                     Qt::AlignRight, QStringLiteral("Siła %1%").arg(qRound(m_force * 100.0)));

    const QString laneLabel = m_lane == 0 ? QStringLiteral("LEWY")
                              : (m_lane == 1 ? QStringLiteral("ŚRODKOWY")
                                             : QStringLiteral("PRAWY"));
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(9, height() / 42), QFont::Bold));
    painter.drawText(QRectF(roadLeft, height() - 35.0, roadWidth, 18.0),
                     Qt::AlignCenter, QStringLiteral("PAS: %1").arg(laneLabel));
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(8, height() / 55)));
    painter.drawText(QRectF(4.0, height() - 17.0, width() - 8.0, 14.0),
                     Qt::AlignCenter,
                     QStringLiteral("Tensometr: lewo / środek / prawo   •   Klawiatura: ← → lub A / D"));
}

void CarRaceGame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    render(painter);

    if (!isRunning()) {
        const QRectF panel(width() * 0.12, height() * 0.32,
                           width() * 0.76, height() * 0.36);
        painter.fillRect(panel, QColor(0, 0, 0, 190));
        painter.setPen(Qt::white);
        painter.setFont(QFont(QStringLiteral("Arial"), qMax(14, height() / 22), QFont::Bold));
        const bool gameOver = m_lives <= 0;
        painter.drawText(QRectF(panel.left() + 8.0, panel.top() + 8.0,
                                panel.width() - 16.0, panel.height() * 0.35),
                         Qt::AlignCenter,
                         gameOver ? QStringLiteral("KONIEC WYŚCIGU")
                                  : QStringLiteral("CAR RACE"));
        painter.setFont(QFont(QStringLiteral("Arial"), qMax(9, height() / 38)));
        const QString message = gameOver
            ? QStringLiteral("Wynik: %1\nDystans: %2 m\nUruchom ponownie, aby zagrać jeszcze raz.")
                  .arg(getScore()).arg(qRound(m_distance / 10.0))
            : QStringLiteral("Steruj naciskiem: lewy, środkowy lub prawy pas.\nPrzeszkody omijaj, zmieniając pas.\nWynik rośnie za każdą ominiętą przeszkodę.");
        painter.drawText(QRectF(panel.left() + 10.0, panel.top() + panel.height() * 0.38,
                                panel.width() - 20.0, panel.height() * 0.56),
                         Qt::AlignCenter, message);
    }
}
} // namespace games
