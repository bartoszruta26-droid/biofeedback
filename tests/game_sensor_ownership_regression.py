#!/usr/bin/env python3
"""Ensure game input has one owner and calibrated firmware values use SI force."""
from pathlib import Path

game_sources = [
    "src/games/PongGame.cpp",
    "src/games/FlappyBirdGame.cpp",
    "src/games/SinGame.cpp",
    "src/games/MarioGame.cpp",
]
for name in game_sources:
    source = Path(name).read_text(encoding="utf-8")
    for forbidden in ("setDataCallback(", "setConnectionCallback(", "startAsyncReading("):
        if forbidden in source:
            raise SystemExit(f"{name}: game must not own serial callbacks ({forbidden})")

main_window = Path("src/gui/MainWindow.cpp").read_text(encoding="utf-8")
training = Path("src/tab/TrainingTab.cpp").read_text(encoding="utf-8")
if "addDataCallback(" not in main_window or "forwardSensorToGame(forceNewtons)" not in main_window:
    raise SystemExit("MainWindow must own and route sensor samples to its global game")
if "gramsToNewtons(data.calibratedValue)" not in main_window:
    raise SystemExit("MainWindow game input must convert firmware grams to newtons")
if "gramsToNewtons(data.calibratedValue)" not in training:
    raise SystemExit("TrainingTab must convert firmware grams to newtons")
if "!data.isValid" not in training:
    raise SystemExit("TrainingTab must reject invalid sensor frames")

print("Game sensor ownership and SI force policy passed")
