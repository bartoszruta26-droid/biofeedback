#!/usr/bin/env python3
"""Static guards for desktop layout, tensile sensor polarity and game windows."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

main = read("src/gui/MainWindow.cpp")
measurement = read("src/tab/MeasurementTab.cpp")
training = read("src/tab/TrainingTab.cpp")

assert "if (!std::isfinite(forceNewtons)) return;" in main
assert "forceNewtons < 0.0" not in main
assert "std::abs(forceNewtons)" in main
assert "m_gameWindow->showMaximized();" in main
assert "selectedGame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);" in main
assert "m_mainLayout->addWidget(m_gameBox, 0);" in training
assert "m_mainLayout->addWidget(m_gameBox, 1);" not in training
assert "setMinimumHeight(90);" in training
assert "if (!rawToNewtons(raw, forceNewtons))" in measurement
assert "rawToNewtons(raw, forceNewtons) || forceNewtons < 0.0" not in measurement
assert "m_mainLayout->addWidget(m_liveBox, 3);" in measurement
assert "m_forcePlot->setMinimumHeight(280);" in measurement
print("desktop_presentation_regression: PASS")
