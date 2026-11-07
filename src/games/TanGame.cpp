#include "games/TanGame.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>
#include <cmath>

namespace games {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

TanGame::TanGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.0)
    , m_targetY(0.0)
    , m_phase(0.0)
    , m_cursorY(0.0)
    , m_isHit(false)
    , m_difficultyLevel(1)
    , m_scrollSpeed(120.0)
    , m_tolerance(42.0)
    , m_frequency(0.016)
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
    m_targetY = height() / 2.0;
    m_cursorY = height() / 2.0;
    m_phase = 0.0;
    m_isHit = false;
}

void TanGame::setDifficultyLevel(int level)
{
    m_difficultyLevel = qBound(1, level, 5);
    m_scrollSpeed = 80.0 + m_difficultyLevel * 40.0;
    m_tolerance = qMax(10.0, 50.0 - m_difficultyLevel * 8.0);
    m_frequency = 0.010 + m_difficultyLevel * 0.004;
    m_isHit = false;
    setScore(0);
}

int TanGame::getDifficultyLevel() const
{
    return m_difficultyLevel;
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
    const double normalized = value / MAX_FORCE;
    m_currentForce = qBound(MIN_FORCE, normalized, 1.0);
    return true;
}

void TanGame::calculateTargetY(double x)
{
    const double amplitude = height() * 0.38;
    const double centerY = height() / 2.0;

    // Tangens jest ograniczany do bezpiecznego zakresu, aby asymptoty
    // nie generowały nieskończonych współrzędnych ani artefaktów QPainter.
    const double angle = std::remainder(x * m_frequency + m_phase, kPi);
    const double tangent = std::tan(angle);
    const double boundedTangent = qBound(-1.0, tangent, 1.0);
    m_targetY = centerY - boundedTangent * amplitude;
}

void TanGame::update(double deltaTime)
{
    if (!isRunning() || width() <= 0 || height() <= 0)
        return;

    // Faza jest w radianach; prędkość przesuwania [px/s] przeliczamy
    // przez częstotliwość przestrzenną [rad/px].
    m_phase = std::remainder(
        m_phase - m_scrollSpeed * m_frequency * deltaTime, kPi);

    const double checkX = width() * 0.75;
    calculateTargetY(checkX);

    const double desiredY = height() * (1.0 - m_currentForce);
    m_cursorY = m_cursorY * 0.8 + desiredY * 0.2;

    const double distance = std::abs(m_cursorY - m_targetY);
    const double exitTolerance = m_tolerance * 1.5;
    if (distance < (m_isHit ? exitTolerance : m_tolerance)) {
        if (!m_isHit) {
            m_isHit = true;
            const double accuracy = qBound(
                0.0, (m_tolerance - distance) / m_tolerance, 1.0);
            addScore(qBound(1, static_cast<int>(std::ceil(accuracy * 10.0)), 10));
        }
    } else {
        m_isHit = false;
    }
}

void TanGame::render(QPainter& painter)
{
    const int w = width();
    const int h = height();
    const double centerY = h / 2.0;
    const double amplitude = h * 0.38;
    const double checkX = w * 0.75;

    painter.fillRect(rect(), QColor(24, 18, 38));
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(65, 55, 85), 1, Qt::DotLine));
    for (int i = 1; i < 4; ++i)
        painter.drawLine(0, h * i / 4, w, h * i / 4);

    QPainterPath tangentPath;
    bool hasPrevious = false;
    double previousY = 0.0;
    for (int x = 0; x <= w; ++x) {
        const double angle = std::remainder(x * m_frequency + m_phase, kPi);
        const double tangent = std::tan(angle);
        const double y = centerY - qBound(-1.0, tangent, 1.0) * amplitude;

        // Przerwij ścieżkę na skoku przy asymptocie tangensa.
        if (!hasPrevious || std::abs(y - previousY) > h * 0.45)
            tangentPath.moveTo(x, y);
        else
            tangentPath.lineTo(x, y);

        previousY = y;
        hasPrevious = true;
    }

    painter.setPen(QPen(getDifficultyColor(), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(tangentPath);

    painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
    painter.drawLine(QPointF(checkX, 0), QPointF(checkX, h));

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 11));
    painter.drawText(12, 22, QStringLiteral("Dopasuj siłę do wykresu tangensa"));
    painter.drawText(12, 44, QStringLiteral("Siła: %1%").arg(qRound(m_currentForce * 100.0)));
    painter.drawText(12, 66, QStringLiteral("Poziom: %1").arg(m_difficultyLevel));
    painter.drawText(12, 88, QStringLiteral("Tolerancja: %1 px").arg(qRound(m_tolerance)));

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::cyan);
    painter.drawEllipse(QPointF(checkX, m_cursorY), 9, 9);

    painter.setBrush(m_isHit ? Qt::green : Qt::red);
    painter.drawEllipse(QPointF(checkX, m_targetY), m_isHit ? 13 : 6, m_isHit ? 13 : 6);

    painter.setPen(Qt::white);
    painter.setFont(QFont(QStringLiteral("Arial"), 20, QFont::Bold));
    painter.drawText(w - 150, 32, QStringLiteral("Wynik: %1").arg(getScore()));

    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(Qt::red);
        painter.setFont(QFont(QStringLiteral("Arial"), 12, QFont::Bold));
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter,
                         QStringLiteral("BRAK POŁĄCZENIA Z CZUJNIKIEM"));
    }
}

QColor TanGame::getDifficultyColor() const
{
    switch (m_difficultyLevel) {
        case 1: return QColor(90, 210, 150);
        case 2: return QColor(70, 205, 190);
        case 3: return QColor(230, 205, 80);
        case 4: return QColor(240, 145, 70);
        default: return QColor(235, 85, 100);
    }
}

} // namespace games
