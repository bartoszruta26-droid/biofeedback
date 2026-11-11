#include "games/RhythmForceGame.hpp"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace games {

namespace {
constexpr double kForceTriggerPercent = 18.0;
constexpr double kPulseStrengthTolerance = 35.0;
}

RhythmForceGame::RhythmForceGame(QWidget *parent)
    : GameEngine(parent)
    , m_difficulty(1)
    , m_nextBeatIndex(1)
    , m_hits(0)
    , m_misses(0)
    , m_perfectHits(0)
    , m_combo(0)
    , m_bestCombo(0)
    , m_songTime(0.0)
    , m_inputAge(kSignalTimeoutSeconds + 1.0)
    , m_pulseElapsed(0.0)
    , m_pulseStartedAt(0.0)
    , m_pulsePeakForce(0.0)
    , m_lastTimingOffsetMs(0.0)
    , m_lastForce(0.0)
    , m_hasSensorSample(false)
    , m_awaitingNeutral(true)
    , m_pulseActive(false)
    , m_roundCompleted(false)
    , m_lastGrade(NoGrade)
    , m_status(QStringLiteral("Oczekiwanie na neutralny sygnał tensometru"))
{
    setMinimumSize(320, 240);
}

QString RhythmForceGame::gameName() const
{
    return QStringLiteral("Rhythm Force — Rytm siły");
}

void RhythmForceGame::setDifficultyLevel(int level)
{
    m_difficulty = qBound(1, level, 5);
    QWidget::update();
}

int RhythmForceGame::beatsPerMinute() const
{
    return 72 + (m_difficulty - 1) * 12;
}

double RhythmForceGame::beatIntervalSeconds() const
{
    return 60.0 / static_cast<double>(beatsPerMinute());
}

double RhythmForceGame::beatTimeSeconds(int beatIndex) const
{
    return kLeadInSeconds + static_cast<double>(beatIndex - 1) * beatIntervalSeconds();
}

double RhythmForceGame::timingWindowSeconds() const
{
    return 0.22 - static_cast<double>(m_difficulty - 1) * 0.02;
}

double RhythmForceGame::perfectWindowSeconds() const
{
    return qMax(0.035, 0.075 - static_cast<double>(m_difficulty - 1) * 0.008);
}

double RhythmForceGame::targetForceForBeat(int beatIndex) const
{
    // Deterministic 40–84% accents: repeatable in tests and learnable in play.
    return 40.0 + static_cast<double>((beatIndex * 17 + m_difficulty * 11) % 45);
}

double RhythmForceGame::nextTargetForcePercent() const
{
    return m_nextBeatIndex <= kRoundBeats ? targetForceForBeat(m_nextBeatIndex) : 0.0;
}

double RhythmForceGame::accuracyPercent() const
{
    const int attempts = m_hits + m_misses;
    return attempts > 0 ? 100.0 * static_cast<double>(m_hits) / attempts : 0.0;
}

bool RhythmForceGame::hasFreshSignal() const
{
    return m_hasSensorSample && !m_awaitingNeutral &&
           std::isfinite(m_inputAge) && m_inputAge <= kSignalTimeoutSeconds;
}

void RhythmForceGame::initialize()
{
    m_nextBeatIndex = 1;
    m_hits = 0;
    m_misses = 0;
    m_perfectHits = 0;
    m_combo = 0;
    m_bestCombo = 0;
    m_songTime = 0.0;
    m_inputAge = kSignalTimeoutSeconds + 1.0;
    m_pulseElapsed = 0.0;
    m_pulseStartedAt = 0.0;
    m_pulsePeakForce = 0.0;
    m_lastTimingOffsetMs = 0.0;
    m_lastForce = 0.0;
    m_hasSensorSample = false;
    m_awaitingNeutral = true;
    m_pulseActive = false;
    m_roundCompleted = false;
    m_lastGrade = NoGrade;
    m_status = QStringLiteral("Oczekiwanie na neutralny sygnał tensometru");
    setScore(0);
}

bool RhythmForceGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    const double force = qBound(0.0, value, 100.0);
    const bool wasFresh = hasFreshSignal();
    m_hasSensorSample = true;
    m_inputAge = 0.0;

    // After startup or signal loss, require a neutral sample before accepting
    // a new pulse. A reconnecting high value must never create a phantom hit.
    if (m_awaitingNeutral || !wasFresh) {
        m_pulseActive = false;
        m_pulseElapsed = 0.0;
        m_lastForce = force;
        if (force <= kRestThresholdPercent) {
            m_awaitingNeutral = false;
            m_status = QStringLiteral("Sygnał gotowy — czekaj na takt");
        } else {
            m_awaitingNeutral = true;
            m_status = QStringLiteral("Zwolnij nacisk, aby uzbroić wejście");
        }
        return true;
    }

    if (!m_pulseActive && force > kForceTriggerPercent) {
        m_pulseActive = true;
        m_pulseStartedAt = m_songTime;
        m_pulseElapsed = 0.0;
        m_pulsePeakForce = force;
        m_status = QStringLiteral("Impuls — zwolnij nacisk");
    } else if (m_pulseActive) {
        m_pulsePeakForce = qMax(m_pulsePeakForce, force);
        if (force <= kRestThresholdPercent)
            finishPulse();
    }

    m_lastForce = force;
    return true;
}

