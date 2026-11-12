#include "games/PrecisionCraneGame.hpp"

#include <QPainter>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

#include <QResizeEvent>
#include <QFont>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <limits>

namespace games {

PrecisionCraneGame::PrecisionCraneGame(QWidget *parent, const QString &recordsFilePath)
    : GameEngine(parent)
    , m_force(0.0)
    , m_inputAge(1.0)
    , m_carriageX(0.5)
    , m_hookY(0.55)
    , m_gestureSeconds(0.0)
    , m_animationTime(0.0)
    , m_sway(0.0)
    , m_swayVelocity(0.0)
    , m_previousCarriageX(0.0)
    , m_trolleyVelocity(0.0)
    , m_cargoMassKg(1.0)
    , m_ropeLength(150.0)
    , m_precisionTotal(0.0)
    , m_obstacleClock(0.0)
    , m_cargoX(0.25)
    , m_targetX(0.76)
    , m_targetWidth(0.13)
    , m_difficulty(1)
    , m_delivered(0)
    , m_mistakesRemaining(3)
    , m_lastPrecisionBonus(0)
    , m_combo(0)
    , m_missionIndex(0)
    , m_collisionCount(0)
    , m_bestScore(0)
    , m_precisionSamples(0)
    , m_lifetimeDeliveries(0)
    , m_lifetimePrecisionTotal(0)
    , m_lifetimePrecisionSamples(0)
    , m_lifetimeCollisions(0)
    , m_cargoType(StandardCargo)
    , m_campaignCompleted(false)
    , m_axis(HorizontalAxis)
    , m_gestureActive(false)
    , m_loadAttached(false)
    , m_status(QStringLiteral("Oczekiwanie na sygnał tensometru"))
    , m_recordsFilePath(recordsFilePath)
{
    m_obstacles = {{0.40, 34.0, 28.0, 0.42, 0.0},
                   {0.58, 42.0, 32.0, 0.31, 2.1}};
    setFocusPolicy(Qt::NoFocus);
}

QString PrecisionCraneGame::gameName() const
{
    return QStringLiteral("Precision Crane — Precyzyjny dźwig");
}

void PrecisionCraneGame::setDifficultyLevel(int level)
{
    m_difficulty = qBound(1, level, 5);
    m_targetWidth = 0.22 - (m_difficulty - 1) * 0.015;
}

double PrecisionCraneGame::floorY() const
{
    return qMax(180.0 * qMax(0.5, height() / 480.0),
                 static_cast<double>(height()) * 0.82);
}

double PrecisionCraneGame::clampCarriage(double x) const
{
    const double left = qMax(42.0 * qMax(0.5, width() / 640.0), width() * 0.08);
    const double right = qMax(left + 1.0, width() * 0.92);
    return qBound(left, x, right);
}

QRectF PrecisionCraneGame::cargoRect() const
{
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const double objectScale = qMin(scaleX, scaleY);
    const double size = qBound(22.0 * objectScale,
        width() * 0.055 * std::sqrt(m_cargoMassKg), 56.0 * objectScale);
    const double swingRatio = qBound(-0.42, m_sway / qMax(1.0, m_ropeLength), 0.42);
    const double verticalSwing = m_loadAttached
        ? m_ropeLength * (1.0 - std::cos(std::asin(swingRatio))) : 0.0;
    const double y = m_loadAttached
        ? qMin(m_hookY + 18.0 * scaleY + verticalSwing, floorY() - size) : floorY() - size;
    const double x = m_loadAttached ? m_carriageX + m_sway - size * 0.5 : m_cargoX * width() - size * 0.5;
    return QRectF(x, y, size, size);
}

QRectF PrecisionCraneGame::targetRect() const
{
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const double w = qMax(34.0 * scaleX, width() * m_targetWidth);
    return QRectF(m_targetX * width() - w * 0.5, floorY() - 9.0 * scaleY,
                  w, 9.0 * scaleY);
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
    configureCargo();
    m_swayVelocity = 0.0;
    m_previousCarriageX = m_carriageX;
    m_trolleyVelocity = 0.0;
}

void PrecisionCraneGame::initialize()
{
    m_force = 0.0;
    m_inputAge = 1.0;
    m_animationTime = 0.0;
    m_obstacleClock = 0.0;
    m_sway = 0.0;
    m_swayVelocity = 0.0;
    m_previousCarriageX = m_carriageX;
    m_trolleyVelocity = 0.0;
    m_delivered = 0;
    m_missionIndex = 0;
    m_collisionCount = 0;
    m_campaignCompleted = false;
    m_precisionTotal = 0.0;
    m_precisionSamples = 0;
    m_mistakesRemaining = 3;
    loadRecords();
    m_lastPrecisionBonus = 0;
    m_combo = 0;
    m_status = QStringLiteral("Krótki impuls: chwyt/zwolnienie • długi: zmiana osi");
    setScore(0);
    setDifficultyLevel(m_difficulty);
    resetRound();
    m_renderPreviousCarriageX = m_carriageX;
    m_previousHookY = m_hookY;
    m_previousSway = m_sway;
    m_previousObstacleClock = m_obstacleClock;
}

bool PrecisionCraneGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    // The central sensor pipeline supplies calibrated force in the 0–100 N domain.
    // Invalid values are ignored; freshness is updated only by a valid sample.
    const bool signalWasStale = m_inputAge > kSignalTimeoutSeconds;
    m_force = qBound(0.0, value / 100.0, 1.0);
    m_inputAge = 0.0;
    if (signalWasStale)
        m_status = QStringLiteral("Sygnał przywrócony — steruj położeniem haka i wózka");
    return true;
}

