#include "games/ArcherGame.hpp"
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace games {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kArrowSpeed = 620.0;
constexpr int kMaximumArrows = 8;
}

ArcherGame::ArcherGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.5)
    , m_aimAngle(-0.2)
    , m_targetDirection(1.0)
    , m_targetDirectionX(1.0)
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
    m_targetDirectionX = 1.0;
    m_targetSpeed = 55.0;
    m_gravity = 520.0;
    m_fireCooldown = 0.0;
    m_elapsed = 0.0;
    m_comboTimer = 0.0;
    m_windStrength = 0.0;
    m_combo = 0;
    m_focusCharges = 3;
    m_round = 1;
    m_shotsFired = 0;
    m_successfulHits = 0;
    m_missedShots = 0;
    m_aimUp = false;
    m_aimDown = false;
    repositionTarget();
}

void ArcherGame::resizeEvent(QResizeEvent* event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double sx = static_cast<double>(newSize.width()) / oldSize.width();
        const double sy = static_cast<double>(newSize.height()) / oldSize.height();
        m_target = QRectF(m_target.x() * sx, m_target.y() * sy,
                          m_target.width() * sx, m_target.height() * sy);
        for (Arrow& arrow : m_arrows)
            arrow.position = QPointF(arrow.position.x() * sx, arrow.position.y() * sy);
    }
    GameEngine::resizeEvent(event);
    if (width() > 0 && height() > 0) {
        const double minX = width() * 0.62;
        const double maxX = qMax(minX, width() - m_target.width() - 12.0);
        m_target.moveLeft(qBound(minX, m_target.left(), maxX));
        m_target.moveTop(qBound(height() * 0.16, m_target.top(),
                                qMax(height() * 0.16, height() * 0.78 - m_target.height())));
        m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
            [this](const Arrow& arrow) {
                return arrow.position.x() > width() + 30.0 ||
                       arrow.position.y() > height() + 30.0 ||
                       arrow.position.y() < -30.0;
            }), m_arrows.end());
    }
}

void ArcherGame::repositionTarget()
{
    const double difficultyScale = qMax(0.62, 1.0 - (m_round - 1) * 0.045);
    const double targetWidth = qMin(qMax(24.0, width() * 0.055),
                                    qMax(24.0, width() * 0.18)) * difficultyScale;
    const double targetHeight = qMin(qMax(54.0, height() * 0.18),
                                     qMax(24.0, height() * 0.30)) * difficultyScale;
    const double left = qBound(24.0, width() * 0.76,
                               qMax(24.0, width() - targetWidth - 12.0));
    const double top = qBound(height() * 0.18, height() * 0.42,
                              qMax(height() * 0.18, height() * 0.78 - targetHeight));
    m_target = QRectF(left, top, targetWidth, targetHeight);
}

bool ArcherGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    m_force = qBound(0.0, value / 100.0, 1.0);
    if (!m_aimUp && !m_aimDown)
        m_aimAngle = 0.55 - m_force * 1.45;
    return true;
}

void ArcherGame::activateFocusShot()
{
    if (!isRunning() || m_focusCharges <= 0 || m_fireCooldown > 0.0)
        return;
    --m_focusCharges;
    fireArrow(true);
}

void ArcherGame::fireArrow(bool focused)
{
    if (!isRunning() || m_fireCooldown > 0.0)
        return;
    const QPointF origin(72.0, height() - 72.0);
    const double speed = kArrowSpeed * (focused ? 1.18 : 1.0);
    Arrow arrow;
    arrow.position = origin;
    arrow.velocity = QPointF(std::cos(m_aimAngle) * speed,
                             std::sin(m_aimAngle) * speed);
    arrow.active = true;
    arrow.focused = focused;
    if (m_arrows.size() >= kMaximumArrows) {
        const auto oldest = std::find_if(m_arrows.begin(), m_arrows.end(),
            [](const Arrow& candidate) { return candidate.active; });
        if (oldest != m_arrows.end()) {
            oldest->active = false;
            ++m_missedShots;
        }
        m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
            [](const Arrow& candidate) { return !candidate.active; }), m_arrows.end());
    }
    m_arrows.push_back(arrow);
    ++m_shotsFired;
    m_fireCooldown = focused ? 0.48 : 0.32;
}

void ArcherGame::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat()) fireArrow(false);
        event->accept();
    } else if (event->key() == Qt::Key_F || event->key() == Qt::Key_E) {
        if (!event->isAutoRepeat()) activateFocusShot();
        event->accept();
    } else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        m_aimUp = true; event->accept();
    } else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        m_aimDown = true; event->accept();
    } else {
        GameEngine::keyPressEvent(event);
    }
}

