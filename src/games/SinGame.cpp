#include "games/SinGame.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPainterPath>
#include <QDebug>
#include <QtMath>
#include <cmath>

namespace games {

SinGame::SinGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.0)
    , m_targetY(0.0)
    , m_phase(0.0)
    , m_cursorY(0.0)
    , m_isHit(false)
    , m_difficultyLevel(1)
    , m_scrollSpeed(115.0)
    , m_tolerance(42.0)
    , m_frequency(0.0)
    , m_inputAge(1.0)
    , m_scoreFraction(0.0)
    , m_comboSeconds(0.0)
    , m_combo(0)
    , m_lastDistance(0.0)
    , m_feedbackText(QStringLiteral("Oczekiwanie na sygnał z czujnika"))
    , m_calibrationMinimum(0.0)
    , m_calibrationMaximum(100.0)
    , m_waveformMode(0)
    , m_automaticDifficulty(false)
    , m_sessionSeconds(0.0)
    , m_timeInTarget(0.0)
    , m_difficultyCheckSeconds(0.0)
    , m_accuracyWindowSeconds(0.0)
    , m_accuracyWindowHitSeconds(0.0)
    , m_bestStreakSeconds(0.0)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
{
}

SinGame::~SinGame()
{
    disconnectSerial();
}

QString SinGame::gameName() const
{
    return QStringLiteral("Sinus Force Trainer");
}

void SinGame::initialize()
{
    setDifficultyLevel(1);
    m_currentForce = 0.0;
    m_cursorY = height() * 0.5;
    m_phase = 0.0;
    m_isHit = false;
    m_inputAge = 1.0;
    m_scoreFraction = 0.0;
    m_comboSeconds = 0.0;
    m_combo = 0;
    m_lastDistance = 0.0;
    m_feedbackText = QStringLiteral("Oczekiwanie na sygnał z czujnika");
    m_sessionSeconds = 0.0;
    m_timeInTarget = 0.0;
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
    m_bestStreakSeconds = 0.0;
    refreshGeometryParameters();
    calculateTargetY(width() * 0.72);
}

void SinGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);

    // Speed is expressed in pixels per second; the waveform is scaled to
    // the current viewport so it always shows the same number of cycles.
    m_scrollSpeed = 90.0 + m_difficultyLevel * 25.0;
    const double toleranceRatio = 0.11 - (m_difficultyLevel - 1) * 0.012;
    m_tolerance = qMax(12.0, qMin(48.0, qMax(240.0, static_cast<double>(height())) * toleranceRatio));
    refreshGeometryParameters();
    m_comboSeconds = 0.0;
    m_combo = 0;
}

int SinGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
}

void SinGame::setForceCalibration(double minimum, double maximum)
{
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
        return;
    m_calibrationMinimum = minimum;
    m_calibrationMaximum = maximum;
    m_currentForce = 0.0;
    m_inputAge = 1.0;
    m_feedbackText = QStringLiteral("Kalibracja zmieniona — oczekiwanie na nowy pomiar");
}

double SinGame::forceCalibrationMinimum() const { return m_calibrationMinimum; }
double SinGame::forceCalibrationMaximum() const { return m_calibrationMaximum; }
double SinGame::normalizedForce() const { return m_currentForce; }

void SinGame::setWaveformMode(int mode)
{
    m_waveformMode = qBound(0, mode, 2);
    m_combo = 0;
    m_comboSeconds = 0.0;
    m_feedbackText = m_waveformMode == 0 ? QStringLiteral("Tryb: fala sinusoidalna")
        : (m_waveformMode == 1 ? QStringLiteral("Tryb: fala trójkątna")
                               : QStringLiteral("Tryb: łagodny plateau"));
}

int SinGame::waveformMode() const { return m_waveformMode; }
void SinGame::setAutomaticDifficulty(bool enabled) { m_automaticDifficulty = enabled; }
bool SinGame::automaticDifficulty() const { return m_automaticDifficulty; }

