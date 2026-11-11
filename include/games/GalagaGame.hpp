#ifndef GAMES_GALAGAGAME_HPP
#define GAMES_GALAGAGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>

class QKeyEvent;
class QResizeEvent;

namespace games {

class GalagaGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit GalagaGame(QWidget *parent = nullptr);
    QString gameName() const override;
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;
    int livesRemaining() const { return m_lives; }
    int currentWave() const { return m_wave; }
    int activeEnemyCount() const;
    int activeProjectileCount() const;
    int activePowerUpCount() const;
    int specialCharges() const { return m_specialCharges; }
    int firePowerLevel() const { return m_firePowerLevel; }
    double playerPositionX() const { return m_player.center().x(); }
    double hitProtectionRemaining() const { return m_hitCooldown; }
    double shieldRemaining() const { return m_shieldRemaining; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Projectile {
        QRectF rect;
        double velocityX;
        double velocityY;
        bool fromPlayer;
    };
    struct Enemy {
        QRectF rect;
        int hitPoints;
        bool active;
        double phase;
    };
    struct PowerUp {
        QRectF rect;
        int type; // 0: triple shot, 1: shield, 2: rapid fire
        double velocity;
    };

    void createWave();
    void firePlayerShot();
    void spawnPowerUp(const QPointF& position);
    void activatePulse();
    void collectPowerUp(const PowerUp& powerUp);
    bool loseLife();

    QRectF m_player;
    QVector<Projectile> m_projectiles;
    QVector<Enemy> m_enemies;
    QVector<PowerUp> m_powerUps;
    double m_force;
    double m_sensorInputAge;
    double m_fireCooldown;
    double m_enemyDirection;
    double m_enemyFireCooldown;
    double m_hitCooldown;
    double m_shieldRemaining;
    double m_firePowerRemaining;
    double m_backgroundOffset;
    double m_elapsed;
    double m_comboTimer;
    int m_lives;
    int m_wave;
    int m_specialCharges;
    int m_firePowerLevel;
    int m_enemiesDefeated;
    int m_combo;
    bool m_fireHeld;
    bool m_sensorInputValid;
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
};

} // namespace games

#endif // GAMES_GALAGAGAME_HPP
