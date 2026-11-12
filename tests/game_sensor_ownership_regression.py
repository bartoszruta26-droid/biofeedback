#!/usr/bin/env python3
"""Ensure the application owns the only serial data stream and routes SI force."""
from pathlib import Path

games = (
    "src/games/PongGame.cpp",
    "src/games/FlappyBirdGame.cpp",
    "src/games/SinGame.cpp",
    "src/games/MarioGame.cpp",
)
for path in games:
    source = Path(path).read_text(encoding="utf-8")
    for forbidden in ("setDataCallback(", "setConnectionCallback(", "startAsyncReading("):
        if forbidden in source:
            raise SystemExit(f"{path}: game must not own serial callbacks ({forbidden})")

main = Path("src/gui/MainWindow.cpp").read_text(encoding="utf-8")
training = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
measurement = Path("src/tab/MeasurementTab.cpp").read_text(encoding="utf-8")
if "gameMeasurementEngine->subscribe" not in main or "enqueueGameSensorInput(normalized)" not in main:
    raise SystemExit("MainWindow must subscribe to calibrated force and enqueue global game input")
if ("m_pendingGameSensorInput = value" not in main or
        "m_gameSensorInputAvailable = true" not in main or
        "forwardSensorToGame(latestValue)" not in main):
    raise SystemExit("MainWindow must coalesce game control to the newest valid sensor value")
if ("m_trainingMeasurementEngine->ingest(input)" not in measurement or
        "rawToNewtons(data.value, forceNewtons)" not in measurement or 'input.unit = "N"' not in measurement):
    raise SystemExit("MeasurementTab must publish locally calibrated force in SI newtons")
if "setMeasurementEngine" not in training or "m_measurementEngine->subscribe" not in training:
    raise SystemExit("TrainingTab must consume the shared MeasurementEngine stream")
if "addDataCallback(" in training or "onSensorDataReceived" in training:
    raise SystemExit("TrainingTab must not create a second serial sample path")
if "const double force = std::abs(sample.value)" not in training:
    raise SystemExit("TrainingTab must use calibrated tensile force in newtons")
if "!sample.isValid()" not in training or 'sample.unit != "N"' not in training:
    raise SystemExit("TrainingTab must reject invalid or non-mass canonical samples")
if "if (!m_serialPort || !m_serialPort->isConnected())" not in training:
    raise SystemExit("Training must refuse to start without a connected sensor")
if "gameReferenceForceN = 150.0" not in main:
    raise SystemExit("All games must normalize calibrated force against the 150 N (approximately 15 kgf) full-scale input")
if "m_sensorThreshold(30.0)" not in Path("src/games/MarioGame.cpp").read_text(encoding="utf-8"):
    raise SystemExit("Mario input threshold must match 0-100 normalized game input")
game_engine = Path("src/games/GameEngine.cpp").read_text(encoding="utf-8")
training = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
arkanoid = Path("src/games/ArkanoidGame.cpp").read_text(encoding="utf-8")
if "m_gameTimer->setInterval(2)" not in game_engine:
    raise SystemExit("Game loop must run at the 500 Hz presentation cadence")
if "m_liveUiTimer->setInterval(8)" not in training:
    raise SystemExit("TrainingTab must refresh the latest game-control sample at 120 Hz")
if "std::exp(-dt/0.025)" not in arkanoid:
    raise SystemExit("Arkanoid paddle must interpolate between sensor samples")
if "setAttribute(Qt::WA_OpaquePaintEvent, true)" not in game_engine:
    raise SystemExit("Game widget must avoid redundant background painting")
print("Centralized game sensor ownership and smoothness regression passed")
