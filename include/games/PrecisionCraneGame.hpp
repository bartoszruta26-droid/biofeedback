#ifndef GAMES_PRECISIONCRANEGAME_HPP
#define GAMES_PRECISIONCRANEGAME_HPP

#include "games/GameEngine.hpp"
#include <QPointF>
#include <QRectF>

namespace games {

class PrecisionCraneGame final : public GameEngine
{
    Q_OBJECT

public:
    enum ControlAxis { HorizontalAxis = 0, VerticalAxis = 1 };

    explicit PrecisionCraneGame(QWidget *parent = nullptr);
    QString gameName() const override;

    void setDifficultyLevel(int level);
    int difficultyLevel() const { return m_difficulty; }
    ControlAxis controlAxis() const { return m_axis; }
    bool loadAttached() const { return m_loadAttached; }
    int deliveredLoads() const { return m_delivered; }
    int mistakesRemaining() const { return m_mistakesRemaining; }
    double carriagePosition() const { return m_carriageX; }
    double hookPosition() const { return m_hookY; }
    double normalizedForce() const { return m_force; }
    bool hasFreshSignal() const { return m_inputAge <= kSignalTimeoutSeconds; }
    bool gestureActive() const { return m_gestureActive; }
    double gestureDuration() const { return m_gestureSeconds; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static constexpr double kGestureForce = 0.82;
    static constexpr double kReleaseForce = 0.18;
    static constexpr double kLongGestureSeconds = 0.85;
    static constexpr double kMinimumGestureSeconds = 0.15;
    static constexpr double kSignalTimeoutSeconds = 0.35;
    static constexpr double kMinimumControlForce = 0.18;
    static constexpr double kMaximumControlForce = 0.70;

    void resetRound();
    void processGesture();
    void toggleGrip();
    void deliverOrDropLoad();
    QRectF cargoRect() const;
    QRectF targetRect() const;
    double floorY() const;
    double clampCarriage(double x) const;

    double m_force;
    double m_inputAge;
    double m_carriageX;
    double m_hookY;
    double m_gestureSeconds;
    double m_animationTime;
    double m_sway;
    double m_cargoX;
    double m_targetX;
    double m_targetWidth;
    int m_difficulty;
    int m_delivered;
    int m_mistakesRemaining;
    int m_combo;
    ControlAxis m_axis;
    bool m_gestureActive;
    bool m_loadAttached;
    QString m_status;
};

} // namespace games

#endif
