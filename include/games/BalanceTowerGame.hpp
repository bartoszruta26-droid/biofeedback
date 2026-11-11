#ifndef GAMES_BALANCETOWERGAME_HPP
#define GAMES_BALANCETOWERGAME_HPP

#include "games/GameEngine.hpp"
#include <QVector>
#include <QPointF>
#include <QStringList>

namespace games {

class BalanceTowerGame final : public GameEngine
{
    Q_OBJECT
public:
    explicit BalanceTowerGame(QWidget *parent = nullptr);
    QString gameName() const override;

    int placedBlocks() const { return m_blocks.size(); }
    int livesRemaining() const { return m_lives; }
    double stability() const { return m_stability; }
    double targetPosition() const { return m_targetPosition; }
    bool awaitingNeutral() const { return m_awaitingNeutral; }
    bool hasFreshSignal() const;
    bool depthAxisActive() const { return m_depthAxis; }
    bool roundCompleted() const { return m_roundCompleted; }
    int personalBestScore() const { return m_personalBestScore; }
    bool hasAchievement(const QString &id) const { return m_achievements.contains(id); }

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter &painter) override;
    bool handleInput(double value) override;

private:
    friend struct BalanceTowerGameTestAccess;
    struct Block { double x; double z; double width; double depth; double mass; };
    static constexpr double kSignalTimeoutSeconds = 0.35;
    static constexpr double kNeutralMin = 10.0;
    static constexpr double kNeutralMax = 30.0;
    static constexpr int kBlocksPerRound = 12;

    void placeBlock();
    void toggleAxis();
    void finishRound(bool stable);
    void unlockAchievement(const QString &id);
    void saveRecords();
    double centerOfMassX() const;
    double centerOfMassZ() const;

    QVector<Block> m_blocks;
    int m_lives{3};
    int m_round{1};
    int m_bestRound{1};
    int m_personalBestScore{0};
    int m_combo{0};
    int m_bestCombo{0};
    int m_falls{0};
    double m_targetPosition{0.0};
    double m_targetDepth{0.0};
    double m_inputAge{1.0};
    double m_gestureDuration{0.0};
    double m_visualTime{0.0};
    double m_stability{1.0};
    double m_tilt{0.0};
    double m_currentForce{0.0};
    double m_blockWidth{0.0};
    bool m_hasSensorSample{false};
    bool m_awaitingNeutral{true};
    bool m_gestureActive{false};
    bool m_depthAxis{false};
    bool m_roundCompleted{false};
    bool m_gameOver{false};
    bool m_campaignClean{true};
    QString m_status;
    QStringList m_achievements;
};

} // namespace games

#endif
