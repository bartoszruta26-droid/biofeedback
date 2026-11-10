#include "games/PongGame.hpp"

#include <QBrush>
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>
#include <utility>

namespace games {

namespace {
constexpr double kSensorTimeoutMs = 350.0;
constexpr double kForceSmoothingSeconds = 0.075;
constexpr double kServeDelaySeconds = 0.65;
constexpr double kPhysicsStep = 1.0 / 120.0;
}

PongGame::PongGame(QWidget *parent)
    : GameEngine(parent)
    , m_leftPaddleY(0.0f)
    , m_rightPaddleY(0.0f)
    , m_ballPos(0.0f, 0.0f)
    , m_ballVel(0.0f, 0.0f)
    , m_ballRadius(8.0f)
    , m_playerScore(0)
    , m_aiScore(0)
    , m_difficultyLevel(1)
    , m_aiSpeed(165.0f)
    , m_ballSpeed(315.0f)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
    , m_paddleWidth(15.0f)
    , m_paddleHeight(80.0f)
    , m_paddleMargin(20.0f)
    , m_currentForce(50.0)
    , m_targetForce(50.0)
    , m_hasSensorSample(false)
    , m_serveDelayRemaining(kServeDelaySeconds)
{
    setMinimumSize(180, 120);
}

PongGame::~PongGame()
{
    disconnectSerial();
}

QString PongGame::gameName() const
{
    return QStringLiteral("Pong Force Trainer");
}

void PongGame::initialize()
{
    m_currentForce = 50.0;
    m_targetForce = 50.0;
    m_hasSensorSample = false;
    m_sensorClock.invalidate();
    m_playerScore = 0;
    m_aiScore = 0;
    m_serveDelayRemaining = kServeDelaySeconds;
    m_lastSize = QSize();
    setScore(0);
    adaptToSize();
    resetBall();
}

void PongGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    // The AI is deliberately slower than the ball at the easiest level.
    m_aiSpeed = 110.0f + static_cast<float>(m_difficultyLevel) * 55.0f;
    m_ballSpeed = 260.0f + static_cast<float>(m_difficultyLevel) * 55.0f;
    m_playerScore = 0;
    m_aiScore = 0;
    setScore(0);
    resetBall();
    m_serveDelayRemaining = kServeDelaySeconds;
    QWidget::update();
}

int PongGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
}

void PongGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }

    m_serialPort = std::move(serial);
    m_ownSerial = false;

    if (m_serialPort && m_serialPort->isConnected())
        emit serialConnected();
}

void PongGame::disconnectSerial()
{
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }
    if (m_serialPort) {
        m_serialPort.reset();
        m_ownSerial = false;
        emit serialDisconnected();
    }
}

bool PongGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

void PongGame::onSensorDataReceived(const sensor::SensorData& data)
{
    if (!data.isValid || !std::isfinite(data.calibratedValue))
        return;
    normalizeForceValue(data.calibratedValue);
}

void PongGame::onConnectionStatusChanged(bool connected, const std::string& message)
{
    if (connected) {
        qDebug() << "PongGame: Połączono z sensorem:" << QString::fromStdString(message);
        emit serialConnected();
    } else {
        qDebug() << "PongGame: Rozłączono z sensorem:" << QString::fromStdString(message);
        emit serialError(QString::fromStdString("Rozłączono: " + message));
        emit serialDisconnected();
    }
}

void PongGame::normalizeForceValue(double rawValue)
{
    // The serial sensor path reports calibrated force in the configured 0–100 range.
    acceptForcePercent(rawValue);
}

void PongGame::acceptForcePercent(double percent)
{
    if (!std::isfinite(percent))
        return;
    m_targetForce = qBound(0.0, percent, 100.0);
    m_hasSensorSample = true;
    m_sensorClock.restart();
}

bool PongGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    acceptForcePercent(value);
    return true;
}

