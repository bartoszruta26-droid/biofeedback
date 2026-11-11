#ifndef GAMES_ARCHERGAME_HPP
#define GAMES_ARCHERGAME_HPP

#include "games/GameEngine.hpp"
#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QVector>

class QResizeEvent;

namespace games {

class ArcherGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit ArcherGame(QWidget *parent = nullptr);
    QString gameName() const override;
    double targetSpeed() const { return m_targetSpeed; }
    int activeArrowCount() const;
    int shotsFired() const { return m_shotsFired; }
    int successfulHits() const { return m_successfulHits; }
    int missedShots() const { return m_missedShots; }
    double aimAngle() const { return m_aimAngle; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void resizeEvent(QResizeEvent* event) override;

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
    double m_autoFireInterval{2.4};
    QElapsedTimer m_sensorClock;
    bool m_hasSensorSample{false};
    int m_shotsFired{0};
    int m_successfulHits{0};
    int m_missedShots{0};
};

} // namespace games

#endif // GAMES_ARCHERGAME_HPP