void RhythmForceGame::finishPulse()
{
    const double duration = m_pulseElapsed;
    const double onset = m_pulseStartedAt;
    const double peak = m_pulsePeakForce;
    m_pulseActive = false;
    m_pulseElapsed = 0.0;

    const int nearestBeat = qMax(1, qRound((onset - kLeadInSeconds) / beatIntervalSeconds()) + 1);
    if (duration < kMinimumPulseSeconds || duration > kMaximumPulseSeconds) {
        registerMiss();
        if (nearestBeat == m_nextBeatIndex &&
            std::abs(onset - beatTimeSeconds(m_nextBeatIndex)) <= timingWindowSeconds()) {
            advanceBeat();
            finishRoundIfReady();
        }
        m_status = QStringLiteral("Nieprawidłowa długość impulsu");
        return;
    }

    if (nearestBeat < m_nextBeatIndex || nearestBeat > kRoundBeats) {
        registerMiss();
        m_status = QStringLiteral("Impuls poza aktywnym taktem");
        return;
    }

    // Notes that have already passed their late window are misses, never hits.
    while (m_nextBeatIndex < nearestBeat) {
        registerMiss();
        advanceBeat();
    }
    if (m_nextBeatIndex > kRoundBeats)
        return;

    const double offset = onset - beatTimeSeconds(m_nextBeatIndex);
    m_lastTimingOffsetMs = offset * 1000.0;
    if (std::abs(offset) > timingWindowSeconds()) {
        registerMiss();
        m_status = offset < 0.0
            ? QStringLiteral("Za wcześnie — kolejny takt pozostaje aktywny")
            : QStringLiteral("Za późno — takt utracony");
        if (offset > 0.0)
            advanceBeat();
        finishRoundIfReady();
        return;
    }

    int timingPoints = 40;
    if (std::abs(offset) <= perfectWindowSeconds()) {
        timingPoints = 100;
        m_lastGrade = Perfect;
        ++m_perfectHits;
    } else if (std::abs(offset) <= timingWindowSeconds() * 0.62) {
        timingPoints = 70;
        m_lastGrade = Good;
    } else {
        m_lastGrade = Okay;
    }

    const double forceError = std::abs(peak - targetForceForBeat(m_nextBeatIndex));
    const int strengthPoints = qRound(50.0 * qBound(0.0, 1.0 - forceError / kPulseStrengthTolerance, 1.0));
    const int comboBonus = qMin(50, m_combo * 2);
    ++m_hits;
    ++m_combo;
    m_bestCombo = qMax(m_bestCombo, m_combo);
    setScore(getScore() + timingPoints + strengthPoints + comboBonus);
    m_status = QStringLiteral("%1  |  czas %2 ms  |  siła %3%")
        .arg(m_lastGrade == Perfect ? QStringLiteral("PERFEKT")
             : (m_lastGrade == Good ? QStringLiteral("DOBRZE") : QStringLiteral("TRAFIENIE")))
        .arg(qRound(m_lastTimingOffsetMs))
        .arg(qRound(peak));
    advanceBeat();
    finishRoundIfReady();
}

void RhythmForceGame::registerMiss()
{
    ++m_misses;
    m_combo = 0;
    m_lastGrade = Miss;
}

void RhythmForceGame::advanceBeat()
{
    ++m_nextBeatIndex;
}

void RhythmForceGame::finishRoundIfReady()
{
    if (m_nextBeatIndex > kRoundBeats && !m_roundCompleted) {
        m_roundCompleted = true;
        m_status = QStringLiteral("Runda zakończona");
        stop();
    }
}

void RhythmForceGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    m_inputAge += qMin(deltaTime, 0.1);
    if (!m_hasSensorSample || m_inputAge > kSignalTimeoutSeconds) {
        if (m_pulseActive) {
            m_pulseActive = false;
            m_pulseElapsed = 0.0;
        }
        m_awaitingNeutral = true;
        m_lastForce = 0.0;
        m_status = QStringLiteral("Brak świeżego sygnału — rytm wstrzymany");
        return; // Freeze the chart; a disconnected sensor cannot cause misses.
    }

    m_songTime += qMin(deltaTime, 0.1);
    if (m_pulseActive)
        m_pulseElapsed += qMin(deltaTime, 0.1);

    while (m_nextBeatIndex <= kRoundBeats &&
           m_songTime > beatTimeSeconds(m_nextBeatIndex) + timingWindowSeconds()) {
        // A pulse that began inside the active note window may legitimately
        // finish later; defer expiry until its release determines pulse length.
        const bool activePulseBelongsToBeat = m_pulseActive &&
            std::abs(m_pulseStartedAt - beatTimeSeconds(m_nextBeatIndex)) <= timingWindowSeconds();
        if (activePulseBelongsToBeat)
            break;
        registerMiss();
        advanceBeat();
    }
    finishRoundIfReady();
}

