#include "games/ArkanoidGame.hpp"

#include <QBrush>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <utility>

namespace games {

namespace {
constexpr double kSensorTimeoutMs = 500.0;
constexpr double kForceSmoothingSeconds = 0.065;
constexpr double kServeDelaySeconds = 0.85;
constexpr double kMaximumDelta = 0.1;
constexpr double kPhysicsStep = 1.0 / 120.0;
}

ArkanoidGame::ArkanoidGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.5)
    , m_targetForce(0.5)
    , m_ballRadius(8.0)
    , m_ballSpeed(280.0)
    , m_paddleWidth(130.0)
    , m_paddleHeight(14.0)
    , m_serveDelayRemaining(kServeDelaySeconds)
    , m_ballLaunched(false)
    , m_hasSensorSample(false)
    , m_difficultyLevel(1)
    , m_level(1)
    , m_lives(3)
{
    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
}

QString ArkanoidGame::gameName() const
{
    return QStringLiteral("Arkanoid Force Trainer");
}

void ArkanoidGame::initialize()
{
    m_level = 1;
    m_lives = 3;
    m_currentForce = 0.5;
    m_targetForce = 0.5;
    m_hasSensorSample = false;
    m_sensorClock.invalidate();
    m_ballLaunched = false;
    m_serveDelayRemaining = kServeDelaySeconds;
    m_lastSize = QSize();

    setDifficultyLevel(m_difficultyLevel);
    adaptToSize();
    buildLevel();
    resetBall();
}

void ArkanoidGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    m_ballSpeed = 230.0 + m_difficultyLevel * 38.0;
    m_paddleWidth = 160.0 - m_difficultyLevel * 14.0;

    if (m_paddle.width() > 0.0) {
        adaptToSize();
        m_paddle.setWidth(qMin(m_paddleWidth,
            qMax(40.0, static_cast<double>(width()) - 24.0)));
        m_paddle.moveLeft(qBound(12.0, m_paddle.left(),
            qMax(12.0, static_cast<double>(width()) - 12.0 - m_paddle.width())));
        if (!m_ballLaunched)
            resetBall();
    }

    if (isRunning()) {
        m_level = 1;
        m_lives = 3;
        setScore(0);
        buildLevel();
        resetBall();
        m_ballLaunched = false;
        m_serveDelayRemaining = kServeDelaySeconds;
    }
    QWidget::update();
}

int ArkanoidGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
}

void ArkanoidGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    m_serialPort = std::move(serial);
}

void ArkanoidGame::disconnectSerial()
{
    m_serialPort.reset();
}

bool ArkanoidGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

double ArkanoidGame::currentForcePercent() const
{
    return m_currentForce * 100.0;
}

bool ArkanoidGame::hasFreshSensorInput() const
{
    return sensorInputFresh();
}

double ArkanoidGame::paddleX() const
{
    return m_paddle.left();
}

QPointF ArkanoidGame::ballPosition() const
{
    return m_ballPosition;
}

int ArkanoidGame::livesRemaining() const
{
    return m_lives;
}

int ArkanoidGame::currentLevel() const
{
    return m_level;
}

int ArkanoidGame::activeBrickCount() const
{
    return activeBrickCountInternal();
}

bool ArkanoidGame::isBallLaunched() const
{
    return m_ballLaunched;
}

bool ArkanoidGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    // TrainingTab forwards a normalized target-force percentage (0–100).
    m_targetForce = qBound(MIN_FORCE, value, MAX_FORCE) / MAX_FORCE;
    m_hasSensorSample = true;
    m_sensorClock.restart();
    return true;
}

bool ArkanoidGame::sensorInputFresh() const
{
    return m_hasSensorSample && m_sensorClock.isValid() &&
           static_cast<double>(m_sensorClock.elapsed()) <= kSensorTimeoutMs;
}

void ArkanoidGame::buildLevel()
{
    m_bricks.clear();
    if (width() <= 0 || height() <= 0)
        return;

    const int columns = qBound(4, width() / 72, 10);
    const int rows = qBound(3, 3 + (m_level - 1) / 2, 7);
    const double margin = qBound(10.0, width() * 0.045, 28.0);
    const double gap = qBound(3.0, width() * 0.009, 7.0);
    const double top = qBound(52.0, height() * 0.14, 76.0);
    const double brickHeight = qBound(12.0, height() * 0.042, 22.0);
    const double availableWidth = qMax(1.0, width() - 2.0 * margin -
        gap * (columns - 1));
    const double brickWidth = availableWidth / columns;

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Brick brick;
            brick.rect = QRectF(margin + column * (brickWidth + gap),
                top + row * (brickHeight + gap), brickWidth, brickHeight);
            brick.row = row;
            brick.hits = (m_level >= 3 && row < qMin(2, rows / 2)) ? 2 : 1;
            brick.active = true;
            m_bricks.push_back(brick);
        }
    }
}

