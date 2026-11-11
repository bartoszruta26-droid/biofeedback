#ifndef GAMES_SCORCHEDEARTHGAME_HPP
#define GAMES_SCORCHEDEARTHGAME_HPP

#include "games/GameEngine.hpp"
#include <QElapsedTimer>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QVector>

class QResizeEvent;

namespace games {

class ScorchedEarthGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit ScorchedEarthGame(QWidget *parent = nullptr);
    QString gameName() const override;
    int playerHealth() const { return m_playerHealth; }
    int enemyHealth() const { return m_enemyHealth; }
    int currentRound() const { return m_round; }
    int terrainPointCount() const { return m_terrain.size(); }
    double aimDegrees() const { return m_aimDegrees; }
    double shotPower() const { return m_power; }
    int activeProjectileCount() const { return m_projectiles.size(); }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void resizeEvent(QResizeEvent* event) override;
    void renderInterpolated(QPainter& painter, double alpha) override;

private:
    struct Projectile {
        QPointF position;
        QPointF velocity;
        bool playerOwned;
        bool active;
    };

    void buildTerrain();
    double terrainY(double x) const;
    void updateTankPositions();
    void fireProjectile(bool playerOwned);
    void resolveImpact(const QPointF& point, bool playerOwned);
    void applyCrater(double x);

    QPolygonF m_terrain;
    QVector<Projectile> m_projectiles;
    QVector<QPointF> m_previousProjectilePositions;
    QRectF m_playerTank;
    QRectF m_enemyTank;
    double m_aimDegrees;
    double m_power;
    double m_wind;
    double m_enemyDelay;
    double m_playerShotDelay{2.0};
    double m_playerShotInterval{2.0};
    QElapsedTimer m_sensorClock;
    bool m_hasSensorSample{false};
    double m_gravity;
    int m_playerHealth;
    int m_enemyHealth;
    int m_round;
    bool m_playerTurn;
};

} // namespace games

#endif // GAMES_SCORCHEDEARTHGAME_HPP
