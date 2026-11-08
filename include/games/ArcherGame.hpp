#ifndef GAMES_ARCHERGAME_HPP
#define GAMES_ARCHERGAME_HPP

#include "games/GameEngine.hpp"
#include <QPointF>
#include <QRectF>
#include <QVector>

class QKeyEvent;

namespace games {

class ArcherGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit ArcherGame(QWidget *parent = nullptr);
    QString gameName() const override;

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    struct Arrow {
        QPointF position;
        QPointF velocity;
        bool active;
    };

    void fireArrow();
    void repositionTarget();

    QVector<Arrow> m_arrows;
    QRectF m_target;
    double m_force;
    double m_aimAngle;
    double m_targetDirection;
    double m_targetSpeed;
    double m_gravity;
    double m_fireCooldown;
};

} // namespace games

#endif // GAMES_ARCHERGAME_HPP
