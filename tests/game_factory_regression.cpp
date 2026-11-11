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
#include "games/PrecisionCraneGame.hpp"
#include "games/RhythmForceGame.hpp"
#include "games/ForceMazeGame.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QKeyEvent>
#include <QStringList>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <memory>

namespace games {
struct PrecisionCraneGameTestAccess
{
    static bool collideWithFirstObstacle(PrecisionCraneGame& game)
    {
        game.m_obstacleClock = 1.25; // Non-zero phase: the container has moved along its track.
        const QRectF obstacle = game.obstacleRect(0);
        game.configureCargo();
        game.m_loadAttached = true;
        game.m_sway = 0.0;
        game.m_carriageX = obstacle.center().x();
        game.m_hookY = obstacle.top() - 18.0;
        return game.processObstacleCollision();
    }

    static double firstObstacleXAt(PrecisionCraneGame& game, double seconds)
    {
        game.m_obstacleClock = seconds;
        return game.obstacleRect(0).left();
    }

    static void deliverAtTarget(PrecisionCraneGame& game)
    {
        game.configureCargo();
        game.m_loadAttached = true;
        game.m_sway = 0.0;
        game.m_carriageX = game.targetRect().center().x();
        game.m_hookY = game.floorY() - game.cargoRect().height() - 18.0;
        game.deliverOrDropLoad();
    }
};

struct ForceMazeGameTestAccess
{
    static void advance(ForceMazeGame& game, double deltaTime) { game.update(deltaTime); }
    static void completeLevel(ForceMazeGame& game) { game.completeLevel(); }
    static void collectCrystal(ForceMazeGame& game, int row) { game.collectCrystal(row); }
};

struct RhythmForceGameTestAccess
{
    static double beatTime(const RhythmForceGame& game, int beat)
    {
        return RhythmForceGame::kLeadInSeconds +
            static_cast<double>(beat - 1) * game.beatIntervalSeconds();
    }

    static double targetForce(const RhythmForceGame& game, int beat)
    {
        return game.targetForceForBeat(beat);
    }

    static void beginPulse(RhythmForceGame& game, double onset, double peak)
    {
        game.m_songTime = onset;
        game.m_inputAge = 0.0;
        game.m_hasSensorSample = true;
        game.m_awaitingNeutral = false;
        game.m_pulseActive = false;
        game.handleInput(peak);
    }

    static void releasePulse(RhythmForceGame& game)
    {
        game.m_inputAge = 0.0;
        game.handleInput(0.0);
    }

    static void emitPulse(RhythmForceGame& game, double onset, double duration, double peak)
    {
        game.m_songTime = onset;
        game.m_inputAge = 0.0;
        game.m_hasSensorSample = true;
        game.m_awaitingNeutral = false;
        game.m_pulseActive = false;
        game.handleInput(peak);
        game.m_pulseElapsed = duration;
        game.m_songTime += duration;
        game.m_inputAge = 0.0;
        game.handleInput(0.0);
    }

    static void advance(RhythmForceGame& game, double deltaTime)
    {
        game.update(deltaTime);
    }

    static void setFinalBeat(RhythmForceGame& game)
    {
        game.m_nextBeatIndex = RhythmForceGame::kRoundBeats;
    }

