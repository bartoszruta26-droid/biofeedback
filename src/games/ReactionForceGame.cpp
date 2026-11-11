#include "games/ReactionForceGame.hpp"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
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
    if (m_combo >= 15)
        unlockAchievement(QStringLiteral("combo_15"), QStringLiteral("Seria piętnastu trafień"));
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
    if (m_hits == kTotalTrials && m_misses == 0)
        unlockAchievement(QStringLiteral("clean_round"), QStringLiteral("Bezbłędna seria"));
    if (m_perfectHits >= 10)
        unlockAchievement(QStringLiteral("perfect_10"), QStringLiteral("Dziesięć reakcji perfekcyjnych"));
    const int accuracy = (100 * m_hits) / qMax(1, m_hits + m_misses);
    QSettings settings(QStringLiteral("Biofeedback"), QStringLiteral("Biofeedback"));
    const int previousBestAccuracy =
        settings.value(QStringLiteral("games/reaction_force/records/best_accuracy"), 0).toInt();
    if (accuracy >= 80)
        unlockAchievement(QStringLiteral("accuracy_80"), QStringLiteral("Celność co najmniej 80%"));
    if (accuracy >= 90)
        unlockAchievement(QStringLiteral("accuracy_90"), QStringLiteral("Celność co najmniej 90%"));
    if (accuracy == 100)
        unlockAchievement(QStringLiteral("accuracy_100"), QStringLiteral("Perfekcyjna celność"));
    if (m_perfectHits >= 15)
        unlockAchievement(QStringLiteral("perfect_15"), QStringLiteral("Piętnaście perfekcyjnych reakcji"));
    if (m_bestCombo >= 15)
        unlockAchievement(QStringLiteral("combo_15"), QStringLiteral("Seria piętnastu trafień"));
    if (m_falseStarts == 0)
        unlockAchievement(QStringLiteral("no_false_starts"), QStringLiteral("Opanowanie"));
    if (m_difficulty == 5 && accuracy >= 80)
        unlockAchievement(QStringLiteral("difficulty_5"), QStringLiteral("Mistrz poziomu piątego"));
    if (getScore() >= 2500)
        unlockAchievement(QStringLiteral("score_2500"), QStringLiteral("Wynik 2500 punktów"));
    if (getScore() > m_personalBestScore)
        unlockAchievement(QStringLiteral("new_record"), QStringLiteral("Nowy rekord osobisty"));
    if (previousBestAccuracy < accuracy)
        settings.setValue(QStringLiteral("games/reaction_force/records/best_accuracy"), accuracy);
    settings.setValue(QStringLiteral("games/reaction_force/statistics/rounds"),
        settings.value(QStringLiteral("games/reaction_force/statistics/rounds"), 0).toInt() + 1);
    settings.setValue(QStringLiteral("games/reaction_force/statistics/total_hits"),
        settings.value(QStringLiteral("games/reaction_force/statistics/total_hits"), 0).toInt() + m_hits);
    settings.setValue(QStringLiteral("games/reaction_force/statistics/total_misses"),
        settings.value(QStringLiteral("games/reaction_force/statistics/total_misses"), 0).toInt() + m_misses);
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
    // Freshness is measured against real elapsed time, not the capped
    // simulation/render step; a delayed UI tick must fail closed immediately.
    m_inputAge += deltaTime;

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

void ReactionForceGame::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    render(painter);
}

