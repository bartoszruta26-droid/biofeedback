#include "games/FunctionTrackGame.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QFont>
#include <QResizeEvent>
#include <QtMath>
#include <cmath>

namespace games {
namespace {
constexpr double kPi = 3.14159265358979323846;
double smoothClamp(double value) { return std::tanh(value * 1.35) / std::tanh(1.35); }
}

FunctionTrackGame::FunctionTrackGame(Mode mode, QWidget* parent)
    : GameEngine(parent), m_mode(mode)
{
}

QString FunctionTrackGame::gameName() const
{
    switch (m_mode) {
    case Mode::Cotangent: return QStringLiteral("Kotangens — ciągły tor siły");
    case Mode::Trapezoid: return QStringLiteral("Trapez — tor siły");
    case Mode::Mixed: return QStringLiteral("Funkcje mieszane — tor siły");
    }
    return QStringLiteral("Tor funkcji");
}

void FunctionTrackGame::initialize()
{
    m_phase = 0.0;
    m_previousPhase = 0.0;
    m_force = 0.0;
    m_cursorY = height() * 0.5;
    m_previousCursorY = m_cursorY;
    m_inputAge = 1.0;
    m_elapsed = m_hitTime = m_scoreFraction = 0.0;
    m_feedback = QStringLiteral("Oczekiwanie na sygnał z tensometru");
    updateTarget();
    m_previousTargetY = m_targetY;
}

void FunctionTrackGame::setDifficultyLevel(int level)
{
    m_difficulty = qBound(1, level, 5);
    // Tolerance is deliberately independent of screen resolution and track width.
    m_tolerance = height() * (0.22 - (m_difficulty - 1) * 0.012);
}

int FunctionTrackGame::difficultyLevel() const { return m_difficulty; }

void FunctionTrackGame::setTrackWidth(double normalizedWidth)
{
    if (std::isfinite(normalizedWidth))
        m_trackWidth = qBound(0.12, normalizedWidth, 0.48);
    updateTarget();
}

double FunctionTrackGame::trackWidth() const { return m_trackWidth; }
void FunctionTrackGame::setSinusWidth(double value) {
    if (std::isfinite(value)) m_sinusWidth = qBound(0.12, value, 0.48);
    updateTarget();
}
void FunctionTrackGame::setTangentWidth(double value) {
    if (std::isfinite(value)) m_tangentWidth = qBound(0.12, value, 0.48);
    updateTarget();
}
void FunctionTrackGame::setTrapezoidWidth(double value) {
    if (std::isfinite(value)) m_trapezoidWidth = qBound(0.12, value, 0.48);
    updateTarget();
}

bool FunctionTrackGame::handleInput(double value)
{
    if (!std::isfinite(value)) return false;
    // TrainingTab forwards the same normalized 0..100 force signal to every game.
    m_force = qBound(0.0, value / 100.0, 1.0);
    m_inputAge = 0.0;
    return true;
}

double FunctionTrackGame::targetAt(double x) const
{
    const double w = qMax(1, width());
    const double h = qMax(1, height());
    const double phase = x / w * 2.0 * kPi + m_phase;
    double selectedWidth = m_trackWidth;
    double value = 0.0;

    auto tangentLike = [](double angle, bool cotangent) {
        const double numerator = cotangent ? std::cos(angle) : std::sin(angle);
        const double denominator = cotangent ? std::abs(std::sin(angle)) + 0.20
                                             : std::abs(std::cos(angle)) + 0.20;
        return smoothClamp(2.4 * numerator / denominator);
    };
    auto trapezoid = [](double angle) {
        const double u = std::fmod(angle / (2.0 * kPi) + 1.0, 1.0);
        if (u < 0.18) return -1.0;
        if (u < 0.40) return -1.0 + (u - 0.18) / 0.22 * 2.0;
        if (u < 0.66) return 1.0;
        if (u < 0.88) return 1.0 - (u - 0.66) / 0.22 * 2.0;
        return -1.0;
    };
    auto smoothBlend = [](double from, double to, double amount) {
        const double t = qBound(0.0, amount, 1.0);
        const double eased = t * t * (3.0 - 2.0 * t);
        return from + (to - from) * eased;
    };

    if (m_mode == Mode::Cotangent) {
        value = tangentLike(phase, true);
        selectedWidth = m_tangentWidth;
    } else if (m_mode == Mode::Trapezoid) {
        value = trapezoid(phase);
        selectedWidth = m_trapezoidWidth;
    } else {
        const double cycle = std::fmod(m_elapsed, 18.0);
        const double sinus = std::sin(phase);
        const double tangent = tangentLike(phase, false);
        const double cotangent = tangentLike(phase, true);
        const double trapezoidValue = trapezoid(phase);
        const double cosinus = std::cos(phase);
        if (cycle < 3.0) {
            value = sinus;
            selectedWidth = m_sinusWidth;
        } else if (cycle < 6.0) {
            value = cosinus;
            selectedWidth = m_sinusWidth;
        } else if (cycle < 12.0) {
            value = cycle < 9.0 ? tangent : cotangent;
            selectedWidth = m_tangentWidth;
        } else {
            value = trapezoidValue;
            selectedWidth = m_trapezoidWidth;
        }

        // Cross-fade the mathematical segments over one second. Both the
        // target and its amplitude remain continuous at every hand-off.
        if (cycle >= 2.5 && cycle < 3.5) {
            const double t = cycle - 2.5;
            value = smoothBlend(sinus, cosinus, t);
        } else if (cycle >= 5.5 && cycle < 6.5) {
            const double t = cycle - 5.5;
            value = smoothBlend(cosinus, tangent, t);
            selectedWidth = smoothBlend(m_sinusWidth, m_tangentWidth, t);
        } else if (cycle >= 8.5 && cycle < 9.5) {
            const double t = cycle - 8.5;
            value = smoothBlend(tangent, cotangent, t);
        } else if (cycle >= 11.5 && cycle < 12.5) {
            const double t = cycle - 11.5;
            value = smoothBlend(cotangent, trapezoidValue, t);
            selectedWidth = smoothBlend(m_tangentWidth, m_trapezoidWidth, t);
        } else if (cycle >= 17.5) {
            const double t = cycle - 17.5;
            value = smoothBlend(trapezoidValue, sinus, t);
            selectedWidth = smoothBlend(m_trapezoidWidth, m_sinusWidth, t);
        } else if (cycle < 0.5) {
            const double t = cycle + 0.5;
            value = smoothBlend(trapezoidValue, sinus, t);
            selectedWidth = smoothBlend(m_trapezoidWidth, m_sinusWidth, t);
        }
    }
    const double amplitude = h * selectedWidth;
    return h * 0.5 - value * amplitude;
}

void FunctionTrackGame::updateTarget()
{
    m_targetY = targetAt(width() * 0.72);
}

void FunctionTrackGame::update(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0 ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0) return;
    const double dt = qMin(deltaTime, 0.05);
    m_tolerance = height() * (0.22 - (m_difficulty - 1) * 0.012);
    m_previousPhase = m_phase;
    m_previousCursorY = m_cursorY;
    m_previousTargetY = m_targetY;
    m_elapsed += dt;
    m_inputAge += dt;
    m_phase = std::fmod(m_phase + dt * (0.75 + m_difficulty * 0.12), 2.0 * kPi);
    updateTarget();

