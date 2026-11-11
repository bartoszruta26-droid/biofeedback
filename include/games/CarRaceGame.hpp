#ifndef GAMES_CARRACEGAME_HPP
#define GAMES_CARRACEGAME_HPP

#include "games/GameEngine.hpp"
#include <QRectF>
#include <QVector>

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
    double animationClock() const { return m_animationTime; }
    double visualLanePosition() const { return m_visualLane; }
    int passStreak() const { return m_passStreak; }
    int collectiblesCollected() const { return m_collectiblesCollected; }
    int difficultyLevel() const { return 1 + static_cast<int>(m_distance / 5000.0); }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Obstacle {
        QRectF rect;
        int lane;
        bool passed;
        bool boostCell;
    };

    void setLane(int lane);
    void spawnObstacle();
    void loseLife();
    void activateBoost();
    QRectF laneRect(int lane, double top, double objectWidth, double objectHeight) const;

    QVector<Obstacle> m_obstacles;
    QRectF m_player;
    double m_force;
    double m_inputAge;
    double m_spawnTimer;
    double m_spawnInterval;
    double m_roadSpeed;
    double m_distance;
    double m_boostTimer;
    double m_boostCooldown;
    double m_boostGestureSeconds;
    double m_collisionTimer;
    double m_scoreDistanceAccumulator;
    double m_animationTime;
    double m_visualLane;
    double m_cameraShakeTimer;
    int m_passStreak;
    int m_collectiblesCollected;
    int m_lane;
    int m_boostGestureLane;
    int m_lives;
    int m_spawnCount;
    bool m_boostGestureActive;
};

} // namespace games

#endif // GAMES_CARRACEGAME_HPP
