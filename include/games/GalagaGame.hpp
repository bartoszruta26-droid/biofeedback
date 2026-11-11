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
    double playerPositionX() const { return m_player.center().x(); }
    double hitProtectionRemaining() const { return m_hitCooldown; }

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
    };

    void createWave();
    void firePlayerShot();
    bool loseLife();

    QRectF m_player;
    QVector<Projectile> m_projectiles;
    QVector<Enemy> m_enemies;
    double m_force;
    double m_keyboardDirection;
    double m_fireCooldown;
    double m_enemyDirection;
    double m_enemyFireCooldown;
    double m_hitCooldown;
    int m_lives;
    int m_wave;
    bool m_fireHeld;
    double m_backgroundOffset;
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
};

} // namespace games

#endif // GAMES_GALAGAGAME_HPP
