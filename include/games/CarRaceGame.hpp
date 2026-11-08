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

    QVector<Obstacle> m_obstacles;
    QRectF m_player;
    double m_force;
    double m_spawnTimer;
    double m_spawnInterval;
    double m_roadSpeed;
    double m_distance;
    int m_lane;
    int m_lives;
    int m_spawnCount;
};

} // namespace games

#endif // GAMES_CARRACEGAME_HPP
