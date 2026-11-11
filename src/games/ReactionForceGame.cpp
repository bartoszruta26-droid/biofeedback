#include "games/ReactionForceGame.hpp"

#include <QFont>
#include <QPainter>
#include <QPen>
#include <QSettings>
#include <algorithm>
#include <cmath>

namespace games {

ReactionForceGame::ReactionForceGame(QWidget* parent)
    : GameEngine(parent)
    , m_difficulty(1)
    , m_trialIndex(0)
    , m_hits(0)
    , m_misses(0)
    , m_falseStarts(0)
    , m_combo(0)
    , m_bestCombo(0)
    , m_perfectHits(0)
    , m_personalBestScore(0)
    , m_personalBestCombo(0)
    , m_personalBestReactionMs(0.0)
    , m_inputAge(kSignalTimeout + 1.0)
    , m_phaseTime(0.0)
    , m_pulseElapsed(0.0)
    , m_reactionAtOnset(0.0)
    , m_pulsePeakForce(0.0)
    , m_lastReactionMs(0.0)
    , m_lastForce(0.0)
    , m_visualTime(0.0)
    , m_feedbackTime(0.0)
    , m_toastTime(0.0)
    , m_hasSensorSample(false)
    , m_awaitingNeutral(true)
    , m_pulseActive(false)
    , m_roundCompleted(false)
    , m_phase(AwaitNeutral)
    , m_lastGrade(NoGrade)
    , m_status(QStringLiteral("Zwolnij tensometr do strefy neutralnej"))
{
    setMinimumSize(320, 240);
}

QString ReactionForceGame::gameName() const
{
    return QStringLiteral("Reaction Force — Refleks siły");
}

void ReactionForceGame::setDifficultyLevel(int level)
{
    m_difficulty = qBound(1, level, 5);
    QWidget::update();
}

double ReactionForceGame::delayForTrial(int trial) const
{
    const int pattern = (trial * 17 + 5) % 6;
    return qMax(0.55, 1.15 + pattern * 0.16 - (m_difficulty - 1) * 0.055);
}

double ReactionForceGame::reactionWindowSeconds() const
{
    return 1.45 - static_cast<double>(m_difficulty - 1) * 0.12;
}

double ReactionForceGame::forceTolerancePercent() const
{
    return qMax(8.0, 15.0 - static_cast<double>(m_difficulty - 1) * 1.5);
}

double ReactionForceGame::targetForTrial(int trial) const
{
    return 38.0 + static_cast<double>((trial * 19 + m_difficulty * 7) % 43);
}

double ReactionForceGame::targetForcePercent() const
{
    return m_trialIndex < kTotalTrials ? targetForTrial(m_trialIndex) : 0.0;
}

double ReactionForceGame::targetLowerPercent() const
{
    return qMax(0.0, targetForcePercent() - forceTolerancePercent());
}

double ReactionForceGame::targetUpperPercent() const
{
    return qMin(100.0, targetForcePercent() + forceTolerancePercent());
}

bool ReactionForceGame::hasFreshSignal() const
{
    return m_hasSensorSample && std::isfinite(m_inputAge) &&
           m_inputAge <= kSignalTimeout && !m_awaitingNeutral;
}

void ReactionForceGame::initialize()
{
    m_trialIndex = 0;
    m_hits = 0;
    m_misses = 0;
    m_falseStarts = 0;
    m_combo = 0;
    m_bestCombo = 0;
    m_perfectHits = 0;
    m_inputAge = kSignalTimeout + 1.0;
    m_phaseTime = 0.0;
    m_pulseElapsed = 0.0;
    m_reactionAtOnset = 0.0;
    m_pulsePeakForce = 0.0;
    m_lastReactionMs = 0.0;
    m_lastForce = 0.0;
    m_visualTime = 0.0;
    m_feedbackTime = 0.0;
    m_toastTime = 0.0;
    m_hasSensorSample = false;
    m_awaitingNeutral = true;
    m_pulseActive = false;
    m_roundCompleted = false;
    m_phase = AwaitNeutral;
    m_lastGrade = NoGrade;
    m_status = QStringLiteral("Zwolnij tensometr do strefy neutralnej");
    m_lastAchievement.clear();

    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    m_personalBestScore = settings.value(QStringLiteral("games/reaction_force/records/best_score"), 0).toInt();
    m_personalBestCombo = settings.value(QStringLiteral("games/reaction_force/records/best_combo"), 0).toInt();
    m_personalBestReactionMs = settings.value(QStringLiteral("games/reaction_force/records/best_reaction_ms"), 0.0).toDouble();
    m_achievements = settings.value(QStringLiteral("games/reaction_force/achievements")).toStringList();
    setScore(0);
}

bool ReactionForceGame::handleInput(double value)
{
    if (!std::isfinite(value))
        return false;

    const double force = qBound(0.0, value, 100.0);
    const bool wasFresh = hasFreshSignal();
    m_hasSensorSample = true;
    m_inputAge = 0.0;

    if (m_roundCompleted) {
        m_lastForce = force;
        return true;
    }

    // A reconnect or first sample must be neutral before it can arm play.
    if (m_awaitingNeutral || !wasFresh) {
        m_pulseActive = false;
        m_pulseElapsed = 0.0;
        m_lastForce = force;
        if (force <= kRestThreshold) {
            m_awaitingNeutral = false;
            if (m_phase == AwaitNeutral) {
                m_phase = Delay;
                m_phaseTime = 0.0;
            }
            m_status = QStringLiteral("Sygnał gotowy — oczekuj na sygnał START");
        } else {
            m_awaitingNeutral = true;
            m_phase = AwaitNeutral;
            m_status = QStringLiteral("Zwolnij nacisk, aby uzbroić grę");
        }
        return true;
    }

    if (!m_pulseActive && force >= kTriggerThreshold) {
        if (m_phase != Go) {
            registerMiss(QStringLiteral("Przedwczesny impuls — poczekaj na sygnał"), true);
            m_phase = AwaitNeutral;
            m_awaitingNeutral = true;
            m_phaseTime = 0.0;
            m_lastForce = force;
            return true;
        }
        m_pulseActive = true;
        m_reactionAtOnset = m_phaseTime;
        m_pulseElapsed = 0.0;
        m_pulsePeakForce = force;
        m_status = QStringLiteral("Impuls wykryty — utrzymaj krótko i zwolnij");
    } else if (m_pulseActive) {
        m_pulsePeakForce = qMax(m_pulsePeakForce, force);
        if (force <= kRestThreshold)
            finishPulse();
    }

    m_lastForce = force;
    return true;
}

void ReactionForceGame::finishPulse()
{
    const double duration = m_pulseElapsed;
    const double reactionMs = m_reactionAtOnset * 1000.0;
    const double peak = m_pulsePeakForce;
    m_pulseActive = false;
    m_pulseElapsed = 0.0;
    m_lastReactionMs = reactionMs;

    if (duration < kMinimumPulse || duration > kMaximumPulse) {
        registerMiss(QStringLiteral("Impuls poza dopuszczalnym czasem"));
        advanceTrial();
        return;
    }
    if (reactionMs > reactionWindowSeconds() * 1000.0) {
        registerMiss(QStringLiteral("Reakcja spóźniona"));
        advanceTrial();
        return;
    }

    const double forceError = std::abs(peak - targetForcePercent());
    if (forceError > forceTolerancePercent()) {
        registerMiss(QStringLiteral("Siła poza wyświetlonym zakresem"));
        advanceTrial();
        return;
    }
    if (m_personalBestReactionMs <= 0.0 || reactionMs < m_personalBestReactionMs)
        m_personalBestReactionMs = reactionMs;

    const int timingPoints = reactionMs <= 250.0 ? 100 : (reactionMs <= 450.0 ? 75 : 45);
    const int forcePoints = qRound(50.0 * qBound(0.0, 1.0 - forceError / forceTolerancePercent(), 1.0));
    const int comboPoints = qMin(40, m_combo * 2);
    ++m_hits;
    ++m_combo;
    m_bestCombo = qMax(m_bestCombo, m_combo);
    setScore(getScore() + timingPoints + forcePoints + comboPoints);

    if (reactionMs <= 250.0) {
        m_lastGrade = Perfect;
        ++m_perfectHits;
        unlockAchievement(QStringLiteral("first_perfect"), QStringLiteral("Błyskawiczny refleks"));
    } else if (reactionMs <= 450.0) {
        m_lastGrade = Good;
    } else {
        m_lastGrade = Hit;
    }

    m_feedbackTime = 0.55;
    m_status = QStringLiteral("%1  •  %2 ms  •  szczyt %3%")
        .arg(m_lastGrade == Perfect ? QStringLiteral("PERFEKT")
             : (m_lastGrade == Good ? QStringLiteral("DOBRZE") : QStringLiteral("TRAFIENIE")))
        .arg(qRound(reactionMs))
        .arg(qRound(peak));

    if (m_hits == 1)
        unlockAchievement(QStringLiteral("first_hit"), QStringLiteral("Pierwsza reakcja"));
    if (m_combo >= 5)
        unlockAchievement(QStringLiteral("combo_5"), QStringLiteral("Seria pięciu trafień"));
    if (m_combo >= 10)
        unlockAchievement(QStringLiteral("combo_10"), QStringLiteral("Refleks mistrza"));
    advanceTrial();
}

void ReactionForceGame::registerMiss(const QString& reason, bool falseStart)
{
    ++m_misses;
    m_combo = 0;
    m_lastGrade = Miss;
    if (falseStart)
        ++m_falseStarts;
    m_feedbackTime = 0.45;
    m_status = reason;
}

void ReactionForceGame::advanceTrial()
{
    ++m_trialIndex;
    m_phaseTime = 0.0;
    m_pulseActive = false;
    m_pulseElapsed = 0.0;
    if (m_trialIndex >= kTotalTrials) {
        finishRound();
        return;
    }
    m_phase = Delay;
    m_status = QStringLiteral("Przygotuj się — następny sygnał za chwilę");
}

void ReactionForceGame::finishRound()
{
    if (m_roundCompleted)
        return;
    m_roundCompleted = true;
    m_phase = Completed;
    m_awaitingNeutral = true;
    if (m_hits == kTotalTrials)
        unlockAchievement(QStringLiteral("clean_round"), QStringLiteral("Bezbłędna seria"));
    if (m_perfectHits >= 10)
        unlockAchievement(QStringLiteral("perfect_10"), QStringLiteral("Dziesięć reakcji perfekcyjnych"));
    if (m_hits >= 16)
        unlockAchievement(QStringLiteral("accuracy_80"), QStringLiteral("Celność co najmniej 80%"));

    persistRecords();
    m_status = QStringLiteral("Runda zakończona — %1/%2 trafień").arg(m_hits).arg(kTotalTrials);
    stop();
}

void ReactionForceGame::persistRecords()
{
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    m_personalBestScore = qMax(m_personalBestScore, getScore());
    m_personalBestCombo = qMax(m_personalBestCombo, m_bestCombo);
    settings.setValue(QStringLiteral("games/reaction_force/records/best_score"), m_personalBestScore);
    settings.setValue(QStringLiteral("games/reaction_force/records/best_combo"), m_personalBestCombo);
    settings.setValue(QStringLiteral("games/reaction_force/records/best_reaction_ms"), m_personalBestReactionMs);
}

void ReactionForceGame::unlockAchievement(const QString& id, const QString& title)
{
    if (m_achievements.contains(id))
        return;
    m_achievements.append(id);
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    settings.setValue(QStringLiteral("games/reaction_force/achievements"), m_achievements);
    m_lastAchievement = title;
    m_toastTime = 2.8;
}

void ReactionForceGame::update(double deltaTime)
{
    if (!isRunning() || !std::isfinite(deltaTime) || deltaTime <= 0.0)
        return;

    const double dt = qMin(deltaTime, 0.1);
    m_visualTime += dt;
    m_feedbackTime = qMax(0.0, m_feedbackTime - dt);
    m_toastTime = qMax(0.0, m_toastTime - dt);
    m_inputAge += dt;

    if (!m_hasSensorSample || m_inputAge > kSignalTimeout) {
        if (m_pulseActive) {
            m_pulseActive = false;
            m_pulseElapsed = 0.0;
        }
        m_awaitingNeutral = true;
        m_phase = AwaitNeutral;
        m_phaseTime = 0.0;
        m_lastForce = 0.0;
        m_status = QStringLiteral("Brak świeżego sygnału — runda wstrzymana");
        return;
    }

    if (m_awaitingNeutral || m_roundCompleted)
        return;

    m_phaseTime += dt;
    if (m_pulseActive) {
        m_pulseElapsed += dt;
        if (m_pulseElapsed > kMaximumPulse + 0.25) {
            registerMiss(QStringLiteral("Zbyt długie przytrzymanie"));
            advanceTrial();
        }
        return;
    }

    if (m_phase == Delay && m_phaseTime >= delayForTrial(m_trialIndex)) {
        m_phase = Go;
        m_phaseTime = 0.0;
        m_status = QStringLiteral("TERAZ — osiągnij wskazany zakres siły");
        return;
    }

    if (m_phase == Go && m_phaseTime > reactionWindowSeconds()) {
        registerMiss(QStringLiteral("Brak reakcji w wyznaczonym czasie"));
        advanceTrial();
    }
}

void ReactionForceGame::render(QPainter& painter)
{
    const int w = width();
    const int h = height();
    painter.fillRect(rect(), QColor(7, 15, 30));
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int margin = qBound(12, w / 28, 28);
    const int titleSize = qBound(14, h / 20, 26);
    const int smallSize = qBound(9, h / 42, 14);
    painter.setPen(QColor(236, 244, 255));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), titleSize, QFont::Bold));
    painter.drawText(QRect(margin, margin, w - 2 * margin, titleSize + 8),
                     Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("REACTION FORCE"));

    painter.setFont(QFont(QStringLiteral("Sans Serif"), smallSize));
    painter.setPen(QColor(160, 184, 210));
    const QString stats = QStringLiteral("PRÓBA %1/%2   WYNIK %3   SERIA %4   TRAFIENIA %5   PUDŁA %6")
        .arg(qMin(m_trialIndex + 1, kTotalTrials)).arg(kTotalTrials)
        .arg(getScore()).arg(m_combo).arg(m_hits).arg(m_misses);
    painter.drawText(QRect(margin, margin + titleSize + 8, w - 2 * margin, smallSize + 8),
                     Qt::AlignLeft | Qt::AlignVCenter, stats);

    const QRectF arena(margin, h * 0.20, w - 2.0 * margin, h * 0.43);
    painter.fillRect(arena, QColor(12, 30, 52));
    painter.setPen(QPen(QColor(50, 95, 130), 1));
    for (int i = 1; i < 5; ++i) {
        const qreal x = arena.left() + arena.width() * i / 5.0;
        painter.drawLine(QPointF(x, arena.top()), QPointF(x, arena.bottom()));
    }
    for (int i = 1; i < 4; ++i) {
        const qreal y = arena.top() + arena.height() * i / 4.0;
        painter.drawLine(QPointF(arena.left(), y), QPointF(arena.right(), y));
    }

    const QPointF target(arena.left() + arena.width() *
        (0.16 + 0.68 * (0.5 + 0.5 * std::sin(m_trialIndex * 1.73))),
        arena.top() + arena.height() *
        (0.22 + 0.56 * (0.5 + 0.5 * std::cos(m_trialIndex * 1.19))));
    const double pulse = 0.5 + 0.5 * std::sin(m_visualTime * 5.0);
    const double radius = qMax(10.0, qMin(arena.width(), arena.height()) * 0.075);
    painter.setPen(QPen(m_phase == Go ? QColor(65, 245, 160) : QColor(80, 160, 210), 2));
    painter.setBrush(QColor(20, 80, 110));
    painter.drawEllipse(target, radius * (1.0 + pulse * 0.18), radius * (1.0 + pulse * 0.18));
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_phase == Go ? QColor(80, 245, 155) : QColor(65, 145, 190));
    painter.drawEllipse(target, radius * 0.48, radius * 0.48);

    QString cue;
    QColor cueColor(205, 220, 238);
    if (m_phase == AwaitNeutral || m_awaitingNeutral) {
        cue = QStringLiteral("ZWOLNIJ NACISK — STREFA NEUTRALNA");
        cueColor = QColor(245, 195, 95);
    } else if (m_phase == Delay) {
        cue = QStringLiteral("GOTOWY");
        cueColor = QColor(120, 190, 245);
    } else if (m_phase == Go) {
        cue = QStringLiteral("TERAZ!");
        cueColor = QColor(80, 245, 155);
    } else {
        cue = QStringLiteral("KONIEC RUNDY");
    }
    painter.setPen(cueColor);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(18, h / 12, 42), QFont::Black));
    painter.drawText(QRectF(arena.left(), arena.center().y() - h * 0.07, arena.width(), h * 0.14),
                     Qt::AlignCenter, cue);

    const QRectF forceBar(margin, h * 0.72, w - 2.0 * margin, qMax(18.0, h * 0.055));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(34, 48, 66));
    painter.drawRoundedRect(forceBar, 6, 6);
    const double lower = targetLowerPercent() / 100.0;
    const double upper = targetUpperPercent() / 100.0;
    QRectF targetBand(forceBar.left() + forceBar.width() * lower, forceBar.top(),
                      forceBar.width() * (upper - lower), forceBar.height());
    painter.setBrush(QColor(35, 150, 100, 180));
    painter.drawRect(targetBand);
    painter.setBrush(QColor(80, 190, 245));
    painter.drawRect(QRectF(forceBar.left(), forceBar.top(),
        forceBar.width() * qBound(0.0, m_lastForce / 100.0, 1.0), forceBar.height()));
    painter.setPen(QColor(235, 243, 252));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), smallSize));
    painter.drawText(QRectF(forceBar.left(), forceBar.bottom() + 5, forceBar.width(), smallSize + 6),
        Qt::AlignLeft, QStringLiteral("Siła %1%   •   Cel %2% ±%3%")
            .arg(qRound(m_lastForce)).arg(qRound(targetForcePercent()))
            .arg(qRound(forceTolerancePercent())));

    const double windowRatio = qBound(0.0, 1.0 - m_phaseTime / qMax(0.01, reactionWindowSeconds()), 1.0);
    const QRectF reactionTrack(margin, h * 0.86, w - 2.0 * margin, qMax(8.0, h * 0.025));
    painter.setBrush(QColor(35, 48, 65));
    painter.drawRoundedRect(reactionTrack, 4, 4);
    painter.setBrush(m_phase == Go ? QColor(75, 220, 145) : QColor(75, 130, 175));
    painter.drawRoundedRect(QRectF(reactionTrack.left(), reactionTrack.top(),
        reactionTrack.width() * (m_phase == Go ? windowRatio : 1.0), reactionTrack.height()), 4, 4);

    painter.setPen(m_feedbackTime > 0.0
        ? (m_lastGrade == Miss ? QColor(255, 105, 100) : QColor(95, 240, 165))
        : QColor(165, 185, 210));
    painter.drawText(QRect(margin, h - margin - smallSize - 4, w - 2 * margin, smallSize + 4),
                     Qt::AlignLeft, m_status);
    painter.setPen(QColor(135, 160, 190));
    painter.drawText(QRect(margin, h - margin - smallSize - 4, w - 2 * margin, smallSize + 4),
                     Qt::AlignRight, QStringLiteral("TRUDNOŚĆ %1/5").arg(m_difficulty));

    if (m_toastTime > 0.0 && !m_lastAchievement.isEmpty()) {
        painter.setPen(QColor(255, 220, 110));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), smallSize, QFont::Bold));
        painter.drawText(QRect(margin, h * 0.65, w - 2 * margin, smallSize + 8),
                         Qt::AlignCenter, QStringLiteral("OSIĄGNIĘCIE: %1").arg(m_lastAchievement));
    }
}

} // namespace games
