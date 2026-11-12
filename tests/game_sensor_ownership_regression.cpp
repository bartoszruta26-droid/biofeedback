#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {
std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "FAIL: cannot read " << path.string() << '\n';
        std::exit(EXIT_FAILURE);
    }
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void requireContains(const std::string& source, const std::string& token, const char* message) {
    if (source.find(token) == std::string::npos) {
        std::cerr << "FAIL: " << message << " (missing: " << token << ")\n";
        std::exit(EXIT_FAILURE);
    }
}

void requireAbsent(const std::string& source, const std::string& token, const char* message) {
    if (source.find(token) != std::string::npos) {
        std::cerr << "FAIL: " << message << " (forbidden: " << token << ")\n";
        std::exit(EXIT_FAILURE);
    }
}
}

int main() {
    const std::vector<std::string> games{
        "src/games/PongGame.cpp",
        "src/games/FlappyBirdGame.cpp",
        "src/games/SinGame.cpp",
        "src/games/MarioGame.cpp"
    };
    for (const auto& path : games) {
        const auto source = readFile(path);
        for (const auto& forbidden : {"setDataCallback(", "setConnectionCallback(", "startAsyncReading("})
            requireAbsent(source, forbidden, "game must not own serial callbacks");
    }

    const auto main = readFile("src/gui/MainWindow.cpp");
    const auto training = readFile("src/tab/TrainingTab.cpp");
    const auto measurement = readFile("src/tab/MeasurementTab.cpp");
    const auto mario = readFile("src/games/MarioGame.cpp");
    const auto engine = readFile("src/games/GameEngine.cpp");
    const auto engineHeader = readFile("include/games/GameEngine.hpp");
    const auto arkanoid = readFile("src/games/ArkanoidGame.cpp");

    requireContains(main, "gameMeasurementEngine->subscribe", "MainWindow subscribes to calibrated force");
    requireContains(main, "enqueueGameSensorInput(normalized)", "MainWindow enqueues normalized game input");
    requireContains(main, "m_pendingGameSensorInput = value", "game input mailbox stores latest value");
    requireContains(main, "m_gameSensorInputAvailable = true", "game input mailbox marks latest value available");
    requireContains(main, "forwardSensorToGame(latestValue)", "game input forwards latest value");
    requireContains(measurement, "m_trainingMeasurementEngine->ingest(input)", "MeasurementTab publishes to shared measurement engine");
    requireContains(measurement, "rawToNewtons(data.value, forceNewtons)", "MeasurementTab converts calibrated force to newtons");
    requireContains(measurement, "input.unit = \"N\"", "MeasurementTab publishes SI newtons");
    requireContains(training, "setMeasurementEngine", "TrainingTab accepts shared measurement engine");
    requireContains(training, "m_measurementEngine->subscribe", "TrainingTab subscribes to shared measurement stream");
    requireAbsent(training, "addDataCallback(", "TrainingTab must not create a second serial sample path");
    requireAbsent(training, "onSensorDataReceived", "TrainingTab must not create a second serial sample path");
    requireContains(training, "const double force = std::abs(sample.value)", "TrainingTab uses calibrated tensile force");
    requireContains(training, "!sample.isValid()", "TrainingTab rejects invalid samples");
    requireContains(training, "sample.unit != \"N\"", "TrainingTab rejects non-SI samples");
    requireContains(training, "if (!m_serialPort || !m_serialPort->isConnected())", "training requires a connected sensor");
    requireContains(main, "gameReferenceForceN = 150.0", "games normalize against 150 N reference force");
    requireContains(mario, "m_sensorThreshold(30.0)", "Mario threshold uses normalized 0-100 input");
    requireContains(engine, "m_gameTimer->setInterval(8)", "game timer pumps the 120 Hz simulation");
    requireContains(engineHeader, "kFixedSimulationStepSeconds = 1.0 / 120.0", "shared simulation timestep is 120 Hz");
    requireContains(training, "m_liveUiTimer->setInterval(8)", "TrainingTab refreshes game input at 120 Hz");
    requireContains(arkanoid, "std::exp(-dt/0.025)", "Arkanoid paddle interpolates sensor samples");
    requireContains(engine, "setAttribute(Qt::WA_OpaquePaintEvent, true)", "game widget avoids redundant background painting");

    std::cout << "Centralized game sensor ownership and 120 Hz simulation regression passed\n";
    return EXIT_SUCCESS;
}