void PrecisionCraneGame::processGesture()
{
    if (m_inputAge > kSignalTimeoutSeconds)
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
        const double targetCenter = target.center().x();
        const double deliveryError = std::abs(load.center().x() - targetCenter);
        const double precisionRadius = qMax(1.0, target.width() * 0.5);
        const double precision = qBound(0.0, 1.0 - deliveryError / precisionRadius, 1.0);
        m_lastPrecisionBonus = qRound(precision * 100.0);
        m_precisionTotal += m_lastPrecisionBonus;
        ++m_precisionSamples;
        ++m_lifetimeDeliveries;
        m_lifetimePrecisionTotal += m_lastPrecisionBonus;
        ++m_lifetimePrecisionSamples;
        ++m_delivered;
        m_missionIndex = m_delivered / 3;
        ++m_combo;
        addScore(100 + m_lastPrecisionBonus +
                 qMin(m_combo - 1, 10) * 20 + m_difficulty * 10);
        // Persist only after scoring so bestScore includes this delivery.
        saveRecords();
        m_status = QStringLiteral("Dostawa! Premia precyzji: +%1 • Seria: %2")
                       .arg(m_lastPrecisionBonus).arg(m_combo);
        if (m_delivered % 4 == 0)
            setDifficultyLevel(m_difficulty + 1);
        if (m_delivered >= 15) {
            m_campaignCompleted = true;
            m_status = QStringLiteral("Kampania ukończona! Dostarczono 15 ładunków.");
            stop();
            return;
        }
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
    m_previousCarriageX = m_carriageX;
    m_renderPreviousCarriageX = m_carriageX;
    m_previousHookY = m_hookY;
    m_previousSway = m_sway;
    m_previousObstacleClock = m_obstacleClock;
    const double dt = qMin(deltaTime, 0.1);
    const double h = static_cast<double>(height());
    m_animationTime += dt;
    m_obstacleClock += dt;
    m_inputAge += dt;

    if (m_inputAge > kSignalTimeoutSeconds) {
        m_gestureActive = false;
        m_gestureSeconds = 0.0;
        m_force = 0.0;
        m_status = QStringLiteral("Brak świeżego sygnału — ruch wstrzymany");
    }
    if (m_gestureActive && m_inputAge <= kSignalTimeoutSeconds)
        m_gestureSeconds += dt;
    processGesture();

    if (m_inputAge <= kSignalTimeoutSeconds && !m_gestureActive &&
        m_force > kReleaseForce && m_force < kGestureForce) {
        // The 0–18% release band is a true dead zone: retain the last position.
        const double control = qBound(0.0,
            (m_force - kMinimumControlForce) /
                (kMaximumControlForce - kMinimumControlForce), 1.0);
        if (m_axis == HorizontalAxis) {
            const double left = width() * 0.08;
            const double right = width() * 0.92;
            const double desired = left + control * (right - left);
            m_carriageX += (desired - m_carriageX) * (1.0 - std::exp(-dt * 7.0));
        } else {
            const double top = height() * 0.28;
            const double bottom = floorY() - 42.0 * qMax(0.5, height() / 480.0);
            const double desired = top + control * (bottom - top);
            m_hookY += (desired - m_hookY) * (1.0 - std::exp(-dt * 7.0));
        }
    }

    if (m_loadAttached)
        processObstacleCollision();

    // Damped-pendulum model: trolley acceleration excites the suspended load.
    // The single force sensor remains the only player input; sway is simulation state.
    const double trolleyVelocity = (m_carriageX - m_previousCarriageX) / dt;
    const double trolleyAcceleration = (trolleyVelocity - m_trolleyVelocity) / dt;
    m_previousCarriageX = m_carriageX;
    m_trolleyVelocity = trolleyVelocity;
    if (m_loadAttached) {
        const double ropeLength = qBound(70.0, m_ropeLength, qMax(71.0, floorY() - h * 0.20));
        const double naturalFrequencySquared =
            (980.0 * qMax(0.5, height() / 480.0)) / (ropeLength * m_cargoMassKg);
        const double damping = 1.15 / std::sqrt(m_cargoMassKg);
        const double swayAcceleration = -naturalFrequencySquared * m_sway
                                        - damping * m_swayVelocity
                                        - trolleyAcceleration * 0.72;
        m_swayVelocity += swayAcceleration * dt;
        m_sway += m_swayVelocity * dt;
        m_sway = qBound(-ropeLength * 0.42, m_sway, ropeLength * 0.42);
    } else {
        m_swayVelocity *= std::exp(-5.0 * dt);
        m_sway *= std::exp(-5.0 * dt);
        if (std::abs(m_sway) < 0.05) m_sway = 0.0;
    }
}


