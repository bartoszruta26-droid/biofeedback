#ifndef GAMES_PRECISIONCRANEGAME_HPP
#define GAMES_PRECISIONCRANEGAME_HPP

#include "games/GameEngine.hpp"
#include <QPointF>
#include <QRectF>
#include <QVector>

namespace games {

class PrecisionCraneGame final : public GameEngine
{
    Q_OBJECT

public:
    enum ControlAxis { HorizontalAxis = 0, VerticalAxis = 1 };
    enum CargoType { LightCargo = 0, StandardCargo = 1, HeavyCargo = 2 };

    explicit PrecisionCraneGame(QWidget *parent = nullptr);
    QString gameName() const override;

    void setDifficultyLevel(int level);
    int difficultyLevel() const { return m_difficulty; }
    ControlAxis controlAxis() const { return m_axis; }
    bool loadAttached() const { return m_loadAttached; }
    int deliveredLoads() const { return m_delivered; }
    int mistakesRemaining() const { return m_mistakesRemaining; }
    int lastPrecisionBonus() const { return m_lastPrecisionBonus; }
    CargoType cargoType() const { return m_cargoType; }
    double cargoMassKg() const { return m_cargoMassKg; }
    int missionNumber() const { return qMin(5, m_missionIndex + 1); }
    int missionDeliveriesRemaining() const { return qMax(0, 3 - (m_delivered % 3)); }
    bool campaignCompleted() const { return m_campaignCompleted; }
    int obstacleCount() const { return 2; }
    int collisionCount() const { return m_collisionCount; }
    int bestScore() const { return m_bestScore; }
    int lifetimeDeliveries() const { return m_lifetimeDeliveries; }
    int lifetimePrecisionSamples() const { return m_lifetimePrecisionSamples; }
    int lifetimeCollisions() const { return m_lifetimeCollisions; }
    int precisionSamples() const { return m_precisionSamples; }
    double averagePrecision() const { return m_precisionSamples > 0 ? m_precisionTotal / m_precisionSamples : 0.0; }
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

    struct MovingObstacle { double y; double width; double height; double speed; double phase; };
    void resetRound();
    QRectF obstacleRect(int index) const;
    bool checkObstacleCollision();
    void configureCargo();
    void loadRecords();
    void saveRecords();
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
    double m_swayVelocity;
    double m_previousCarriageX;
    double m_trolleyVelocity;
    double m_cargoMassKg;
    double m_ropeLength;
    double m_precisionTotal;
    double m_obstacleClock;
    double m_cargoX;
    double m_targetX;
    double m_targetWidth;
    int m_difficulty;
    int m_delivered;
    int m_mistakesRemaining;
    int m_lastPrecisionBonus;
    int m_combo;
    int m_missionIndex;
    int m_collisionCount;
    int m_bestScore;
    int m_precisionSamples;
    int m_lifetimeDeliveries;
    int m_lifetimePrecisionTotal;
    int m_lifetimePrecisionSamples;
    int m_lifetimeCollisions;
    CargoType m_cargoType;
    bool m_campaignCompleted;
    QVector<MovingObstacle> m_obstacles;
    ControlAxis m_axis;
    bool m_gestureActive;
    bool m_loadAttached;
    QString m_status;
};

} // namespace games

#endif
