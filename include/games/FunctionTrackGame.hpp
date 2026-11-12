#ifndef GAMES_FUNCTIONTRACKGAME_HPP
#define GAMES_FUNCTIONTRACKGAME_HPP

#include "games/GameEngine.hpp"
#include <QString>

namespace games {

class FunctionTrackGame final : public GameEngine
{
    Q_OBJECT
public:
    enum class Mode { Cotangent, Trapezoid, Mixed };
    explicit FunctionTrackGame(Mode mode, QWidget* parent = nullptr);
    QString gameName() const override;
    void setDifficultyLevel(int level);
    int difficultyLevel() const;
    void setTrackWidth(double normalizedWidth);
    double trackWidth() const;
    void update(double deltaTime) override;

protected:
    void initialize() override;
    void render(QPainter& painter) override;
    void renderInterpolated(QPainter& painter, double alpha) override;
    bool handleInput(double value) override;

private:
    double targetAt(double x) const;
    void updateTarget();
    Mode m_mode;
    int m_difficulty{1};
    double m_trackWidth{0.30};
    double m_phase{0.0};
    double m_previousPhase{0.0};
    double m_cursorY{0.0};
    double m_previousCursorY{0.0};
    double m_targetY{0.0};
    double m_previousTargetY{0.0};
    double m_force{0.0};
    double m_inputAge{1.0};
    double m_tolerance{80.0};
    double m_elapsed{0.0};
    double m_hitTime{0.0};
    double m_scoreFraction{0.0};
    double m_smoothingSeconds{0.055};
    QString m_feedback{QStringLiteral("Oczekiwanie na sygnał z tensometru")};
};

} // namespace games
#endif
