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
    void award(int points) { addScore(points); }
protected:
    void initialize() override { m_lastDelta = 0.0; }
    void update(double deltaTime) override { m_lastDelta = deltaTime; }
    void render(QPainter&) override {}
    bool handleInput(double) override { ++m_inputCount; return true; }
private:
    double m_lastDelta = 0.0;
    int m_inputCount = 0;
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

    // Start while active must not reset or reinitialize the running session.
    game.start();
    REQUIRE(game.isRunning() && !game.isPaused());
    waitForEvents(70);
    const QString runningStats = game.getGameStats();
    REQUIRE(runningStats.contains(QStringLiteral("state=running")));
    REQUIRE(runningStats.contains(QStringLiteral("frames=")));
    game.award(5);
    game.start();
    REQUIRE(game.getScore() == 5);

    game.pause();
    REQUIRE(game.isPaused());
    const QString pausedStats = game.getGameStats();
    REQUIRE(pausedStats.contains(QStringLiteral("state=paused")));
    game.start();
    REQUIRE(game.isPaused()); // start() must not silently discard a paused session
    game.resume();
    REQUIRE(game.isRunning() && !game.isPaused());
    waitForEvents(40);
    REQUIRE(game.lastDelta() >= 0.0 && game.lastDelta() <= 0.1);

    // A sensor callback from a worker thread is queued to the widget thread.
    std::thread worker([&game]() { game.handleSensorInput(42.5); });
    worker.join();
    waitForEvents(10);
    REQUIRE(std::abs(game.lastInput() - 42.5) < 1e-9);

    const int inputsBeforeInvalid = game.inputCount();
    game.handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(game.inputCount() == inputsBeforeInvalid);
    REQUIRE(game.getGameStats().contains(QStringLiteral("invalid_inputs=1")));

    game.award(std::numeric_limits<int>::max());
    game.award(1); // score arithmetic must saturate instead of overflowing
    REQUIRE(game.getScore() == std::numeric_limits<int>::max());

    game.stop();
    REQUIRE(!game.isRunning() && !game.isPaused());
    game.resetGameStats();
    const QString resetStats = game.getGameStats();
    REQUIRE(resetStats.contains(QStringLiteral("elapsed_ms=0")));
    REQUIRE(resetStats.contains(QStringLiteral("frames=0")));
    REQUIRE(resetStats.contains(QStringLiteral("invalid_inputs=0")));
    return 0;
}
