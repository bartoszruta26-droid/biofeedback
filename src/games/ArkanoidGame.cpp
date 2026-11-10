#include "games/ArkanoidGame.hpp"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <utility>

namespace games {

ArkanoidGame::ArkanoidGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.5)
    , m_ballRadius(8.0)
    , m_ballSpeed(280.0)
    , m_paddleWidth(130.0)
    , m_paddleHeight(14.0)
    , m_difficultyLevel(1)
    , m_level(1)
    , m_lives(3)
{
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
    setDifficultyLevel(m_difficultyLevel);

    const double availableWidth = qMax(40.0, static_cast<double>(width()) - 20.0);
    m_paddleWidth = qMin(m_paddleWidth, availableWidth);
    m_paddle = QRectF((width() - m_paddleWidth) / 2.0,
                      height() - 42.0, m_paddleWidth, m_paddleHeight);

    buildLevel();
    resetBall();
}

void ArkanoidGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    m_ballSpeed = 240.0 + m_difficultyLevel * 40.0;
    m_paddleWidth = 154.0 - m_difficultyLevel * 12.0;
    setScore(0);

    if (!m_paddle.isNull()) {
        const double availableWidth = qMax(40.0, static_cast<double>(width()) - 20.0);
        m_paddle.setWidth(qMin(m_paddleWidth, availableWidth));
        m_paddle.moveLeft(qBound(10.0, m_paddle.left(),
                                 qMax(10.0, static_cast<double>(width()) - 10.0 - m_paddle.width())));
    }

    if (isRunning()) {
        m_level = 1;
        m_lives = 3;
        buildLevel();
        resetBall();
    }
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

bool ArkanoidGame::handleInput(double value)
{
    const double normalized = value / MAX_FORCE;
    m_currentForce = qBound(MIN_FORCE, normalized, 1.0);
    return true;
}

void ArkanoidGame::buildLevel()
{
    m_bricks.clear();
    if (width() <= 0 || height() <= 120)
        return;

    constexpr int columns = 8;
    const int rows = qBound(4, 4 + (m_level - 1) / 2, 7);
    const double margin = 24.0;
    const double gap = 6.0;
    const double brickHeight = 18.0;
    const double totalGap = gap * (columns - 1);
    const double brickWidth = qMax(12.0,
        (width() - 2.0 * margin - totalGap) / columns);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            Brick brick;
            brick.rect = QRectF(margin + column * (brickWidth + gap),
                                58.0 + row * (brickHeight + gap),
                                brickWidth, brickHeight);
            brick.row = row;
            brick.hits = (m_level >= 3 && row == 0) ? 2 : 1;
            brick.active = true;
            m_bricks.push_back(brick);
        }
    }
}

void ArkanoidGame::resetBall()
{
    m_ballPosition = QPointF(m_paddle.center().x(),
                             m_paddle.top() - m_ballRadius - 1.0);
    const double horizontal = m_ballSpeed * 0.38;
    const double vertical = std::sqrt(qMax(0.0,
        m_ballSpeed * m_ballSpeed - horizontal * horizontal));
    m_ballVelocity = QPointF(horizontal, -vertical);
}

bool ArkanoidGame::hasActiveBricks() const
{
    return std::any_of(m_bricks.cbegin(), m_bricks.cend(),
                       [](const Brick& brick) { return brick.active; });
}

void ArkanoidGame::advanceLevel()
{
    ++m_level;
    m_ballSpeed = qMin(m_ballSpeed * 1.06, 800.0);
    addScore(100);
    buildLevel();
    resetBall();
}

void ArkanoidGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0 ||
        width() <= 0 || height() <= 0)
        return;

    // Match the engine's bounded catch-up policy for defensive direct calls.
    // At most twelve 1/120 s physics steps are executed per update.
    deltaTime = qMin(deltaTime, 0.1);
    constexpr double maxPhysicsStep = 1.0 / 120.0;
    const int steps = qBound(1, static_cast<int>(std::ceil(deltaTime / maxPhysicsStep)), 12);
    const double stepTime = deltaTime / static_cast<double>(steps);
    for (int step = 0; step < steps && isRunning(); ++step)
        updatePhysicsStep(stepTime);
}