QRectF PrecisionCraneGame::obstacleRect(int index) const
{
    if (index < 0 || index >= m_obstacles.size()) return QRectF();
    const MovingObstacle& obstacle = m_obstacles.at(index);
    const double scaleX = qMax(0.5, width() / 640.0);
    const double scaleY = qMax(0.5, height() / 480.0);
    const double obstacleWidth = obstacle.width * scaleX;
    const double obstacleHeight = obstacle.height * scaleY;
    const double travel = qMax(0.0, width() - obstacleWidth - 2.0 * width() * 0.12);
    const double phase = m_obstacleClock * obstacle.speed + obstacle.phase;
    const double x = width() * 0.12 + (travel * 0.5) * (1.0 + std::sin(phase));
    return QRectF(x, obstacle.y * height(), obstacleWidth, obstacleHeight);
}

bool PrecisionCraneGame::checkObstacleCollision()
{
    const QRectF load = cargoRect();
    for (int i = 0; i < m_obstacles.size(); ++i) {
        if (load.intersects(obstacleRect(i))) return true;
    }
    return false;
}

bool PrecisionCraneGame::processObstacleCollision()
{
    if (!m_loadAttached || !checkObstacleCollision())
        return false;

    m_loadAttached = false;
    m_cargoX = qBound(0.08, (m_carriageX + m_sway) / qMax(1, width()), 0.92);
    ++m_collisionCount;
    ++m_lifetimeCollisions;
    saveRecords();
    --m_mistakesRemaining;
    m_combo = 0;
    m_swayVelocity = 0.0;
    m_status = QStringLiteral("Kolizja z przeszkodą portową — pozostało prób: %1")
                   .arg(m_mistakesRemaining);
    if (m_mistakesRemaining <= 0)
        stop();
    return true;
}

void PrecisionCraneGame::configureCargo()
{
    const int variant = m_delivered % 3;
    m_cargoType = variant == 0 ? LightCargo : (variant == 1 ? StandardCargo : HeavyCargo);
    m_cargoMassKg = m_cargoType == LightCargo ? 0.65 :
                    (m_cargoType == HeavyCargo ? 1.65 : 1.0);
    m_ropeLength = (m_cargoType == LightCargo ? 118.0 :
                    (m_cargoType == HeavyCargo ? 205.0 : 155.0)) *
                   qMax(0.5, height() / 480.0);
}

void PrecisionCraneGame::loadRecords()
{
    const QString path = m_recordsFilePath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
              + QStringLiteral("/precision_crane_records.json")
        : m_recordsFilePath;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) return;
    const QJsonObject record = document.object();
    m_bestScore = qMax(0, record.value(QStringLiteral("bestScore")).toInt());
    m_lifetimeDeliveries = qMax(0, record.value(QStringLiteral("totalDeliveries")).toInt());
    m_lifetimePrecisionTotal = qMax(0, record.value(QStringLiteral("precisionTotal")).toInt());
    m_lifetimePrecisionSamples = qMax(0, record.value(QStringLiteral("precisionSamples")).toInt());
    m_lifetimeCollisions = qMax(0, record.value(QStringLiteral("totalCollisions")).toInt());
}

void PrecisionCraneGame::saveRecords()
{
    const QString path = m_recordsFilePath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
              + QStringLiteral("/precision_crane_records.json")
        : m_recordsFilePath;
    const QString directory = QFileInfo(path).absolutePath();
    if (path.isEmpty() || directory.isEmpty() || !QDir().mkpath(directory)) return;
    m_bestScore = qMax(m_bestScore, getScore());
    QJsonObject record;
    record.insert(QStringLiteral("schemaVersion"), 1);
    record.insert(QStringLiteral("bestScore"), m_bestScore);
    record.insert(QStringLiteral("totalDeliveries"), m_lifetimeDeliveries);
    record.insert(QStringLiteral("precisionTotal"), m_lifetimePrecisionTotal);
    record.insert(QStringLiteral("precisionSamples"), m_lifetimePrecisionSamples);
    record.insert(QStringLiteral("averagePrecision"), m_lifetimePrecisionSamples > 0
        ? static_cast<double>(m_lifetimePrecisionTotal) / m_lifetimePrecisionSamples : 0.0);
    record.insert(QStringLiteral("totalCollisions"), m_lifetimeCollisions);
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(QJsonDocument(record).toJson(QJsonDocument::Indented));
    file.commit();
}

