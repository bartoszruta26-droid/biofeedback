#!/usr/bin/env python3
"""Static guards for desktop layout, tensile sensor polarity and game windows."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

main = read("src/gui/MainWindow.cpp")
measurement = read("src/tab/MeasurementTab.cpp")
training = read("src/tab/TrainingTab.cpp")
measurement_header = read("include/tab/MeasurementTab.hpp")

assert "if (!sample.isValid() || sample.unit != \"N\"" in main
assert "std::abs(sample.value)" in main
assert "m_gameWindow->showFullScreen();" in main
assert "QShortcut(QKeySequence(Qt::Key_Escape), m_gameWindow)" in main
assert "gameMeasurementEngine->subscribe" in main
assert "m_measurementTab->hasCalibrationProfile()" in main
assert "m_measurementTab->trainingMeasurementEngine()" in main
assert "m_gameWindow->showFullScreen();" in training
assert "QDialog(this, Qt::Window)" in training
assert "m_btnOpenGame" in training
assert "showGameWindow();" in training
assert 'input.unit = "N";' in measurement
assert "rawToNewtons(data.value, forceNewtons)" in measurement
assert "trainingMeasurementEngine()" in measurement_header
assert 'sample.unit != "N"' in training
assert "const double force = std::abs(sample.value);" in training
assert "m_currentGame->handleSensorInput(gameInput);" in training
assert "m_trainingMeasurementEngine->ingest(input);" in measurement
assert "m_trainingTab->setMeasurementEngine(m_measurementTab->trainingMeasurementEngine());" in main
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
    assert "QSplitter::splitterMoved" in source
    assert "const int availableWidth = workspaceSplitter->width() - workspaceSplitter->handleWidth();" in source
    assert "const int maxVisualWidth = availableWidth * 3 / 4;" in source
    assert "if (position < maxVisualWidth)" in source
    assert "workspaceSplitter->setSizes({maxVisualWidth, availableWidth - maxVisualWidth});" in source

# Exercise the sidebar-width constraint at common desktop resolutions. The
# drag handle may reduce the sidebar freely, but may never exceed 25%.
for viewport_width in (1024, 1280, 1366, 1600, 1920, 2560):
    available = viewport_width - 5
    max_visual = available * 3 // 4
    max_options = available - max_visual
    assert max_options <= available * 0.25 + 1
    for dragged_position in (0, available // 2, max_visual, available):
        visual = max(dragged_position, max_visual)
        options = available - visual
        assert options <= available * 0.25 + 1

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
