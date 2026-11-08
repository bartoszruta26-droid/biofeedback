#include "games/GameFactory.hpp"
#include "games/TanGame.hpp"
#include "games/ArkanoidGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/PongGame.hpp"
#include "games/GalagaGame.hpp"
#include "games/ArcherGame.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QStringList>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <memory>

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::fprintf(stderr, "Game factory regression failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        std::abort(); \
    } \
} while (false)

static void waitForEvents(int milliseconds)
{
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    const QStringList ids = {
        QStringLiteral("sin_game"),
        QStringLiteral("tan_game"),
        QStringLiteral("pong"),
        QStringLiteral("arkanoid"),
        QStringLiteral("flappy_bird"),
        QStringLiteral("mario"),
        QStringLiteral("galaga"),
        QStringLiteral("archer")
    };

    for (const QString& id : ids) {
        std::unique_ptr<games::GameEngine> game(games::createGame(id));
        REQUIRE(game != nullptr);
        REQUIRE(!game->gameName().trimmed().isEmpty());
        game->resize(640, 480);
        game->start();
        REQUIRE(game->isRunning());
        game->handleSensorInput(25.0);
        waitForEvents(35);
        game->pause();
        REQUIRE(game->isPaused());
        game->start();
        REQUIRE(game->isRunning() && !game->isPaused());
        game->stop();
        REQUIRE(!game->isRunning());
    }

    std::unique_ptr<games::GameEngine> arkanoid(games::createGame(QStringLiteral("arkanoid")));
    auto* arkanoidGame = dynamic_cast<games::ArkanoidGame*>(arkanoid.get());
    REQUIRE(arkanoidGame != nullptr);
    arkanoidGame->setDifficultyLevel(5);
    REQUIRE(arkanoidGame->getDifficultyLevel() == 5);
    arkanoidGame->setDifficultyLevel(0);
    REQUIRE(arkanoidGame->getDifficultyLevel() == 1);
    arkanoidGame->start();
    waitForEvents(1800);
    REQUIRE(arkanoidGame->getScore() > 0); // Ball must hit at least one brick.
    arkanoidGame->stop();

    std::unique_ptr<games::GameEngine> tan(games::createGame(QStringLiteral("tan")));
    auto* tanGame = dynamic_cast<games::TanGame*>(tan.get());
    REQUIRE(tanGame != nullptr);
    tanGame->setDifficultyLevel(5);
    REQUIRE(tanGame->getDifficultyLevel() == 5);
    tanGame->setDifficultyLevel(0);
    REQUIRE(tanGame->getDifficultyLevel() == 1);

    std::unique_ptr<games::GameEngine> flappy(games::createGame(QStringLiteral("flappy")));
    auto* flappyGame = dynamic_cast<games::FlappyBirdGame*>(flappy.get());
    REQUIRE(flappyGame != nullptr);
    const float defaultSpeed = flappyGame->getGameSpeed();
    flappyGame->resize(640, 480);
    flappyGame->start();
    flappyGame->update(std::numeric_limits<double>::max());
    REQUIRE(flappyGame->isRunning());
    flappyGame->update(std::numeric_limits<double>::quiet_NaN());
    flappyGame->update(std::numeric_limits<double>::infinity());
    REQUIRE(flappyGame->isRunning());
    flappyGame->stop();
    flappyGame->setGameSpeed(std::numeric_limits<float>::quiet_NaN());
    REQUIRE(std::abs(flappyGame->getGameSpeed() - defaultSpeed) < 1e-6f);

    std::unique_ptr<games::GameEngine> pong(games::createGame(QStringLiteral("pong")));
    auto* pongGame = dynamic_cast<games::PongGame*>(pong.get());
    REQUIRE(pongGame != nullptr);
    pongGame->resize(640, 480);
    pongGame->start();
    pongGame->update(std::numeric_limits<double>::max());
    REQUIRE(pongGame->isRunning()); // Huge finite deltas are bounded before substep conversion.
    pongGame->update(std::numeric_limits<double>::quiet_NaN());
    pongGame->update(std::numeric_limits<double>::infinity());
    REQUIRE(pongGame->isRunning()); // Invalid deltas are ignored.
    pongGame->stop();

    std::unique_ptr<games::GameEngine> galaga(games::createGame(QStringLiteral("galaga")));
    auto* galagaGame = dynamic_cast<games::GalagaGame*>(galaga.get());
    REQUIRE(galagaGame != nullptr);
    galagaGame->resize(640, 480);
    galagaGame->start();
    galagaGame->handleSensorInput(75.0);
    waitForEvents(80);
    REQUIRE(galagaGame->isRunning());
    galagaGame->stop();

    std::unique_ptr<games::GameEngine> archer(games::createGame(QStringLiteral("archer")));
    auto* archerGame = dynamic_cast<games::ArcherGame*>(archer.get());
    REQUIRE(archerGame != nullptr);
    archerGame->resize(640, 480);
    archerGame->start();
    archerGame->handleSensorInput(80.0);
    waitForEvents(80);
    REQUIRE(archerGame->isRunning());
    archerGame->stop();

    REQUIRE(games::createGame(QStringLiteral("unknown-game")) == nullptr);
    return 0;
}
