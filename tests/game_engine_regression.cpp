#include "games/GameEngine.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <atomic>
#include <cassert>
#include <cmath>
#include <thread>

class TestGame final : public games::GameEngine {
public:
    using GameEngine::GameEngine;
    QString gameName() const override { return QStringLiteral("TestGame"); }
    int updates{0};
    double lastStep{0.0};
    double lastInput{0.0};
protected:
    void initialize() override { updates = 0; }
    void update(double dt) override { ++updates; lastStep = dt; }
    void render(QPainter&) override {}
    bool handleInput(double value) override { lastInput = value; return true; }
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    TestGame game;
    game.resize(320, 240);
    game.start();
    assert(game.isRunning());
    QThread::msleep(25);
    game.gameLoop();
    assert(game.updates > 0);
    assert(game.lastStep > 0.0 && game.lastStep <= (1.0 / 120.0) + 1e-9);

    game.pause();
    assert(game.isPaused());
    const int pausedUpdates = game.updates;
    QThread::msleep(25);
    game.gameLoop();
    assert(game.updates == pausedUpdates);

    std::thread sensor([&game] { game.handleSensorInput(42.5); });
    sensor.join();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    assert(std::abs(game.lastInput - 42.5) < 1e-9);

    game.resume();
    assert(!game.isPaused() && game.isRunning());
    QThread::msleep(25);
    game.gameLoop();
    assert(game.updates > pausedUpdates);
    game.stop();
    assert(!game.isRunning());
    assert(game.getGameStats().contains(QStringLiteral("elapsed_ms=")));
    game.resetGameStats();
    assert(game.getScore() == 0);
    return 0;
}
