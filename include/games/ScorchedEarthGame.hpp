#ifndef GAMES_SCORCHEDEARTHGAME_HPP
#define GAMES_SCORCHEDEARTHGAME_HPP

#include "games/GameEngine.hpp"
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QVector>

class QKeyEvent;
class QMouseEvent;
class QResizeEvent;
class QWheelEvent;

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
    int trajectoryPreviewPointCount() const;
    double windStrength() const { return m_wind; }
    bool isPlayerTurn() const { return m_playerTurn; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

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
    void chooseEnemyShot();
    void resolveImpact(const QPointF& point, bool playerOwned);
    void applyCrater(double x, double power);
    QVector<QPointF> trajectoryPreview() const;
    void updateWind();

    QPolygonF m_terrain;
    QVector<Projectile> m_projectiles;
    QRectF m_playerTank;
    QRectF m_enemyTank;
    double m_aimDegrees;
    double m_power;
    double m_enemyAimDegrees;
    double m_enemyPower;
    double m_wind;
    double m_enemyDelay;
    double m_gravity;
    int m_playerHealth;
    int m_enemyHealth;
    int m_round;
    bool m_playerTurn;
};

} // namespace games

#endif // GAMES_SCORCHEDEARTHGAME_HPP
