#include "games/ArcherGame.hpp"
#include <QKeyEvent>
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
    , m_gravity(520.0)
    , m_fireCooldown(0.0)
{
    setFocusPolicy(Qt::StrongFocus);
}

QString ArcherGame::gameName() const
{
    return QStringLiteral("Archer Force Trainer");
}

void ArcherGame::initialize()
{
    m_arrows.clear();
    m_force = 0.5;
    m_aimAngle = -0.2;
    m_targetDirection = 1.0;
    m_targetSpeed = 55.0;
    m_gravity = 520.0;
    m_fireCooldown = 0.0;
    repositionTarget();
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
    m_force = qBound(0.0, value / 100.0, 1.0);
    // Low force aims lower; high force raises the bow.
    m_aimAngle = 0.55 - m_force * 1.45;
    return true;
}

void ArcherGame::fireArrow()
{
    if (!isRunning() || m_fireCooldown > 0.0)
        return;
    const QPointF origin(72.0, height() - 72.0);
    constexpr double speed = 620.0;
    Arrow arrow;
    arrow.position = origin;
    arrow.velocity = QPointF(std::cos(m_aimAngle) * speed,
                             std::sin(m_aimAngle) * speed);
    arrow.active = true;
    m_arrows.push_back(arrow);
    m_fireCooldown = 0.32;
}

void ArcherGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
        fireArrow();
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W)
        m_aimAngle = qMax(-1.25, m_aimAngle - 0.06);
    else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S)
        m_aimAngle = qMin(0.85, m_aimAngle + 0.06);
    else
        GameEngine::keyPressEvent(event);
}

void ArcherGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);
    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);

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
            if (m_target.contains(point)) {
                hit = true;
                break;
            }
        }
        if (hit) {
            arrow.active = false;
            addScore(10);
            m_targetSpeed = qMin(150.0, m_targetSpeed + 4.0);
            repositionTarget();
            continue;
        }
        if (arrow.position.x() > width() || arrow.position.y() > height() ||
            arrow.position.y() < -20.0)
            arrow.active = false;
    }

    m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
        [](const Arrow& arrow) { return !arrow.active; }), m_arrows.end());
}

void ArcherGame::render(QPainter& painter)
{
    painter.fillRect(rect(), QColor(28, 42, 34));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(130, 95, 55), 7));
    painter.drawLine(QPointF(72.0, height() - 72.0),
                     QPointF(72.0 + std::cos(m_aimAngle) * 54.0,
                             height() - 72.0 + std::sin(m_aimAngle) * 54.0));

    painter.setPen(QPen(QColor(235, 225, 190), 2));
    painter.setBrush(QColor(210, 70, 65));
    painter.drawEllipse(m_target);
    painter.setBrush(Qt::white);
    painter.drawEllipse(m_target.center(), 5.0, 5.0);

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
    painter.setFont(QFont(QStringLiteral("Arial"), 10));
    painter.drawText(14, height() - 12,
        QStringLiteral("Czujnik reguluje kąt | SPACJA: strzał | W/S: korekta"));
}

} // namespace games
