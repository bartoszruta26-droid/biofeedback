#include "games/TanGame.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QtMath>
#include <cmath>
#include <utility>

namespace games {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kSampleTimeoutSeconds = 0.35;
constexpr double kCursorTimeConstantSeconds = 0.055;
constexpr double kAsymptoteGuard = 0.10;

double tangentWave(double angle, double sharpness)
{
    // atan(tan(x)) is bounded to [-pi/2, pi/2] on each branch.
    // The scale controls steepness without creating long clipped plateaus.
    return (2.0 / kPi) * std::atan(sharpness * std::tan(angle));
}
}

TanGame::TanGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.0)
    , m_calibrationMinimum(0.0)
    , m_calibrationMaximum(100.0)
    , m_lastRawForce(0.0)
    , m_tangentSharpness(0.70)
    , m_targetY(0.0)
    , m_phase(0.0)
    , m_cursorY(0.0)
    , m_isHit(false)
    , m_nearAsymptote(false)
    , m_difficultyLevel(1)
    , m_scrollSpeed(105.0)
    , m_tolerance(42.0)
    , m_frequency(0.0)
    , m_inputAge(1.0)
    , m_scoreFraction(0.0)
    , m_trackingSeconds(0.0)
    , m_sessionSeconds(0.0)
    , m_timeInTarget(0.0)
    , m_accuracyWindowSeconds(0.0)
    , m_accuracyWindowHitSeconds(0.0)
    , m_automaticDifficulty(false)
    , m_combo(0)
    , m_feedbackText(QStringLiteral("Oczekiwanie na sygnał z czujnika"))
{
}

QString TanGame::gameName() const
{
    return QStringLiteral("Tangent Force Trainer");
}

void TanGame::initialize()
{
    setDifficultyLevel(1);
    m_currentForce = 0.0;
    m_targetY = height() * 0.5;
    m_cursorY = height() * 0.5;
    m_phase = 0.0;
    m_isHit = false;
    m_nearAsymptote = false;
    m_inputAge = 1.0;
    m_scoreFraction = 0.0;
    m_trackingSeconds = 0.0;
    m_combo = 0;
    m_sessionSeconds = 0.0;
    m_timeInTarget = 0.0;
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
    m_feedbackText = QStringLiteral("Oczekiwanie na sygnał z czujnika");
    updateSpatialFrequency();
}

void TanGame::updateSpatialFrequency()
{
    // 1.5 tangent periods are visible at every viewport width.
    m_frequency = width() > 0
        ? (kPi * 1.5 / static_cast<double>(width())) : 0.0;
}

void TanGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    m_scrollSpeed = 75.0 + m_difficultyLevel * 30.0;
    const double toleranceRatio = 0.115 - (m_difficultyLevel - 1) * 0.013;
    m_tolerance = qMax(12.0,
        qMin(48.0, qMax(240.0, static_cast<double>(height())) * toleranceRatio));
    updateSpatialFrequency();
    m_isHit = false;
    m_trackingSeconds = 0.0;
    m_combo = 0;
    m_scoreFraction = 0.0;
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
}

int TanGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
}

void TanGame::setForceCalibration(double minimum, double maximum)
{
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
        return;
    m_calibrationMinimum = minimum;
    m_calibrationMaximum = maximum;
    m_currentForce = 0.0;
    m_inputAge = 1.0;
    m_trackingSeconds = 0.0;
    m_combo = 0;
    m_feedbackText = QStringLiteral("Kalibracja zmieniona — oczekiwanie na nowy pomiar");
}

double TanGame::forceCalibrationMinimum() const { return m_calibrationMinimum; }
double TanGame::forceCalibrationMaximum() const { return m_calibrationMaximum; }
double TanGame::normalizedForce() const { return m_currentForce; }
void TanGame::setAutomaticDifficulty(bool enabled)
{
    m_automaticDifficulty = enabled;
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
}
bool TanGame::automaticDifficulty() const { return m_automaticDifficulty; }
void TanGame::setTangentSharpness(double sharpness)
{
    if (std::isfinite(sharpness))
        m_tangentSharpness = qBound(0.25, sharpness, 1.5);
}
double TanGame::tangentSharpness() const { return m_tangentSharpness; }

