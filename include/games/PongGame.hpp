#ifndef GAMES_PONGGAME_HPP
#define GAMES_PONGGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QElapsedTimer>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <memory>

namespace games {

class PongGame : public GameEngine {
    Q_OBJECT

public:
    explicit PongGame(QWidget *parent = nullptr);
    ~PongGame() override;

    QString gameName() const override;
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter &painter) override;
    bool handleInput(double value) override;

    void setDifficultyLevel(int level);
    int getDifficultyLevel() const;

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

    // Read-only state accessors used by diagnostics and deterministic regressions.
    double currentForcePercent() const;
    bool hasFreshSensorInput() const;
    double playerPaddleY() const;
    double aiPaddleY() const;
    double paddleWidth() const;
    double paddleHeight() const;
    QPointF ballPosition() const;
    int playerMatchScore() const;
    int aiMatchScore() const;

signals:
    void serialError(const QString &message);
    void serialConnected();
    void serialDisconnected();

private slots:
    void onSensorDataReceived(const sensor::SensorData& data);
    void onConnectionStatusChanged(bool connected, const std::string& message);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void renderInterpolated(QPainter& painter, double alpha) override;

private:
    friend struct PongGameTestAccess;

    QRectF m_leftPaddle;
    QRectF m_rightPaddle;
    float m_leftPaddleY;
    float m_rightPaddleY;

    QPointF m_ballPos;
    QPointF m_ballVel;
    QPointF m_previousBallPos;
    float m_ballRadius;
    float m_previousLeftPaddleY;
    float m_previousRightPaddleY;

    int m_playerScore;
    int m_aiScore;

    int m_difficultyLevel;
    float m_aiSpeed;
    float m_ballSpeed;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    bool m_ownSerial;

    float m_paddleWidth;
    float m_paddleHeight;
    float m_paddleMargin;

    double m_currentForce;
    double m_targetForce;
    bool m_hasSensorSample;
    QElapsedTimer m_sensorClock;
    double m_serveDelayRemaining;
    QSize m_lastSize;

    void resetBall(int horizontalDirection = 0);
    void normalizeForceValue(double rawValue);
    void acceptForcePercent(double percent);
    QColor getDifficultyColor() const;
    void updateAIPaddle(double deltaTime);
    void checkCollisions(const QPointF& previousBallPos);
    void adaptToSize();
    bool sensorInputFresh() const;
};

} // namespace games

#endif // GAMES_PONGGAME_HPP
