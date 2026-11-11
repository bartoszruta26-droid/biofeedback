#ifndef GAMES_RHYTHMFORCEGAME_HPP
#define GAMES_RHYTHMFORCEGAME_HPP

#include "games/GameEngine.hpp"
#include <QString>
#include <QStringList>

namespace games {

class RhythmForceGame final : public GameEngine
{
    Q_OBJECT

public:
    enum HitGrade { NoGrade = 0, Miss = 1, Okay = 2, Good = 3, Perfect = 4 };

    explicit RhythmForceGame(QWidget *parent = nullptr);
    QString gameName() const override;

    void setDifficultyLevel(int level);
    int difficultyLevel() const { return m_difficulty; }
    int beatsPerMinute() const;
    int combo() const { return m_combo; }
    int bestCombo() const { return m_bestCombo; }
    int hitCount() const { return m_hits; }
    int missCount() const { return m_misses; }
    int perfectCount() const { return m_perfectHits; }
    int nextBeatNumber() const { return m_nextBeatIndex; }
    int totalBeats() const { return kRoundBeats; }
    double accuracyPercent() const;
    double songTimeSeconds() const { return m_songTime; }
    double nextTargetForcePercent() const;
    double lastTimingOffsetMs() const { return m_lastTimingOffsetMs; }
    HitGrade lastHitGrade() const { return m_lastGrade; }
    bool hasFreshSignal() const;
    bool pulseActive() const { return m_pulseActive; }
    bool roundCompleted() const { return m_roundCompleted; }
    int unlockedAchievementsCount() const { return m_achievements.size(); }
    QString lastAchievement() const { return m_lastAchievement; }
    bool hasAchievement(const QString &id) const { return m_achievements.contains(id); }
    double visualTimeSeconds() const { return m_visualTime; }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter &painter) override;
    bool handleInput(double value) override;

private:
    friend struct RhythmForceGameTestAccess;

    static constexpr int kRoundBeats = 32;
    static constexpr double kRestThresholdPercent = 18.0;
    static constexpr double kSignalTimeoutSeconds = 0.35;
    static constexpr double kLeadInSeconds = 1.0;
    static constexpr double kMinimumPulseSeconds = 0.06;
    static constexpr double kMaximumPulseSeconds = 0.75;

    double beatIntervalSeconds() const;
    double beatTimeSeconds(int beatIndex) const;
    double timingWindowSeconds() const;
    double perfectWindowSeconds() const;
    double targetForceForBeat(int beatIndex) const;
    void finishPulse();
    void registerMiss();
    void advanceBeat();
    void finishRoundIfReady();
    void unlockAchievement(const QString &id, const QString &title);
    void evaluateRoundAchievements();

    int m_difficulty;
    int m_nextBeatIndex;
    int m_hits;
    int m_misses;
    int m_perfectHits;
    int m_combo;
    int m_bestCombo;
    double m_songTime;
    double m_inputAge;
    double m_pulseElapsed;
    double m_pulseStartedAt;
    double m_pulsePeakForce;
    double m_lastTimingOffsetMs;
    double m_lastForce;
    double m_visualTime;
    double m_hitFlashSeconds;
    double m_achievementToastSeconds;
    bool m_hasSensorSample;
    bool m_awaitingNeutral;
    bool m_pulseActive;
    bool m_roundCompleted;
    HitGrade m_lastGrade;
    QString m_status;
    QStringList m_achievements;
    QString m_lastAchievement;
};

} // namespace games

#endif // GAMES_RHYTHMFORCEGAME_HPP
