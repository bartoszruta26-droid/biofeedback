#ifndef GAMES_FORCEMAZEGAME_HPP
#define GAMES_FORCEMAZEGAME_HPP

#include "games/GameEngine.hpp"
#include <QVector>
#include <QStringList>
#include <QPointF>

namespace games {

class ForceMazeGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit ForceMazeGame(QWidget *parent = nullptr);
    QString gameName() const override;

    int level() const { return m_level; }
    int livesRemaining() const { return m_lives; }
    int crystalsCollected() const { return m_crystals; }
    int totalCrystals() const { return m_totalCrystals; }
    int checkpointsReached() const { return m_checkpoints; }
    double progressRows() const { return m_progressRows; }
    double playerColumn() const { return m_playerColumn; }
    bool hasFreshSignal() const;
    bool awaitingNeutral() const { return m_awaitingNeutral; }
    bool campaignCompleted() const { return m_campaignCompleted; }
    int personalBestScore() const { return m_personalBestScore; }
    int unlockedAchievementsCount() const { return m_achievements.size(); }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter &painter) override;
    bool handleInput(double value) override;

private:
    friend struct ForceMazeGameTestAccess;

    static constexpr int kColumns = 9;
    static constexpr int kRowsPerLevel = 18;
    static constexpr int kCampaignLevels = 5;
    static constexpr double kSignalTimeoutSeconds = 0.35;
    static constexpr double kNeutralThreshold = 18.0;
    static constexpr double kCorridorHalfWidth = 1.28;

    int corridorCenter(int row) const;
    bool corridorContains(double column, int row) const;
    void processRowCrossing(int row);
    void hitWall(int row);
    void collectCrystal(int row);
    void reachCheckpoint(int row);
    void completeLevel();
    void finishCampaign();
    void unlockAchievement(const QString &id);
    void saveRecords();

    int m_level;
    int m_lives;
    int m_crystals;
    int m_totalCrystals;
    int m_checkpoints;
    int m_lastProcessedRow;
    int m_personalBestScore;
    int m_personalBestCrystals;
    double m_progressRows;
    double m_playerColumn;
    double m_targetColumn;
    double m_inputAge;
    double m_visualTime;
    double m_forwardSpeed;
    double m_hitFlash;
    double m_checkpointFlash;
    double m_crystalPulse;
    bool m_hasSensorSample;
    bool m_awaitingNeutral;
    bool m_campaignCompleted;
    bool m_levelClean;
    QString m_status;
    QVector<int> m_collectedCrystals;
    QVector<int> m_reachedCheckpoints;
    QStringList m_achievements;
};

} // namespace games

#endif
