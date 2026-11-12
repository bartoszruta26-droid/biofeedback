#include "games/RhythmForceGame.hpp"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QSettings>
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
    , m_visualTime(0.0)
    , m_hitFlashSeconds(0.0)
    , m_achievementToastSeconds(0.0)
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
    return 60 + (m_difficulty - 1) * 8;
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
    return 0.28 - static_cast<double>(m_difficulty - 1) * 0.025;
}

double RhythmForceGame::perfectWindowSeconds() const
{
    return qMax(0.05, 0.09 - static_cast<double>(m_difficulty - 1) * 0.01);
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
    m_visualTime = 0.0;
    m_hitFlashSeconds = 0.0;
    m_achievementToastSeconds = 0.0;
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    m_achievements = settings.value(QStringLiteral("games/rhythm_force/achievements")).toStringList();
    m_personalBestScore = settings.value(QStringLiteral("games/rhythm_force/records/best_score"), 0).toInt();
    m_personalBestAccuracy = settings.value(QStringLiteral("games/rhythm_force/records/best_accuracy"), 0.0).toDouble();
    m_personalBestCombo = settings.value(QStringLiteral("games/rhythm_force/records/best_combo"), 0).toInt();
    m_lastAchievement.clear();
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
    m_hitFlashSeconds = 0.42;
    if (m_hits == 1)
        unlockAchievement(QStringLiteral("first_hit"), QStringLiteral("Pierwszy takt"));
    if (m_lastGrade == Perfect)
        unlockAchievement(QStringLiteral("first_perfect"), QStringLiteral("Perfekcyjne wyczucie"));
    if (m_combo >= 8)
        unlockAchievement(QStringLiteral("combo_8"), QStringLiteral("Rytm mistrza — seria 8"));
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
        evaluateRoundAchievements();
        m_personalBestScore = qMax(m_personalBestScore, getScore());
        m_personalBestAccuracy = qMax(m_personalBestAccuracy, accuracyPercent());
        m_personalBestCombo = qMax(m_personalBestCombo, m_bestCombo);
        QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
        settings.setValue(QStringLiteral("games/rhythm_force/records/best_score"), m_personalBestScore);
        settings.setValue(QStringLiteral("games/rhythm_force/records/best_accuracy"), m_personalBestAccuracy);
        settings.setValue(QStringLiteral("games/rhythm_force/records/best_combo"), m_personalBestCombo);
        m_status = QStringLiteral("Runda zakończona");
        stop();
    }
}

void RhythmForceGame::unlockAchievement(const QString &id, const QString &title)
{
    if (m_achievements.contains(id))
        return;
    m_achievements.append(id);
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/rhythm_force/achievements"), m_achievements);
    m_lastAchievement = title;
    m_achievementToastSeconds = 3.2;
}

void RhythmForceGame::evaluateRoundAchievements()
{
    if (m_misses == 0 && m_hits == kRoundBeats)
        unlockAchievement(QStringLiteral("flawless_round"), QStringLiteral("Bez pomyłki — pełna runda"));
    if (m_perfectHits >= 16)
        unlockAchievement(QStringLiteral("perfect_16"), QStringLiteral("Mistrz perfekcji — 16 trafień"));
    if (accuracyPercent() >= 90.0)
        unlockAchievement(QStringLiteral("accuracy_90"), QStringLiteral("Precyzja powyżej 90%"));
}

void RhythmForceGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;
    m_previousSongTime = m_songTime;
    m_previousVisualTime = m_visualTime;

    const double frameStep = qMin(deltaTime, 0.1);
    m_visualTime += frameStep;
    m_hitFlashSeconds = qMax(0.0, m_hitFlashSeconds - frameStep);
    m_achievementToastSeconds = qMax(0.0, m_achievementToastSeconds - frameStep);
    m_inputAge += frameStep;
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

    m_songTime += frameStep;
    if (m_pulseActive)
        m_pulseElapsed += frameStep;

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

void RhythmForceGame::renderInterpolated(QPainter& painter, double alpha)
{
    alpha = qBound(0.0, alpha, 1.0);
    const double songTime = m_songTime;
    const double visualTime = m_visualTime;
    m_songTime = m_previousSongTime + (songTime - m_previousSongTime) * alpha;
    m_visualTime = m_previousVisualTime + (visualTime - m_previousVisualTime) * alpha;
    render(painter);
    m_songTime = songTime;
    m_visualTime = visualTime;
}

