#ifndef GAMES_CARRACEGAME_HPP
#define GAMES_CARRACEGAME_HPP

#include "games/GameEngine.hpp"
#include <QRectF>
#include <QVector>

class QKeyEvent;
class QResizeEvent;

namespace games {

class CarRaceGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit CarRaceGame(QWidget *parent = nullptr);
    QString gameName() const override;
    int livesRemaining() const { return m_lives; }
    int obstacleCount() const { return m_obstacles.size(); }
    int currentLane() const { return m_lane; }
    bool boostActive() const { return m_boostTimer > 0.0; }
    double boostCooldownRemaining() const { return m_boostCooldown; }
    double collisionProtectionRemaining() const { return m_collisionTimer; }
    double collisionProtectionRemaining() const { return m_collisionTimer; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Obstacle {
        QRectF rect;
        int lane;
        bool passed;
    };

    void setLane(int lane);
    void spawnObstacle();
    void loseLife();
    void activateBoost();
    QRectF laneRect(int lane, double top, double objectWidth, double objectHeight) const;

    QVector<Obstacle> m_obstacles;
    QRectF m_player;
    double m_force;
    double m_spawnTimer;
    double m_spawnInterval;
    double m_roadSpeed;
    double m_distance;
    double m_boostTimer;
    double m_boostCooldown;
    double m_collisionTimer;
    double m_scoreDistanceAccumulator;
    int m_lane;
    int m_lives;
    int m_spawnCount;
};

} // namespace games

#endif // GAMES_CARRACEGAME_HPP