bool PongGame::sensorInputFresh() const
{
    return m_hasSensorSample && m_sensorClock.isValid() &&
           static_cast<double>(m_sensorClock.elapsed()) <= kSensorTimeoutMs;
}

double PongGame::currentForcePercent() const
{
    return m_currentForce;
}

bool PongGame::hasFreshSensorInput() const
{
    return sensorInputFresh();
}

double PongGame::playerPaddleY() const
{
    return m_leftPaddleY;
}

double PongGame::aiPaddleY() const
{
    return m_rightPaddleY;
}

QPointF PongGame::ballPosition() const
{
    return m_ballPos;
}

int PongGame::playerMatchScore() const
{
    return m_playerScore;
}

int PongGame::aiMatchScore() const
{
    return m_aiScore;
}

void PongGame::adaptToSize()
{
    const QSize newSize = size();
    if (newSize.width() < 2 || newSize.height() < 2)
        return;

    if (m_lastSize.width() > 1 && m_lastSize.height() > 1) {
        const double sx = static_cast<double>(newSize.width()) / m_lastSize.width();
        const double sy = static_cast<double>(newSize.height()) / m_lastSize.height();
        m_ballPos.setX(m_ballPos.x() * sx);
        m_ballPos.setY(m_ballPos.y() * sy);
        m_ballVel.setX(m_ballVel.x() * sx);
        m_ballVel.setY(m_ballVel.y() * sy);
        m_leftPaddleY = static_cast<float>(m_leftPaddleY * sy);
        m_rightPaddleY = static_cast<float>(m_rightPaddleY * sy);
    } else {
        m_ballPos = QPointF(newSize.width() / 2.0, newSize.height() / 2.0);
        m_leftPaddleY = (newSize.height() - m_paddleHeight) / 2.0f;
        m_rightPaddleY = m_leftPaddleY;
    }

    m_paddleHeight = qBound(36.0f, static_cast<float>(newSize.height()) * 0.18f, 90.0f);
    m_paddleWidth = qBound(8.0f, static_cast<float>(newSize.width()) * 0.022f, 16.0f);
    m_paddleMargin = qBound(8.0f, static_cast<float>(newSize.width()) * 0.035f, 24.0f);
    m_ballRadius = qBound(4.0f, static_cast<float>(qMin(newSize.width(), newSize.height())) * 0.018f, 10.0f);

    const float maxPaddleY = qMax(0.0f, static_cast<float>(newSize.height()) - m_paddleHeight);
    m_leftPaddleY = qBound(0.0f, m_leftPaddleY, maxPaddleY);
    m_rightPaddleY = qBound(0.0f, m_rightPaddleY, maxPaddleY);

    m_leftPaddle = QRectF(m_paddleMargin, m_leftPaddleY, m_paddleWidth, m_paddleHeight);
    m_rightPaddle = QRectF(newSize.width() - m_paddleMargin - m_paddleWidth,
                           m_rightPaddleY, m_paddleWidth, m_paddleHeight);
    m_lastSize = newSize;
}

void PongGame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adaptToSize();
    QWidget::update();
}

void PongGame::resetBall(int horizontalDirection)
{
    m_ballPos = QPointF(width() / 2.0, height() / 2.0);
    const int direction = horizontalDirection == 0
        ? (QRandomGenerator::global()->bounded(2) == 0 ? -1 : 1)
        : (horizontalDirection < 0 ? -1 : 1);
    double vertical = QRandomGenerator::global()->bounded(2) == 0 ? -0.42 : 0.42;
    const double length = std::hypot(1.0, vertical);
    m_ballVel.setX(direction * m_ballSpeed / length);
    m_ballVel.setY(vertical * m_ballSpeed / length);
}