void SinGame::keyPressEvent(QKeyEvent* event)
{
    if (!event || event->isAutoRepeat()) {
        if (event) event->accept();
        return;
    }
    switch (event->key()) {
        case Qt::Key_1: setWaveformMode(0); break;
        case Qt::Key_2: setWaveformMode(1); break;
        case Qt::Key_3: setWaveformMode(2); break;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
        case Qt::Key_Up: setDifficultyLevel(m_difficultyLevel + 1); break;
        case Qt::Key_Minus:
        case Qt::Key_Down: setDifficultyLevel(m_difficultyLevel - 1); break;
        case Qt::Key_A: setAutomaticDifficulty(!m_automaticDifficulty); break;
        default: QWidget::keyPressEvent(event); return;
    }
    event->accept();
}

void SinGame::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    refreshGeometryParameters();
    calculateTargetY(width() * 0.72);
    update();
}

void SinGame::refreshGeometryParameters()
{
    const double w = qMax(1, width());
    const double h = qMax(240, height());
    m_frequency = 2.0 * 3.14159265358979323846 * 1.5 / w;
    const double toleranceRatio = 0.11 - (m_difficultyLevel - 1) * 0.012;
    m_tolerance = qBound(12.0, h * toleranceRatio, 48.0);
}

double SinGame::waveformValue(double phase) const
{
    const double sine = std::sin(phase);
    if (m_waveformMode == 1)
        return (2.0 / 3.14159265358979323846) * std::asin(sine);
    if (m_waveformMode == 2) {
        const double limit = std::tanh(2.2);
        return std::tanh(2.2 * sine) / limit;
    }
    return sine;
}

void SinGame::updateAutomaticDifficulty(double deltaTime)
{
    if (!m_automaticDifficulty || m_inputAge > 0.35)
        return;
    if (m_accuracyWindowSeconds < 12.0)
        return;
    const double ratio = m_accuracyWindowHitSeconds / m_accuracyWindowSeconds;
    if (ratio >= 0.82 && m_difficultyLevel < 5)
        setDifficultyLevel(m_difficultyLevel + 1);
    else if (ratio < 0.42 && m_difficultyLevel > 1)
        setDifficultyLevel(m_difficultyLevel - 1);
    m_accuracyWindowSeconds = 0.0;
    m_accuracyWindowHitSeconds = 0.0;
    m_difficultyCheckSeconds = 0.0;
}

void SinGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
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

void SinGame::disconnectSerial()
{
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
        m_serialPort.reset();
        m_ownSerial = false;
        emit serialDisconnected();
    } else if (m_serialPort) {
        m_serialPort.reset();
        emit serialDisconnected();
    }
}

bool SinGame::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}

void SinGame::onSensorDataReceived(const sensor::SensorData& data)
{
    if (data.isValid)
        normalizeForceValue(data.calibratedValue);
}

void SinGame::onConnectionStatusChanged(bool connected, const std::string& message)
{
    if (connected) {
        qDebug() << "SinGame: Połączono z sensorem:" << QString::fromStdString(message);
        emit serialConnected();
    } else {
        qDebug() << "SinGame: Rozłączono z sensorem:" << QString::fromStdString(message);
        emit serialError(QString::fromStdString("Rozłączono: " + message));
        emit serialDisconnected();
    }
}

void SinGame::normalizeForceValue(double rawValue)
{
    if (!std::isfinite(rawValue) || !std::isfinite(m_maxForce) || m_maxForce <= 0.0)
        return;

    const double span = m_calibrationMaximum - m_calibrationMinimum;
    if (!std::isfinite(span) || span <= 0.0)
        return;
    const double normalizedForce = (rawValue - m_calibrationMinimum) / span;
    if (!std::isfinite(normalizedForce))
        return;

    m_currentForce = qBound(0.0, normalizedForce, 1.0);
    m_inputAge = 0.0;
}

bool SinGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    // TrainingTab supplies force as a percentage of the exercise target.
    // 0..100 therefore maps to the full vertical control range.
    normalizeForceValue(value);
    return true;
}

void SinGame::calculateTargetY(double x)
{
    const double h = static_cast<double>(height());
    const double amplitude = h * 0.32;
    const double centerY = h * 0.5;
    m_targetY = centerY + waveformValue(x * m_frequency + m_phase) * amplitude;
}