void ArcherGame::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat()) { event->accept(); return; }
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_W) {
        m_aimUp = false; event->accept();
    } else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_S) {
        m_aimDown = false; event->accept();
    } else {
        GameEngine::keyReleaseEvent(event);
    }
}

void ArcherGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    deltaTime = qMin(deltaTime, 0.1);
    m_elapsed += deltaTime;
    m_fireCooldown = qMax(0.0, m_fireCooldown - deltaTime);
    m_comboTimer = qMax(0.0, m_comboTimer - deltaTime);
    if (m_comboTimer <= 0.0)
        m_combo = 0;

    const double aimDirection = (m_aimDown ? 1.0 : 0.0) - (m_aimUp ? 1.0 : 0.0);
    m_aimAngle = qBound(-1.25, m_aimAngle + aimDirection * 1.35 * deltaTime, 0.85);
    m_windStrength = std::sin(m_elapsed * 0.43 + m_round * 0.71) * (18.0 + qMin(18.0, m_round * 1.5));

    // The target now moves in two axes, but remains inside a readable play zone.
    m_target.translate(m_targetDirectionX * (18.0 + m_round * 1.4) * deltaTime,
                       m_targetDirection * m_targetSpeed * deltaTime);
    const double minX = width() * 0.66;
    const double maxX = qMax(minX, width() - m_target.width() - 12.0);
    if (m_target.left() <= minX) {
        m_target.moveLeft(minX);
        m_targetDirectionX = 1.0;
    } else if (m_target.right() >= width() - 12.0) {
        m_target.moveRight(width() - 12.0);
        m_targetDirectionX = -1.0;
    }
    if (m_target.top() < height() * 0.16) {
        m_target.moveTop(height() * 0.16);
        m_targetDirection = 1.0;
    } else if (m_target.bottom() > height() * 0.78) {
        m_target.moveBottom(height() * 0.78);
        m_targetDirection = -1.0;
    }

    for (Arrow& arrow : m_arrows) {
        if (!arrow.active)
            continue;
        const QPointF previous = arrow.position;
        arrow.velocity.rx() += m_windStrength * (arrow.focused ? 0.12 : 1.0) * deltaTime;
        arrow.velocity.ry() += m_gravity * deltaTime;
        arrow.position += arrow.velocity * deltaTime;

        const QPointF segment = arrow.position - previous;
        const int samples = qMax(1, static_cast<int>(std::ceil(
            std::hypot(segment.x(), segment.y()) / 4.0)));
        bool hit = false;
        QPointF impact = arrow.position;
        for (int sample = 1; sample <= samples; ++sample) {
            const QPointF point = previous + segment * (static_cast<double>(sample) / samples);
            const double rx = qMax(1.0, m_target.width() * 0.5);
            const double ry = qMax(1.0, m_target.height() * 0.5);
            const double nx = (point.x() - m_target.center().x()) / rx;
            const double ny = (point.y() - m_target.center().y()) / ry;
            if (nx * nx + ny * ny <= 1.0) {
                hit = true;
                impact = point;
                break;
            }
        }
        if (hit) {
            arrow.active = false;
            const double dx = (impact.x() - m_target.center().x()) / qMax(1.0, m_target.width() * 0.5);
            const double dy = (impact.y() - m_target.center().y()) / qMax(1.0, m_target.height() * 0.5);
            const double radial = std::sqrt(dx * dx + dy * dy);
            const int basePoints = radial <= 0.20 ? 30 : (radial <= 0.55 ? 20 : 10);
            ++m_combo;
            m_comboTimer = 3.0;
            addScore(basePoints * qMin(4, 1 + (m_combo - 1) / 3) + (arrow.focused ? 10 : 0));
            ++m_successfulHits;
            m_targetSpeed = qMin(165.0, m_targetSpeed + 3.5);
            if (m_successfulHits % 5 == 0) {
                ++m_round;
                m_focusCharges = qMin(5, m_focusCharges + 1);
                m_targetSpeed = qMin(165.0, m_targetSpeed + 8.0);
            }
            repositionTarget();
            continue;
        }
        if (arrow.position.x() > width() + 24.0 || arrow.position.y() > height() + 24.0 ||
            arrow.position.y() < -24.0 || arrow.position.x() < -24.0) {
            arrow.active = false;
            ++m_missedShots;
            m_combo = 0;
            m_comboTimer = 0.0;
        }
    }

    m_arrows.erase(std::remove_if(m_arrows.begin(), m_arrows.end(),
        [](const Arrow& arrow) { return !arrow.active; }), m_arrows.end());
}

