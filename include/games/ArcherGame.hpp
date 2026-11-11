#ifndef GAMES_ARCHERGAME_HPP
#define GAMES_ARCHERGAME_HPP

#include "games/GameEngine.hpp"
#include <QPointF>
#include <QRectF>
#include <QVector>

class QKeyEvent;
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
    int comboCount() const { return m_combo; }
    int focusCharges() const { return m_focusCharges; }
    double windStrength() const { return m_windStrength; }
    double aimAngle() const { return m_aimAngle; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Arrow {
        QPointF position;
        QPointF velocity;
        bool active;
        bool focused;
    };

    void fireArrow(bool focused = false);
    void repositionTarget();
    void activateFocusShot();

    QVector<Arrow> m_arrows;
    QRectF m_target;
    double m_force;
    double m_aimAngle;
    double m_targetDirection;
    double m_targetDirectionX{1.0};
    double m_targetSpeed;
    double m_gravity;
    double m_fireCooldown;
    double m_elapsed{0.0};
    double m_comboTimer{0.0};
    double m_windStrength{0.0};
    int m_combo{0};
    int m_focusCharges{3};
    int m_round{1};
    int m_shotsFired{0};
    int m_successfulHits{0};
    int m_missedShots{0};
    bool m_aimUp{false};
    bool m_aimDown{false};
};

} // namespace games

#endif // GAMES_ARCHERGAME_HPP
