#!/usr/bin/env python3
"""Static guards for desktop layout, tensile sensor polarity and game windows."""
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

main = read("src/gui/MainWindow.cpp")
application = read("src/core/Application.cpp")
measurement = read("src/tab/MeasurementTab.cpp")
training = read("src/tab/TrainingTab.cpp")
patient = read("src/tab/PatientTab.cpp")
outline = read("src/tab/OutlineTab.cpp")
debug = read("src/tab/DebugTab.cpp")
graph = read("src/gui/GraphWidget.cpp")
games = [read(path) for path in (
    "src/games/PongGame.cpp",
    "src/games/FlappyBirdGame.cpp",
    "src/games/SinGame.cpp",
    "src/games/MarioGame.cpp",
    "src/games/GameEngine.cpp",
)]
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
# Measurement and training use an 89:11 horizontal workspace: visual content
# remains on the left while configuration is isolated in a compact sidebar.
for source in (measurement, training):
    assert "QSplitter(Qt::Horizontal" in source
    assert "class ProportionalWorkspaceSplitter final : public QSplitter" in source
    assert "void resizeEvent(QResizeEvent* event) override" in source
    assert "const int visualWidth = availableWidth * 89 / 100;" in source
    assert "setSizes({visualWidth, availableWidth - visualWidth});" in source
    assert "auto* visualScroll = new QScrollArea(workspaceSplitter);" in source
    assert "auto* optionsScroll = new QScrollArea(workspaceSplitter);" in source
    assert "QSplitter::splitterMoved" in source
    assert "const int availableWidth = workspaceSplitter->width() - workspaceSplitter->handleWidth();" in source
    assert "const int maxVisualWidth = availableWidth * 89 / 100;" in source
    assert "if (position < maxVisualWidth)" in source
    assert "workspaceSplitter->setSizes({maxVisualWidth, availableWidth - maxVisualWidth});" in source