void ReactionForceGame::render(QPainter& painter)
{
    const qreal w = width(), h = height();
    if (w <= 0.0 || h <= 0.0) return;
    const qreal unit = qMin(w / 960.0, h / 540.0);
    const qreal margin = qBound(8.0, qMin(w, h) * 0.035, 30.0);
    const int titleSize = qBound(13, qRound(unit * 25.0), 30);
    const int bodySize = qBound(9, qRound(unit * 16.0), 18);
    const int cueSize = qBound(18, qRound(unit * 46.0), 54);
    const qreal line = qMax<qreal>(1.0, unit * 1.5);
    painter.save();
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    painter.fillRect(rect(), QColor(8, 19, 38));
    // Low-cost animated light pools use a small fixed number of translucent
    // solid ellipses instead of per-frame gradient shader work.
    const QPointF glowA(w * (0.18 + 0.025 * std::sin(m_visualTime * 0.31)), h * 0.18);
    const QPointF glowB(w * 0.82, h * (0.48 + 0.025 * std::cos(m_visualTime * 0.27)));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(20, 105, 150, 18));
    painter.drawEllipse(glowA, qMax(w, h) * 0.19, qMax(w, h) * 0.19);
    painter.setBrush(QColor(20, 105, 150, 12));
    painter.drawEllipse(glowA, qMax(w, h) * 0.29, qMax(w, h) * 0.29);
    painter.setBrush(QColor(80, 55, 160, 14));
    painter.drawEllipse(glowB, qMax(w, h) * 0.17, qMax(w, h) * 0.17);

    const QRectF header(margin, margin * 0.55, w - 2.0 * margin, h * 0.105);
    painter.setPen(QColor(239, 247, 255));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), titleSize, QFont::Black));
    painter.drawText(header, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("REACTION FORCE"));
    painter.setPen(QColor(115, 194, 225));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), bodySize, QFont::DemiBold));
    const QString stats = QStringLiteral("PRÓBA %1/%2   •   WYNIK %3   •   SERIA %4   •   %5/%2 TRAFIEŃ")
        .arg(qMin(m_trialIndex + 1, kTotalTrials)).arg(kTotalTrials)
        .arg(getScore()).arg(m_combo).arg(m_hits);
    const QRectF statsRect(margin, header.bottom() + h * 0.004, w - 2.0 * margin, h * 0.055);
    painter.drawText(statsRect, Qt::AlignLeft | Qt::AlignVCenter,
        QFontMetrics(painter.font()).elidedText(stats, Qt::ElideRight, qMax(1, qRound(statsRect.width()))));

    // 2.5D arena with a converging horizon and perspective floor grid.
    const QRectF arena(margin, h * 0.19, w - 2.0 * margin, h * 0.445);
    painter.setPen(QPen(QColor(53, 115, 155, 180), line));
    painter.setBrush(QColor(8, 24, 45));
    painter.drawRoundedRect(arena, qMax(6.0, unit * 12.0), qMax(6.0, unit * 12.0));
    painter.save();
    painter.setClipRect(arena.adjusted(line, line, -line, -line));
    const qreal horizonY = arena.top() + arena.height() * 0.19;
    const qreal centerX = arena.center().x();
    painter.fillRect(QRectF(arena.left(), horizonY, arena.width(), arena.bottom() - horizonY),
                     QColor(25, 105, 150, 24));
    painter.setPen(QPen(QColor(58, 133, 174, 76), line * 0.8));
    for (int i = -6; i <= 6; ++i) {
        const qreal bottomX = centerX + arena.width() * i * 0.115;
        painter.drawLine(QPointF(centerX + i * arena.width() * 0.012, horizonY),
                         QPointF(bottomX, arena.bottom()));
    }
    for (int row = 0; row < 7; ++row) {
        const qreal t = static_cast<qreal>(row + 1) / 7.0;
        const qreal y = horizonY + (arena.bottom() - horizonY) * t * t;
        painter.setPen(QPen(QColor(61, 139, 177, 35 + row * 8), line * (0.65 + t * 0.6)));
        painter.drawLine(QPointF(arena.left(), y), QPointF(arena.right(), y));
    }

    const qreal travel = 0.5 + 0.5 * std::sin(m_visualTime * (m_phase == Go ? 1.35 : 0.72));
    const QPointF target(
        arena.left() + arena.width() * (0.18 + 0.64 * (0.5 + 0.5 * std::sin(m_trialIndex * 1.73 + travel * 0.48))),
        arena.top() + arena.height() * (0.36 + 0.26 * (0.5 + 0.5 * std::cos(m_trialIndex * 1.19 + travel * 0.35))));
    const qreal baseRadius = qBound(10.0, qMin(arena.width(), arena.height()) * 0.082, 48.0);
    const qreal pulse = 0.5 + 0.5 * std::sin(m_visualTime * (m_phase == Go ? 8.0 : 3.2));
    const qreal radius = baseRadius * (1.0 + pulse * (m_phase == Go ? 0.25 : 0.12));
    const QColor accent = m_phase == Go ? QColor(76, 255, 174)
        : (m_phase == Delay ? QColor(90, 190, 255) : QColor(242, 190, 92));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 16));
    painter.drawEllipse(target, radius * 2.8, radius * 2.8);
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 28));
    painter.drawEllipse(target, radius * 2.0, radius * 2.0);
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 45));
    painter.drawEllipse(target, radius * 1.35, radius * 1.35);
    painter.setPen(QPen(accent, qMax<qreal>(1.5, line * 2.0)));
    painter.setBrush(QColor(8, 25, 43, 210));
    painter.drawEllipse(target, radius, radius);
    painter.setPen(QPen(QColor(170, 230, 250, 190), line));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(target, radius * 0.72, radius * 0.72);
    painter.drawLine(QPointF(target.x() - radius * 1.28, target.y()), QPointF(target.x() - radius * 0.88, target.y()));
    painter.drawLine(QPointF(target.x() + radius * 0.88, target.y()), QPointF(target.x() + radius * 1.28, target.y()));
    painter.drawLine(QPointF(target.x(), target.y() - radius * 1.28), QPointF(target.x(), target.y() - radius * 0.88));
    painter.drawLine(QPointF(target.x(), target.y() + radius * 0.88), QPointF(target.x(), target.y() + radius * 1.28));
    painter.setPen(Qt::NoPen);
    painter.setBrush(accent);
    painter.drawEllipse(target, radius * 0.19, radius * 0.19);
    QPainterPath trail;
    trail.moveTo(arena.left() + arena.width() * 0.08, arena.bottom() - arena.height() * 0.06);
    trail.quadTo(centerX, horizonY + arena.height() * 0.16, target.x(), target.y());
    painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), 115),
                        qMax<qreal>(1.0, line * 1.4), Qt::DashLine));
    painter.drawPath(trail);
    painter.restore();

    const QRectF cuePlate(arena.left() + arena.width() * 0.06, arena.top() + arena.height() * 0.075,
                          arena.width() * 0.88, arena.height() * 0.22);
    painter.setPen(QPen(QColor(accent.red(), accent.green(), accent.blue(), 125), line));
    painter.setBrush(QColor(3, 14, 29, 205));
    painter.drawRoundedRect(cuePlate, qMax(5.0, unit * 8.0), qMax(5.0, unit * 8.0));
    const QString cue = (m_phase == AwaitNeutral || m_awaitingNeutral)
        ? QStringLiteral("ZWOLNIJ NACISK — STREFA NEUTRALNA")
        : (m_phase == Delay ? QStringLiteral("GOTOWY")
           : (m_phase == Go ? QStringLiteral("TERAZ!") : QStringLiteral("KONIEC RUNDY")));
    painter.setPen(accent);
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(11, qRound(cueSize * 0.66), 34), QFont::Black));
    painter.drawText(cuePlate, Qt::AlignCenter, QFontMetrics(painter.font()).elidedText(
        cue, Qt::ElideRight, qMax(1, qRound(cuePlate.width() - 2.0 * margin))));

    const QRectF forceBar(margin, h * 0.695, w - 2.0 * margin, qMax(13.0, h * 0.052));
    painter.setPen(QPen(QColor(77, 111, 145, 180), line));
    painter.setBrush(QColor(20, 35, 54));
    painter.drawRoundedRect(forceBar, 5, 5);
    const QRectF band(forceBar.left() + forceBar.width() * targetLowerPercent() / 100.0,
                      forceBar.top(), forceBar.width() * (targetUpperPercent() - targetLowerPercent()) / 100.0,
                      forceBar.height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(46, 190, 133, 175));
    painter.drawRoundedRect(band, 3, 3);
    painter.setBrush(QColor(77, 184, 248, 225));
    painter.drawRoundedRect(QRectF(forceBar.left(), forceBar.top(),
        forceBar.width() * qBound(0.0, m_lastForce / 100.0, 1.0), forceBar.height()), 4, 4);
    painter.setPen(QColor(239, 247, 255));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), bodySize, QFont::DemiBold));
    painter.drawText(QRectF(margin, forceBar.bottom() + h * 0.008, w - 2.0 * margin, h * 0.045),
        Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("SIŁA %1%     CEL %2% ±%3%")
            .arg(qRound(m_lastForce)).arg(qRound(targetForcePercent())).arg(qRound(forceTolerancePercent())));

    const qreal windowRatio = qBound(0.0, 1.0 - m_phaseTime / qMax(0.01, reactionWindowSeconds()), 1.0);
    const QRectF reactionTrack(margin, h * 0.835, w - 2.0 * margin, qMax(6.0, h * 0.021));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(28, 43, 62));
    painter.drawRoundedRect(reactionTrack, 4, 4);
    painter.setBrush(m_phase == Go ? QColor(75, 235, 154) : QColor(76, 135, 184));
    painter.drawRoundedRect(QRectF(reactionTrack.left(), reactionTrack.top(),
        reactionTrack.width() * (m_phase == Go ? windowRatio : 1.0), reactionTrack.height()), 4, 4);

    const QRectF footer(margin, h * 0.88, w - 2.0 * margin, h * 0.085);
    painter.setPen(m_feedbackTime > 0.0
        ? (m_lastGrade == Miss ? QColor(255, 112, 112) : QColor(105, 248, 174))
        : QColor(169, 193, 218));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), bodySize, QFont::DemiBold));
    painter.drawText(footer, Qt::AlignLeft | Qt::AlignVCenter, QFontMetrics(painter.font()).elidedText(
        m_status, Qt::ElideRight, qMax(1, qRound(footer.width() * 0.76))));
    painter.setPen(QColor(134, 163, 193));
    painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, bodySize - 1)));
    painter.drawText(footer, Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("%1/5").arg(m_difficulty));

    if (m_toastTime > 0.0 && !m_lastAchievement.isEmpty()) {
        const QRectF toast(margin, arena.bottom() - h * 0.085, w - 2.0 * margin, h * 0.06);
        painter.setPen(QPen(QColor(255, 207, 103, 210), line));
        painter.setBrush(QColor(42, 31, 12, 225));
        painter.drawRoundedRect(toast, 5, 5);
        painter.setPen(QColor(255, 222, 142));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qMax(9, bodySize - 1), QFont::Bold));
        painter.drawText(toast.adjusted(6, 0, -6, 0), Qt::AlignCenter,
            QFontMetrics(painter.font()).elidedText(QStringLiteral("OSIĄGNIĘCIE: %1").arg(m_lastAchievement),
                Qt::ElideRight, qMax(1, qRound(toast.width() - 12))));
    }

    if (m_roundCompleted) {
        const QRectF panel(w * 0.075, h * 0.235, w * 0.85, h * 0.56);
        painter.setPen(QPen(QColor(83, 226, 169), qMax<qreal>(1.5, line * 1.4)));
        painter.setBrush(QColor(4, 15, 30, 248));
        painter.drawRoundedRect(panel, qMax(8.0, unit * 16.0), qMax(8.0, unit * 16.0));
        painter.setPen(QColor(106, 255, 184));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), qBound(15, qRound(unit * 30.0), 34), QFont::Black));
        painter.drawText(QRectF(panel.left() + margin, panel.top() + h * 0.025,
                                panel.width() - 2.0 * margin, h * 0.09),
                         Qt::AlignCenter, QStringLiteral("RUNDA ZAKOŃCZONA"));
        painter.setPen(QColor(232, 243, 255));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), bodySize, QFont::DemiBold));
        const int accuracy = (100 * m_hits) / qMax(1, m_hits + m_misses);
        const QString summary = QStringLiteral(
            "Wynik: %1 pkt    Celność: %2%    Trafienia: %3/%4\n"
            "Najlepsza seria: %5    Perfekcyjne: %6    Falstarty: %7\n"
            "Rekord wyniku: %8    Rekord serii: %9    Najlepsza reakcja: %10 ms")
            .arg(getScore()).arg(accuracy).arg(m_hits).arg(kTotalTrials)
            .arg(m_bestCombo).arg(m_perfectHits).arg(m_falseStarts)
            .arg(m_personalBestScore).arg(m_personalBestCombo)
            .arg(m_personalBestReactionMs > 0.0 ? qRound(m_personalBestReactionMs) : 0);
        painter.drawText(QRectF(panel.left() + margin, panel.top() + h * 0.13,
                                panel.width() - 2.0 * margin, panel.height() - h * 0.17),
                         Qt::AlignCenter | Qt::TextWordWrap, summary);
    }
    painter.restore();
}

} // namespace games
