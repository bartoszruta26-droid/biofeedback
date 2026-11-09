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
# Measurement and training use a 3:1 horizontal workspace: visual content
# remains on the left while configuration is isolated in a scrollable sidebar.
for source in (measurement, training):
    assert "QSplitter(Qt::Horizontal" in source
    assert "workspaceSplitter->setStretchFactor(0, 3);" in source
    assert "workspaceSplitter->setStretchFactor(1, 1);" in source
    assert "auto* visualScroll = new QScrollArea(workspaceSplitter);" in source
    assert "auto* optionsScroll = new QScrollArea(workspaceSplitter);" in source
    assert "workspaceSplitter->setSizes({900, 300});" in source

assert "optionsLayout->addLayout(m_controlLayout);" in measurement
assert "m_visualLayout->addWidget(m_liveBox, 3);" in measurement
assert "m_visualLayout->addWidget(m_statsBox);" in measurement
assert "m_visualLayout->addWidget(m_biomechBox);" in measurement
assert "m_visualLayout->addWidget(m_rawBox);" in measurement
assert "m_forcePlot->setMinimumHeight(360);" in measurement
assert "setMinimumHeight(280);" in training
assert "QSizePolicy::Expanding, QSizePolicy::Preferred" in training
assert "optionsLayout->addWidget(m_configBox);" in training
assert "visualLayout->addWidget(m_progressBox, 2);" in training
assert "visualLayout->addWidget(m_gameBox, 0);" in training
assert "visualLayout->addWidget(m_statsBox, 3);" in training
assert "QVBoxLayout* configLayout = new QVBoxLayout(m_configBox);" in training
assert "if (!rawToNewtons(raw, forceNewtons))" in measurement
assert "rawToNewtons(raw, forceNewtons) || forceNewtons < 0.0" not in measurement
print("desktop_presentation_regression: PASS")