void RhythmForceGame::render(QPainter &painter)
{
    const int w = width();
    const int h = height();
    painter.fillRect(rect(), QColor(13, 18, 34));
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int margin = qBound(12, w / 28, 28);
    const int titleSize = qBound(12, h / 22, 24);
    painter.setPen(QColor(240, 244, 255));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), titleSize, QFont::DemiBold));
    painter.drawText(QRect(margin, margin, w - 2 * margin, titleSize + 8),
                     Qt::AlignLeft, QStringLiteral("RHYTHM FORCE"));

    const int hudSize = qBound(8, h / 42, 13);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), hudSize));
    painter.setPen(QColor(180, 195, 220));
    const QString stats = QStringLiteral("%1 BPM   •   Wynik %2   •   Seria %3   •   Trafienia %4   •   Pudła %5")
        .arg(beatsPerMinute()).arg(getScore()).arg(m_combo).arg(m_hits).arg(m_misses);
    painter.drawText(QRect(margin, margin + titleSize + 10, w - 2 * margin, hudSize + 8),
                     Qt::AlignLeft, stats);

    const QRectF lane(margin, h * 0.37, w - 2.0 * margin, h * 0.28);
    painter.setPen(QPen(QColor(70, 88, 120), 1));
    painter.setBrush(QColor(25, 34, 56));
    painter.drawRoundedRect(lane, 12, 12);

    const double centreX = lane.center().x();
    painter.setPen(QPen(QColor(245, 220, 110), 2));
    painter.drawLine(QPointF(centreX, lane.top() - 12), QPointF(centreX, lane.bottom() + 12));
    painter.setPen(Qt::NoPen);

    const double interval = beatIntervalSeconds();
    for (int beat = m_nextBeatIndex; beat <= qMin(kRoundBeats, m_nextBeatIndex + 5); ++beat) {
        const double secondsToBeat = beatTimeSeconds(beat) - m_songTime;
        const double x = centreX + secondsToBeat / (interval * 2.5) * lane.width();
        if (x < lane.left() - 18 || x > lane.right() + 18)
            continue;
        const double targetForce = targetForceForBeat(beat);
        const double radius = 8.0 + targetForce / 18.0;
        const QColor accent = beat == m_nextBeatIndex
            ? QColor(80, 205, 240) : QColor(95, 125, 170);
        painter.setBrush(accent);
        painter.drawEllipse(QPointF(x, lane.center().y()), radius, radius);
        painter.setPen(QColor(225, 235, 250));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(7, hudSize - 1)));
        painter.drawText(QRectF(x - 22, lane.bottom() + 5, 44, hudSize + 4),
                         Qt::AlignHCenter, QString::number(qRound(targetForce)) + QLatin1Char('%'));
        painter.setPen(Qt::NoPen);
    }

    const QRectF forceTrack(margin, h * 0.73, w - 2.0 * margin, 12);
    painter.setBrush(QColor(40, 50, 72));
    painter.drawRoundedRect(forceTrack, 6, 6);
    const double force = m_hasSensorSample && m_inputAge <= kSignalTimeoutSeconds
        ? qBound(0.0, m_lastForce / 100.0, 1.0) : 0.0;
    painter.setBrush(m_pulseActive ? QColor(255, 170, 75) : QColor(90, 190, 150));
    painter.drawRoundedRect(QRectF(forceTrack.left(), forceTrack.top(),
        forceTrack.width() * force, forceTrack.height()), 6, 6);

    painter.setPen(QColor(215, 225, 240));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), hudSize));
    const QString status = hasFreshSignal()
        ? m_status
        : (m_awaitingNeutral ? m_status : QStringLiteral("Oczekiwanie na sygnał tensometru"));
    painter.drawText(QRect(margin, h * 0.80, w - 2 * margin, hudSize + 12),
                     Qt::AlignHCenter | Qt::AlignVCenter, status);
    painter.drawText(QRect(margin, h - hudSize - margin, w - 2 * margin, hudSize + 4),
        Qt::AlignHCenter, QStringLiteral("Impuls 18–100% • zwolnienie ≤18% • dokładność %1%")
            .arg(accuracyPercent(), 0, 'f', 1));
}

} // namespace games
