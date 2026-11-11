#include "games/PrecisionCraneGame.hpp"

#include <QPainter>
#include <QLinearGradient>
#include <QResizeEvent>
#include <QFont>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace games {
namespace {
constexpr double kGestureForce = 0.82;
constexpr double kReleaseForce = 0.18;
constexpr double kLongGestureSeconds = 0.85;
constexpr double kMinimumGestureSeconds = 0.15;
}

PrecisionCraneGame::PrecisionCraneGame(QWidget *parent)
    : GameEngine(parent)
    , m_force(0.0)
    , m_inputAge(1.0)
    , m_carriageX(0.5)
    , m_hookY(0.55)
    , m_gestureSeconds(0.0)
    , m_animationTime(0.0)
    , m_sway(0.0)
    , m_cargoX(0.25)
    , m_targetX(0.76)
    , m_targetWidth(0.13)
    , m_difficulty(1)
    , m_delivered(0)
    , m_mistakesRemaining(3)
    , m_combo(0)
    , m_axis(HorizontalAxis)
    , m_gestureActive(false)
    , m_loadAttached(false)
    , m_status(QStringLiteral("Oczekiwanie na sygnał tensometru"))
{
    setFocusPolicy(Qt::NoFocus);
}

QString PrecisionCraneGame::gameName() const
{
    return QStringLiteral("Precision Crane — Precyzyjny dźwig");
}

void PrecisionCraneGame::setDifficultyLevel(int level)
{
    m_difficulty = qBound(1, level, 5);
    m_targetWidth = 0.18 - (m_difficulty - 1) * 0.022;
}

double PrecisionCraneGame::floorY() const
{
    return qMax(180.0, static_cast<double>(height()) * 0.82);
}

double PrecisionCraneGame::clampCarriage(double x) const
{
    const double left = qMax(42.0, width() * 0.08);
    const double right = qMax(left + 1.0, width() * 0.92);
    return qBound(left, x, right);
}

QRectF PrecisionCraneGame::cargoRect() const
{
    const double size = qBound(22.0, width() * 0.055, 48.0);
    const double y = m_loadAttached ? qMin(m_hookY + 18.0, floorY() - size) : floorY() - size;
    const double x = m_loadAttached ? m_carriageX + m_sway - size * 0.5 : m_cargoX * width() - size * 0.5;
    return QRectF(x, y, size, size);
}

QRectF PrecisionCraneGame::targetRect() const
{
    const double w = qMax(34.0, width() * m_targetWidth);
    return QRectF(m_targetX * width() - w * 0.5, floorY() - 9.0, w, 9.0);
}

void PrecisionCraneGame::resetRound()
{
    m_loadAttached = false;
    m_axis = HorizontalAxis;
    m_carriageX = width() * 0.5;
    m_hookY = height() * 0.38;
    m_cargoX = 0.20 + (m_delivered % 4) * 0.13;
    if (m_cargoX > 0.58) m_cargoX = 0.28;
    m_targetX = 0.76 - (m_delivered % 3) * 0.12;
    m_gestureActive = false;
    m_gestureSeconds = 0.0;
}

void PrecisionCraneGame::initialize()
{
    m_force = 0.0;
    m_inputAge = 1.0;
    m_animationTime = 0.0;
    m_sway = 0.0;
    m_delivered = 0;
    m_mistakesRemaining = 3;
    m_combo = 0;
    m_status = QStringLiteral("Krótki impuls: chwyt/zwolnienie • długi: zmiana osi");
    setScore(0);
    setDifficultyLevel(m_difficulty);
    resetRound();
}

bool PrecisionCraneGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;
    // GameEngine's sensor pipeline provides calibrated force in the 0–100 range.
    const bool signalWasStale = m_inputAge > 0.35;
    m_force = qBound(0.0, value / 100.0, 1.0);
    m_inputAge = 0.0;
    if (signalWasStale)
        m_status = QStringLiteral("Sygnał przywrócony — steruj położeniem haka i wózka");
    return true;
}

void PrecisionCraneGame::processGesture()
{
    if (m_inputAge > 0.35)
        return;

    if (m_force >= kGestureForce) {
        if (!m_gestureActive) {
            m_gestureActive = true;
            m_gestureSeconds = 0.0;
        }
        return;
    }

    if (m_gestureActive && m_force <= kReleaseForce) {
        const double duration = m_gestureSeconds;
        m_gestureActive = false;
        m_gestureSeconds = 0.0;
        if (duration >= kLongGestureSeconds) {
            m_axis = (m_axis == HorizontalAxis) ? VerticalAxis : HorizontalAxis;
            m_status = m_axis == HorizontalAxis
                ? QStringLiteral("Oś pozioma — sterowanie położeniem dźwigu")
                : QStringLiteral("Oś pionowa — sterowanie wysokością haka");
        } else if (duration >= kMinimumGestureSeconds) {
            toggleGrip();
        }
    }
}