void TanGame::keyPressEvent(QKeyEvent* event)
{
    if (!event || event->isAutoRepeat()) {
        if (event) event->accept();
        return;
    }
    switch (event->key()) {
        case Qt::Key_Plus:
        case Qt::Key_Equal:
        case Qt::Key_Up: setDifficultyLevel(m_difficultyLevel + 1); break;
        case Qt::Key_Minus:
        case Qt::Key_Down: setDifficultyLevel(m_difficultyLevel - 1); break;
        case Qt::Key_A: setAutomaticDifficulty(!m_automaticDifficulty); break;
        case Qt::Key_BracketLeft:
            if (m_lastRawForce < m_calibrationMaximum)
                setForceCalibration(m_lastRawForce, m_calibrationMaximum);
            break;
        case Qt::Key_BracketRight:
            if (m_lastRawForce > m_calibrationMinimum)
                setForceCalibration(m_calibrationMinimum, m_lastRawForce);
            break;
        default: QWidget::keyPressEvent(event); return;
    }
    event->accept();
}

void TanGame::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSpatialFrequency();
    const double ratio = 0.115 - (m_difficultyLevel - 1) * 0.013;
    m_tolerance = qMax(12.0, qMin(48.0, qMax(240.0, static_cast<double>(height())) * ratio));
    QWidget::update();
}

void TanGame::updateAutomaticDifficulty(double deltaTime)
{
    if (!m_automaticDifficulty || m_inputAge > kSampleTimeoutSeconds)
        return;
    m_accuracyWindowSeconds += deltaTime;
    if (m_accuracyWindowSeconds < 12.0)
        return;
    const double ratio = m_accuracyWindowHitSeconds / m_accuracyWindowSeconds;
    if (ratio >= 0.82 && m_difficultyLevel < 5)
        setDifficultyLevel(m_difficultyLevel + 1);
    else if (ratio < 0.42 && m_difficultyLevel > 1)
        setDifficultyLevel(m_difficultyLevel - 1);
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
}

void TanGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    m_serialPort = std::move(serial);
}

void TanGame::disconnectSerial()
{
    m_serialPort.reset();
}

bool TanGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

bool TanGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    m_lastRawForce = value;
    const double span = m_calibrationMaximum - m_calibrationMinimum;
    if (!std::isfinite(span) || span <= 0.0)
        return false;
    // TrainingTab normally provides a percentage. The calibration interval
    // can also be captured from raw force values for a dedicated sensor range.
    m_currentForce = qBound(0.0, (value - m_calibrationMinimum) / span, 1.0);
    m_inputAge = 0.0;
    return true;
}

void TanGame::calculateTargetY(double x)
{
    const double angle = std::remainder(x * m_frequency + m_phase, kPi);
    const double cosine = std::abs(std::cos(angle));
    m_nearAsymptote = cosine < kAsymptoteGuard;

    const double normalized = qBound(-1.0, tangentWave(angle, m_tangentSharpness), 1.0);
    const double amplitude = static_cast<double>(height()) * 0.38;
    const double centerY = static_cast<double>(height()) * 0.5;
    m_targetY = centerY - normalized * amplitude;
}

