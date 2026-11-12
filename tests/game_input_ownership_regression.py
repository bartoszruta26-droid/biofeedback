#!/usr/bin/env python3
"""Static guardrails for the single-tensometer arcade input and render profile."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
GAME_SOURCES = sorted((ROOT / "src" / "games").glob("*.cpp"))
GAME_HEADERS = sorted((ROOT / "include" / "games").glob("*.hpp"))

if not GAME_SOURCES:
    raise SystemExit("No game source files were found.")

for path in GAME_SOURCES + GAME_HEADERS:
    source = path.read_text(encoding="utf-8")
    forbidden = (
        r"\bkeyPressEvent\s*\(",
        r"\bkeyReleaseEvent\s*\(",
        r"\bmousePressEvent\s*\(",
        r"\bmouseReleaseEvent\s*\(",
        r"\bmouseMoveEvent\s*\(",
        r"#\s*include\s*[<\"]Q(?:Key|Mouse)Event",
    )
    for pattern in forbidden:
        if re.search(pattern, source):
            raise SystemExit(f"Forbidden keyboard/mouse gameplay path in {path.relative_to(ROOT)}: {pattern}")
    if "setFocusPolicy(Qt::StrongFocus)" in source:
        raise SystemExit(f"Game widget must not capture keyboard focus: {path.relative_to(ROOT)}")
    if re.search(r"Q(?:Linear|Radial)Gradient", source):
        raise SystemExit(f"Expensive gradient rendering is not allowed in the arcade profile: {path.relative_to(ROOT)}")
    if re.search(r"setRenderHint\(QPainter::Antialiasing\s*(?:,\s*true)?\s*\)", source):
        raise SystemExit(f"Antialiasing must remain disabled in the arcade profile: {path.relative_to(ROOT)}")

engine = (ROOT / "src" / "games" / "GameEngine.cpp").read_text(encoding="utf-8")
engine_header = (ROOT / "include" / "games" / "GameEngine.hpp").read_text(encoding="utf-8")
required_engine_tokens = (
    "m_gameTimer->setInterval(8)",
    "m_gameTimer->setTimerType(Qt::PreciseTimer)",
    "constexpr double fixedStep = kFixedSimulationStepSeconds;",
    "m_monotonicClock.nsecsElapsed()",
    "/ 1.0e9",
    "Qt::WA_OpaquePaintEvent",
    "Qt::WA_NoSystemBackground",
)
for token in required_engine_tokens:
    if token not in engine:
        raise SystemExit(f"GameEngine is missing the arcade performance requirement: {token}")

if "return m_renderInterpolationAlpha;" not in engine_header:
    raise SystemExit("GameEngine must bias rendering toward the newest state to reduce interpolation latency.")

for path in GAME_SOURCES:
    if "::renderInterpolated(" not in path.read_text(encoding="utf-8"):
        raise SystemExit(f"Game is missing frame-rate-independent render interpolation: {path.relative_to(ROOT)}")
if "static constexpr double kFixedSimulationStepSeconds = 1.0 / 240.0;" not in engine_header:
    raise SystemExit("GameEngine fixed simulation step must run at 240 Hz.")
if "QWidget::update();" not in engine:
    raise SystemExit("Game loop must schedule non-blocking presentation updates.")
if "constexpr double maxStep=1.0/240.0" not in (ROOT / "src" / "games" / "ArkanoidGame.cpp").read_text(encoding="utf-8"):
    raise SystemExit("Arkanoid physics must use the shared 240 Hz motion profile.")
if "constexpr double kPhysicsStep = 1.0 / 240.0;" not in (ROOT / "src" / "games" / "PongGame.cpp").read_text(encoding="utf-8"):
    raise SystemExit("Pong physics must use the shared 240 Hz motion profile.")

car_race = (ROOT / "src" / "games" / "CarRaceGame.cpp").read_text(encoding="utf-8")
galaga = (ROOT / "src" / "games" / "GalagaGame.cpp").read_text(encoding="utf-8")
flappy = (ROOT / "src" / "games" / "FlappyBirdGame.cpp").read_text(encoding="utf-8")
for token in ("m_boostGestureActive", "force >= 95.0", "activateBoost()"):
    if token not in car_race:
        raise SystemExit(f"Car Race is missing its tensometer boost gesture: {token}")
for token in ("m_specialGestureActive", "target >= 0.95", "firePlayerShot()"):
    if token not in galaga:
        raise SystemExit(f"Galaga is missing its automatic/gestural action: {token}")
if "FlappyBirdGame::keyPressEvent" in flappy or "FlappyBirdGame::mousePressEvent" in flappy:
    raise SystemExit("Flappy Bird must use the tensometer impulse only.")

print(f"PASS: {len(GAME_SOURCES)} game sources and {len(GAME_HEADERS)} headers follow the single-tensometer arcade profile.")
