#include "games/GameFactory.hpp"
#include "games/TanGame.hpp"
#include "games/SinGame.hpp"
#include "games/ArkanoidGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/PongGame.hpp"
#include "games/GalagaGame.hpp"
#include "games/ArcherGame.hpp"
#include "games/CarRaceGame.hpp"
#include "games/ScorchedEarthGame.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QKeyEvent>
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
        QStringLiteral("archer"),
        QStringLiteral("car_race"),
        QStringLiteral("scorched_earth")
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
    tanGame->resize(640, 480);
    tanGame->start();
    tanGame->handleSensorInput(35.0);
    tanGame->update(std::numeric_limits<double>::max());
    REQUIRE(tanGame->isRunning());
    tanGame->update(std::numeric_limits<double>::quiet_NaN());
    tanGame->update(std::numeric_limits<double>::infinity());
    tanGame->update(-1.0);
    tanGame->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    tanGame->handleSensorInput(std::numeric_limits<double>::infinity());
    REQUIRE(tanGame->isRunning());
    tanGame->stop();

    std::unique_ptr<games::GameEngine> sinus(games::createGame(QStringLiteral("sin_game")));
    auto* sinusGame = dynamic_cast<games::SinGame*>(sinus.get());
    REQUIRE(sinusGame != nullptr);
    sinusGame->resize(640, 480);
    sinusGame->start();
    sinusGame->handleSensorInput(35.0);
    sinusGame->update(std::numeric_limits<double>::max());
    REQUIRE(sinusGame->isRunning());
    sinusGame->update(std::numeric_limits<double>::quiet_NaN());
    sinusGame->update(std::numeric_limits<double>::infinity());
    sinusGame->update(-1.0);
    sinusGame->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    sinusGame->handleSensorInput(std::numeric_limits<double>::infinity());
    REQUIRE(sinusGame->isRunning());
    sinusGame->stop();

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
    REQUIRE(galagaGame->livesRemaining() == 3);
    REQUIRE(galagaGame->currentWave() == 1);
    REQUIRE(galagaGame->activeEnemyCount() == 24);
    galagaGame->handleSensorInput(50.0);
    QKeyEvent galagaFireDown(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QKeyEvent galagaFireUp(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(galagaGame, &galagaFireDown);
    waitForEvents(900); // Space is held so the repeat-fire mechanic can shoot.
    QApplication::sendEvent(galagaGame, &galagaFireUp);
    REQUIRE(galagaGame->isRunning());
    REQUIRE(galagaGame->getScore() > 0); // A player shot must hit an enemy.
    galagaGame->resize(180, 120);
    waitForEvents(80);
    REQUIRE(galagaGame->isRunning());
    galagaGame->resize(640, 480);
    waitForEvents(80);
    REQUIRE(galagaGame->isRunning());
    REQUIRE(galagaGame->livesRemaining() > 0);
    galagaGame->stop();
    galagaGame->reset();
    REQUIRE(galagaGame->livesRemaining() == 3);
    REQUIRE(galagaGame->currentWave() == 1);
    REQUIRE(galagaGame->activeEnemyCount() == 24);

    std::unique_ptr<games::GameEngine> archer(games::createGame(QStringLiteral("archer")));
    auto* archerGame = dynamic_cast<games::ArcherGame*>(archer.get());
    REQUIRE(archerGame != nullptr);
    archerGame->resize(640, 480);
    archerGame->start();
    archerGame->handleSensorInput(80.0);
    QKeyEvent archerFire(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(archerGame, &archerFire);
    waitForEvents(900);
    REQUIRE(archerGame->isRunning());
    REQUIRE(archerGame->getScore() >= 10); // The aimed arrow must hit the target.
    REQUIRE(archerGame->targetSpeed() > 55.0); // Successful hits increase target difficulty.
    archerGame->resize(180, 120);
    waitForEvents(80);
    REQUIRE(archerGame->isRunning());
    archerGame->resize(640, 480);
    waitForEvents(80);
    REQUIRE(archerGame->isRunning());
    archerGame->stop();

    // A deliberately low aim must miss cleanly and retire the arrow.
    std::unique_ptr<games::GameEngine> archerMiss(games::createGame(QStringLiteral("archer")));
    auto* archerMissGame = dynamic_cast<games::ArcherGame*>(archerMiss.get());
    REQUIRE(archerMissGame != nullptr);
    archerMissGame->resize(640, 480);
    archerMissGame->start();
    archerMissGame->handleSensorInput(0.0);
    QKeyEvent archerMissFire(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(archerMissGame, &archerMissFire);
    waitForEvents(1200);
    REQUIRE(archerMissGame->getScore() == 0);
    REQUIRE(archerMissGame->activeArrowCount() == 0);
    archerMissGame->stop();

    std::unique_ptr<games::GameEngine> carRace(games::createGame(QStringLiteral("car_race")));
    auto* carRaceGame = dynamic_cast<games::CarRaceGame*>(carRace.get());
    REQUIRE(carRaceGame != nullptr);
    carRaceGame->resize(640, 480);
    carRaceGame->start();
    carRaceGame->handleSensorInput(50.0); // Stay in the first obstacle's deterministic lane.
    waitForEvents(4000); // First obstacle spawns at 1.25 s and reaches the player after ~2 s.
    REQUIRE(carRaceGame->livesRemaining() < 3);
    REQUIRE(carRaceGame->isRunning());
    carRaceGame->resize(180, 120);
    carRaceGame->handleSensorInput(50.0);
    waitForEvents(120);
    REQUIRE(carRaceGame->isRunning());
    carRaceGame->resize(640, 480);
    waitForEvents(120);
    REQUIRE(carRaceGame->isRunning());
    carRaceGame->stop();
    carRaceGame->reset();
    REQUIRE(carRaceGame->livesRemaining() == 3);
    REQUIRE(carRaceGame->obstacleCount() == 0);

    std::unique_ptr<games::GameEngine> scorched(games::createGame(QStringLiteral("scorched_earth")));
    auto* scorchedGame = dynamic_cast<games::ScorchedEarthGame*>(scorched.get());
    REQUIRE(scorchedGame != nullptr);
    scorchedGame->resize(640, 480);
    scorchedGame->start();
    REQUIRE(scorchedGame->playerHealth() == 100);
    REQUIRE(scorchedGame->enemyHealth() == 100);
    REQUIRE(scorchedGame->currentRound() == 1);
    REQUIRE(scorchedGame->terrainPointCount() > 1);
    scorchedGame->handleSensorInput(65.0);
    QKeyEvent artilleryFire(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(scorchedGame, &artilleryFire);
    waitForEvents(180);
    scorchedGame->resize(180, 120);
    waitForEvents(80);
    REQUIRE(scorchedGame->isRunning());
    scorchedGame->resize(640, 480);
    waitForEvents(2600); // Allow the aimed shell to resolve against the enemy tank and the AI reply to begin.
    REQUIRE(scorchedGame->isRunning());
    REQUIRE(scorchedGame->enemyHealth() < 100); // A real tank hit must apply damage.
    REQUIRE(scorchedGame->playerHealth() > 0);
    REQUIRE(scorchedGame->currentRound() == 1);
    scorchedGame->stop();
    scorchedGame->reset();
    REQUIRE(scorchedGame->playerHealth() == 100);
    REQUIRE(scorchedGame->enemyHealth() == 100);
    REQUIRE(scorchedGame->currentRound() == 1);

    std::unique_ptr<games::GameEngine> legacyScoredEarth(
        games::createGame(QStringLiteral("scored_earth")));
    REQUIRE(dynamic_cast<games::ScorchedEarthGame*>(legacyScoredEarth.get()) != nullptr);

    // Public reset must be safe before a widget receives its final size.
    std::unique_ptr<games::GameEngine> marioReset(games::createGame(QStringLiteral("mario")));
    REQUIRE(marioReset != nullptr);
    marioReset->reset();
    REQUIRE(!marioReset->isRunning());
    marioReset->resize(640, 480);
    marioReset->start();
    REQUIRE(marioReset->isRunning());
    marioReset->reset();
    REQUIRE(!marioReset->isRunning());
    marioReset->start();
    REQUIRE(marioReset->isRunning());
    marioReset->stop();

    REQUIRE(games::createGame(QStringLiteral("unknown-game")) == nullptr);
    return 0;
}
