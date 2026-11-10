#ifndef GAMES_ARKANOIDGAME_HPP
#define GAMES_ARKANOIDGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QSize>
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

    double currentForcePercent() const;
    bool hasFreshSensorInput() const;
    double paddleX() const;
    QPointF ballPosition() const;
    int livesRemaining() const;
    int currentLevel() const;
    int activeBrickCount() const;
    bool isBallLaunched() const;

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

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
    int activeBrickCountInternal() const;
    QColor brickColor(const Brick& brick) const;
    void adaptToSize();
    bool sensorInputFresh() const;
    void launchBall();
    void loseLife();
    void updatePaddle(double deltaTime);
    void drawHud(QPainter& painter);

    QRectF m_paddle;
    QPointF m_ballPosition;
    QPointF m_ballVelocity;
    std::vector<Brick> m_bricks;

    double m_currentForce;
    double m_targetForce;
    double m_ballRadius;
    double m_ballSpeed;
    double m_paddleWidth;
    double m_paddleHeight;
    double m_serveDelayRemaining;
    bool m_ballLaunched;
    bool m_hasSensorSample;
    QElapsedTimer m_sensorClock;
    QSize m_lastSize;

    int m_difficultyLevel;
    int m_level;
    int m_lives;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;

    static constexpr double MIN_FORCE = 0.0;
    static constexpr double MAX_FORCE = 100.0;
};

} // namespace games

#endif // GAMES_ARKANOIDGAME_HPP