void PongGame::updateAIPaddle(double deltaTime)
{
    const float centre = (height() - m_paddleHeight) / 2.0f;
    const float targetY = m_ballVel.x() > 0.0
        ? static_cast<float>(m_ballPos.y() - m_paddleHeight / 2.0)
        : centre;
    const float deltaY = targetY - m_rightPaddleY;
    const float maxMove = m_aiSpeed * static_cast<float>(deltaTime);
    if (std::abs(deltaY) > maxMove)
        m_rightPaddleY += deltaY > 0.0f ? maxMove : -maxMove;
    else
        m_rightPaddleY = targetY;

    m_rightPaddleY = qBound(0.0f, m_rightPaddleY,
                            qMax(0.0f, static_cast<float>(height()) - m_paddleHeight));
    m_rightPaddle.moveTop(m_rightPaddleY);
}

void PongGame::checkCollisions()
{
    const float maxPaddleY = qMax(0.0f, static_cast<float>(height()) - m_paddleHeight);
    m_leftPaddleY = static_cast<float>((1.0 - m_currentForce / 100.0) * maxPaddleY);
    m_leftPaddleY = qBound(0.0f, m_leftPaddleY, maxPaddleY);
    m_leftPaddle.moveTop(m_leftPaddleY);

    if (m_ballPos.y() - m_ballRadius <= 0.0 ||
        m_ballPos.y() + m_ballRadius >= height()) {
        m_ballPos.setY(qBound(static_cast<double>(m_ballRadius), m_ballPos.y(),
                               static_cast<double>(height()) - m_ballRadius));
        m_ballVel.setY(-m_ballVel.y());
    }

    if (m_ballVel.x() < 0.0 &&
        m_ballPos.x() - m_ballRadius <= m_leftPaddle.right() &&
        m_ballPos.x() + m_ballRadius >= m_leftPaddle.left() &&
        m_ballPos.y() + m_ballRadius >= m_leftPaddle.top() &&
        m_ballPos.y() - m_ballRadius <= m_leftPaddle.bottom()) {
        const double hitOffset = (m_ballPos.y() - m_leftPaddle.center().y()) /
                                 qMax(1.0, m_paddleHeight / 2.0);
        const double angle = qBound(-0.95, hitOffset, 0.95) * 1.05;
        const double speed = qMin(std::hypot(m_ballVel.x(), m_ballVel.y()) * 1.035,
                                  static_cast<double>(m_ballSpeed) * 1.8);
        m_ballVel = QPointF(std::cos(angle) * speed, std::sin(angle) * speed);
        m_ballPos.setX(m_leftPaddle.right() + m_ballRadius);
    }

    if (m_ballVel.x() > 0.0 &&
        m_ballPos.x() + m_ballRadius >= m_rightPaddle.left() &&
        m_ballPos.x() - m_ballRadius <= m_rightPaddle.right() &&
        m_ballPos.y() + m_ballRadius >= m_rightPaddle.top() &&
        m_ballPos.y() - m_ballRadius <= m_rightPaddle.bottom()) {
        const double hitOffset = (m_ballPos.y() - m_rightPaddle.center().y()) /
                                 qMax(1.0, m_paddleHeight / 2.0);
        const double angle = qBound(-0.95, hitOffset, 0.95) * 1.05;
        const double speed = qMin(std::hypot(m_ballVel.x(), m_ballVel.y()) * 1.025,
                                  static_cast<double>(m_ballSpeed) * 1.8);
        m_ballVel = QPointF(-std::cos(angle) * speed, std::sin(angle) * speed);
        m_ballPos.setX(m_rightPaddle.left() - m_ballRadius);
    }

    // A point is awarded only after the complete ball has left the court.
    if (m_ballPos.x() + m_ballRadius < 0.0) {
        ++m_aiScore;
        resetBall(1); // Serve toward the player who conceded the point.
        m_serveDelayRemaining = kServeDelaySeconds;
    } else if (m_ballPos.x() - m_ballRadius > width()) {
        ++m_playerScore;
        addScore(10);
        resetBall(-1);
        m_serveDelayRemaining = kServeDelaySeconds;
    }
}

void PongGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    adaptToSize();
    if (!sensorInputFresh())
        return; // Freeze the rally instead of scoring against a missing sensor.

    deltaTime = qMin(deltaTime, 0.1);
    const int steps = qMax(1, static_cast<int>(std::ceil(deltaTime / kPhysicsStep)));
    const double stepTime = deltaTime / steps;

    for (int step = 0; step < steps && isRunning(); ++step) {
        const double alpha = 1.0 - std::exp(-stepTime / kForceSmoothingSeconds);
        m_currentForce += (m_targetForce - m_currentForce) * alpha;

        if (m_serveDelayRemaining > 0.0) {
            m_serveDelayRemaining = qMax(0.0, m_serveDelayRemaining - stepTime);
            const float maxPaddleY = qMax(0.0f, static_cast<float>(height()) - m_paddleHeight);
            m_leftPaddleY = static_cast<float>((1.0 - m_currentForce / 100.0) * maxPaddleY);
            m_leftPaddle.moveTop(qBound(0.0f, m_leftPaddleY, maxPaddleY));
            updateAIPaddle(stepTime);
            continue;
        }

        m_ballPos += m_ballVel * stepTime;
        updateAIPaddle(stepTime);
        checkCollisions();
    }
}

QColor PongGame::getDifficultyColor() const
{
    switch (m_difficultyLevel) {
        case 1: return QColor(75, 205, 130);
        case 2: return QColor(65, 190, 220);
        case 3: return QColor(235, 205, 75);
        case 4: return QColor(235, 145, 65);
        default: return QColor(230, 85, 85);
    }
}

void PongGame::render(QPainter &painter)
{
    adaptToSize();
    const int w = width();
    const int h = height();
    painter.fillRect(rect(), QColor(10, 16, 30));
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor(85, 105, 135), 1, Qt::DashLine));
    painter.drawLine(w / 2, 8, w / 2, h - 8);

    painter.setPen(Qt::NoPen);
    painter.setBrush(getDifficultyColor());
    painter.drawRoundedRect(m_leftPaddle, 4, 4);
    painter.setBrush(QColor(235, 100, 100));
    painter.drawRoundedRect(m_rightPaddle, 4, 4);

    painter.setBrush(Qt::white);
    painter.drawEllipse(m_ballPos, m_ballRadius, m_ballRadius);

    const int scoreFont = qBound(10, h / 17, 22);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), scoreFont, QFont::DemiBold));
    painter.drawText(QRect(8, 5, w / 2 - 16, scoreFont + 8), Qt::AlignHCenter,
                     QString::number(m_playerScore));
    painter.drawText(QRect(w / 2 + 8, 5, w / 2 - 16, scoreFont + 8), Qt::AlignHCenter,
                     QString::number(m_aiScore));

    const int hudFont = qBound(7, h / 35, 11);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), hudFont));
    painter.setPen(QColor(205, 215, 230));
    const QString status = !sensorInputFresh()
        ? QStringLiteral("Oczekiwanie na sygnał z czujnika")
        : (m_serveDelayRemaining > 0.0
            ? QStringLiteral("Przygotuj się do odbicia")
            : QStringLiteral("Siła: %1%  |  Poziom: %2")
                .arg(qRound(m_currentForce)).arg(m_difficultyLevel));
    painter.drawText(QRect(8, h - hudFont - 8, w - 16, hudFont + 6),
                     Qt::AlignHCenter | Qt::AlignVCenter, status);

    if (sensorInputFresh()) {
        const int meterWidth = qBound(35, w / 5, 90);
        const int meterHeight = 5;
        const QRect meterRect((w - meterWidth) / 2, h - hudFont - 17, meterWidth, meterHeight);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(65, 75, 95));
        painter.drawRoundedRect(meterRect, 2, 2);
        painter.setBrush(getDifficultyColor());
        painter.drawRoundedRect(QRect(meterRect.x(), meterRect.y(),
            qRound(meterWidth * m_currentForce / 100.0), meterHeight), 2, 2);
    }
}

} // namespace games