void ArkanoidGame::resetBall()
{
    if (m_paddle.width() <= 0.0)
        return;
    m_ballPosition = QPointF(m_paddle.center().x(),
        m_paddle.top() - m_ballRadius - 1.0);
    const double horizontal = m_ballSpeed * 0.28;
    const double vertical = std::sqrt(qMax(0.0,
        m_ballSpeed * m_ballSpeed - horizontal * horizontal));
    m_ballVelocity = QPointF(horizontal, -vertical);
    m_ballLaunched = false;
}

void ArkanoidGame::launchBall()
{
    if (!isRunning() || !sensorInputFresh() || m_ballLaunched)
        return;
    m_ballLaunched = true;
    m_serveDelayRemaining = 0.0;
}

bool ArkanoidGame::hasActiveBricks() const
{
    return activeBrickCountInternal() > 0;
}

int ArkanoidGame::activeBrickCountInternal() const
{
    return static_cast<int>(std::count_if(m_bricks.cbegin(), m_bricks.cend(),
        [](const Brick& brick) { return brick.active; }));
}

void ArkanoidGame::advanceLevel()
{
    ++m_level;
    m_ballSpeed = qMin(m_ballSpeed * 1.055, 760.0);
    addScore(100);
    buildLevel();
    resetBall();
    m_serveDelayRemaining = kServeDelaySeconds;
}

void ArkanoidGame::loseLife()
{
    --m_lives;
    if (m_lives <= 0) {
        stop();
        return;
    }
    resetBall();
    m_serveDelayRemaining = kServeDelaySeconds;
}

void ArkanoidGame::adaptToSize()
{
    const QSize newSize = size();
    if (newSize.width() < 2 || newSize.height() < 2)
        return;

    const double paddleWidth = qMin(m_paddleWidth,
        qMax(48.0, static_cast<double>(newSize.width()) - 24.0));
    const double paddleHeight = qBound(10.0, newSize.height() * 0.032, 18.0);
    const double ballRadius = qBound(5.0,
        qMin(newSize.width(), newSize.height()) * 0.018, 10.0);

    if (m_lastSize.width() > 1 && m_lastSize.height() > 1 &&
        m_lastSize != newSize) {
        const double sx = static_cast<double>(newSize.width()) / m_lastSize.width();
        const double sy = static_cast<double>(newSize.height()) / m_lastSize.height();
        m_ballPosition = QPointF(m_ballPosition.x() * sx, m_ballPosition.y() * sy);
        m_ballVelocity = QPointF(m_ballVelocity.x() * sx, m_ballVelocity.y() * sy);
        const double resizedSpeed = std::hypot(m_ballVelocity.x(), m_ballVelocity.y());
        if (resizedSpeed > 1e-6) {
            m_ballVelocity *= m_ballSpeed / resizedSpeed;
        }
        m_paddle = QRectF(m_paddle.x() * sx, m_paddle.y() * sy,
                          paddleWidth, paddleHeight);
        for (Brick& brick : m_bricks) {
            brick.rect = QRectF(brick.rect.x() * sx, brick.rect.y() * sy,
                                brick.rect.width() * sx, brick.rect.height() * sy);
        }
    } else if (m_paddle.width() <= 0.0) {
        m_paddle = QRectF((newSize.width() - paddleWidth) / 2.0,
            newSize.height() - qMax(34.0, newSize.height() * 0.09),
            paddleWidth, paddleHeight);
    }

    // Keep the difficulty-derived preferred width separate from the
    // viewport-clamped width, so enlarging the window restores paddle size.
    m_paddleHeight = paddleHeight;
    m_ballRadius = ballRadius;

    const double minX = 12.0;
    const double maxX = qMax(minX, newSize.width() - 12.0 - m_paddle.width());
    m_paddle.setWidth(paddleWidth);
    m_paddle.setHeight(paddleHeight);
    m_paddle.moveLeft(qBound(minX, m_paddle.left(), maxX));
    m_paddle.moveTop(qBound(24.0, m_paddle.top(),
        qMax(24.0, newSize.height() - paddleHeight - 12.0)));

    m_ballPosition.setX(qBound(m_ballRadius,
        m_ballPosition.x(), newSize.width() - m_ballRadius));
    m_ballPosition.setY(qBound(m_ballRadius,
        m_ballPosition.y(), newSize.height() - m_ballRadius));

    m_lastSize = newSize;
    if (!m_ballLaunched)
        m_ballPosition = QPointF(m_paddle.center().x(),
            m_paddle.top() - m_ballRadius - 1.0);
}