void TanGame::update(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0 ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    const double dt = qMin(deltaTime, 0.1);
    updateSpatialFrequency();
    m_phase = std::remainder(
        m_phase - m_scrollSpeed * m_frequency * dt, kPi);
    m_inputAge += dt;
    m_sessionSeconds += dt;

    const double h = static_cast<double>(height());
    const double w = static_cast<double>(width());
    const double gateX = w * 0.72;
    calculateTargetY(gateX);

    const double desiredY = h * (1.0 - m_currentForce);
    const double alpha = 1.0 - std::exp(-dt / kCursorTimeConstantSeconds);
    m_cursorY += (desiredY - m_cursorY) * alpha;

    if (m_inputAge > kSampleTimeoutSeconds) {
        m_isHit = false;
        m_trackingSeconds = 0.0;
        m_combo = 0;
        m_feedbackText = QStringLiteral("Brak aktualnego sygnału — sprawdź czujnik");
        return;
    }

    // The singularity is a transition, not a target to chase. Pause scoring
    // around it so the unavoidable top-to-bottom jump is never penalized.
    if (m_nearAsymptote) {
        m_isHit = false;
        m_trackingSeconds = 0.0;
        m_combo = 0;
        m_feedbackText = QStringLiteral("Przejście asymptoty — przygotuj się");
        return;
    }

    const double distance = std::abs(m_cursorY - m_targetY);
    m_isHit = distance <= m_tolerance;

    m_accuracyWindowSeconds += dt;
    if (m_isHit) {
        m_accuracyWindowHitSeconds += dt;
        m_timeInTarget += dt;
        m_trackingSeconds += dt;
        m_combo = qMin(5, 1 + static_cast<int>(m_trackingSeconds / 2.0));
        const double accuracy = qBound(
            0.0, 1.0 - distance / m_tolerance, 1.0);
        const double pointsPerSecond = 2.0 + 4.0 * accuracy + 0.5 * m_combo;
        m_scoreFraction += pointsPerSecond * dt;
        const int earned = static_cast<int>(m_scoreFraction);
        if (earned > 0) {
            addScore(earned);
            m_scoreFraction -= earned;
        }
        m_feedbackText = QStringLiteral("Dobrze — utrzymaj siłę  •  dokładność %1%")
            .arg(qRound(accuracy * 100.0));
        updateAutomaticDifficulty(dt);
    } else {
        m_trackingSeconds = 0.0;
        m_combo = 0;
        m_feedbackText = m_cursorY > m_targetY
            ? QStringLiteral("Zwiększ siłę")
            : QStringLiteral("Zmniejsz siłę");
        updateAutomaticDifficulty(dt);
    }
}

