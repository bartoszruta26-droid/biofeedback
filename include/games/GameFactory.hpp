#ifndef GAMES_GAMEFACTORY_HPP
#define GAMES_GAMEFACTORY_HPP

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

    return nullptr;
}

} // namespace games

#endif // GAMES_GAMEFACTORY_HPP