void SinGame::update(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0 ||
        !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    const double dt = qMin(deltaTime, 0.1);
    const double w = static_cast<double>(width());
    const double h = static_cast<double>(height());
    refreshGeometryParameters();
    m_phase -= m_scrollSpeed * m_frequency * dt;
    m_inputAge += dt;
    m_sessionSeconds += dt;

    const double checkX = w * 0.72;
    calculateTargetY(checkX);

    // Time-constant smoothing: unlike a fixed 0.2 blend, this behaves the
    // same at 60, 120 or irregular GUI frame rates.
    const double rawCursorY = h * (1.0 - m_currentForce);
    const double alpha = 1.0 - std::exp(-dt / 0.055);
    m_cursorY += (rawCursorY - m_cursorY) * alpha;

    if (m_inputAge > 0.35) {
        m_isHit = false;
        m_comboSeconds = 0.0;
        m_combo = 0;
        m_feedbackText = QStringLiteral("Brak aktualnego sygnału — sprawdź czujnik");
        return;
    }

    m_lastDistance = std::abs(m_cursorY - m_targetY);
    m_isHit = m_lastDistance <= m_tolerance;
    m_accuracyWindowSeconds += dt;
    if (m_isHit) {
        m_accuracyWindowHitSeconds += dt;
        m_timeInTarget += dt;
        m_comboSeconds += dt;
        m_bestStreakSeconds = qMax(m_bestStreakSeconds, m_comboSeconds);
        m_combo = qMin(5, 1 + static_cast<int>(m_comboSeconds / 2.0));
        const double accuracy = qBound(0.0, 1.0 - m_lastDistance / m_tolerance, 1.0);
        const double pointsPerSecond = 2.0 + 4.0 * accuracy + 0.5 * m_combo;
        m_scoreFraction += pointsPerSecond * dt;
        const int earned = static_cast<int>(m_scoreFraction);
        if (earned > 0) {
            addScore(earned);
            m_scoreFraction -= earned;
        }
        m_feedbackText = QStringLiteral("Dobrze — utrzymaj siłę  •  dokładność %1%")
            .arg(static_cast<int>(std::round(accuracy * 100.0)));
        updateAutomaticDifficulty(dt);
    } else {
        m_comboSeconds = 0.0;
        m_combo = 0;
        m_feedbackText = m_cursorY > m_targetY
            ? QStringLiteral("Zwiększ siłę")
            : QStringLiteral("Zmniejsz siłę");
        updateAutomaticDifficulty(dt);
    }
}