void PrecisionCraneGame::toggleGrip()
{
    if (!m_loadAttached) {
        const QRectF cargo = cargoRect();
        const QPointF hook(m_carriageX + m_sway, m_hookY + 12.0);
        const QPointF cargoCenter = cargo.center();
        const double dx = hook.x() - cargoCenter.x();
        const double dy = hook.y() - cargoCenter.y();
        if (std::sqrt(dx * dx + dy * dy) <= qMax(42.0, width() * 0.075)) {
            m_loadAttached = true;
            m_status = QStringLiteral("Ładunek podniesiony — przenieś go do strefy celu");
        } else {
            m_status = QStringLiteral("Hak poza zasięgiem ładunku — ustaw dźwig dokładniej");
            m_combo = 0;
        }
    } else {
        deliverOrDropLoad();
    }
}

void PrecisionCraneGame::deliverOrDropLoad()
{
    const QRectF load = cargoRect();
    const QRectF target = targetRect();
    const bool lowEnough = m_hookY + 18.0 + load.height() >= floorY() - 18.0;
    if (lowEnough && target.adjusted(-load.width() * 0.25, -4.0,
                                     load.width() * 0.25, 0.0).intersects(load)) {
        ++m_delivered;
        ++m_combo;
        addScore(100 + qMin(m_combo - 1, 10) * 20 + m_difficulty * 10);
        m_status = QStringLiteral("Dostawa precyzyjna! Seria: %1").arg(m_combo);
        if (m_delivered % 4 == 0)
            setDifficultyLevel(m_difficulty + 1);
        resetRound();
    } else {
        m_loadAttached = false;
        m_cargoX = qBound(0.08, m_carriageX / qMax(1, width()), 0.92);
        --m_mistakesRemaining;
        m_combo = 0;
        m_status = QStringLiteral("Ładunek odłożony poza celem — pozostało prób: %1")
                       .arg(m_mistakesRemaining);
        if (m_mistakesRemaining <= 0) {
            stop();
            return;
        }
    }
}

void PrecisionCraneGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    const double dt = qMin(deltaTime, 0.1);
    m_animationTime += dt;
    m_inputAge += dt;

    if (m_inputAge > 0.35) {
        m_gestureActive = false;
        m_gestureSeconds = 0.0;
        m_status = QStringLiteral("Brak świeżego sygnału — ruch wstrzymany");
    }
    if (m_gestureActive && m_inputAge <= 0.35)
        m_gestureSeconds += dt;
    processGesture();

    if (m_inputAge <= 0.35 && !m_gestureActive &&
        m_force > kReleaseForce && m_force < kGestureForce) {
        // The release band is a dead zone: it holds the current position,
        // so finishing a gesture never snaps the crane to an endpoint.
        const double control = qBound(0.0, (m_force - kReleaseForce) / (0.70 - kReleaseForce), 1.0);
        if (m_axis == HorizontalAxis) {
            const double left = width() * 0.08;
            const double right = width() * 0.92;
            const double desired = left + control * (right - left);
            m_carriageX += (desired - m_carriageX) * (1.0 - std::exp(-dt * 7.0));
        } else {
            const double top = height() * 0.28;
            const double bottom = floorY() - 42.0;
            const double desired = top + control * (bottom - top);
            m_hookY += (desired - m_hookY) * (1.0 - std::exp(-dt * 7.0));
        }
    }

    // Gentle pendulum motion communicates load inertia without adding another input.
    if (m_loadAttached)
        m_sway = std::sin(m_animationTime * (2.2 + m_difficulty * 0.12)) *
                 (2.0 + m_force * 7.0);
    else
        m_sway *= 0.88;
}

void PrecisionCraneGame::resizeEvent(QResizeEvent *event)
{
    GameEngine::resizeEvent(event);
    m_carriageX = clampCarriage(m_carriageX);
    m_hookY = qBound(height() * 0.24, m_hookY, floorY() - 35.0);
}

