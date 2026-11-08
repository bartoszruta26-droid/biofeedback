#ifndef GAMES_ARKANOIDGAME_HPP
#define GAMES_ARKANOIDGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QColor>
#include <QPointF>
#include <QRectF>
#include <memory>
#include <vector>

namespace games {

class ArkanoidGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit ArkanoidGame(QWidget *parent = nullptr);
    ~ArkanoidGame() override = default;

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
    struct Brick {
        QRectF rect;
        int hits{1};
        bool active{true};
        int row{0};
    };

    void updatePhysicsStep(double deltaTime);
    void buildLevel();
    void resetBall();
    void advanceLevel();
    bool hasActiveBricks() const;
    QColor brickColor(const Brick& brick) const;

    QRectF m_paddle;
    QPointF m_ballPosition;
    QPointF m_ballVelocity;
    std::vector<Brick> m_bricks;

    double m_currentForce;
    double m_ballRadius;
    double m_ballSpeed;
    double m_paddleWidth;
    double m_paddleHeight;

    int m_difficultyLevel;
    int m_level;
    int m_lives;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;

    static constexpr double MIN_FORCE = 0.0;
    static constexpr double MAX_FORCE = 100.0;
};

} // namespace games

#endif // GAMES_ARKANOIDGAME_HPP