void ArcherGame::render(QPainter& painter)
{
    painter.save();
    painter.fillRect(rect(), QColor(22, 37, 31));
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Distant woodland silhouettes and a subtle ground strip.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(31, 54, 42));
    for (int i = 0; i < 8; ++i) {
        const double x = i * (width() / 7.0);
        painter.drawEllipse(QPointF(x, height() * 0.76), width() * 0.12, height() * 0.17);
    }
    painter.fillRect(QRectF(0, height() * 0.84, width(), height() * 0.16), QColor(46, 58, 37));

    const QPointF origin(72.0, height() - 72.0);
    painter.setPen(QPen(QColor(165, 119, 69), 7));
    painter.drawLine(origin, QPointF(origin.x() + std::cos(m_aimAngle) * 54.0,
                                     origin.y() + std::sin(m_aimAngle) * 54.0));
    painter.setPen(QPen(QColor(225, 205, 160), 2));
    painter.drawArc(QRectF(origin.x() - 8.0, origin.y() - 34.0, 28.0, 68.0), -90 * 16, 180 * 16);

    // Target rings have distinct values; the moving target is outlined for contrast.
    painter.setPen(QPen(QColor(245, 230, 195), 2));
    painter.setBrush(QColor(190, 58, 58));
    painter.drawEllipse(m_target);
    painter.setBrush(QColor(230, 184, 66));
    painter.drawEllipse(m_target.center(), m_target.width() * 0.275, m_target.height() * 0.275);
    painter.setBrush(QColor(230, 239, 235));
    painter.drawEllipse(m_target.center(), m_target.width() * 0.11, m_target.height() * 0.11);
    painter.setBrush(QColor(45, 70, 90));
    painter.drawEllipse(m_target.center(), 3.0, 3.0);

    for (const Arrow& arrow : m_arrows) {
        const QPointF tail = arrow.position - arrow.velocity / 24.0;
        painter.setPen(QPen(arrow.focused ? QColor(100, 220, 255) : QColor(238, 221, 178),
                            arrow.focused ? 4.0 : 3.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(tail, arrow.position);
        if (arrow.focused) {
            painter.setPen(QPen(QColor(100, 220, 255, 110), 1.0));
            painter.drawEllipse(arrow.position, 7.0, 7.0);
        }
    }

    // Predicted trajectory uses the same gravity and wind model as the projectile.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 235, 190, 145));
    const QPointF initialVelocity(std::cos(m_aimAngle) * kArrowSpeed,
                                  std::sin(m_aimAngle) * kArrowSpeed);
    for (int i = 1; i <= 22; ++i) {
        const double t = i * 0.045;
        const QPointF preview(origin.x() + initialVelocity.x() * t +
                0.5 * m_windStrength * t * t,
            origin.y() + initialVelocity.y() * t + 0.5 * m_gravity * t * t);
        if (preview.x() > width() || preview.y() > height()) break;
        painter.drawEllipse(preview, 2.0, 2.0);
    }

    painter.setPen(Qt::white);
    const int fontSize = qBound(9, height() / 34, 14);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), fontSize, QFont::Bold));
    painter.drawText(12, fontSize + 8, QStringLiteral("Wynik: %1").arg(getScore()));
    painter.drawText(12, 2 * (fontSize + 8),
        QStringLiteral("Kąt: %1°  Wiatr: %2")
            .arg(qRound(m_aimAngle * 180.0 / kPi))
            .arg(m_windStrength >= 0.0 ? QStringLiteral("→ %1").arg(qRound(m_windStrength))
                                      : QStringLiteral("← %1").arg(qRound(-m_windStrength))));
    painter.drawText(12, 3 * (fontSize + 8),
        QStringLiteral("Strzały: %1  Trafienia: %2  Pudła: %3")
            .arg(m_shotsFired).arg(m_successfulHits).arg(m_missedShots));
    painter.drawText(width() - 150, fontSize + 8,
        QStringLiteral("Seria: x%1").arg(m_combo));
    painter.drawText(width() - 150, 2 * (fontSize + 8),
        QStringLiteral("Skupienie: %1").arg(m_focusCharges));
    painter.drawText(width() - 150, 3 * (fontSize + 8),
        QStringLiteral("Runda: %1").arg(m_round));

    painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(8, fontSize - 2, 11)));
    painter.drawText(QRect(10, height() - fontSize - 9, width() - 20, fontSize + 4),
        Qt::AlignHCenter | Qt::AlignVCenter,
        QStringLiteral("Czujnik: kąt  |  W/↑: wyżej  |  S/↓: niżej  |  SPACJA: strzał  |  F/E: skupiony strzał"));
    painter.restore();
}

} // namespace games