void PrecisionCraneGame::render(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const double w = width(), h = height();
    const double ground = floorY();

    QLinearGradient sky(0, 0, 0, h);
    sky.setColorAt(0.0, QColor(12, 34, 62));
    sky.setColorAt(1.0, QColor(30, 88, 112));
    painter.fillRect(rect(), sky);

    // Distant harbour skyline and water.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(35, 68, 83));
    for (int i = 0; i < 7; ++i) {
        const double x = i * w / 7.0;
        const double bw = w / 11.0;
        const double bh = h * (0.10 + (i % 3) * 0.035);
        painter.drawRect(QRectF(x, ground - bh - 22.0, bw, bh));
    }
    painter.fillRect(QRectF(0, ground, w, h - ground), QColor(17, 62, 83));
    painter.setPen(QPen(QColor(86, 178, 190, 120), 1.0));
    for (int i = 0; i < 8; ++i) {
        const double y = ground + 12.0 + i * 8.0;
        painter.drawLine(QPointF(0, y), QPointF(w, y + std::sin(m_animationTime + i) * 2.0));
    }

    // Crane gantry, rails and moving trolley.
    const double railY = h * 0.20;
    painter.setPen(QPen(QColor(225, 183, 93), qMax(5.0, w * 0.009), Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QPointF(w * 0.06, railY), QPointF(w * 0.94, railY));
    painter.drawLine(QPointF(w * 0.08, railY), QPointF(w * 0.08, ground));
    painter.drawLine(QPointF(w * 0.92, railY), QPointF(w * 0.92, ground));
    painter.setPen(QPen(QColor(250, 211, 125), 2.0));
    for (int i = 0; i < 16; ++i) {
        const double x = w * 0.08 + i * w * 0.84 / 15.0;
        painter.drawLine(QPointF(x, railY - 6.0), QPointF(x, railY + 6.0));
    }

    const double trolleyW = qMax(30.0, w * 0.065);
    const QRectF trolley(m_carriageX - trolleyW * 0.5, railY - 12.0, trolleyW, 24.0);
    painter.setPen(QPen(QColor(30, 38, 48), 2.0));
    painter.setBrush(QColor(235, 151, 54));
    painter.drawRoundedRect(trolley, 5.0, 5.0);

    const double hookX = m_carriageX + m_sway;
    painter.setPen(QPen(QColor(222, 231, 236), 3.0));
    painter.drawLine(QPointF(hookX, trolley.bottom()), QPointF(hookX, m_hookY));
    painter.setPen(QPen(QColor(245, 192, 89), 4.0, Qt::SolidLine, Qt::RoundCap));
    painter.drawArc(QRectF(hookX - 9.0, m_hookY - 2.0, 18.0, 22.0), 0, -180 * 16);

    // Target bay.
    const QRectF target = targetRect();
    painter.setPen(QPen(QColor(95, 232, 178), 2.0, Qt::DashLine));
    painter.setBrush(QColor(47, 178, 137, 65));
    painter.drawRoundedRect(QRectF(target.left(), ground - 38.0, target.width(), 38.0), 5.0, 5.0);
    painter.setPen(QPen(QColor(95, 232, 178), 3.0));
    painter.drawLine(QPointF(target.left(), ground - 4.0), QPointF(target.right(), ground - 4.0));

    // Cargo container and its attachment point.
    const QRectF cargo = cargoRect();
    painter.setPen(QPen(QColor(26, 39, 50), 2.0));
    painter.setBrush(m_loadAttached ? QColor(255, 190, 75) : QColor(220, 113, 67));
    painter.drawRoundedRect(cargo, 4.0, 4.0);
    painter.setPen(QPen(QColor(255, 224, 175, 150), 1.0));
    for (int i = 1; i < 5; ++i) {
        const double x = cargo.left() + cargo.width() * i / 5.0;
        painter.drawLine(QPointF(x, cargo.top() + 4.0), QPointF(x, cargo.bottom() - 4.0));
    }
    if (m_loadAttached) {
        painter.setPen(QPen(QColor(255, 231, 162), 2.0));
        painter.drawLine(QPointF(hookX, m_hookY + 12.0), QPointF(cargo.center().x(), cargo.top()));
    }

    // Force meter and compact HUD.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(8, 18, 32, 210));
    painter.drawRoundedRect(QRectF(14.0, 14.0, qMin(340.0, w - 28.0), 74.0), 10.0, 10.0);
    painter.setPen(QColor(240, 246, 250));
    QFont hud = painter.font(); hud.setBold(true); hud.setPointSizeF(qMax(8.0, hud.pointSizeF()));
    painter.setFont(hud);
    painter.drawText(QRectF(26.0, 22.0, w - 52.0, 20.0),
        QStringLiteral("PRECISION CRANE   WYNIK %1   DOSTAWY %2   PRÓBY %3")
            .arg(getScore()).arg(m_delivered).arg(m_mistakesRemaining));
    painter.setPen(QColor(170, 218, 230));
    painter.drawText(QRectF(26.0, 48.0, w - 52.0, 20.0),
        QStringLiteral("Oś: %1   Siła: %2%   Seria: %3")
            .arg(m_axis == HorizontalAxis ? QStringLiteral("POZIOMA") : QStringLiteral("PIONOWA"))
            .arg(static_cast<int>(std::round(m_force * 100.0))).arg(m_combo));

    painter.setPen(QColor(255, 245, 218));
    painter.drawText(QRectF(16.0, h - 34.0, w - 32.0, 24.0), Qt::AlignCenter, m_status);
    painter.restore();
}

} // namespace games