    const double desiredY = height() * (1.0 - m_force);
    const double alpha = 1.0 - std::exp(-dt / m_smoothingSeconds);
    m_cursorY += (desiredY - m_cursorY) * alpha;

    if (m_inputAge > 0.35) {
        m_hitTime = 0.0;
        m_feedback = QStringLiteral("Brak aktualnego sygnału — sprawdź tensometr");
        return;
    }
    const double distance = std::abs(m_cursorY - m_targetY);
    const double tolerance = m_tolerance;
    if (distance <= tolerance) {
        m_hitTime += dt;
        const double accuracy = qBound(0.0, 1.0 - distance / tolerance, 1.0);
        m_scoreFraction += (2.0 + 4.0 * accuracy) * dt;
        const int earned = static_cast<int>(m_scoreFraction);
        if (earned > 0) { addScore(earned); m_scoreFraction -= earned; }
        m_feedback = QStringLiteral("Dobrze — utrzymaj płynny nacisk");
    } else {
        m_hitTime = 0.0;
        m_feedback = m_cursorY > m_targetY
            ? QStringLiteral("Zwiększ siłę") : QStringLiteral("Zmniejsz siłę");
    }
}

void FunctionTrackGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const double phase = m_phase, cursor = m_cursorY, target = m_targetY;
    // Interpolate along the forward circular phase, not through the full
    // range when the phase wraps from 2*pi back to zero.
    double phaseDelta = std::fmod(phase - m_previousPhase + 2.0 * kPi, 2.0 * kPi);
    if (phaseDelta < 0.0) phaseDelta += 2.0 * kPi;
    m_phase = std::fmod(m_previousPhase + phaseDelta * alpha, 2.0 * kPi);
    m_cursorY = m_previousCursorY + (cursor - m_previousCursorY) * alpha;
    m_targetY = m_previousTargetY + (target - m_previousTargetY) * alpha;
    render(painter);
    m_phase = phase; m_cursorY = cursor; m_targetY = target;
}

