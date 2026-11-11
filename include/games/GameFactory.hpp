#ifndef GAMES_GAMEFACTORY_HPP
#define GAMES_GAMEFACTORY_HPP

#include "games/ArcherGame.hpp"
#include "games/CarRaceGame.hpp"
#include "games/ScorchedEarthGame.hpp"
#include "games/PrecisionCraneGame.hpp"
#include "games/RhythmForceGame.hpp"
#include "games/ForceMazeGame.hpp"
#include "games/BalanceTowerGame.hpp"
#include "games/ReactionForceGame.hpp"
#include "games/ArkanoidGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/GalagaGame.hpp"
#include "games/MarioGame.hpp"
#include "games/PongGame.hpp"
#include "games/SinGame.hpp"
#include "games/TanGame.hpp"

namespace games {

// Zwraca nowy widget gry dla identyfikatora używanego przez TrainingTab
// lub OutlineTab. Własność obiektu przejmuje wywołujący.
inline GameEngine* createGame(const QString& gameId, QWidget* parent = nullptr)
{
    const QString id = gameId.trimmed().toLower();

    if (id == QStringLiteral("sin") || id == QStringLiteral("sin_game") ||
        id == QStringLiteral("sinusoidal") || id == QStringLiteral("sinusoidalna"))
        return new SinGame(parent);

    if (id == QStringLiteral("tan") || id == QStringLiteral("tan_game"))
        return new TanGame(parent);

    if (id == QStringLiteral("pong"))
        return new PongGame(parent);

    if (id == QStringLiteral("arkanoid"))
        return new ArkanoidGame(parent);

    if (id == QStringLiteral("flappy") || id == QStringLiteral("flappy_bird"))
        return new FlappyBirdGame(parent);

    if (id == QStringLiteral("mario") || id == QStringLiteral("mario_run"))
        return new MarioGame(parent);

    if (id == QStringLiteral("galaga") || id == QStringLiteral("galaga_force"))
        return new GalagaGame(parent);

    if (id == QStringLiteral("archer") || id == QStringLiteral("archer_force"))
        return new ArcherGame(parent);

    if (id == QStringLiteral("car_race") || id == QStringLiteral("carrace") ||
        id == QStringLiteral("car-race"))
        return new CarRaceGame(parent);

    if (id == QStringLiteral("precision_crane") || id == QStringLiteral("precision-crane") ||
        id == QStringLiteral("precision crane"))
        return new PrecisionCraneGame(parent);

    if (id == QStringLiteral("rhythm_force") || id == QStringLiteral("rhythm-force") ||
        id == QStringLiteral("rhythm force"))
        return new RhythmForceGame(parent);

    if (id == QStringLiteral("force_maze") || id == QStringLiteral("force-maze") ||
        id == QStringLiteral("force maze"))
        return new ForceMazeGame(parent);

    if (id == QStringLiteral("balance_tower") || id == QStringLiteral("balance-tower") ||
        id == QStringLiteral("balance tower"))
        return new BalanceTowerGame(parent);

    if (id == QStringLiteral("reaction_force") || id == QStringLiteral("reaction-force") ||
        id == QStringLiteral("reaction force"))
        return new ReactionForceGame(parent);

    if (id == QStringLiteral("scorched_earth") || id == QStringLiteral("scored_earth") ||
        id == QStringLiteral("scorched-earth") ||
        id == QStringLiteral("scorchedearth"))
        return new ScorchedEarthGame(parent);

    return nullptr;
}

} // namespace games

#endif // GAMES_GAMEFACTORY_HPP