void PrecisionCraneGame::resizeEvent(QResizeEvent *event)
{
    const QSize oldSize = event->oldSize();
    const QSize newSize = event->size();
    if (oldSize.width() > 0 && oldSize.height() > 0 &&
        newSize.width() > 0 && newSize.height() > 0) {
        const double scaleX = static_cast<double>(newSize.width()) / oldSize.width();
        const double scaleY = static_cast<double>(newSize.height()) / oldSize.height();
        m_carriageX *= scaleX;
        m_previousCarriageX *= scaleX;
        m_trolleyVelocity *= scaleX;
        m_sway *= scaleX;
        m_swayVelocity *= scaleX;
        m_hookY *= scaleY;
        m_ropeLength *= scaleY;
    }
    GameEngine::resizeEvent(event);
    m_carriageX = clampCarriage(m_carriageX);
    m_hookY = qBound(height() * 0.24, m_hookY,
                     floorY() - 35.0 * qMax(0.5, height() / 480.0));

    // A resize changes the coordinate system. Start interpolation from the
    // transformed state, never from a position in the previous viewport.
    m_renderPreviousCarriageX = m_carriageX;
    m_previousHookY = m_hookY;
    m_previousSway = m_sway;
    m_previousObstacleClock = m_obstacleClock;
}

void PrecisionCraneGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const double carriage = m_carriageX;
    const double hook = m_hookY;
    const double sway = m_sway;
    const double obstacleClock = m_obstacleClock;
    m_carriageX = m_renderPreviousCarriageX +
        (carriage - m_renderPreviousCarriageX) * alpha;
    m_hookY = m_previousHookY + (hook - m_previousHookY) * alpha;
    m_sway = m_previousSway + (sway - m_previousSway) * alpha;
    m_obstacleClock = m_previousObstacleClock +
        (obstacleClock - m_previousObstacleClock) * alpha;
    render(painter);
    m_carriageX = carriage;
    m_hookY = hook;
    m_sway = sway;
    m_obstacleClock = obstacleClock;
}

void PrecisionCraneGame::render(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const double w = width(), h = height();
    const double ground = floorY();

    painter.fillRect(rect(), QColor(18, 51, 72));

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

    // Dynamic port obstacles occupy the middle air corridor.
    painter.setPen(QPen(QColor(25, 34, 43), 2.0));
    for (int i = 0; i < m_obstacles.size(); ++i) {
        const QRectF obstacle = obstacleRect(i);
        painter.setBrush(i == 0 ? QColor(198, 92, 65) : QColor(86, 139, 177));
        painter.drawRoundedRect(obstacle, 4.0, 4.0);
        painter.setPen(QPen(QColor(255, 225, 185, 150), 1.0));
        for (int stripe = 1; stripe < 4; ++stripe) {
            const double x = obstacle.left() + obstacle.width() * stripe / 4.0;
            painter.drawLine(QPointF(x, obstacle.top() + 3.0), QPointF(x, obstacle.bottom() - 3.0));
        }
        painter.setPen(QPen(QColor(25, 34, 43), 2.0));
    }

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
        QStringLiteral("Oś: %1   Siła: %2%   Seria: %3   Premia: +%4")
            .arg(m_axis == HorizontalAxis ? QStringLiteral("POZIOMA") : QStringLiteral("PIONOWA"))
            .arg(static_cast<int>(std::round(m_force * 100.0))).arg(m_combo)
            .arg(m_lastPrecisionBonus));
    painter.setPen(QColor(210, 230, 238));
    painter.drawText(QRectF(16.0, 94.0, w - 32.0, 18.0), Qt::AlignCenter,
        QStringLiteral("MISJA %1/5 • ŁADUNEK %2 • MASA %3 kg • REKORD %4")
            .arg(qMin(5, m_missionIndex + 1))
            .arg(m_cargoType == LightCargo ? QStringLiteral("LEKKI") :
                 (m_cargoType == HeavyCargo ? QStringLiteral("CIĘŻKI") : QStringLiteral("STANDARDOWY")))
            .arg(m_cargoMassKg, 0, 'f', 1).arg(m_bestScore));

    painter.setPen(QColor(255, 245, 218));
    painter.drawText(QRectF(16.0, h - 34.0, w - 32.0, 24.0), Qt::AlignCenter, m_status);
    painter.restore();
}

} // namespace games
