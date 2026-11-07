#include "games/GameEngine.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <cassert>
#include <cmath>
#include <limits>
#include <thread>

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
    bool handleInput(double value) override { ++m_inputCount; m_lastHandled = value; return true; }
private:
    double m_lastDelta = 0.0;
    double m_lastHandled = 0.0;
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
    assert(game.isRunning() && !game.isPaused());
    waitForEvents(70);
    const QString runningStats = game.getGameStats();
    assert(runningStats.contains(QStringLiteral("state=running")));
    assert(runningStats.contains(QStringLiteral("frames=")));

    game.pause();
    assert(game.isPaused());
    const QString pausedStats = game.getGameStats();
    assert(pausedStats.contains(QStringLiteral("state=paused")));
    game.start();
    assert(game.isPaused()); // start() must not silently discard a paused session
    game.resume();
    assert(game.isRunning() && !game.isPaused());
    waitForEvents(40);
    assert(game.lastDelta() >= 0.0 && game.lastDelta() <= 0.1);

    // A sensor callback from a worker thread is queued to the widget thread.
    std::thread worker([&game]() { game.handleSensorInput(42.5); });
    worker.join();
    waitForEvents(10);
    assert(std::abs(game.lastInput() - 42.5) < 1e-9);

    const int inputsBeforeInvalid = game.inputCount();
    game.handleSensorInput(std::numeric_limits<double>::quiet_NaN());
    assert(game.inputCount() == inputsBeforeInvalid);
    assert(game.getGameStats().contains(QStringLiteral("invalid_inputs=1")));

    game.award(std::numeric_limits<int>::max());
    game.award(1); // score arithmetic must saturate instead of overflowing
    assert(game.getScore() == std::numeric_limits<int>::max());

    game.stop();
    assert(!game.isRunning() && !game.isPaused());
    game.resetGameStats();
    const QString resetStats = game.getGameStats();
    assert(resetStats.contains(QStringLiteral("elapsed_ms=0")));
    assert(resetStats.contains(QStringLiteral("frames=0")));
    assert(resetStats.contains(QStringLiteral("invalid_inputs=0")));
    return 0;
}
