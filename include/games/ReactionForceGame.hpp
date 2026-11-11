#ifndef GAMES_REACTIONFORCEGAME_HPP
#define GAMES_REACTIONFORCEGAME_HPP

#include "games/GameEngine.hpp"
#include <QString>
#include <QStringList>

namespace games {

class ReactionForceGame final : public GameEngine
{
    Q_OBJECT

public:
    enum HitGrade { NoGrade = 0, Miss = 1, Hit = 2, Good = 3, Perfect = 4 };
    enum Phase { AwaitNeutral = 0, Delay = 1, Go = 2, Completed = 3 };

    explicit ReactionForceGame(QWidget* parent = nullptr);
    QString gameName() const override;

    void setDifficultyLevel(int level);
    int difficultyLevel() const { return m_difficulty; }
    int trialIndex() const { return m_trialIndex; }
    int totalTrials() const { return kTotalTrials; }
    int hits() const { return m_hits; }
    int misses() const { return m_misses; }
    int falseStarts() const { return m_falseStarts; }
    int combo() const { return m_combo; }
    int bestCombo() const { return m_bestCombo; }
    int perfectHits() const { return m_perfectHits; }
    double lastReactionMs() const { return m_lastReactionMs; }
    double targetForcePercent() const;
    double targetLowerPercent() const;
    double targetUpperPercent() const;
    double currentForcePercent() const { return m_lastForce; }
    double inputAgeSeconds() const { return m_inputAge; }
    bool hasFreshSignal() const;
    bool awaitingNeutral() const { return m_awaitingNeutral; }
    bool stimulusActive() const { return m_phase == Go; }
    bool pulseActive() const { return m_pulseActive; }
    bool roundCompleted() const { return m_roundCompleted; }
    Phase phase() const { return m_phase; }
    HitGrade lastGrade() const { return m_lastGrade; }
    QString statusText() const { return m_status; }
    int personalBestScore() const { return m_personalBestScore; }
    int personalBestCombo() const { return m_personalBestCombo; }
    double personalBestReactionMs() const { return m_personalBestReactionMs; }
    QStringList achievements() const { return m_achievements; }
    bool hasAchievement(const QString& id) const { return m_achievements.contains(id); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

private:
    friend struct ReactionForceGameTestAccess;

    static constexpr int kTotalTrials = 20;
    static constexpr double kRestThreshold = 18.0;
    static constexpr double kTriggerThreshold = 22.0;
    static constexpr double kSignalTimeout = 0.35;
    static constexpr double kMinimumPulse = 0.07;
    static constexpr double kMaximumPulse = 0.60;

    double delayForTrial(int trial) const;
    double reactionWindowSeconds() const;
    double forceTolerancePercent() const;
    double targetForTrial(int trial) const;
    void finishPulse();
    void registerMiss(const QString& reason, bool falseStart = false);
    void advanceTrial();
    void finishRound();
    void unlockAchievement(const QString& id, const QString& title);
    void persistRecords();

    int m_difficulty;
    int m_trialIndex;
    int m_hits;
    int m_misses;
    int m_falseStarts;
    int m_combo;
    int m_bestCombo;
    int m_perfectHits;
    int m_personalBestScore;
    int m_personalBestCombo;
    double m_personalBestReactionMs;
    double m_inputAge;
    double m_phaseTime;
    double m_pulseElapsed;
    double m_reactionAtOnset;
    double m_pulsePeakForce;
    double m_lastReactionMs;
    double m_lastForce;
    double m_visualTime;
    double m_feedbackTime;
    double m_toastTime;
    bool m_hasSensorSample;
    bool m_awaitingNeutral;
    bool m_pulseActive;
    bool m_roundCompleted;
    Phase m_phase;
    HitGrade m_lastGrade;
    QString m_status;
    QString m_lastAchievement;
    QStringList m_achievements;
};

} // namespace games

#endif // GAMES_REACTIONFORCEGAME_HPP
