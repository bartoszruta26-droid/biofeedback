#include "games/GameEngine.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <thread>

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::fprintf(stderr, "GameEngine regression failed at %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        std::abort(); \
    } \
} while (false)

class TestGame final : public games::GameEngine {
public:
    using GameEngine::GameEngine;
    QString gameName() const override { return QStringLiteral("TestGame"); }
    double lastInput() const { return m_sensorValue; }
    double lastDelta() const { return m_lastDelta; }
    int inputCount() const { return m_inputCount; }
    int initializeCount() const { return m_initializeCount; }
    void award(int points) { addScore(points); }
protected:
    void initialize() override { m_lastDelta = 0.0; ++m_initializeCount; }
    void update(double deltaTime) override { m_lastDelta = deltaTime; }
    void render(QPainter&) override {}
    bool handleInput(double) override { ++m_inputCount; return true; }
private:
    double m_lastDelta = 0.0;
    int m_inputCount = 0;
    int m_initializeCount = 0;
};

static void waitForEvents(int milliseconds)
{
    QEventLoop loop;
    QTimer::singleShot(milliseconds, &loop, &QEventLoop::quit);
    loop.exec();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    TestGame game;
    game.resize(320, 240);

    game.start();
    REQUIRE(game.isRunning() && !game.isPaused());
    REQUIRE(game.initializeCount() == 1);
    waitForEvents(70);
    REQUIRE(game.getGameStats().contains(QStringLiteral("state=running")));
    game.award(5);
    game.start();
    REQUIRE(game.getScore() == 5); // Repeated start preserves the active game.

    game.pause();
    REQUIRE(game.isPaused());
    REQUIRE(game.getGameStats().contains(QStringLiteral("state=paused")));
    game.start(); // start() while paused resumes instead of resetting.
    REQUIRE(game.isRunning() && !game.isPaused());
    REQUIRE(game.initializeCount() == 1);

    game.pause();
    game.resume();
    REQUIRE(game.isRunning() && !game.isPaused());
    waitForEvents(40);
    REQUIRE(game.lastDelta() > 0.0 && game.lastDelta() <= 1.0 / 480.0 + 1e-9);

    // A paused session must not continue advancing the fixed-step simulation.
    game.pause();
    const double pausedDelta = game.lastDelta();
    waitForEvents(40);
    REQUIRE(std::abs(game.lastDelta() - pausedDelta) < 1e-12);
    REQUIRE(game.isPaused());
    game.resume();

    const int inputsBeforeBurst = game.inputCount();
    std::thread worker([&game]() {
        game.handleSensorInput(std::numeric_limits<double>::quiet_NaN());
        for (int sample = 0; sample < 1000; ++sample)
            game.handleSensorInput(static_cast<double>(sample));
    });
    worker.join();
    waitForEvents(10);
    REQUIRE(std::abs(game.lastInput() - 999.0) < 1e-9);
    // A burst produced before the GUI loop drains must be coalesced rather
    // than creating one queued Qt event per sensor sample.
    REQUIRE(game.inputCount() == inputsBeforeBurst + 1);
    REQUIRE(game.getGameStats().contains(QStringLiteral("invalid_inputs=1")));

    // A stopped/restarted engine must not apply an already queued old sample.
    std::thread staleWorker([&game]() { game.handleSensorInput(12345.0); });
    staleWorker.join();
    game.stop();
    game.start();
    const int inputsAfterRestart = game.inputCount();
    waitForEvents(20);
    REQUIRE(game.inputCount() == inputsAfterRestart);
    REQUIRE(std::abs(game.lastInput()) < 1e-9);

    // Public reset() must also discard a queued worker sample and its
    // availability flag before a subsequent start.
    std::thread resetWorker([&game]() { game.handleSensorInput(23456.0); });
    resetWorker.join();
    game.reset();
    REQUIRE(!game.isRunning() && !game.isPaused());
    REQUIRE(game.initializeCount() == 3); // Explicit reset rebuilds derived state once.
    game.start();
    REQUIRE(game.initializeCount() == 4); // Starting after reset initializes a fresh session once.
    const int inputsAfterReset = game.inputCount();
    waitForEvents(20);
    REQUIRE(game.inputCount() == inputsAfterReset);
    REQUIRE(std::abs(game.lastInput()) < 1e-9);

    // Invalid worker samples are counted on the GUI thread and must not
    // overwrite the most recent valid sample in the coalescing mailbox.
    std::thread invalidWorker([&game]() {
        game.handleSensorInput(42.5);
        game.handleSensorInput(std::numeric_limits<double>::infinity());
    });
    invalidWorker.join();
    waitForEvents(10);
    REQUIRE(std::abs(game.lastInput() - 42.5) < 1e-9);
    REQUIRE(game.getGameStats().contains(QStringLiteral("invalid_inputs=1")));

    const int inputsBeforeInvalid = game.inputCount();
    game.handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(game.inputCount() == inputsBeforeInvalid);
    REQUIRE(game.getGameStats().contains(QStringLiteral("invalid_inputs=2")));

    game.award(std::numeric_limits<int>::max());
    game.award(1);
    REQUIRE(game.getScore() == std::numeric_limits<int>::max());

    game.stop();
    REQUIRE(!game.isRunning() && !game.isPaused());
    game.resetGameStats();
    const QString stats = game.getGameStats();
    REQUIRE(stats.contains(QStringLiteral("elapsed_ms=0")));
    REQUIRE(stats.contains(QStringLiteral("frames=0")));
    REQUIRE(stats.contains(QStringLiteral("invalid_inputs=0")));
    REQUIRE(game.getScore() == 0);
    return 0;
}
