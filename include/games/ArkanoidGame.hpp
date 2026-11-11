#ifndef GAMES_ARKANOIDGAME_HPP
#define GAMES_ARKANOIDGAME_HPP
#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QColor>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>
#include <vector>
class QResizeEvent;
namespace games {
class ArkanoidGame final : public GameEngine {
 Q_OBJECT
public:
 explicit ArkanoidGame(QWidget *parent=nullptr);
 ~ArkanoidGame() override=default;
 QString gameName() const override;
 void setDifficultyLevel(int level);
 int getDifficultyLevel() const;
 double paddlePositionX() const;
 int comboCount() const;
 void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
 void disconnectSerial();
 bool isSerialConnected() const;
protected:
 void initialize() override;
 void update(double deltaTime) override;
 void render(QPainter& painter) override;
 bool handleInput(double value) override;
 void resizeEvent(QResizeEvent *event) override;
 void renderInterpolated(QPainter& painter, double alpha) override;
private:
 struct Brick{QRectF rect;int hits{1};bool active{true};int row{0};};
 void updatePhysicsStep(double deltaTime);
 void buildLevel();
 void resetBall();
 void advanceLevel();
 bool hasActiveBricks() const;
 QColor brickColor(const Brick& brick) const;
 QRectF m_paddle;
 QPointF m_ballPosition;
 QPointF m_previousBallPosition;
 QPointF m_ballVelocity;
 QRectF m_previousPaddle;
 QVector<QPointF> m_ballTrail;
 std::vector<Brick> m_bricks;
 double m_currentForce,m_ballRadius,m_ballSpeed,m_paddleWidth,m_paddleHeight,m_comboRemaining;
 int m_difficultyLevel,m_level,m_lives,m_combo;

 std::shared_ptr<sensor::SerialCommunication> m_serialPort;
 static constexpr double MIN_FORCE=0.0;
 static constexpr double MAX_FORCE=100.0;
};
}
#endif