void SinGame::render(QPainter& painter)
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
    const double amplitude = heightD * 0.32;
    const double gateX = widthD * 0.72;
    const double band = m_tolerance;

    painter.fillRect(rect(), QColor(13, 20, 32));

    // Subtle grid and force scale make the direction of control obvious.
    painter.setPen(QPen(QColor(42, 55, 72), 1, Qt::DotLine));
    for (int i = 1; i < 4; ++i) {
        const int y = h * i / 4;
        painter.drawLine(0, y, w, y);
    }
    painter.setPen(QPen(QColor(74, 91, 112), 1));
    painter.drawLine(0, static_cast<int>(centerY), w, static_cast<int>(centerY));

    QPainterPath targetPath;
    QPainterPath targetBand;
    bool first = true;
    for (int x = 0; x <= w; ++x) {
        const double target = centerY +
            waveformValue(static_cast<double>(x) * m_frequency + m_phase) * amplitude;
        if (first) {
            targetPath.moveTo(x, target);
            targetBand.moveTo(x, target - band);
            first = false;
        } else {
            targetPath.lineTo(x, target);
            targetBand.lineTo(x, target - band);
        }
    }
    for (int x = w; x >= 0; --x) {
        const double target = centerY +
            std::sin(static_cast<double>(x) * m_frequency + m_phase) * amplitude;
        targetBand.lineTo(x, target + band);
    }
    targetBand.closeSubpath();
    painter.fillPath(targetBand, QColor(32, 190, 130, 35));

    painter.setPen(QPen(getDifficultyColor(), 3.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(targetPath);

    // Target gate and tolerance zone at the point where the player tracks the wave.
    painter.setPen(QPen(QColor(230, 238, 248, 150), 1.5, Qt::DashLine));
    painter.drawLine(QPointF(gateX, 0), QPointF(gateX, heightD));
    painter.setPen(QPen(QColor(100, 220, 170, 190), 1, Qt::DashLine));
    painter.drawLine(QPointF(gateX - 22, m_targetY - band),
                     QPointF(gateX + 22, m_targetY - band));
    painter.drawLine(QPointF(gateX - 22, m_targetY + band),
                     QPointF(gateX + 22, m_targetY + band));

    // Live cursor: horizontal guide plus a high-contrast marker.
    const QColor cursorColor = m_inputAge > 0.35 ? QColor(145, 155, 170)
        : (m_isHit ? QColor(75, 235, 155) : QColor(75, 205, 245));
    painter.setPen(QPen(QColor(cursorColor.red(), cursorColor.green(), cursorColor.blue(), 105), 1));
    painter.drawLine(QPointF(0, m_cursorY), QPointF(widthD, m_cursorY));
    painter.setBrush(cursorColor);
    painter.setPen(QPen(Qt::white, 2));
    painter.drawEllipse(QPointF(gateX, m_cursorY), 10, 10);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 255, 255, 190), 1.5));
    painter.drawEllipse(QPointF(gateX, m_targetY), 7, 7);

    // Large feedback panel; text remains legible in fullscreen and compact windows.
    const int margin = qMax(14, w / 80);
    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(12, h / 34), QFont::Bold));
    painter.drawText(QRect(margin, margin, w - 2 * margin, 34),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("SINUS FORCE  •  POZIOM %1").arg(m_difficultyLevel));

    painter.setFont(QFont(QStringLiteral("Arial"), qMax(11, h / 42), QFont::DemiBold));
    const QColor feedbackColor = m_inputAge > 0.35 ? QColor(255, 190, 90)
        : (m_isHit ? QColor(90, 245, 165) : QColor(255, 205, 105));
    painter.setPen(feedbackColor);
    painter.drawText(QRect(margin, margin + 38, w - 2 * margin, 30),
                     Qt::AlignLeft | Qt::AlignVCenter, m_feedbackText);

    painter.setPen(QColor(210, 220, 235));
    painter.setFont(QFont(QStringLiteral("Arial"), qMax(10, h / 52)));
    const double accuracyPercent = m_sessionSeconds > 0.0
        ? 100.0 * m_timeInTarget / m_sessionSeconds : 0.0;
    const QString modeName = m_waveformMode == 0 ? QStringLiteral("SINUS")
        : (m_waveformMode == 1 ? QStringLiteral("TRÓJKĄT") : QStringLiteral("PLATEAU"));
    painter.drawText(QRect(margin, h - 78, w - 2 * margin, 22),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("Siła: %1%    Wynik: %2    Seria: x%3    Cel: %4%")
            .arg(static_cast<int>(std::round(m_currentForce * 100.0)))
            .arg(getScore()).arg(m_combo)
            .arg(static_cast<int>(std::round(accuracyPercent))));

    painter.drawText(QRect(margin, h - 52, w - 2 * margin, 20),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("%1 | Kalibracja: %2–%3 | Auto: %4 | 1-3: tryb, +/-: trudność, A: auto")
            .arg(modeName)
            .arg(m_calibrationMinimum, 0, 'f', 1)
            .arg(m_calibrationMaximum, 0, 'f', 1)
            .arg(m_automaticDifficulty ? QStringLiteral("wł.") : QStringLiteral("wył.")));

    // Compact force meter with a clear 0..100% scale.
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
    painter.drawRoundedRect(QRect(meterX + 2, meterY + meterH - fillH,
                                  meterW - 4, qMax(0, fillH - 2)), 3, 3);
    painter.restore();
}

QColor SinGame::getDifficultyColor() const
{
    switch (m_difficultyLevel) {
        case 1: return QColor(80, 220, 150);
        case 2: return QColor(75, 205, 220);
        case 3: return QColor(245, 205, 90);
        case 4: return QColor(245, 155, 85);
        default: return QColor(245, 105, 105);
    }
}

} // namespace games