void ArkanoidGame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adaptToSize();
    QWidget::update();
}

void ArkanoidGame::updatePaddle(double deltaTime)
{
    const double maxX = qMax(12.0,
        static_cast<double>(width()) - 12.0 - m_paddle.width());
    const double desiredX = 12.0 + m_currentForce *
        qMax(0.0, static_cast<double>(width()) - 24.0 - m_paddle.width());
    const double alpha = 1.0 - std::exp(-deltaTime / 0.025);
    m_paddle.moveLeft(qBound(12.0,
        m_paddle.left() + (desiredX - m_paddle.left()) * alpha, maxX));
    if (!m_ballLaunched)
        m_ballPosition = QPointF(m_paddle.center().x(),
            m_paddle.top() - m_ballRadius - 1.0);
}

void ArkanoidGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0 ||
        width() <= 0 || height() <= 0)
        return;

    adaptToSize();

    // A stale sensor must not let the paddle drift or cause unobserved losses.
    if (!sensorInputFresh())
        return;

    deltaTime = qMin(deltaTime, kMaximumDelta);
    const int steps = qBound(1,
        static_cast<int>(std::ceil(deltaTime / kPhysicsStep)), 12);
    const double stepTime = deltaTime / steps;

    for (int step = 0; step < steps && isRunning(); ++step) {
        const double alpha = 1.0 - std::exp(-stepTime / kForceSmoothingSeconds);
        m_currentForce += (m_targetForce - m_currentForce) * alpha;
        updatePaddle(stepTime);

        if (!m_ballLaunched) {
            m_serveDelayRemaining = qMax(0.0, m_serveDelayRemaining - stepTime);
            if (m_serveDelayRemaining <= 0.0)
                launchBall();
            continue;
        }

        updatePhysicsStep(stepTime);
    }
}

void ArkanoidGame::updatePhysicsStep(double deltaTime)
{
    if (!isRunning() || !m_ballLaunched)
        return;

    const QPointF previous = m_ballPosition;
    m_ballPosition += m_ballVelocity * deltaTime;

    // Side and ceiling walls.
    if (m_ballPosition.x() - m_ballRadius < 0.0) {
        m_ballPosition.setX(m_ballRadius);
        m_ballVelocity.setX(std::abs(m_ballVelocity.x()));
    } else if (m_ballPosition.x() + m_ballRadius > width()) {
        m_ballPosition.setX(width() - m_ballRadius);
        m_ballVelocity.setX(-std::abs(m_ballVelocity.x()));
    }
    if (m_ballPosition.y() - m_ballRadius < 0.0) {
        m_ballPosition.setY(m_ballRadius);
        m_ballVelocity.setY(std::abs(m_ballVelocity.y()));
    }

    // Misses are counted only after the whole ball has left the playfield.
    if (m_ballPosition.y() - m_ballRadius > height()) {
        loseLife();
        return;
    }

    // Paddle collision: require downward travel and test the entire ball bounds.
    const QRectF ballBounds(m_ballPosition.x() - m_ballRadius,
        m_ballPosition.y() - m_ballRadius,
        2.0 * m_ballRadius, 2.0 * m_ballRadius);
    if (m_ballVelocity.y() > 0.0 &&
        previous.y() + m_ballRadius <= m_paddle.top() &&
        ballBounds.intersects(m_paddle)) {
        const double halfWidth = qMax(1.0, m_paddle.width() / 2.0);
        const double offset = qBound(-1.0,
            (m_ballPosition.x() - m_paddle.center().x()) / halfWidth, 1.0);
        const double speed = qMin(m_ballSpeed * 1.65,
            std::hypot(m_ballVelocity.x(), m_ballVelocity.y()) * 1.015);
        const double angle = offset * 1.05;
        m_ballVelocity = QPointF(std::sin(angle) * speed,
                                 -std::abs(std::cos(angle) * speed));
        m_ballPosition.setY(m_paddle.top() - m_ballRadius - 0.2);
    }

    // Rebuild bounds after paddle resolution; use previous position to choose
    // the collision face and push the ball out to prevent repeated scoring.
    const QRectF currentBounds(m_ballPosition.x() - m_ballRadius,
        m_ballPosition.y() - m_ballRadius,
        2.0 * m_ballRadius, 2.0 * m_ballRadius);

    for (Brick& brick : m_bricks) {
        if (!brick.active || !currentBounds.intersects(brick.rect))
            continue;

        --brick.hits;
        if (brick.hits <= 0) {
            brick.active = false;
            addScore(10);
        } else {
            addScore(5);
        }

        const double previousBottom = previous.y() + m_ballRadius;
        const double previousTop = previous.y() - m_ballRadius;
        const double previousRight = previous.x() + m_ballRadius;
        const double previousLeft = previous.x() - m_ballRadius;
        const double overlapTop = std::abs(previousBottom - brick.rect.top());
        const double overlapBottom = std::abs(brick.rect.bottom() - previousTop);
        const double overlapLeft = std::abs(previousRight - brick.rect.left());
        const double overlapRight = std::abs(brick.rect.right() - previousLeft);
        const double minOverlap = qMin(qMin(overlapTop, overlapBottom),
                                       qMin(overlapLeft, overlapRight));

        if (minOverlap == overlapTop || minOverlap == overlapBottom) {
            m_ballVelocity.setY(-m_ballVelocity.y());
            m_ballPosition.setY(minOverlap == overlapTop
                ? brick.rect.top() - m_ballRadius - 0.2
                : brick.rect.bottom() + m_ballRadius + 0.2);
        } else {
            m_ballVelocity.setX(-m_ballVelocity.x());
            m_ballPosition.setX(minOverlap == overlapLeft
                ? brick.rect.left() - m_ballRadius - 0.2
                : brick.rect.right() + m_ballRadius + 0.2);
        }

        if (!hasActiveBricks())
            advanceLevel();
        break;
    }
}

