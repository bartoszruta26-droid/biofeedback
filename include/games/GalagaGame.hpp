#ifndef GAMES_GALAGAGAME_HPP
#define GAMES_GALAGAGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPointF>
#include <QRectF>
#include <memory>
#include <vector>

namespace games {

class GalagaGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit GalagaGame(QWidget *parent = nullptr);
    ~GalagaGame() override = default;

    QString gameName() const override;
    void setDifficultyLevel(int level);
    int getDifficultyLevel() const;

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

private:
    struct Enemy {
        QRectF rect;
        bool active{true};
        int row{0};
    };

    struct Projectile {
        QPointF position;
        QPointF velocity;
        bool fromEnemy{false};
    };

    void spawnWave();
    void spawnEnemyProjectile();
    void updateFormation(double deltaTime);
    void updateProjectiles(double deltaTime);
    void loseLife();
    void advanceLevel();
    bool hasActiveEnemies() const;

    QRectF m_ship;
    std::vector<Enemy> m_enemies;
    std::vector<Projectile> m_projectiles;

    double m_currentForce;
    double m_enemyDirection;
    double m_enemySpeed;
    double m_playerFireInterval;
    double m_playerFireTimer;
    double m_enemyFireInterval;
    double m_enemyFireTimer;
    double m_enemyProjectileSpeed;

    int m_difficultyLevel;
    int m_level;
    int m_lives;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;

    static constexpr double MIN_FORCE = 0.0;
    static constexpr double MAX_FORCE = 100.0;
};

} // namespace games

#endif // GAMES_GALAGAGAME_HPP