void ArkanoidGame::updatePhysicsStep(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0)
        return;

    const double maxPaddleX = qMax(10.0,
        static_cast<double>(width()) - 10.0 - m_paddle.width());
    const double desiredPaddleX = qBound(10.0, 10.0 +
        m_currentForce * qMax(0.0, static_cast<double>(width()) - 20.0 - m_paddle.width()),
        maxPaddleX);
    // Interpolate between 80 SPS sensor updates at the 120 Hz game cadence.
    // A short 25 ms time constant removes visible stepping without a long control lag.
    const double paddleResponse = 1.0 - std::exp(-deltaTime / 0.025);
    const double smoothedPaddleX = m_paddle.left() +
        (desiredPaddleX - m_paddle.left()) * paddleResponse;
    m_paddle.moveLeft(qBound(10.0, smoothedPaddleX, maxPaddleX));

    const QPointF previousPosition = m_ballPosition;
    m_ballPosition += m_ballVelocity * deltaTime;

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

    if (m_ballPosition.y() - m_ballRadius > height()) {
        --m_lives;
        if (m_lives <= 0) {
            stop();
            return;
        }
        resetBall();
        return;
    }

    if (m_ballVelocity.y() > 0.0 &&
        previousPosition.y() + m_ballRadius <= m_paddle.top() &&
        QRectF(m_ballPosition.x() - m_ballRadius,
               m_ballPosition.y() - m_ballRadius,
               2.0 * m_ballRadius, 2.0 * m_ballRadius).intersects(m_paddle)) {
        const double halfWidth = qMax(1.0, m_paddle.width() / 2.0);
        const double hitOffset = qBound(-1.0,
            (m_ballPosition.x() - m_paddle.center().x()) / halfWidth, 1.0);
        const double speed = std::hypot(m_ballVelocity.x(), m_ballVelocity.y());
        const double angle = hitOffset * 0.95;
        m_ballVelocity.setX(std::sin(angle) * speed);
        m_ballVelocity.setY(-std::abs(std::cos(angle) * speed));
        m_ballPosition.setY(m_paddle.top() - m_ballRadius - 0.5);
    }

    const QRectF ballBounds(m_ballPosition.x() - m_ballRadius,
                            m_ballPosition.y() - m_ballRadius,
                            2.0 * m_ballRadius, 2.0 * m_ballRadius);
    for (Brick& brick : m_bricks) {
        if (!brick.active || !ballBounds.intersects(brick.rect))
            continue;

        --brick.hits;
        if (brick.hits <= 0) {
            brick.active = false;
            addScore(10);
        } else {
            addScore(5);
        }

        const bool hitVerticalFace =
            previousPosition.y() + m_ballRadius <= brick.rect.top() ||
            previousPosition.y() - m_ballRadius >= brick.rect.bottom();
        // Wypchnij piłkę poza klocek po odbiciu, aby ten sam kontakt
        // nie został policzony ponownie w kolejnych krokach symulacji.
        if (hitVerticalFace) {
            if (previousPosition.y() <= brick.rect.top())
                m_ballPosition.setY(brick.rect.top() - m_ballRadius - 0.1);
            else
                m_ballPosition.setY(brick.rect.bottom() + m_ballRadius + 0.1);
            m_ballVelocity.setY(-m_ballVelocity.y());
        } else {
            if (previousPosition.x() <= brick.rect.left())
                m_ballPosition.setX(brick.rect.left() - m_ballRadius - 0.1);
            else
                m_ballPosition.setX(brick.rect.right() + m_ballRadius + 0.1);
            m_ballVelocity.setX(-m_ballVelocity.x());
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

void ArkanoidGame::render(QPainter& painter)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(12, 18, 32));

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

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(18, 28, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(18, 50, QStringLiteral("Poziom: %1").arg(m_level));
    painter.drawText(width() - 130, 28, QStringLiteral("Życia: %1").arg(m_lives));

    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(18, height() - 12,
                     QStringLiteral("Steruj położeniem paletki siłą nacisku"));
    painter.drawText(width() - 150, height() - 12,
                     QStringLiteral("Trudność: %1").arg(m_difficultyLevel));

    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(Qt::red);
        painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter,
                         QStringLiteral("BRAK POŁĄCZENIA Z CZUJNIKIEM"));
    }
}

} // namespace games