QColor ArkanoidGame::brickColor(const Brick& brick) const
{
    if (brick.hits > 1)
        return QColor(245, 90, 90);
    switch (brick.row % 5) {
        case 0: return QColor(245, 110, 110);
        case 1: return QColor(245, 170, 80);
        case 2: return QColor(235, 215, 90);
        case 3: return QColor(100, 200, 130);
        default: return QColor(90, 160, 230);
    }
}

void ArkanoidGame::drawHud(QPainter& painter)
{
    const int fontSize = qBound(10, height() / 35, 15);
    painter.setPen(QColor(235, 240, 250));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), fontSize, QFont::DemiBold));
    painter.drawText(QRect(12, 8, width() / 3, fontSize + 10),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(QRect(width() / 3, 8, width() / 3, fontSize + 10),
        Qt::AlignHCenter | Qt::AlignVCenter,
        QStringLiteral("Poziom: %1").arg(m_level));
    painter.drawText(QRect(width() * 2 / 3, 8, width() / 3 - 12, fontSize + 10),
        Qt::AlignRight | Qt::AlignVCenter,
        QStringLiteral("Życia: %1").arg(m_lives));

    const int meterWidth = qMin(120, qMax(48, width() / 5));
    const QRect meter(width() - meterWidth - 12, height() - 14,
                      meterWidth, 5);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(55, 65, 85));
    painter.drawRoundedRect(meter, 2, 2);
    painter.setBrush(QColor(75, 195, 220));
    painter.drawRoundedRect(QRect(meter.x(), meter.y(),
        qRound(meter.width() * m_currentForce), meter.height()), 2, 2);

    painter.setPen(QColor(205, 215, 230));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(8, fontSize - 2, 12)));
    QString status;
    if (!sensorInputFresh())
        status = QStringLiteral("Oczekiwanie na sygnał tensometru");
    else if (!m_ballLaunched)
        status = QStringLiteral("Przygotowanie serwisu — Spacja / kliknięcie");
    else
        status = QStringLiteral("Siła: %1%  |  Trudność: %2/5")
            .arg(qRound(m_currentForce * 100.0)).arg(m_difficultyLevel);
    painter.drawText(QRect(12, height() - fontSize - 18,
        width() - meterWidth - 32, fontSize + 8),
        Qt::AlignLeft | Qt::AlignVCenter, status);
}

void ArkanoidGame::render(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(12, 18, 32));

    painter.setPen(QPen(QColor(45, 60, 82), 1));
    painter.drawLine(8, 38, width() - 8, 38);

    for (const Brick& brick : m_bricks) {
        if (!brick.active)
            continue;
        painter.setBrush(brickColor(brick));
        painter.setPen(QPen(QColor(20, 25, 40), 1));
        painter.drawRoundedRect(brick.rect, 3.0, 3.0);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(90, 210, 235));
    painter.drawRoundedRect(m_paddle, 6.0, 6.0);

    painter.setBrush(Qt::white);
    painter.drawEllipse(m_ballPosition, m_ballRadius, m_ballRadius);
    drawHud(painter);
}

void ArkanoidGame::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter) {
        launchBall();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_P) {
        if (isRunning() && !isPaused())
            pause();
        else if (isRunning() && isPaused())
            resume();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void ArkanoidGame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        launchBall();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

} // namespace games