void RhythmForceGame::render(QPainter &painter)
{
    const int w = width();
    const int h = height();
    painter.fillRect(rect(), QColor(8, 19, 37));
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QPointF horizon(w * 0.5, h * 0.34);
    painter.setPen(QPen(QColor(74, 154, 202, 35), 1));
    for (int i = -5; i <= 5; ++i)
        painter.drawLine(horizon, QPointF(w * 0.5 + i * w * 0.13, h));
    for (int i = 1; i <= 5; ++i) {
        const double t = i / 5.0;
        const double y = horizon.y() + (h - horizon.y()) * t * t;
        painter.drawLine(QPointF(0, y), QPointF(w, y));
    }

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
    QPolygonF runway;
    runway << QPointF(lane.center().x() - lane.width() * 0.31, lane.top())
           << QPointF(lane.center().x() + lane.width() * 0.31, lane.top())
           << QPointF(lane.right(), lane.bottom())
           << QPointF(lane.left(), lane.bottom());
    painter.setPen(QPen(QColor(76, 179, 222, 150), 1.4));
    painter.setBrush(QColor(14, 34, 58));
    painter.drawPolygon(runway);
    const double centreX = lane.center().x();
    painter.setPen(QPen(QColor(245, 220, 110), 2));
    painter.drawLine(QPointF(centreX, lane.top() - 12), QPointF(centreX, lane.bottom() + 12));
    painter.setPen(Qt::NoPen);

    const double interval = beatIntervalSeconds();
    for (int beat = m_nextBeatIndex; beat <= qMin(kRoundBeats, m_nextBeatIndex + 5); ++beat) {
        const double secondsToBeat = beatTimeSeconds(beat) - m_songTime;
        const double depth = qBound(0.0, 1.0 - secondsToBeat / (interval * 2.8), 1.0);
        const double y = lane.top() + depth * lane.height();
        const double perspective = 0.42 + depth * 0.78;
        const double x = centreX + secondsToBeat / (interval * 2.8) * lane.width() * 0.26;
        if (x < lane.left() - 24 || x > lane.right() + 24)
            continue;
        const double targetForce = targetForceForBeat(beat);
        const double radius = (6.0 + targetForce / 20.0) * perspective;
        const bool active = beat == m_nextBeatIndex;
        const QColor accent = active ? QColor(72, 218, 255) : QColor(95, 140, 184);
        if (active) {
            painter.setBrush(QColor(67, 208, 255, 35));
            painter.drawEllipse(QPointF(x, y), radius * 2.0, radius * 2.0);
        }
        painter.setPen(QPen(accent, active ? 2.2 : 1.0));
        painter.setBrush(active ? QColor(38, 160, 205) : QColor(45, 82, 122));
        painter.drawEllipse(QPointF(x, y), radius, radius);
        painter.setPen(QColor(232, 243, 255));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(7, qRound((hudSize - 1) * perspective))));
        painter.drawText(QRectF(x - 25 * perspective, y + radius + 2, 50 * perspective, hudSize + 3),
                         Qt::AlignHCenter, QString::number(qRound(targetForce)) + QLatin1Char('%'));
        painter.setPen(Qt::NoPen);
    }

    if (m_pulseActive) {
        const double pulseScale = 1.0 + 0.16 * std::sin(m_visualTime * 13.0);
        painter.setBrush(QColor(255, 166, 72, 32));
        painter.drawEllipse(QPointF(centreX, lane.center().y()),
                            lane.width() * 0.08 * pulseScale, lane.width() * 0.08 * pulseScale);
    }
    if (m_hitFlashSeconds > 0.0) {
        const double alpha = qBound(0.0, m_hitFlashSeconds / 0.42, 1.0);
        const double radius = 18.0 + (1.0 - alpha) * 38.0;
        painter.setPen(QPen(QColor(116, 255, 190, qRound(alpha * 220)), 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(centreX, lane.center().y()), radius, radius);
        painter.setPen(Qt::NoPen);
    }

    const QRectF forceTrack(margin, h * 0.73, w - 2.0 * margin, 12);
    painter.setPen(QPen(QColor(93, 130, 165, 150), 1));
    painter.setBrush(QColor(22, 34, 54));
    painter.drawRoundedRect(forceTrack, 6, 6);
    const double target = nextTargetForcePercent();
    const double targetLow = qBound(0.0, target - kPulseStrengthTolerance, 100.0);
    const double targetHigh = qBound(0.0, target + kPulseStrengthTolerance, 100.0);
    const QRectF targetBand(forceTrack.left() + forceTrack.width() * targetLow / 100.0,
                            forceTrack.top(),
                            forceTrack.width() * (targetHigh - targetLow) / 100.0,
                            forceTrack.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(87, 208, 170, 82));
    painter.drawRoundedRect(targetBand, 5, 5);
    const double force = m_hasSensorSample && m_inputAge <= kSignalTimeoutSeconds
        ? qBound(0.0, m_lastForce / 100.0, 1.0) : 0.0;
    painter.setBrush(m_pulseActive ? QColor(255, 170, 75) : QColor(90, 190, 150));
    painter.drawRoundedRect(QRectF(forceTrack.left(), forceTrack.top(),
        forceTrack.width() * force, forceTrack.height()), 6, 6);
    const double targetMarker = forceTrack.left() + forceTrack.width() * target / 100.0;
    painter.setPen(QPen(QColor(255, 226, 137), 2));
    painter.drawLine(QPointF(targetMarker, forceTrack.top() - 4),
                     QPointF(targetMarker, forceTrack.bottom() + 4));
    painter.setPen(Qt::NoPen);

    painter.setPen(QColor(215, 225, 240));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), hudSize));
    const QString status = hasFreshSignal()
        ? m_status
        : (m_awaitingNeutral ? m_status : QStringLiteral("Oczekiwanie na sygnał tensometru"));
    painter.drawText(QRect(margin, h * 0.80, w - 2 * margin, hudSize + 12),
                     Qt::AlignHCenter | Qt::AlignVCenter, status);
    if (m_achievementToastSeconds > 0.0 && !m_lastAchievement.isEmpty()) {
        const double toastWidth = qMin<double>(w - 2 * margin, 360.0);
        const QRectF toast((w - toastWidth) * 0.5, h * 0.12, toastWidth, 34.0);
        painter.setPen(QPen(QColor(255, 207, 103, 220), 1.2));
        painter.setBrush(QColor(48, 40, 23, 235));
        painter.drawRoundedRect(toast, 9, 9);
        painter.setPen(QColor(255, 225, 153));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, hudSize), QFont::DemiBold));
        painter.drawText(toast, Qt::AlignCenter,
                         QStringLiteral("OSIĄGNIĘCIE  •  %1").arg(m_lastAchievement));
    }

    if (m_roundCompleted) {
        const double panelWidth = qMin<double>(w - 2 * margin, 430.0);
        const double panelHeight = qMin<double>(h * 0.48, 220.0);
        const QRectF panel((w - panelWidth) * 0.5, h * 0.40,
                           panelWidth, panelHeight);
        painter.setPen(QPen(QColor(90, 205, 235, 220), 1.5));
        painter.setBrush(QColor(7, 17, 34, 242));
        painter.drawRoundedRect(panel, 14, 14);

        painter.setPen(QColor(117, 232, 255));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(14, hudSize + 5), QFont::Bold));
        painter.drawText(QRectF(panel.left() + 14, panel.top() + 12,
                                panel.width() - 28, 30),
                         Qt::AlignCenter, QStringLiteral("RUNDA ZAKOŃCZONA"));

        const QString rank = m_misses == 0 ? QStringLiteral("RANGA S — BEZBŁĘDNIE")
            : accuracyPercent() >= 90.0 ? QStringLiteral("RANGA A — DOSKONAŁA CELNOŚĆ")
            : accuracyPercent() >= 75.0 ? QStringLiteral("RANGA B — DOBRY RYTM")
            : accuracyPercent() >= 55.0 ? QStringLiteral("RANGA C — TRENUJ DALEJ")
            : QStringLiteral("RANGA D — KOLEJNA PRÓBA");
        painter.setPen(QColor(255, 218, 135));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(10, hudSize + 1), QFont::DemiBold));
        painter.drawText(QRectF(panel.left() + 12, panel.top() + 46,
                                panel.width() - 24, 24),
                         Qt::AlignCenter, rank);

        painter.setPen(QColor(231, 241, 252));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, hudSize)));
        const QString summary = QStringLiteral("Wynik: %1    Celność: %2%    Perfekty: %3 / 32")
            .arg(getScore()).arg(accuracyPercent(), 0, 'f', 1).arg(m_perfectHits);
        painter.drawText(QRectF(panel.left() + 12, panel.top() + 80,
                                panel.width() - 24, 22),
                         Qt::AlignCenter, summary);
        const QString streak = QStringLiteral("Rekord punktów: %1    Celność: %2%    Seria: %3")
            .arg(m_personalBestScore).arg(m_personalBestAccuracy, 0, 'f', 1).arg(m_personalBestCombo);
        painter.drawText(QRectF(panel.left() + 12, panel.top() + 106,
                                panel.width() - 24, 22),
                         Qt::AlignCenter, streak);

        QStringList recent;
        const QStringList achievementIds = {
            QStringLiteral("flawless_round"), QStringLiteral("perfect_16"),
            QStringLiteral("accuracy_90"), QStringLiteral("combo_8"),
            QStringLiteral("first_perfect"), QStringLiteral("first_hit")
        };
        const QStringList achievementNames = {
            QStringLiteral("Bez pomyłki"), QStringLiteral("16 perfekcyjnych"),
            QStringLiteral("Celność 90%"), QStringLiteral("Seria 8"),
            QStringLiteral("Perfekcyjne wyczucie"), QStringLiteral("Pierwszy takt")
        };
        for (int i = 0; i < achievementIds.size(); ++i) {
            if (m_achievements.contains(achievementIds.at(i)))
                recent.append(achievementNames.at(i));
            if (recent.size() == 3)
                break;
        }
        painter.setPen(QColor(163, 205, 225));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(8, hudSize - 1)));
        painter.drawText(QRectF(panel.left() + 12, panel.top() + 134,
                                panel.width() - 24, panel.height() - 140),
                         Qt::AlignCenter, recent.isEmpty()
                            ? QStringLiteral("Pierwsze osiągnięcia wciąż przed Tobą")
                            : QStringLiteral("ODBLOKOWANE: %1").arg(recent.join(QStringLiteral("  •  "))));
    }
    painter.drawText(QRect(margin, h - hudSize - margin, w - 2 * margin, hudSize + 4),
        Qt::AlignHCenter, QStringLiteral("Impuls 18–100% • zwolnienie ≤18% • dokładność %1%")
            .arg(accuracyPercent(), 0, 'f', 1));
}

} // namespace games