    static void setClock(RhythmForceGame& game, double songTime)
    {
        game.m_songTime = songTime;
        game.m_inputAge = 0.0;
        game.m_hasSensorSample = true;
        game.m_awaitingNeutral = false;
        game.m_pulseActive = false;
    }
};
} // namespace games

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
        QStringLiteral("scorched_earth"),
        QStringLiteral("precision_crane"),
        QStringLiteral("rhythm_force"),
        QStringLiteral("force_maze")
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

    // One-axis force steering, neutral rearming and fail-closed timeout.
    std::unique_ptr<games::GameEngine> forceMazeBase(games::createGame(QStringLiteral("force_maze")));
    auto* forceMaze = dynamic_cast<games::ForceMazeGame*>(forceMazeBase.get());
    REQUIRE(forceMaze != nullptr);
    REQUIRE(forceMaze->gameName().contains(QStringLiteral("Force Maze")));
    forceMaze->resize(640, 480);
    forceMaze->start();
    REQUIRE(forceMaze->awaitingNeutral());
    forceMaze->handleSensorInput(20.0);
    REQUIRE(!forceMaze->awaitingNeutral());
    REQUIRE(std::abs(forceMaze->targetColumn() - 4.0) < 0.001);
    forceMaze->handleSensorInput(0.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 1.0) < 0.001);
    forceMaze->handleSensorInput(10.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 4.0) < 0.001);
    forceMaze->handleSensorInput(20.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 4.0) < 0.001);
    forceMaze->handleSensorInput(30.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 4.0) < 0.001);
    forceMaze->handleSensorInput(30.1);
    REQUIRE(forceMaze->targetColumn() > 4.0);
    forceMaze->handleSensorInput(100.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 7.0) < 0.001);

    // Switching hands reverses the direction, persists the preference and
    // requires a neutral sample before accepting further steering.
    forceMaze->setLeftHanded(true);
    REQUIRE(forceMaze->isLeftHanded());
    REQUIRE(forceMaze->awaitingNeutral());
    forceMaze->handleSensorInput(20.0);
    REQUIRE(!forceMaze->awaitingNeutral());
    forceMaze->handleSensorInput(0.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 7.0) < 0.001);
    forceMaze->handleSensorInput(100.0);
    REQUIRE(std::abs(forceMaze->targetColumn() - 1.0) < 0.001);
    forceMaze->setLeftHanded(false);
    REQUIRE(!forceMaze->isLeftHanded());
    forceMaze->handleSensorInput(20.0);
    games::ForceMazeGameTestAccess::advance(*forceMaze, 0.25);
    REQUIRE(forceMaze->hasFreshSignal());
    REQUIRE(forceMaze->playerColumn() > 4.0);
    REQUIRE(forceMaze->progressRows() > 0.0);
    games::ForceMazeGameTestAccess::advance(*forceMaze, 0.40);
    REQUIRE(!forceMaze->hasFreshSignal());
    const double forceMazeFrozenProgress = forceMaze->progressRows();
    games::ForceMazeGameTestAccess::advance(*forceMaze, 0.50);
    REQUIRE(std::abs(forceMaze->progressRows() - forceMazeFrozenProgress) < 0.0001);
    forceMaze->handleSensorInput(90.0);
    REQUIRE(forceMaze->awaitingNeutral());
    forceMaze->handleSensorInput(0.0);
    REQUIRE(!forceMaze->awaitingNeutral());
    forceMaze->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(forceMaze->hasFreshSignal());
    forceMaze->stop();

    // Achievement milestones are cumulative and persisted independently of
    // the current run; a clean five-level campaign unlocks its own distinction.
    std::unique_ptr<games::GameEngine> forceMazeAchievementsBase(
        games::createGame(QStringLiteral("force_maze")));
    auto* forceMazeAchievements =
        dynamic_cast<games::ForceMazeGame*>(forceMazeAchievementsBase.get());
    REQUIRE(forceMazeAchievements != nullptr);
    forceMazeAchievements->start();
    for (int row = 0; row < 10; ++row)
        games::ForceMazeGameTestAccess::collectCrystal(*forceMazeAchievements, row);
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("first_crystal")));
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("crystal_collector")));
    for (int level = 0; level < 5; ++level)
        games::ForceMazeGameTestAccess::completeLevel(*forceMazeAchievements);
    REQUIRE(forceMazeAchievements->campaignCompleted());
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("three_clean_levels")));
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("level_three")));
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("level_five")));
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("maze_master")));
    REQUIRE(forceMazeAchievements->hasAchievement(QStringLiteral("untouched_campaign")));
    REQUIRE(forceMazeAchievements->personalBestLevel() >= 5);
    forceMazeAchievements->stop();

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
    flappyGame->resize(180, 120);
    waitForEvents(80);
    REQUIRE(flappyGame->isRunning()); // Active bird and obstacles survive a compact resize.
    flappyGame->resize(640, 480);
    waitForEvents(80);
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

    // EMP is edge-triggered, consumes one charge, clears hostile fire and
    // wipes the first-wave formation; wave progression then rebuilds enemies.
    std::unique_ptr<games::GameEngine> galagaPulse(games::createGame(QStringLiteral("galaga")));
    auto* galagaPulseGame = dynamic_cast<games::GalagaGame*>(galagaPulse.get());
    REQUIRE(galagaPulseGame != nullptr);
    galagaPulseGame->resize(640, 480);
    galagaPulseGame->start();
    REQUIRE(galagaPulseGame->specialCharges() == 2);
    QKeyEvent galagaEmp(QEvent::KeyPress, Qt::Key_X, Qt::NoModifier);
    QApplication::sendEvent(galagaPulseGame, &galagaEmp);
    REQUIRE(galagaPulseGame->specialCharges() == 1);
    REQUIRE(galagaPulseGame->activeEnemyCount() == 0);
    waitForEvents(80);
    REQUIRE(galagaPulseGame->currentWave() == 2);
    REQUIRE(galagaPulseGame->activeEnemyCount() > 0);
    galagaPulseGame->stop();

    // The first deterministic hostile shot is aligned with the far-right
    // sensor position. It must remove exactly one life and leave the session
    // active; the post-hit protection then recentres the player.
    std::unique_ptr<games::GameEngine> galagaHit(games::createGame(QStringLiteral("galaga")));
    auto* galagaHitGame = dynamic_cast<games::GalagaGame*>(galagaHit.get());
    REQUIRE(galagaHitGame != nullptr);
    galagaHitGame->resize(640, 480);
    galagaHitGame->start();
    galagaHitGame->handleSensorInput(100.0);
    // Wait for the first hit rather than assuming a fixed wall-clock time:
    // CI load and event-loop scheduling can move the collision by several frames.
    for (int attempt = 0; attempt < 500 && galagaHitGame->livesRemaining() == 3; ++attempt)
        waitForEvents(10);
    REQUIRE(galagaHitGame->livesRemaining() == 2);
    REQUIRE(galagaHitGame->isRunning());
    const double protectionAtHit = galagaHitGame->hitProtectionRemaining();
    REQUIRE(protectionAtHit > 0.0);
    waitForEvents(50);
    REQUIRE(galagaHitGame->livesRemaining() == 2);
    REQUIRE(galagaHitGame->hitProtectionRemaining() > 0.0);
    REQUIRE(galagaHitGame->hitProtectionRemaining() < protectionAtHit);
    galagaHitGame->stop();

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

    // Car Race steering uses filtered, hysteretic lane selection and a timed boost.
    std::unique_ptr<games::GameEngine> carRaceControls(games::createGame(QStringLiteral("car_race")));
    auto* carRaceControlsGame = dynamic_cast<games::CarRaceGame*>(carRaceControls.get());
    REQUIRE(carRaceControlsGame != nullptr);
    carRaceControlsGame->resize(640, 480);
    carRaceControlsGame->start();
    REQUIRE(carRaceControlsGame->currentLane() == 1);
    carRaceControlsGame->handleSensorInput(0.0);
    REQUIRE(carRaceControlsGame->currentLane() == 0);
    carRaceControlsGame->handleSensorInput(50.0);
    REQUIRE(carRaceControlsGame->currentLane() == 1);
    carRaceControlsGame->handleSensorInput(100.0);
    REQUIRE(carRaceControlsGame->currentLane() == 2);
    carRaceControlsGame->handleSensorInput(50.0);
    REQUIRE(carRaceControlsGame->currentLane() == 1);
    carRaceControlsGame->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(carRaceControlsGame->currentLane() == 1);
    QKeyEvent carRaceBoost(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(carRaceControlsGame, &carRaceBoost);
    REQUIRE(carRaceControlsGame->boostActive());
    REQUIRE(carRaceControlsGame->boostCooldownRemaining() > 0.0);
    const double clockBefore = carRaceControlsGame->animationClock();
    waitForEvents(180);
    REQUIRE(carRaceControlsGame->animationClock() > clockBefore);
    REQUIRE(carRaceControlsGame->visualLanePosition() >= 0.0);
    REQUIRE(carRaceControlsGame->visualLanePosition() <= 2.0);
    REQUIRE(carRaceControlsGame->passStreak() == 0);
    REQUIRE(carRaceControlsGame->collectiblesCollected() == 0);
    REQUIRE(carRaceControlsGame->difficultyLevel() == 1);
    carRaceControlsGame->stop();

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

    // Three predictable obstacle lanes exercise the complete game-over path.
    std::unique_ptr<games::GameEngine> carRaceGameOver(games::createGame(QStringLiteral("car_race")));
    auto* carRaceGameOverInstance = dynamic_cast<games::CarRaceGame*>(carRaceGameOver.get());
    REQUIRE(carRaceGameOverInstance != nullptr);
    carRaceGameOverInstance->resize(640, 480);
    carRaceGameOverInstance->start();
    carRaceGameOverInstance->handleSensorInput(50.0); // First obstacle: middle lane.
    waitForEvents(3600);
    REQUIRE(carRaceGameOverInstance->livesRemaining() == 2);
    carRaceGameOverInstance->handleSensorInput(0.0); // Second obstacle: left lane.
    waitForEvents(1100);
    REQUIRE(carRaceGameOverInstance->livesRemaining() == 1);
    // Allow the collision-protection window to expire before exposing the next lane.
    waitForEvents(1200);
    carRaceGameOverInstance->handleSensorInput(99.0); // Third obstacle: right lane.
    waitForEvents(1900);
    REQUIRE(carRaceGameOverInstance->livesRemaining() == 0);
    REQUIRE(!carRaceGameOverInstance->isRunning());

    std::unique_ptr<games::GameEngine> scorched(games::createGame(QStringLiteral("scorched_earth")));
    auto* scorchedGame = dynamic_cast<games::ScorchedEarthGame*>(scorched.get());
    REQUIRE(scorchedGame != nullptr);
    scorchedGame->resize(640, 480);
    scorchedGame->start();
    REQUIRE(scorchedGame->playerHealth() == 100);
    REQUIRE(scorchedGame->enemyHealth() == 100);
    REQUIRE(scorchedGame->currentRound() == 1);
    REQUIRE(scorchedGame->terrainPointCount() > 1);
    scorchedGame->handleSensorInput(40.0); // The trajectory enters the enemy tank's expanded collision bounds.
    QKeyEvent artilleryFire(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QApplication::sendEvent(scorchedGame, &artilleryFire);
    waitForEvents(2200); // Let the undisturbed ballistic trajectory resolve before testing resize.
    REQUIRE(scorchedGame->isRunning());
    REQUIRE(scorchedGame->enemyHealth() < 100); // A real tank hit must apply damage.
    REQUIRE(scorchedGame->playerHealth() > 0);
    REQUIRE(scorchedGame->currentRound() == 1);
    scorchedGame->resize(180, 120);
    waitForEvents(80);
    REQUIRE(scorchedGame->isRunning());
    scorchedGame->resize(640, 480);
    waitForEvents(80);
    REQUIRE(scorchedGame->isRunning());
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
    marioReset->show();
    QApplication::processEvents();
    marioReset->start();
    REQUIRE(marioReset->isRunning());
    marioReset->resize(180, 120);
    waitForEvents(80);
    REQUIRE(marioReset->isRunning()); // Active player and obstacles survive a compact resize.
    marioReset->resize(640, 480);
    waitForEvents(80);
    REQUIRE(marioReset->isRunning());
    marioReset->reset();
    REQUIRE(!marioReset->isRunning());
    marioReset->start();
    REQUIRE(marioReset->isRunning());
    marioReset->stop();

    // Precision Crane uses one continuous force channel and time-coded gestures.
    std::unique_ptr<games::GameEngine> craneBase(
        games::createGame(QStringLiteral("precision_crane")));
    auto* crane = dynamic_cast<games::PrecisionCraneGame*>(craneBase.get());
    REQUIRE(crane != nullptr);
    REQUIRE(crane->gameName().contains(QStringLiteral("Precision Crane")));
    REQUIRE(crane->obstacleCount() == 2);
    REQUIRE(crane->cargoMassKg() > 0.0);
    REQUIRE(crane->missionNumber() == 1);
    REQUIRE(crane->missionDeliveriesRemaining() == 3);
    REQUIRE(!crane->campaignCompleted());
    REQUIRE(crane->precisionSamples() == 0);
    REQUIRE(crane->lifetimeDeliveries() >= 0);
    REQUIRE(crane->lifetimePrecisionSamples() >= 0);
    REQUIRE(crane->lifetimeCollisions() >= 0);
    REQUIRE(crane->averagePrecision() == 0.0);
    crane->setDifficultyLevel(5);
    REQUIRE(crane->difficultyLevel() == 5);
    crane->setDifficultyLevel(0);
    REQUIRE(crane->difficultyLevel() == 1);
    crane->resize(640, 480);
    crane->start();
    REQUIRE(crane->isRunning());
    REQUIRE(crane->controlAxis() == games::PrecisionCraneGame::HorizontalAxis);
    const double initialCarriage = crane->carriagePosition();
    for (int i = 0; i < 5; ++i) {
        crane->handleSensorInput(65.0);
        waitForEvents(35);
    }
    REQUIRE(crane->normalizedForce() > 0.60);
    REQUIRE(crane->carriagePosition() > initialCarriage);
    // A sustained high-force hold followed by release changes the active axis.
    for (int i = 0; i < 14; ++i) {
        crane->handleSensorInput(90.0);
        waitForEvents(80);
    }
    crane->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(crane->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
    const double hookBefore = crane->hookPosition();
    for (int i = 0; i < 5; ++i) {
        crane->handleSensorInput(65.0);
        waitForEvents(35);
    }
    REQUIRE(crane->hookPosition() > hookBefore);
    crane->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    crane->handleSensorInput(std::numeric_limits<double>::infinity());
    REQUIRE(crane->isRunning());
    crane->resize(180, 120);
    waitForEvents(60);
    REQUIRE(crane->isRunning());
    crane->resize(640, 480);
    waitForEvents(60);
    REQUIRE(crane->isRunning());
    crane->stop();
    REQUIRE(!crane->isRunning());

    // Sensor loss is fail-closed: movement and pending gestures stop until a valid
    // fresh sample arrives. Invalid samples must not refresh the watchdog.
    std::unique_ptr<games::GameEngine> craneSafetyBase(
        games::createGame(QStringLiteral("precision_crane")));
    auto* craneSafety = dynamic_cast<games::PrecisionCraneGame*>(craneSafetyBase.get());
    REQUIRE(craneSafety != nullptr);
    craneSafety->resize(640, 480);
    craneSafety->start();
    craneSafety->handleSensorInput(65.0);
    waitForEvents(120);
    const double positionBeforeTimeout = craneSafety->carriagePosition();
    REQUIRE(craneSafety->hasFreshSignal());
    REQUIRE(!craneSafety->gestureActive());
    waitForEvents(420);
    REQUIRE(!craneSafety->hasFreshSignal());
    REQUIRE(craneSafety->normalizedForce() == 0.0);
    const double positionAtTimeout = craneSafety->carriagePosition();
    REQUIRE(positionAtTimeout >= positionBeforeTimeout);
    craneSafety->handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    craneSafety->handleSensorInput(std::numeric_limits<double>::infinity());
    waitForEvents(120);
    REQUIRE(!craneSafety->hasFreshSignal());
    REQUIRE(std::abs(craneSafety->carriagePosition() - positionAtTimeout) < 0.001);
    craneSafety->handleSensorInput(65.0);
    waitForEvents(80);
    REQUIRE(craneSafety->hasFreshSignal());
    REQUIRE(craneSafety->carriagePosition() > positionAtTimeout);
    craneSafety->stop();

    // A high-force gesture abandoned by a sensor timeout must be cancelled.
    // A later release sample must not execute the stale gesture.
    std::unique_ptr<games::GameEngine> craneStaleGestureBase(
        games::createGame(QStringLiteral("precision_crane")));
    auto* craneStaleGesture = dynamic_cast<games::PrecisionCraneGame*>(craneStaleGestureBase.get());
    REQUIRE(craneStaleGesture != nullptr);
    craneStaleGesture->resize(640, 480);
    craneStaleGesture->start();
    craneStaleGesture->handleSensorInput(90.0);
    waitForEvents(160);
    REQUIRE(craneStaleGesture->gestureActive());
    waitForEvents(420); // Exceed the 350 ms input freshness limit.
    REQUIRE(!craneStaleGesture->hasFreshSignal());
    REQUIRE(!craneStaleGesture->gestureActive());
    craneStaleGesture->handleSensorInput(0.0);
    waitForEvents(60);
    REQUIRE(craneStaleGesture->controlAxis() == games::PrecisionCraneGame::HorizontalAxis);
    REQUIRE(!craneStaleGesture->loadAttached());
    craneStaleGesture->stop();

    // A sub-minimum pulse is ignored; a valid short pulse grips only when a valid short pulse grips only when
    // the single-sensor-controlled hook is aligned with the cargo.
    std::unique_ptr<games::GameEngine> craneGestureBase(
        games::createGame(QStringLiteral("precision_crane")));
    auto* craneGesture = dynamic_cast<games::PrecisionCraneGame*>(craneGestureBase.get());
    REQUIRE(craneGesture != nullptr);
    craneGesture->resize(640, 480);
    craneGesture->start();
    const auto driveGestureCrane = [craneGesture](double force, int milliseconds) {
        for (int elapsed = 0; elapsed < milliseconds; elapsed += 80) {
            craneGesture->handleSensorInput(force);
            waitForEvents(80);
        }
    };
    driveGestureCrane(25.43, 1200); // Align trolley with the first cargo.
    driveGestureCrane(90.0, 1120);  // Select vertical axis.
    craneGesture->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneGesture->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
    driveGestureCrane(65.0, 1120); // Lower hook to cargo height.
    craneGesture->handleSensorInput(90.0);
    waitForEvents(80);             // Deliberately shorter than 0.15 s.
    craneGesture->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(!craneGesture->loadAttached());
    craneGesture->handleSensorInput(90.0);
    waitForEvents(260);
    REQUIRE(craneGesture->gestureActive());
    craneGesture->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneGesture->loadAttached());
    craneGesture->stop();

    // End-to-end single-sensor delivery: align, lower, grip, transfer, lower and release.
    std::unique_ptr<games::GameEngine> craneDeliveryBase(
        games::createGame(QStringLiteral("precision_crane")));
    auto* craneDelivery = dynamic_cast<games::PrecisionCraneGame*>(craneDeliveryBase.get());
    REQUIRE(craneDelivery != nullptr);
    craneDelivery->resize(640, 480);
    craneDelivery->start();
    const auto driveCrane = [craneDelivery](double force, int milliseconds) {
        for (int elapsed = 0; elapsed < milliseconds; elapsed += 80) {
            craneDelivery->handleSensorInput(force);
            waitForEvents(80);
        }
    };
    driveCrane(25.43, 1200); // Align trolley with the first cargo.
    driveCrane(90.0, 1120);   // Long gesture selects vertical axis.
    craneDelivery->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
    driveCrane(70.0, 1120);  // Lower the hook fully to the cargo pickup height.
    driveCrane(90.0, 320);   // Short gesture grips the cargo.
    craneDelivery->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneDelivery->loadAttached());
    driveCrane(90.0, 1120);  // Long gesture selects horizontal axis.
    craneDelivery->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::HorizontalAxis);
    driveCrane(60.10, 1200); // Move above the delivery bay.
    driveCrane(90.0, 1120);  // Return to vertical control.
    craneDelivery->handleSensorInput(0.0);
    waitForEvents(40);
    REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
    driveCrane(70.0, 1120);  // Lower the cargo fully into the delivery bay.
    driveCrane(90.0, 320);   // Release the load.
    craneDelivery->handleSensorInput(0.0);
    waitForEvents(80);
    REQUIRE(craneDelivery->deliveredLoads() == 1);
    REQUIRE(craneDelivery->cargoType() == games::PrecisionCraneGame::StandardCargo);
    REQUIRE(craneDelivery->missionDeliveriesRemaining() == 2);
    REQUIRE(craneDelivery->precisionSamples() == 1);
    REQUIRE(craneDelivery->averagePrecision() > 0.0);
    REQUIRE(craneDelivery->lastPrecisionBonus() > 0);
    REQUIRE(craneDelivery->getScore() >= 110 + craneDelivery->lastPrecisionBonus());
    REQUIRE(craneDelivery->bestScore() >= craneDelivery->getScore());

    // Repeat two complete deliveries to verify the light -> standard -> heavy
    // cargo rotation. Compute sensor values from the documented 18–70% axis map.
    const auto forceForHorizontalPosition = [](double normalizedX) {
        const double left = 640.0 * 0.08;
        const double right = 640.0 * 0.92;
        const double control = (normalizedX * 640.0 - left) / (right - left);
        return 100.0 * (0.18 + qBound(0.0, control, 1.0) * 0.52);
    };
    const auto deliverNextCargo = [&](double cargoX, double targetX) {
        driveCrane(forceForHorizontalPosition(cargoX), 1200);
        driveCrane(90.0, 1120);
        craneDelivery->handleSensorInput(0.0);
        waitForEvents(40);
        REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
        driveCrane(70.0, 1120);
        driveCrane(90.0, 320);
        craneDelivery->handleSensorInput(0.0);
        waitForEvents(40);
        REQUIRE(craneDelivery->loadAttached());
        driveCrane(90.0, 1120);
        craneDelivery->handleSensorInput(0.0);
        waitForEvents(40);
        REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::HorizontalAxis);
        driveCrane(forceForHorizontalPosition(targetX), 1200);
        driveCrane(90.0, 1120);
        craneDelivery->handleSensorInput(0.0);
        waitForEvents(40);
        REQUIRE(craneDelivery->controlAxis() == games::PrecisionCraneGame::VerticalAxis);
        driveCrane(70.0, 1120);
        driveCrane(90.0, 320);
        craneDelivery->handleSensorInput(0.0);
        waitForEvents(80);
    };

    deliverNextCargo(0.33, 0.64);
    REQUIRE(craneDelivery->deliveredLoads() == 2);
    REQUIRE(craneDelivery->cargoType() == games::PrecisionCraneGame::HeavyCargo);
    REQUIRE(craneDelivery->cargoMassKg() > 1.5);
    REQUIRE(craneDelivery->missionDeliveriesRemaining() == 1);

    deliverNextCargo(0.46, 0.52);
    REQUIRE(craneDelivery->deliveredLoads() == 3);
    REQUIRE(craneDelivery->cargoType() == games::PrecisionCraneGame::LightCargo);
    REQUIRE(craneDelivery->cargoMassKg() < 0.7);
    REQUIRE(craneDelivery->missionNumber() == 2);
    REQUIRE(craneDelivery->missionDeliveriesRemaining() == 3);
    REQUIRE(craneDelivery->precisionSamples() == 3);
    craneDelivery->stop();

    // Deterministic collision regression: place the suspended cargo at the
    // first container's analytically known position at a non-zero time.
    QTemporaryDir collisionRecords;
    REQUIRE(collisionRecords.isValid());
    {
        games::PrecisionCraneGame collisionGame(
            nullptr, collisionRecords.filePath(QStringLiteral("records.json")));
        collisionGame.resize(640, 480);
        collisionGame.start();
        const double obstacleStartX =
            games::PrecisionCraneGameTestAccess::firstObstacleXAt(collisionGame, 0.0);
        const double obstaclePeakX =
            games::PrecisionCraneGameTestAccess::firstObstacleXAt(
                collisionGame, 1.5707963267948966 / 0.42);
        REQUIRE(obstaclePeakX > obstacleStartX + 100.0);
        REQUIRE(games::PrecisionCraneGameTestAccess::collideWithFirstObstacle(collisionGame));
        REQUIRE(collisionGame.collisionCount() == 1);
        REQUIRE(collisionGame.lifetimeCollisions() == 1);
        REQUIRE(collisionGame.mistakesRemaining() == 2);
        REQUIRE(!collisionGame.loadAttached());
        REQUIRE(collisionGame.deliveredLoads() == 0);
        collisionGame.stop();
    }
    {
        games::PrecisionCraneGame collisionRestored(
            nullptr, collisionRecords.filePath(QStringLiteral("records.json")));
        collisionRestored.resize(640, 480);
        collisionRestored.start();
        REQUIRE(collisionRestored.lifetimeCollisions() == 1);
        collisionRestored.stop();
    }

    // Exercise all 15 campaign deliveries without wall-clock sleeps. The
    // test positions the load in a physically valid target pose and invokes
    // the same production delivery transition used by the short gesture.
    QTemporaryDir campaignRecords;
    REQUIRE(campaignRecords.isValid());
    const QString campaignPath = campaignRecords.filePath(QStringLiteral("precision_crane_records.json"));
    int completedMissions = 0;
    {
        games::PrecisionCraneGame campaign(nullptr, campaignPath);
        campaign.resize(640, 480);
        campaign.start();
        REQUIRE(campaign.missionNumber() == 1);
        for (int delivery = 1; delivery <= 15; ++delivery) {
            const int missionBefore = campaign.missionNumber();
            games::PrecisionCraneGameTestAccess::deliverAtTarget(campaign);
            REQUIRE(campaign.deliveredLoads() == delivery);
            REQUIRE(campaign.precisionSamples() == delivery);
            REQUIRE(campaign.lifetimeDeliveries() == delivery);
            if (delivery % 3 == 0 && delivery < 15) {
                REQUIRE(campaign.missionNumber() == missionBefore + 1);
                ++completedMissions;
            }
            if (delivery < 15) {
                REQUIRE(campaign.isRunning());
                REQUIRE(campaign.missionDeliveriesRemaining() ==
                        (delivery % 3 == 0 ? 3 : 3 - (delivery % 3)));
            }
        }
        REQUIRE(completedMissions == 4);
        REQUIRE(campaign.campaignCompleted());
        REQUIRE(campaign.deliveredLoads() == 15);
        REQUIRE(campaign.missionNumber() == 5);
        REQUIRE(campaign.missionDeliveriesRemaining() == 0);
        REQUIRE(!campaign.isRunning());
        REQUIRE(campaign.lifetimePrecisionSamples() == 15);
        REQUIRE(campaign.bestScore() == campaign.getScore());
    }
    {
        QFile persisted(campaignPath);
        REQUIRE(persisted.open(QIODevice::ReadOnly));
        const QJsonDocument document = QJsonDocument::fromJson(persisted.readAll());
        REQUIRE(document.isObject());
        const QJsonObject record = document.object();
        REQUIRE(record.value(QStringLiteral("totalDeliveries")).toInt() == 15);
        REQUIRE(record.value(QStringLiteral("precisionSamples")).toInt() == 15);
        REQUIRE(record.value(QStringLiteral("bestScore")).toInt() > 0);
        REQUIRE(record.value(QStringLiteral("averagePrecision")).toDouble() > 0.0);
    }

    // A new game instance simulates an application restart and must restore
    // cumulative counters and best score from the same atomically-written file.
    {
        games::PrecisionCraneGame restored(nullptr, campaignPath);
        restored.resize(640, 480);
        restored.start();
        REQUIRE(restored.lifetimeDeliveries() == 15);
        REQUIRE(restored.lifetimePrecisionSamples() == 15);
        REQUIRE(restored.lifetimeCollisions() == 0);
        REQUIRE(restored.bestScore() > 0);
        REQUIRE(!restored.campaignCompleted());
        REQUIRE(restored.deliveredLoads() == 0);
        REQUIRE(restored.missionNumber() == 1);
        restored.stop();
    }

    // Rhythm Force is a one-sensor rhythm game. Timing and peak force are
    // evaluated from short force pulses; no keyboard or second input is used.
    std::unique_ptr<games::GameEngine> rhythmBase(
        games::createGame(QStringLiteral("rhythm_force")));
    auto* rhythm = dynamic_cast<games::RhythmForceGame*>(rhythmBase.get());
    REQUIRE(rhythm != nullptr);
    rhythm->resize(640, 480);
    rhythm->setDifficultyLevel(1);
    REQUIRE(rhythm->beatsPerMinute() == 72);
    rhythm->start();
    REQUIRE(rhythm->nextBeatNumber() == 1);
    REQUIRE(!rhythm->hasFreshSignal());
    games::RhythmForceGameTestAccess::emitPulse(
        *rhythm, 1.0, 0.12,
        games::RhythmForceGameTestAccess::targetForce(*rhythm, 1));
    REQUIRE(rhythm->hitCount() == 1);
    REQUIRE(rhythm->perfectCount() == 1);
    REQUIRE(rhythm->lastHitGrade() == games::RhythmForceGame::Perfect);
    REQUIRE(rhythm->nextBeatNumber() == 2);
    REQUIRE(rhythm->combo() == 1);
    REQUIRE(rhythm->getScore() >= 100);
    REQUIRE(rhythm->accuracyPercent() == 100.0);
    REQUIRE(rhythm->hasAchievement(QStringLiteral("first_hit")));
    REQUIRE(rhythm->hasAchievement(QStringLiteral("first_perfect")));
    REQUIRE(rhythm->unlockedAchievementsCount() >= 2);
    const double visualClockBeforeSignal = rhythm->visualTimeSeconds();
    games::RhythmForceGameTestAccess::advance(*rhythm, 0.05);
    REQUIRE(rhythm->visualTimeSeconds() > visualClockBeforeSignal);

    const double secondBeat = games::RhythmForceGameTestAccess::beatTime(*rhythm, 2);
    games::RhythmForceGameTestAccess::emitPulse(
        *rhythm, secondBeat + 0.10, 0.12,
        games::RhythmForceGameTestAccess::targetForce(*rhythm, 2));
    REQUIRE(rhythm->hitCount() == 2);
    REQUIRE(rhythm->lastHitGrade() == games::RhythmForceGame::Good);
    REQUIRE(rhythm->combo() == 2);
    REQUIRE(rhythm->bestCombo() == 2);
    REQUIRE(rhythm->lastTimingOffsetMs() > 90.0);
    REQUIRE(rhythm->lastTimingOffsetMs() < 110.0);

    // A pulse that starts inside the note window may finish after the visual
    // timing window. Its pending note must not expire before release.
    const double thirdBeat = games::RhythmForceGameTestAccess::beatTime(*rhythm, 3);
    games::RhythmForceGameTestAccess::beginPulse(
        *rhythm, thirdBeat,
        games::RhythmForceGameTestAccess::targetForce(*rhythm, 3));
    games::RhythmForceGameTestAccess::advance(*rhythm, 0.10);
    games::RhythmForceGameTestAccess::advance(*rhythm, 0.10);
    games::RhythmForceGameTestAccess::advance(*rhythm, 0.10);
    REQUIRE(rhythm->missCount() == 0);
    REQUIRE(rhythm->nextBeatNumber() == 3);
    games::RhythmForceGameTestAccess::releasePulse(*rhythm);
    REQUIRE(rhythm->hitCount() == 3);
    REQUIRE(rhythm->nextBeatNumber() == 4);

    // Too-short pulses are rejected exactly once and consume the active note.
    const double fourthBeat = games::RhythmForceGameTestAccess::beatTime(*rhythm, 4);
    games::RhythmForceGameTestAccess::emitPulse(
        *rhythm, fourthBeat, 0.02,
        games::RhythmForceGameTestAccess::targetForce(*rhythm, 4));
    REQUIRE(rhythm->missCount() == 1);
    REQUIRE(rhythm->combo() == 0);
    REQUIRE(rhythm->nextBeatNumber() == 5);
    const double fifthBeat = games::RhythmForceGameTestAccess::beatTime(*rhythm, 5);
    games::RhythmForceGameTestAccess::setClock(
        *rhythm, fifthBeat + 0.24);
    games::RhythmForceGameTestAccess::advance(*rhythm, 0.01);
    REQUIRE(rhythm->missCount() == 2);
    REQUIRE(rhythm->nextBeatNumber() == 6);
    REQUIRE(rhythm->accuracyPercent() > 0.0);
    rhythm->setDifficultyLevel(99);
    REQUIRE(rhythm->difficultyLevel() == 5);
    REQUIRE(rhythm->beatsPerMinute() == 120);
    rhythm->setDifficultyLevel(0);
    REQUIRE(rhythm->difficultyLevel() == 1);
    rhythm->stop();

    // Signal loss cancels an in-progress pulse and freezes the chart. A high
    // reconnecting sample is ignored until a neutral sample rearms the input.
    std::unique_ptr<games::RhythmForceGame> rhythmSafety(new games::RhythmForceGame);
    rhythmSafety->resize(640, 480);
    rhythmSafety->start();
    rhythmSafety->handleSensorInput(0.0);
    rhythmSafety->handleSensorInput(68.0);
    REQUIRE(rhythmSafety->pulseActive());
    for (int step = 0; step < 4; ++step)
        games::RhythmForceGameTestAccess::advance(*rhythmSafety, 0.10);
    REQUIRE(!rhythmSafety->hasFreshSignal());
    REQUIRE(!rhythmSafety->pulseActive());
    const double frozenRhythmTime = rhythmSafety->songTimeSeconds();
    const double visualTimeAtTimeout = rhythmSafety->visualTimeSeconds();
    games::RhythmForceGameTestAccess::advance(*rhythmSafety, 0.10);
    REQUIRE(std::abs(rhythmSafety->songTimeSeconds() - frozenRhythmTime) < 0.001);
    REQUIRE(rhythmSafety->visualTimeSeconds() > visualTimeAtTimeout);
    rhythmSafety->handleSensorInput(90.0);
    REQUIRE(!rhythmSafety->pulseActive());
    rhythmSafety->handleSensorInput(0.0);
    rhythmSafety->handleSensorInput(70.0);
    REQUIRE(rhythmSafety->pulseActive());
    rhythmSafety->stop();

    // Resolving beat 32 closes the finite round and stops the game cleanly.
    std::unique_ptr<games::RhythmForceGame> rhythmFinal(new games::RhythmForceGame);
    rhythmFinal->resize(640, 480);
    rhythmFinal->start();
    games::RhythmForceGameTestAccess::setFinalBeat(*rhythmFinal);
    const double finalBeat = games::RhythmForceGameTestAccess::beatTime(*rhythmFinal, 32);
    games::RhythmForceGameTestAccess::emitPulse(
        *rhythmFinal, finalBeat, 0.12,
        games::RhythmForceGameTestAccess::targetForce(*rhythmFinal, 32));
    REQUIRE(rhythmFinal->roundCompleted());
    REQUIRE(rhythmFinal->hitCount() == 1);
    REQUIRE(rhythmFinal->nextBeatNumber() == 33);
    REQUIRE(!rhythmFinal->isRunning());

    // A complete perfect campaign unlocks long-form achievements and persists
    // them across a game reset (the same storage used after application restart).
    std::unique_ptr<games::RhythmForceGame> rhythmAchievements(new games::RhythmForceGame);
    rhythmAchievements->resize(640, 480);
    rhythmAchievements->start();
    for (int beat = 1; beat <= rhythmAchievements->totalBeats(); ++beat) {
        games::RhythmForceGameTestAccess::emitPulse(
            *rhythmAchievements,
            games::RhythmForceGameTestAccess::beatTime(*rhythmAchievements, beat),
            0.12,
            games::RhythmForceGameTestAccess::targetForce(*rhythmAchievements, beat));
    }
    REQUIRE(rhythmAchievements->roundCompleted());
    REQUIRE(rhythmAchievements->hitCount() == 32);
    REQUIRE(rhythmAchievements->missCount() == 0);
    REQUIRE(rhythmAchievements->perfectCount() == 32);
    REQUIRE(rhythmAchievements->hasAchievement(QStringLiteral("combo_8")));
    REQUIRE(rhythmAchievements->hasAchievement(QStringLiteral("flawless_round")));
    REQUIRE(rhythmAchievements->hasAchievement(QStringLiteral("perfect_16")));
    REQUIRE(rhythmAchievements->hasAchievement(QStringLiteral("accuracy_90")));
    REQUIRE(rhythmAchievements->personalBestScore() >= rhythmAchievements->getScore());
    REQUIRE(rhythmAchievements->personalBestAccuracy() >= 100.0);
    REQUIRE(rhythmAchievements->personalBestCombo() >= 32);
    const int savedPersonalBest = rhythmAchievements->personalBestScore();
    rhythmAchievements->reset();
    REQUIRE(rhythmAchievements->hasAchievement(QStringLiteral("flawless_round")));
    REQUIRE(rhythmAchievements->personalBestScore() == savedPersonalBest);

    REQUIRE(games::createGame(QStringLiteral("unknown-game")) == nullptr);
    return 0;
}