# Exercise the sidebar-width constraint at common desktop resolutions. The
# drag handle may reduce the sidebar freely, but may never exceed 11%.
for viewport_width in (1024, 1280, 1366, 1600, 1920, 2560):
    available = viewport_width - 5
    max_visual = available * 89 // 100
    max_options = available - max_visual
    assert max_options <= available * 0.11 + 1
    for dragged_position in (0, available // 2, max_visual, available):
        visual = max(dragged_position, max_visual)
        options = available - visual
        assert options <= available * 0.11 + 1

assert "optionsLayout->addLayout(m_controlLayout);" in measurement
assert "QApplication::setFont(QFont(\"Arial\", 8));" in main
assert "QWidget{font-size:8pt;}" in main
assert "m_exerciseSelector->setMinimumWidth(0);" in training
# No secondary tab or custom graph may reintroduce oversized explicit fonts.
for source in (patient, outline, debug, graph):
    assert 'QFont("Arial", 10' not in source
    assert 'QFont("Arial", 11' not in source
    assert 'QFont("Arial", 12' not in source
    assert 'QFont("Arial", 14' not in source
assert 'QFont("Sans Serif", 12' not in outline
assert 'QFont("Sans Serif", 14' not in outline
assert 'QFont("Courier New", 9' not in debug
assert "titleFont.setPointSize(12);" not in graph
# Full-screen game HUD typography is compacted as well; only the title/score
# hierarchy remains moderately larger than ordinary controls.
for source in games:
    assert 'QFont("Arial", 36' not in source
    assert 'QFont("Arial", 24' not in source
    assert 'QFont("Arial", 16' not in source

assert "m_visualLayout->addWidget(m_liveBox, 3);" in measurement
assert "m_visualLayout->addWidget(m_statsBox);" in measurement
assert "m_visualLayout->addWidget(m_biomechBox);" in measurement
assert "m_visualLayout->addWidget(m_rawBox);" in measurement
assert "m_forcePlot->setMinimumHeight(360);" in measurement
assert "setMinimumHeight(280);" in training
assert "QSizePolicy::Expanding, QSizePolicy::Preferred" in training
assert "optionsLayout->addWidget(m_configBox);" in training
# Outline workspace: exercise media, game area, progress and live graph are left;
# outline selection, metadata, game selection and session controls are right.
assert "QSplitter(Qt::Horizontal, this)" in outline
assert "m_leftLayout = new QVBoxLayout(visualPanel);" in outline
assert "m_rightLayout = new QVBoxLayout(settingsPanel);" in outline
assert "m_rightLayout->addWidget(m_outlineGroup);" in outline
assert "m_rightLayout->addWidget(m_outlineInfoGroup);" in outline
assert "m_rightLayout->addWidget(m_gameSelectionGroup);" in outline
assert "m_rightLayout->addWidget(m_controlGroup);" in outline
assert "m_leftLayout->addWidget(m_exerciseGroup);" in outline
assert "m_leftLayout->addWidget(m_gameAreaGroup, 1);" in outline
assert "new gui::GraphWidget(graphGroup)" in outline
assert "m_forceGraph->addDataPoint(value, timestamp)" in outline
assert "m_measurementEngine->subscribe" in outline
assert "sample.unit != \"N\"" in outline
assert "m_outlineTab->setMeasurementEngine(m_measurementTab->trainingMeasurementEngine());" in main
# The rehabilitation workspace receives the repository UUID, not the PESEL
# or the legacy per-patient filesystem path.
assert "setCurrentPatientContext(const QString& patientUuid" in read("include/tab/OutlineTab.hpp")
assert "m_currentPatientUuid = patientUuid.trimmed();" in outline
assert "patient.value(QStringLiteral(\"id\")).toString()" in application
assert "m_outlineTab->setCurrentPatientContext(" in application
assert "rehabilitationPatientContext" in outline
assert "setResourceRepository(m_sharedResources)" in read("src/core/Application.cpp")
assert "createRehabilitationEpisode()" in read("include/tab/OutlineTab.hpp")
assert "rehabilitation-episodes" in outline
assert "m_resourceRepository->create(QStringLiteral(\"rehabilitation-episodes\")" in outline
assert "{QStringLiteral(\"patient_id\"), m_currentPatientUuid}" in outline
assert "m_currentEpisodeUuid = m_episodeSelector->currentData().toString();" in outline
assert "rehabilitation-assessments" in outline
assert "createRehabilitationAssessment()" in read("include/tab/OutlineTab.hpp")
assert 'QStringLiteral("pain_nrs")' in outline
assert 'QStringLiteral("rom_degrees")' in outline
assert 'QStringLiteral("strength_mrc")' in outline
assert 'QStringLiteral("swelling_cm")' in outline
assert "Missing measurement is not zero" in outline
assert "rehabilitation-goals" in outline
assert "createRehabilitationGoal()" in read("include/tab/OutlineTab.hpp")
assert 'QStringLiteral("baseline_value")' in outline
assert 'QStringLiteral("target_value")' in outline
assert "refreshRehabilitationGoals()" in outline
assert "startRehabilitationSession()" in read("include/tab/OutlineTab.hpp")
assert "finishRehabilitationSession()" in outline
assert "recordRehabilitation24HourFollowUp()" in outline
assert 'QStringLiteral("pre_session_pain_nrs")' in outline
assert 'QStringLiteral("post_session_pain_nrs")' in outline
assert 'QStringLiteral("pain_at_24h_nrs")' in outline
assert 'QStringLiteral("awaiting_24h")' in outline
assert "m_resourceRepository->create(QStringLiteral(\"rehabilitation-assessments\")" in outline
assert 'outline.clinicalProtocolHtml = root["clinicalProtocolHtml"].toString();' in outline
# Both clinical outlines must be valid, non-empty, executable JSON plans.
import json
for outline_path in (
    "data/outlines/rehabilitacja-lakotka-kolana.json",
    "data/outlines/rehabilitacja-zespol-bolesnego-barku.json",
):
    with (ROOT / outline_path).open(encoding="utf-8") as stream:
        protocol = json.load(stream)
    assert protocol["id"] and protocol["clinicalProtocolHtml"]
    assert len(protocol["exercises"]) >= 8
    assert all(item["id"] and item["name"] and item["description"] for item in protocol["exercises"])
assert "visualLayout->addWidget(m_progressBox, 2);" in training
assert "visualLayout->addWidget(m_gameBox, 0);" in training
assert "visualLayout->addWidget(m_statsBox, 3);" in training
assert "QVBoxLayout* configLayout = new QVBoxLayout(m_configBox);" in training
assert "if (!rawToNewtons(raw, forceNewtons))" in measurement
assert "rawToNewtons(raw, forceNewtons) || forceNewtons < 0.0" not in measurement
print("desktop_presentation_regression: PASS")
