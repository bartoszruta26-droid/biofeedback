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
if "addDataCallback(" not in main or "forwardSensorToGame(normalizedGameInput)" not in main:
    raise SystemExit("MainWindow must own and route the global game input")
if "gramsToNewtons(data.calibratedValue)" not in main:
    raise SystemExit("Global game input must convert calibrated grams to SI force")
if "gramsToNewtons(data.calibratedValue)" not in training:
    raise SystemExit("TrainingTab must convert calibrated grams to SI force")
if "!data.isValid" not in training:
    raise SystemExit("TrainingTab must reject invalid sensor frames")
if "if (!m_serialPort || !m_serialPort->isConnected())" not in training:
    raise SystemExit("Training must refuse to start without a connected sensor")
if "m_sensorThreshold(30.0)" not in Path("src/games/MarioGame.cpp").read_text(encoding="utf-8"):
    raise SystemExit("Mario input threshold must match 0-100 normalized game input")
print("Centralized game sensor ownership regression passed")