void FunctionTrackGame::render(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const int w = width(), h = height();
    if (w <= 0 || h <= 0) { painter.restore(); return; }
    painter.fillRect(rect(), QColor(15, 22, 36));
    QPainterPath path;
    QPainterPath toleranceBand;
    for (int x = 0; x <= w; ++x) {
        const double y = targetAt(x);
        if (x == 0) {
            path.moveTo(x, y);
            toleranceBand.moveTo(x, y - m_tolerance);
        } else {
            path.lineTo(x, y);
            toleranceBand.lineTo(x, y - m_tolerance);
        }
    }
    for (int x = w; x >= 0; --x)
        toleranceBand.lineTo(x, targetAt(x) + m_tolerance);
    toleranceBand.closeSubpath();

    // Match the visible acceptance corridor to the tolerance used for scoring.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(32, 190, 130, 48));
    painter.drawPath(toleranceBand);
    painter.setPen(QPen(QColor(90, 215, 170), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);

    const double gateX = w * 0.72;
    const double tolerance = m_tolerance;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(32, 190, 130, 78));
    painter.drawRoundedRect(QRectF(gateX - 25.0, m_targetY - tolerance,
                                   50.0, 2.0 * tolerance), 8.0, 8.0);
    painter.setPen(QPen(QColor(125, 245, 190, 235), 2.0, Qt::DashLine));
    painter.drawLine(QPointF(gateX - 28, m_targetY - tolerance),
                     QPointF(gateX + 28, m_targetY - tolerance));
    painter.drawLine(QPointF(gateX - 28, m_targetY + tolerance),
                     QPointF(gateX + 28, m_targetY + tolerance));
    painter.setPen(QPen(QColor(235, 240, 250, 150), 1.5, Qt::DashLine));
    painter.drawLine(QPointF(gateX, 0), QPointF(gateX, h));
    const bool stale = m_inputAge > 0.35;
    const QColor cursor = stale ? QColor(145,155,170) :
        (std::abs(m_cursorY - m_targetY) <= tolerance ? QColor(75,235,155) : QColor(75,205,245));
    painter.setPen(QPen(QColor(cursor.red(), cursor.green(), cursor.blue(), 100), 1));
    painter.drawLine(QPointF(0, m_cursorY), QPointF(w, m_cursorY));
    painter.setBrush(cursor); painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPointF(gateX, m_cursorY), 10, 10);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(12, h / 34), QFont::Bold));
    painter.drawText(QRect(16, 14, w - 32, 32), Qt::AlignLeft | Qt::AlignVCenter, gameName());
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(11, h / 42)));
    painter.setPen(stale ? QColor(255,190,90) : QColor(170,235,205));
    painter.drawText(QRect(16, 50, w - 32, 28), Qt::AlignLeft | Qt::AlignVCenter, m_feedback);
    painter.setPen(QColor(210,220,235));
    painter.drawText(QRect(16, h - 38, w - 32, 24), Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("Siła: %1%   Wynik: %2   Pasmo błędu: ±%3 px   Poziom: %4")
        .arg(qRound(m_force * 100.0)).arg(getScore()).arg(qRound(m_tolerance)).arg(m_difficulty));
    painter.restore();
}
} // namespace games
