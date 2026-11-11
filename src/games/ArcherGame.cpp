#include "games/ArcherGame.hpp"
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {

ArcherGame::ArcherGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_aimAngle(-0.2)
    , m_targetDirection(1.0)
    , m_targetSpeed(55.0)
    , m_gravity(260.0)
    , m_fireCooldown(2.4)
{
    setFocusPolicy(Qt::NoFocus);
}

QString ArcherGame::gameName() const
{
    return QStringLiteral("Archer Force Trainer");
}

int ArcherGame::activeArrowCount() const
{
    return static_cast<int>(std::count_if(m_arrows.cbegin(), m_arrows.cend(),
        [](const Arrow& arrow) { return arrow.active; }));
}

void ArcherGame::initialize()
{
    m_arrows.clear();
    m_force = 0.5;
    m_aimAngle = -0.2;
    m_targetDirection = 1.0;
    const double scaleY = qMax(0.25, height() / 480.0);
    m_targetSpeed = 55.0 * scaleY;
    m_gravity = 260.0 * scaleY;
    m_fireCooldown = m_autoFireInterval;
    m_sensorClock.invalidate();
    m_hasSensorSample = false;
    m_shotsFired = 0;
    m_successfulHits = 0;
    m_missedShots = 0;
    repositionTarget();
    m_previousTarget = m_target;
    m_previousArrowPositions.clear();
}

void ArcherGame::resizeEvent(QResizeEvent* event)
{
    GameEngine::resizeEvent(event);
    if (width() > 0 && height() > 0) {
        const double scaleY = qMax(0.25, height() / 480.0);
        m_targetSpeed = 55.0 * scaleY;
        m_gravity = 260.0 * scaleY;
        repositionTarget();
        m_previousTarget = m_target;
        m_previousArrowPositions.clear();
        for (Arrow& arrow : m_arrows) {
            if (arrow.position.x() > width() || arrow.position.y() > height())
                arrow.active = false;
        }
        m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
            [](const Arrow& arrow) { return !arrow.active; }), m_arrows.end());
    }
}

void ArcherGame::repositionTarget()
{
    const double targetWidth = qMin(qMax(24.0, width() * 0.055),
                                    qMax(24.0, width() * 0.18));
    const double targetHeight = qMin(qMax(54.0, height() * 0.18),
                                     qMax(24.0, height() * 0.30));
    const double left = qBound(24.0, width() * 0.76,
                               qMax(24.0, width() - targetWidth - 12.0));
    const double top = qBound(height() * 0.18, height() * 0.42,
                              qMax(height() * 0.18,
                                   height() * 0.78 - targetHeight));
    m_target = QRectF(left, top, targetWidth, targetHeight);
}

bool ArcherGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    m_force = qBound(0.0, value / 100.0, 1.0);
    m_sensorClock.restart();
    m_hasSensorSample = true;
    m_aimAngle = 0.55 - m_force * 1.25;
    return true;
}

void ArcherGame::fireArrow()
{
    if (!isRunning() || m_fireCooldown > 0.0)
        return;
    const double scaleX = qMax(0.25, width() / 640.0);
    const double scaleY = qMax(0.25, height() / 480.0);
    const QPointF origin(width() * 0.11, height() * 0.85);
    const double speed = 780.0;
    Arrow arrow;
    arrow.position = origin;
    arrow.velocity = QPointF(std::cos(m_aimAngle) * speed * scaleX,
                             std::sin(m_aimAngle) * speed * scaleY);
    arrow.active = true;
    if (m_arrows.size() >= 8) {
        const auto oldest = std::find_if(m_arrows.begin(), m_arrows.end(),
            [](const Arrow& candidate) { return candidate.active; });
        if (oldest != m_arrows.end()) { oldest->active = false; ++m_missedShots; }
        m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
            [](const Arrow& candidate) { return !candidate.active; }), m_arrows.end());
    }
    m_arrows.push_back(arrow);
    ++m_shotsFired;
    m_fireCooldown = m_autoFireInterval;
}

void ArcherGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);
    m_previousTarget = m_target;
    m_previousArrowPositions.clear();
    m_previousArrowPositions.reserve(m_arrows.size());
    for (const Arrow& arrow : m_arrows)
        m_previousArrowPositions.push_back(arrow.position);

    const bool sensorFresh = m_hasSensorSample && m_sensorClock.isValid() &&
                              m_sensorClock.elapsed() <= 350;
    if (sensorFresh) {
        m_fireCooldown -= deltaTime;
        if (m_fireCooldown <= 0.0) {
            fireArrow();
            m_fireCooldown = m_autoFireInterval;
        }
    } else {
        m_fireCooldown = m_autoFireInterval;
    }
    m_target.translate(0.0, m_targetDirection * m_targetSpeed * deltaTime);
    if (m_target.top() < height() * 0.18) {
        m_target.moveTop(height() * 0.18);
        m_targetDirection = 1.0;
    } else if (m_target.bottom() > height() * 0.78) {
        m_target.moveBottom(height() * 0.78);
        m_targetDirection = -1.0;
    }

    for (Arrow& arrow : m_arrows) {
        if (!arrow.active)
            continue;
        const QPointF previous = arrow.position;
        arrow.velocity.ry() += m_gravity * deltaTime;
        arrow.position += arrow.velocity * deltaTime;

        // Segment sampling avoids skipping the target at high arrow velocity.
        const QPointF segment = arrow.position - previous;
        const int samples = qMax(1, static_cast<int>(std::ceil(
            std::hypot(segment.x(), segment.y()) / 6.0)));
        bool hit = false;
        for (int sample = 1; sample <= samples; ++sample) {
            const QPointF point = previous + segment * (static_cast<double>(sample) / samples);
            const double rx = qMax(1.0, m_target.width() * 0.5);
            const double ry = qMax(1.0, m_target.height() * 0.5);
            const double nx = (point.x() - m_target.center().x()) / rx;
            const double ny = (point.y() - m_target.center().y()) / ry;
            if (nx * nx + ny * ny <= 1.0) { hit = true; break; }
        }
        if (hit) {
            arrow.active = false;
            const double dx = (arrow.position.x() - m_target.center().x()) / qMax(1.0, m_target.width() * 0.5);
            const double dy = (arrow.position.y() - m_target.center().y()) / qMax(1.0, m_target.height() * 0.5);
            const double radial = std::sqrt(dx * dx + dy * dy);
            addScore(radial <= 0.22 ? 30 : (radial <= 0.55 ? 20 : 10));
            ++m_successfulHits;
            const double targetScaleY = qMax(0.25, height() / 480.0);
            m_targetSpeed = qMin(150.0 * targetScaleY, m_targetSpeed + 4.0 * targetScaleY);
            repositionTarget();
            continue;
        }
        if (arrow.position.x() > width() || arrow.position.y() > height() ||
            arrow.position.y() < -20.0) { arrow.active = false; ++m_missedShots; }
    }

    m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
        [](const Arrow& arrow) { return !arrow.active; }), m_arrows.end());
}

void ArcherGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const QRectF currentTarget = m_target;
    m_target = QRectF(
        m_previousTarget.x() + (currentTarget.x() - m_previousTarget.x()) * alpha,
        m_previousTarget.y() + (currentTarget.y() - m_previousTarget.y()) * alpha,
        currentTarget.width(), currentTarget.height());

    QVector<QPointF> currentArrowPositions;
    currentArrowPositions.reserve(m_arrows.size());
    for (const Arrow& arrow : m_arrows)
        currentArrowPositions.push_back(arrow.position);
    if (m_previousArrowPositions.size() == m_arrows.size()) {
        for (int i = 0; i < m_arrows.size(); ++i)
            m_arrows[i].position = m_previousArrowPositions[i] +
                (currentArrowPositions[i] - m_previousArrowPositions[i]) * alpha;
    }

    render(painter);

    m_target = currentTarget;
    for (int i = 0; i < m_arrows.size() && i < currentArrowPositions.size(); ++i)
        m_arrows[i].position = currentArrowPositions[i];
}

void ArcherGame::render(QPainter& painter)
{
    painter.fillRect(rect(), QColor(28, 42, 34));
    painter.setRenderHint(QPainter::Antialiasing, false);
    const double scaleX = qMax(0.25, width() / 640.0);
    const double scaleY = qMax(0.25, height() / 480.0);
    const QPointF origin(width() * 0.11, height() * 0.85);
    painter.setPen(QPen(QColor(130, 95, 55), 7.0 * qMin(scaleX, scaleY)));
    painter.drawLine(origin,
                     QPointF(origin.x() + std::cos(m_aimAngle) * 54.0 * scaleX,
                             origin.y() + std::sin(m_aimAngle) * 54.0 * scaleY));

    painter.setPen(QPen(QColor(235, 225, 190), 2));
    painter.setBrush(QColor(210, 70, 65));
    painter.drawEllipse(m_target);
    painter.setBrush(QColor(235, 205, 90));
    painter.drawEllipse(m_target.center(), m_target.width() * 0.275, m_target.height() * 0.275);
    painter.setBrush(QColor(235, 245, 245));
    painter.drawEllipse(m_target.center(), m_target.width() * 0.11, m_target.height() * 0.11);
    painter.setBrush(QColor(55, 70, 90));
    painter.drawEllipse(m_target.center(), 3.0, 3.0);

    painter.setPen(QPen(QColor(235, 220, 180), 3));
    for (const Arrow& arrow : m_arrows) {
        const QPointF tail = arrow.position - arrow.velocity / 24.0;
        painter.drawLine(tail, arrow.position);
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 14, QFont::Bold));
    painter.drawText(14, 24, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(14, 46, QStringLiteral("Celowanie: %1°")
        .arg(qRound(m_aimAngle * 180.0 / 3.14159265358979323846)));
    painter.drawText(14, 68, QStringLiteral("Strzały: %1 | Trafienia: %2 | Pudła: %3")
        .arg(m_shotsFired).arg(m_successfulHits).arg(m_missedShots));
    painter.drawText(width() - 150, 24, QStringLiteral("W locie: %1").arg(activeArrowCount()));
    const QPointF initialVelocity(std::cos(m_aimAngle) * 780.0 * scaleX,
                                  std::sin(m_aimAngle) * 780.0 * scaleY);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 235, 190, 145));
    for (int i = 1; i <= 18; ++i) {
        const double t = i * 0.055;
        const QPointF preview(origin.x() + initialVelocity.x() * t,
            origin.y() + initialVelocity.y() * t + 0.5 * m_gravity * t * t);
        if (preview.x() > width() || preview.y() > height()) break;
        painter.drawEllipse(preview, 2.0, 2.0);
    }
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Tensometr: celowanie | Strzał automatyczny co %1 s")
            .arg(m_autoFireInterval, 0, 'f', 1));
}

} // namespace games