void TanGame::render(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int w = width();
    const int h = height();
    if (w <= 0 || h <= 0) {
        painter.restore();
        return;
    }

    const double widthD = static_cast<double>(w);
    const double heightD = static_cast<double>(h);
    const double centerY = heightD * 0.5;
    const double amplitude = heightD * 0.38;
    const double gateX = widthD * 0.72;

    painter.fillRect(rect(), QColor(18, 20, 35));

    painter.setPen(QPen(QColor(48, 58, 78), 1, Qt::DotLine));
    for (int i = 1; i < 4; ++i) {
        const int y = h * i / 4;
        painter.drawLine(0, y, w, y);
    }
    painter.setPen(QPen(QColor(78, 91, 112), 1));
    painter.drawLine(0, h / 2, w, h / 2);

    QPainterPath tangentPath;
    bool hasPrevious = false;
    bool previousBranchPositive = false;
    double previousY = 0.0;
    for (int x = 0; x <= w; ++x) {
        const double angle = std::remainder(
            static_cast<double>(x) * m_frequency + m_phase, kPi);
        const double cosine = std::cos(angle);
        const double normalized = qBound(-1.0, tangentWave(angle, m_tangentSharpness), 1.0);
        const double y = centerY - normalized * amplitude;
        const bool branchPositive = cosine >= 0.0;

        // Never connect the two sides of a tangent asymptote.
        if (!hasPrevious || branchPositive != previousBranchPositive ||
            std::abs(y - previousY) > heightD * 0.35) {
            tangentPath.moveTo(x, y);
        } else {
            tangentPath.lineTo(x, y);
        }
        previousY = y;
        previousBranchPositive = branchPositive;
        hasPrevious = true;
    }

    painter.setPen(QPen(getDifficultyColor(), 3.0, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(tangentPath);

    // Target gate and its tolerance band.
    painter.setPen(QPen(QColor(230, 238, 248, 150), 1.5, Qt::DashLine));
    painter.drawLine(QPointF(gateX, 0), QPointF(gateX, heightD));
    painter.setPen(QPen(QColor(90, 220, 160, 190), 1, Qt::DashLine));
    painter.drawLine(QPointF(gateX - 22, m_targetY - m_tolerance),
                     QPointF(gateX + 22, m_targetY - m_tolerance));
    painter.drawLine(QPointF(gateX - 22, m_targetY + m_tolerance),
                     QPointF(gateX + 22, m_targetY + m_tolerance));

    const bool stale = m_inputAge > kSampleTimeoutSeconds;
    const QColor cursorColor = stale ? QColor(145, 155, 170)
        : (m_nearAsymptote ? QColor(255, 190, 85)
                           : (m_isHit ? QColor(75, 235, 155) : QColor(75, 205, 245)));
    painter.setPen(QPen(QColor(cursorColor.red(), cursorColor.green(),
                               cursorColor.blue(), 105), 1));
    painter.drawLine(QPointF(0, m_cursorY), QPointF(widthD, m_cursorY));
    painter.setBrush(cursorColor);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPointF(gateX, m_cursorY), 10, 10);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 255, 255, 190), 1.5));
    painter.drawEllipse(QPointF(gateX, m_targetY), 7, 7);

    const int margin = qMax(14, w / 80);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(12, h / 34), QFont::Bold));
    painter.drawText(QRect(margin, margin, w - 2 * margin, 34),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("TANGENS FORCE  •  POZIOM %1").arg(m_difficultyLevel));

    painter.setFont(QFont(QStringLiteral("Arial"), qMax(11, h / 42), QFont::DemiBold));
    painter.setPen(stale ? QColor(255, 190, 90)
        : (m_nearAsymptote ? QColor(255, 190, 90)
                           : (m_isHit ? QColor(90, 245, 165) : QColor(255, 205, 105))));
    painter.drawText(QRect(margin, margin + 38, w - 2 * margin, 30),
                     Qt::AlignLeft | Qt::AlignVCenter, m_feedbackText);

    painter.setPen(QColor(210, 220, 235));
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(10, h / 52)));
    const double targetAccuracy = m_sessionSeconds > 0.0
        ? 100.0 * m_timeInTarget / m_sessionSeconds : 0.0;
    painter.drawText(QRect(margin, h - 78, w - 2 * margin, 22),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("Siła: %1%    Wynik: %2    Seria: x%3    Czas w celu: %4%")
            .arg(qRound(m_currentForce * 100.0)).arg(getScore()).arg(m_combo)
            .arg(qRound(targetAccuracy)));
    painter.drawText(QRect(margin, h - 52, w - 2 * margin, 20),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("Trudność: +/- | Auto: A | Kalibracja: [ minimum, ] maksimum | Nachylenie: %1")
            .arg(m_tangentSharpness, 0, 'f', 2));

    const int meterW = qMax(12, qMin(22, w / 45));
    const int meterX = w - margin - meterW;
    const int meterY = margin + 8;
    const int meterH = qMax(80, h - 2 * margin - 92);
    painter.setPen(QPen(QColor(180, 195, 215), 1));
    painter.setBrush(QColor(28, 39, 54));
    painter.drawRoundedRect(QRect(meterX, meterY, meterW, meterH), 5, 5);
    const int fillH = static_cast<int>(m_currentForce * meterH);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(245, 190, 75));
    if (fillH > 0)
        painter.drawRoundedRect(QRect(meterX + 2, meterY + meterH - fillH,
                                      meterW - 4, qMax(1, fillH - 2)), 3, 3);

    painter.restore();
}

QColor TanGame::getDifficultyColor() const
{
    switch (m_difficultyLevel) {
        case 1: return QColor(90, 220, 150);
        case 2: return QColor(75, 205, 210);
        case 3: return QColor(240, 205, 90);
        case 4: return QColor(245, 155, 85);
        default: return QColor(245, 105, 105);
    }
}

} // namespace games
