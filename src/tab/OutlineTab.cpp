#include "tab/OutlineTab.hpp"
#include "games/GameEngine.hpp"
#include "games/PongGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/MarioGame.hpp"
#include "games/SinGame.hpp"
#include "gui/GraphWidget.hpp"
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QMetaObject>
#include <QFrame>
#include <cmath>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDate>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardItemModel>

namespace tab {

OutlineTab::OutlineTab(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_outlineGroup(nullptr)
    , m_outlineSelectorLayout(nullptr)
    , m_outlineLabel(nullptr)
    , m_outlineCombo(nullptr)
    , m_loadOutlineButton(nullptr)
    , m_outlineInfoGroup(nullptr)
    , m_outlineNameLabel(nullptr)
    , m_outlineDescriptionLabel(nullptr)
    , m_outlineAuthorLabel(nullptr)
    , m_outlineDurationLabel(nullptr)
    , m_outlineExerciseCountLabel(nullptr)
    , m_exerciseGroup(nullptr)
    , m_exerciseLayout(nullptr)
    , m_exerciseInfoWidget(nullptr)
    , m_exerciseInfoLayout(nullptr)
    , m_exerciseNameLabel(nullptr)
    , m_exerciseDescriptionLabel(nullptr)
    , m_mediaLayout(nullptr)
    , m_exerciseImageLabel(nullptr)
    , m_videoWidget(nullptr)
    , m_playVideoButton(nullptr)
    , m_gameSelectionGroup(nullptr)
    , m_gameSelectionLayout(nullptr)
    , m_gameLabel(nullptr)
    , m_gameCombo(nullptr)
    , m_configureGameButton(nullptr)
    , m_controlGroup(nullptr)
    , m_controlLayout(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_pauseResumeButton(nullptr)
    , m_prevButton(nullptr)
    , m_nextButton(nullptr)
    , m_progressGroup(nullptr)
    , m_progressLayout(nullptr)
    , m_progressLabel(nullptr)
    , m_sessionProgress(nullptr)
    , m_currentExerciseLabel(nullptr)
    , m_gameAreaGroup(nullptr)
    , m_gameAreaLayout(nullptr)
    , m_gameStack(nullptr)
    , m_gamePlaceholder(nullptr)
    , m_sessionTimer(nullptr)
    , m_currentOutlineIndex(-1)
    , m_currentExerciseIndex(-1)
    , m_isSessionActive(false)
    , m_isPaused(false)
    , m_completedExercises(0)
    , m_sessionStartTime(0)
    , m_exerciseStartTime(0)
    , m_outlinesDirectory("./data/outlines")
    , m_currentGame(nullptr)
    , m_serialPort(nullptr)
{
    setupUI();
    createConnections();
    
    // Domyślnie załaduj konspekty z katalogu
    loadOutlines(m_outlinesDirectory);
}

OutlineTab::~OutlineTab()
{
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    if (m_isSessionActive) {
        stopSession();
    }
}

void OutlineTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(6);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);

    auto* workspace = new QSplitter(Qt::Horizontal, this);
    workspace->setChildrenCollapsible(false);
    workspace->setHandleWidth(6);

    auto* visualScroll = new QScrollArea(workspace);
    visualScroll->setWidgetResizable(true);
    visualScroll->setFrameShape(QFrame::NoFrame);
    auto* visualPanel = new QWidget(visualScroll);
    m_leftLayout = new QVBoxLayout(visualPanel);
    m_leftLayout->setContentsMargins(4, 4, 4, 4);
    m_leftLayout->setSpacing(8);
    visualScroll->setWidget(visualPanel);

    auto* settingsScroll = new QScrollArea(workspace);
    settingsScroll->setWidgetResizable(true);
    settingsScroll->setFrameShape(QFrame::NoFrame);
    settingsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* settingsPanel = new QWidget(settingsScroll);
    m_rightLayout = new QVBoxLayout(settingsPanel);
    m_rightLayout->setContentsMargins(4, 4, 4, 4);
    m_rightLayout->setSpacing(8);
    settingsScroll->setWidget(settingsPanel);

    workspace->addWidget(visualScroll);
    workspace->addWidget(settingsScroll);
    workspace->setStretchFactor(0, 72);
    workspace->setStretchFactor(1, 28);
    workspace->setSizes({720, 280});
    m_mainLayout->addWidget(workspace, 1);

    m_patientContextLabel = new QLabel(tr("Pacjent: nie wybrano"), visualPanel);
    m_patientContextLabel->setObjectName(QStringLiteral("rehabilitationPatientContext"));
    m_patientContextLabel->setWordWrap(true);
    m_patientContextLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_leftLayout->addWidget(m_patientContextLabel);

    auto* episodeGroup = new QGroupBox(tr("Epizod rehabilitacji"), visualPanel);
    auto* episodeLayout = new QVBoxLayout(episodeGroup);
    m_episodeSelector = new QComboBox(episodeGroup);
    m_episodeSelector->setObjectName(QStringLiteral("rehabilitationEpisodeSelector"));
    m_episodeDiagnosisEdit = new QLineEdit(episodeGroup);
    m_episodeDiagnosisEdit->setPlaceholderText(tr("Rozpoznanie / problem kliniczny"));
    m_episodePathwayCombo = new QComboBox(episodeGroup);
    m_episodePathwayCombo->addItem(tr("Postępowanie zachowawcze"), QStringLiteral("conservative"));
    m_episodePathwayCombo->addItem(tr("Po meniscektomii"), QStringLiteral("post_meniscectomy"));
    m_episodePathwayCombo->addItem(tr("Po naprawie / szyciu"), QStringLiteral("post_repair"));
    m_episodePathwayCombo->addItem(tr("Inna ścieżka"), QStringLiteral("other"));
    m_createEpisodeButton = new QPushButton(tr("Utwórz epizod"), episodeGroup);
    m_episodeStatusLabel = new QLabel(tr("Wybierz pacjenta, aby zarządzać epizodami."), episodeGroup);
    m_episodeStatusLabel->setWordWrap(true);
    episodeLayout->addWidget(m_episodeSelector);
    episodeLayout->addWidget(m_episodeDiagnosisEdit);
    episodeLayout->addWidget(m_episodePathwayCombo);
    episodeLayout->addWidget(m_createEpisodeButton);
    episodeLayout->addWidget(m_episodeStatusLabel);
    m_leftLayout->addWidget(episodeGroup);

    auto* assessmentGroup = new QGroupBox(tr("Ocena kliniczna pacjenta"), visualPanel);
    auto* assessmentLayout = new QVBoxLayout(assessmentGroup);
    auto addAssessmentField = [assessmentGroup, assessmentLayout](const QString& label,
                                                                  QLineEdit*& field,
                                                                  const QString& placeholder) {
        assessmentLayout->addWidget(new QLabel(label, assessmentGroup));
        field = new QLineEdit(assessmentGroup);
        field->setPlaceholderText(placeholder);
        assessmentLayout->addWidget(field);
    };
    addAssessmentField(tr("Ból — NRS (0–10)"), m_assessmentPainNrsEdit,
                       tr("Pozostaw puste, jeśli nie oceniono"));
    addAssessmentField(tr("Zakres ruchu — ROM (stopnie)"), m_assessmentRomEdit,
                       tr("Wartość w stopniach; 0 jest prawidłową wartością pomiaru"));
    addAssessmentField(tr("Siła mięśniowa — MRC (0–5)"), m_assessmentStrengthEdit,
                       tr("Stopień MRC; pozostaw puste, jeśli nie oceniono"));
    addAssessmentField(tr("Obwód / obrzęk (cm)"), m_assessmentSwellingEdit,
                       tr("Wartość w cm; pozostaw puste, jeśli nie zmierzono"));
    addAssessmentField(tr("Test funkcjonalny / wynik"), m_assessmentFunctionalTestEdit,
                       tr("Nazwa testu i wynik, np. TUG: 12,4 s"));
    m_saveAssessmentButton = new QPushButton(tr("Zapisz ocenę początkową"), assessmentGroup);
    m_assessmentStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod, aby zapisać ocenę."), assessmentGroup);
    m_assessmentStatusLabel->setWordWrap(true);
    m_assessmentHistoryBrowser = new QTextBrowser(assessmentGroup);
    m_assessmentHistoryBrowser->setMaximumHeight(130);
    m_assessmentHistoryBrowser->setOpenExternalLinks(false);
    assessmentLayout->addWidget(m_saveAssessmentButton);
    assessmentLayout->addWidget(m_assessmentStatusLabel);
    assessmentLayout->addWidget(m_assessmentHistoryBrowser);
    m_leftLayout->addWidget(assessmentGroup);

    auto* goalsGroup = new QGroupBox(tr("Cele terapeutyczne"), visualPanel);
    auto* goalsLayout = new QVBoxLayout(goalsGroup);
    auto addGoalField = [goalsGroup, goalsLayout](const QString& label,
                                                  QLineEdit*& field,
                                                  const QString& placeholder) {
        goalsLayout->addWidget(new QLabel(label, goalsGroup));
        field = new QLineEdit(goalsGroup);
        field->setPlaceholderText(placeholder);
        goalsLayout->addWidget(field);
    };
    addGoalField(tr("Cel funkcjonalny"), m_goalTitleEdit, tr("Np. samodzielne wejście po schodach"));
    addGoalField(tr("Wskaźnik / metoda pomiaru"), m_goalMetricEdit, tr("Np. TUG, ROM zgięcia kolana"));
    addGoalField(tr("Wartość początkowa"), m_goalBaselineEdit, tr("Wartość liczbowa"));
    addGoalField(tr("Wartość docelowa"), m_goalTargetEdit, tr("Wartość liczbowa"));
    addGoalField(tr("Jednostka"), m_goalUnitEdit, tr("Np. s, stopnie, N, wynik skali"));
    addGoalField(tr("Termin (RRRR-MM-DD, opcjonalnie)"), m_goalTargetDateEdit, tr("2026-12-01"));
    m_createGoalButton = new QPushButton(tr("Zapisz cel terapeutyczny"), goalsGroup);
    m_goalStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod, aby zarządzać celami."), goalsGroup);
    m_goalStatusLabel->setWordWrap(true);
    m_goalHistoryBrowser = new QTextBrowser(goalsGroup);
    m_goalHistoryBrowser->setMaximumHeight(150);
    m_goalHistoryBrowser->setOpenExternalLinks(false);
    goalsLayout->addWidget(m_createGoalButton);
    goalsLayout->addWidget(m_goalStatusLabel);
    goalsLayout->addWidget(m_goalHistoryBrowser);
    m_leftLayout->addWidget(goalsGroup);

    auto* sessionGroup = new QGroupBox(tr("Sesja rehabilitacyjna — ból i tolerancja"), visualPanel);
    auto* sessionLayout = new QVBoxLayout(sessionGroup);
    auto addSessionPainField = [sessionGroup, sessionLayout](const QString& label,
                                                             QLineEdit*& field,
                                                             const QString& placeholder) {
        sessionLayout->addWidget(new QLabel(label, sessionGroup));
        field = new QLineEdit(sessionGroup);
        field->setPlaceholderText(placeholder);
        sessionLayout->addWidget(field);
    };
    addSessionPainField(tr("Ból przed sesją — NRS (0–10)"), m_sessionPainBeforeEdit,
                        tr("Wartość wymagana do rozpoczęcia"));
    addSessionPainField(tr("Ból po sesji — NRS (0–10)"), m_sessionPainAfterEdit,
                        tr("Wartość wymagana do zakończenia"));
    addSessionPainField(tr("Ból po 24 godzinach — NRS (0–10)"), m_sessionPain24hEdit,
                        tr("Uzupełnij po upływie około 24 godzin"));
    m_startRehabilitationSessionButton = new QPushButton(tr("Rozpocznij sesję"), sessionGroup);
    m_finishRehabilitationSessionButton = new QPushButton(tr("Zakończ sesję"), sessionGroup);
    m_saveRehabilitation24hButton = new QPushButton(tr("Zapisz kontrolę po 24 godzinach"), sessionGroup);
    m_rehabilitationSessionStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod."), sessionGroup);
    m_rehabilitationSessionStatusLabel->setWordWrap(true);
    sessionLayout->addWidget(m_startRehabilitationSessionButton);
    sessionLayout->addWidget(m_finishRehabilitationSessionButton);
    sessionLayout->addWidget(m_saveRehabilitation24hButton);
    sessionLayout->addWidget(m_rehabilitationSessionStatusLabel);
    m_leftLayout->addWidget(sessionGroup);
    connect(m_startRehabilitationSessionButton, &QPushButton::clicked,
            this, &OutlineTab::startRehabilitationSession);
    connect(m_finishRehabilitationSessionButton, &QPushButton::clicked,
            this, &OutlineTab::finishRehabilitationSession);
    connect(m_saveRehabilitation24hButton, &QPushButton::clicked,
            this, &OutlineTab::recordRehabilitation24HourFollowUp);
    m_startRehabilitationSessionButton->setEnabled(false);
    m_finishRehabilitationSessionButton->setEnabled(false);
    m_saveRehabilitation24hButton->setEnabled(false);
    m_sessionPainBeforeEdit->setEnabled(false);
    m_sessionPainAfterEdit->setEnabled(false);
    m_sessionPain24hEdit->setEnabled(false);


    auto* observationGroup = new QGroupBox(tr("Obserwacja wykonania ćwiczenia"), visualPanel);
    auto* observationLayout = new QVBoxLayout(observationGroup);
    auto addObservationField = [observationGroup, observationLayout](const QString& label,
                                                                      QLineEdit*& field,
                                                                      const QString& placeholder) {
        observationLayout->addWidget(new QLabel(label, observationGroup));
        field = new QLineEdit(observationGroup);
        field->setPlaceholderText(placeholder);
        observationLayout->addWidget(field);
    };
    addObservationField(tr("Ćwiczenie"), m_observationExerciseEdit, tr("Nazwa ćwiczenia"));
    addObservationField(tr("Wykonane serie"), m_observationSetsEdit, tr("Liczba całkowita"));
    addObservationField(tr("Wykonane powtórzenia"), m_observationRepetitionsEdit, tr("Łączna liczba powtórzeń"));
    addObservationField(tr("Czas wykonania (sekundy)"), m_observationDurationEdit, tr("Opcjonalnie"));
    addObservationField(tr("Odczuwalny wysiłek — RPE (0–10)"), m_observationRpeEdit, tr("Opcjonalnie"));
    observationLayout->addWidget(new QLabel(tr("Jakość ruchu"), observationGroup));
    m_observationMovementQualityCombo = new QComboBox(observationGroup);
    m_observationMovementQualityCombo->addItem(tr("Nie oceniono"), QStringLiteral("not_assessed"));
    m_observationMovementQualityCombo->addItem(tr("Prawidłowa"), QStringLiteral("acceptable"));
    m_observationMovementQualityCombo->addItem(tr("Nieprawidłowa"), QStringLiteral("unacceptable"));
    observationLayout->addWidget(m_observationMovementQualityCombo);
    addObservationField(tr("Ból przed ćwiczeniem — NRS (0–10)"), m_observationPainBeforeEdit,
                        tr("Wartość wymagana"));
    addObservationField(tr("Ból po ćwiczeniu — NRS (0–10)"), m_observationPainAfterEdit,
                        tr("Wartość wymagana"));
    addObservationField(tr("Ból po 24 godzinach — NRS (0–10)"), m_observationPain24hEdit,
                        tr("Opcjonalnie, po kontroli"));
    m_saveObservationButton = new QPushButton(tr("Zapisz obserwację ćwiczenia"), observationGroup);
    m_observationStatusLabel = new QLabel(tr("Wymagana aktywna sesja rehabilitacyjna."), observationGroup);
    m_observationStatusLabel->setWordWrap(true);
    m_observationHistoryBrowser = new QTextBrowser(observationGroup);
    m_observationHistoryBrowser->setMaximumHeight(160);
    m_observationHistoryBrowser->setOpenExternalLinks(false);
    observationLayout->addWidget(m_saveObservationButton);
    observationLayout->addWidget(m_observationStatusLabel);
    observationLayout->addWidget(m_observationHistoryBrowser);
    m_leftLayout->addWidget(observationGroup);
    connect(m_saveObservationButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationObservation);
    m_saveObservationButton->setEnabled(false);
    for (QLineEdit* field : {m_observationExerciseEdit, m_observationSetsEdit,
                             m_observationRepetitionsEdit, m_observationDurationEdit,
                             m_observationRpeEdit, m_observationPainBeforeEdit,
                             m_observationPainAfterEdit, m_observationPain24hEdit})
        field->setEnabled(false);

    connect(m_createGoalButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationGoal);
    m_createGoalButton->setEnabled(false);
    for (QLineEdit* field : {m_goalTitleEdit, m_goalMetricEdit, m_goalBaselineEdit,
                             m_goalTargetEdit, m_goalUnitEdit, m_goalTargetDateEdit})
        field->setEnabled(false);

    connect(m_saveAssessmentButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationAssessment);
    m_saveAssessmentButton->setEnabled(false);
    for (QLineEdit* field : {m_assessmentPainNrsEdit, m_assessmentRomEdit,
                             m_assessmentStrengthEdit, m_assessmentSwellingEdit,
                             m_assessmentFunctionalTestEdit}) {
        field->setEnabled(false);
    }

    connect(m_episodeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                m_currentEpisodeUuid = index >= 0
                    ? m_episodeSelector->itemData(index).toString() : QString();
                refreshRehabilitationAssessments();
                refreshRehabilitationGoals();
                refreshRehabilitationSession();
                refreshRehabilitationObservations();
            });
    connect(m_createEpisodeButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationEpisode);
    m_episodeSelector->setEnabled(false);
    m_episodeDiagnosisEdit->setEnabled(false);
    m_episodePathwayCombo->setEnabled(false);
    m_createEpisodeButton->setEnabled(false);

    setupOutlineSelector();
    setupExerciseView();
    setupControlPanel();
    setupGameArea();

    auto* graphGroup = new QGroupBox(tr("Biofeedback — siła w czasie rzeczywistym"), visualPanel);
    auto* graphLayout = new QVBoxLayout(graphGroup);
    m_forceGraph = new gui::GraphWidget(graphGroup);
    m_forceGraph->setTitle(tr("Siła z czujnika"));
    m_forceGraph->setYLabel(tr("Siła"));
    m_forceGraph->setUnit(tr("N"));
    m_forceGraph->setTimeRange(30);
    m_forceGraph->setMinimumHeight(220);
    graphLayout->addWidget(m_forceGraph);
    m_leftLayout->insertWidget(1, graphGroup);

    m_clinicalProtocolBrowser = new QTextBrowser(visualPanel);
    m_clinicalProtocolBrowser->setOpenExternalLinks(true);
    m_clinicalProtocolBrowser->setMinimumHeight(150);
    m_clinicalProtocolBrowser->setPlaceholderText(tr("Wybierz konspekt, aby zobaczyć protokół kliniczny."));
    m_leftLayout->addWidget(m_clinicalProtocolBrowser);
}

void OutlineTab::setupOutlineSelector()
{
    // Sekcja wyboru konspektu
    m_outlineGroup = new QGroupBox(tr("Wybór Konspektu"), this);
    m_outlineGroup->setFont(QFont("Arial", 8, QFont::Bold));
    m_outlineSelectorLayout = new QHBoxLayout();
    
    m_outlineLabel = new QLabel(tr("Konspekt:"), this);
    m_outlineLabel->setFont(QFont("Arial", 7, QFont::Bold));
    m_outlineCombo = new QComboBox(this);
    m_outlineCombo->setMinimumWidth(300);
    m_outlineCombo->setMinimumHeight(35);
    m_outlineCombo->setFont(QFont("Arial", 7));
    m_outlineCombo->addItem(tr("-- Wybierz konspekt --"), QString());
    
    m_loadOutlineButton = new QPushButton(tr("Wczytaj..."), this);
    m_loadOutlineButton->setToolTip(tr("Wczytaj plik JSON konspektu"));
    m_loadOutlineButton->setMinimumHeight(35);
    m_loadOutlineButton->setFont(QFont("Arial", 7));
    
    m_outlineSelectorLayout->addWidget(m_outlineLabel);
    m_outlineSelectorLayout->addWidget(m_outlineCombo, 1);
    m_outlineSelectorLayout->addWidget(m_loadOutlineButton);
    
    m_outlineGroup->setLayout(m_outlineSelectorLayout);
    m_rightLayout->addWidget(m_outlineGroup);
    
    // Panel informacji o konspekcie
    m_outlineInfoGroup = new QGroupBox(tr("Informacje o Konspekcie"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout();
    
    m_outlineNameLabel = new QLabel(tr("Nazwa: -"), this);
    m_outlineNameLabel->setFont(QFont("Sans Serif", 8, QFont::Bold));
    m_outlineNameLabel->setWordWrap(true);
    
    m_outlineDescriptionLabel = new QLabel(tr("Opis: -"), this);
    m_outlineDescriptionLabel->setWordWrap(true);
    
    QHBoxLayout* metaLayout = new QHBoxLayout();
    m_outlineAuthorLabel = new QLabel(tr("Autor: -"), this);
    m_outlineDurationLabel = new QLabel(tr("Czas: -"), this);
    m_outlineExerciseCountLabel = new QLabel(tr("Ćwiczenia: 0"), this);
    
    metaLayout->addWidget(m_outlineAuthorLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_outlineDurationLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_outlineExerciseCountLabel);
    
    infoLayout->addWidget(m_outlineNameLabel);
    infoLayout->addWidget(m_outlineDescriptionLabel);
    infoLayout->addLayout(metaLayout);
    
    m_outlineInfoGroup->setLayout(infoLayout);
    m_rightLayout->addWidget(m_outlineInfoGroup);
}

void OutlineTab::setupExerciseView()
{
    m_exerciseGroup = new QGroupBox(tr("Ćwiczenie"), this);
    m_exerciseLayout = new QVBoxLayout();
    
    // Informacje o ćwiczeniu
    m_exerciseInfoWidget = new QWidget(this);
    m_exerciseInfoLayout = new QVBoxLayout(m_exerciseInfoWidget);
    m_exerciseInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    m_exerciseNameLabel = new QLabel(tr("Nazwa ćwiczenia"), this);
    m_exerciseNameLabel->setFont(QFont("Sans Serif", 8, QFont::Bold));
    m_exerciseNameLabel->setWordWrap(true);
    
    m_exerciseDescriptionLabel = new QLabel(tr("Opis ćwiczenia pojawi się tutaj."), this);
    m_exerciseDescriptionLabel->setWordWrap(true);
    m_exerciseDescriptionLabel->setMinimumHeight(60);
    
    m_exerciseInfoLayout->addWidget(m_exerciseNameLabel);
    m_exerciseInfoLayout->addWidget(m_exerciseDescriptionLabel);
    
    m_exerciseLayout->addWidget(m_exerciseInfoWidget);
    
    // Multimedia (obraz i wideo)
    m_mediaLayout = new QHBoxLayout();
    
    m_exerciseImageLabel = new QLabel(this);
    m_exerciseImageLabel->setMinimumSize(320, 240);
    m_exerciseImageLabel->setMaximumSize(320, 240);
    m_exerciseImageLabel->setAlignment(Qt::AlignCenter);
    m_exerciseImageLabel->setText(tr("Brak obrazu"));
    m_exerciseImageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    
    m_videoWidget = new QWidget(this);
    m_videoWidget->setMinimumSize(320, 240);
    m_videoWidget->setMaximumSize(320, 240);
    m_videoWidget->setStyleSheet("QWidget { background-color: #000; }");
    
    QVBoxLayout* videoLayout = new QVBoxLayout(m_videoWidget);
    m_playVideoButton = new QPushButton(tr("▶ Odtwórz film"), this);
    m_playVideoButton->setMinimumHeight(40);
    videoLayout->addWidget(m_playVideoButton);
    
    m_mediaLayout->addWidget(m_exerciseImageLabel);
    m_mediaLayout->addWidget(m_videoWidget);
    
    m_exerciseLayout->addLayout(m_mediaLayout);
    
    // Wybór gry
    m_gameSelectionGroup = new QGroupBox(tr("Gra Biofeedback"), this);
    m_gameSelectionLayout = new QHBoxLayout();
    
    m_gameLabel = new QLabel(tr("Gra:"), this);
    m_gameCombo = new QComboBox(this);
    m_gameCombo->setMinimumWidth(200);
    populateGameCombo();
    
    m_configureGameButton = new QPushButton(tr("Konfiguruj..."), this);
    m_configureGameButton->setEnabled(false);
    
    m_gameSelectionLayout->addWidget(m_gameLabel);
    m_gameSelectionLayout->addWidget(m_gameCombo, 1);
    m_gameSelectionLayout->addWidget(m_configureGameButton);
    
    m_gameSelectionGroup->setLayout(m_gameSelectionLayout);
    // Gra jest ustawieniem protokołu i dlatego pozostaje w prawym panelu.
    m_rightLayout->addWidget(m_gameSelectionGroup);

    m_exerciseGroup->setLayout(m_exerciseLayout);
    m_leftLayout->addWidget(m_exerciseGroup);
}

void OutlineTab::setupControlPanel()
{
    m_controlGroup = new QGroupBox(tr("Sterowanie Sesją"), this);
    m_controlLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton(tr("▶ Start"), this);
    m_startButton->setToolTip(tr("Rozpocznij sesję z wybranym konspektem"));
    m_startButton->setEnabled(false);
    
    m_stopButton = new QPushButton(tr("■ Stop"), this);
    m_stopButton->setToolTip(tr("Zatrzymaj sesję"));
    m_stopButton->setEnabled(false);
    
    m_pauseResumeButton = new QPushButton(tr("❚❚ Pauza"), this);
    m_pauseResumeButton->setToolTip(tr("Pauzuj/Wznów sesję"));
    m_pauseResumeButton->setEnabled(false);
    
    m_prevButton = new QPushButton(tr("◀ Wstecz"), this);
    m_prevButton->setToolTip(tr("Poprzednie ćwiczenie"));
    m_prevButton->setEnabled(false);
    
    m_nextButton = new QPushButton(tr("Dalej ▶"), this);
    m_nextButton->setToolTip(tr("Następne ćwiczenie"));
    m_nextButton->setEnabled(false);
    
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_stopButton);
    m_controlLayout->addWidget(m_pauseResumeButton);
    m_controlLayout->addSpacing(20);
    m_controlLayout->addWidget(m_prevButton);
    m_controlLayout->addWidget(m_nextButton);
    m_controlLayout->addStretch();
    
    m_controlGroup->setLayout(m_controlLayout);
    m_rightLayout->addWidget(m_controlGroup);
    
    // Pasek postępu
    m_progressGroup = new QGroupBox(tr("Postęp"), this);
    m_progressLayout = new QHBoxLayout();
    
    m_progressLabel = new QLabel(tr("Postęp:"), this);
    m_sessionProgress = new QProgressBar(this);
    m_sessionProgress->setMinimum(0);
    m_sessionProgress->setMaximum(100);
    m_sessionProgress->setValue(0);
    m_sessionProgress->setFormat("%p%");
    
    m_currentExerciseLabel = new QLabel(tr("Ćwiczenie: - / -"), this);
    
    m_progressLayout->addWidget(m_progressLabel);
    m_progressLayout->addWidget(m_sessionProgress, 1);
    m_progressLayout->addWidget(m_currentExerciseLabel);
    
    m_progressGroup->setLayout(m_progressLayout);
    m_leftLayout->addWidget(m_progressGroup);
}

void OutlineTab::setupGameArea()
{
    m_gameAreaGroup = new QGroupBox(tr("Obszar Gry"), this);
    m_gameAreaLayout = new QVBoxLayout();
    
    m_gameStack = new QStackedWidget(this);
    
    // Placeholder gdy gra nie jest aktywna
    m_gamePlaceholder = new QWidget(this);
    QVBoxLayout* placeholderLayout = new QVBoxLayout(m_gamePlaceholder);
    QLabel* placeholderLabel = new QLabel(tr("Gra zostanie uruchomiona po rozpoczęciu sesji"), this);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setMinimumHeight(200);
    placeholderLayout->addWidget(placeholderLabel);
    
    m_gameStack->addWidget(m_gamePlaceholder);
    
    m_gameAreaLayout->addWidget(m_gameStack);
    m_gameAreaGroup->setLayout(m_gameAreaLayout);
    m_leftLayout->addWidget(m_gameAreaGroup, 1);
}

void OutlineTab::createConnections()
{
    connect(m_outlineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutlineTab::onOutlineSelected);
    
    connect(m_loadOutlineButton, &QPushButton::clicked,
            this, [this]() {
                QString filename = QFileDialog::getOpenFileName(
                    this, tr("Wczytaj Konspekt"), m_outlinesDirectory,
                    tr("Pliki JSON (*.json)"));
                if (!filename.isEmpty()) {
                    loadOutlineFromFile(filename);
                }
            });
    
    connect(m_startButton, &QPushButton::clicked,
            this, &OutlineTab::onStartSessionClicked);
    
    connect(m_stopButton, &QPushButton::clicked,
            this, &OutlineTab::onStopSessionClicked);
    
    connect(m_pauseResumeButton, &QPushButton::clicked,
            this, &OutlineTab::onPauseResumeClicked);
    
    connect(m_prevButton, &QPushButton::clicked,
            this, &OutlineTab::onPreviousExerciseClicked);
    
    connect(m_nextButton, &QPushButton::clicked,
            this, &OutlineTab::onNextExerciseClicked);
    
    connect(m_gameCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutlineTab::onGameSelectionChanged);
    
    connect(m_playVideoButton, &QPushButton::clicked,
            this, &OutlineTab::onVideoPlayerButtonClicked);
    
    // Timer sesji
    m_sessionTimer = new QTimer(this);
    connect(m_sessionTimer, &QTimer::timeout,
            this, &OutlineTab::onSessionTimerTick);
}

void OutlineTab::loadOutlines(const QString& directory)
{
    m_outlinesDirectory = directory;
    m_outlines.clear();
    
    loadOutlineFiles(directory);
    populateOutlineCombo();
}

void OutlineTab::loadOutlineFiles(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        return;
    }
    
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    for (const QString& file : files) {
        QString fullPath = dir.absoluteFilePath(file);
        OutlineData outline;
        if (parseOutlineFile(fullPath, outline)) {
            m_outlines.append(outline);
        }
    }
}

bool OutlineTab::parseOutlineFile(const QString& filename, OutlineData& outline)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Parsowanie podstawowych danych konspektu
    outline.id = root["id"].toString();
    outline.name = root["name"].toString();
    outline.description = root["description"].toString();
    outline.author = root["author"].toString();
    outline.category = root["category"].toString();
    outline.estimatedDurationMinutes = root["estimatedDurationMinutes"].toInt(0);
    outline.clinicalProtocolHtml = root["clinicalProtocolHtml"].toString();
    
    // Parsowanie ćwiczeń
    QJsonArray exercisesArray = root["exercises"].toArray();
    
    for (const QJsonValue& exerciseValue : exercisesArray) {
        QJsonObject exerciseObj = exerciseValue.toObject();
        
        ExerciseData exercise;
        exercise.id = exerciseObj["id"].toString();
        exercise.name = exerciseObj["name"].toString();
        exercise.description = exerciseObj["description"].toString();
        exercise.imagePath = exerciseObj["imagePath"].toString();
        exercise.videoPath = exerciseObj["videoPath"].toString();
        exercise.gameId = exerciseObj["gameId"].toString();
        if (exercise.gameId == QStringLiteral("scored_earth"))
            exercise.gameId = QStringLiteral("scorched_earth");
        exercise.durationSeconds = exerciseObj["durationSeconds"].toInt(0);
        exercise.targetRepetitions = exerciseObj["targetRepetitions"].toInt(0);
        exercise.difficulty = exerciseObj["difficulty"].toString("medium");
        
        outline.exercises.append(exercise);
    }
    
    return !outline.id.isEmpty() && !outline.exercises.isEmpty();
}

bool OutlineTab::loadOutlineFromFile(const QString& filename)
{
    OutlineData outline;
    if (!parseOutlineFile(filename, outline)) {
        QMessageBox::warning(this, tr("Błąd ładowania"),
                            tr("Nie udało się wczytać pliku konspektu."));
        return false;
    }
    
    // Sprawdź czy konspekt już istnieje, jeśli tak to zaktualizuj
    int existingIndex = -1;
    for (int i = 0; i < m_outlines.size(); ++i) {
        if (m_outlines[i].id == outline.id) {
            existingIndex = i;
            break;
        }
    }
    
    if (existingIndex >= 0) {
        m_outlines[existingIndex] = outline;
    } else {
        m_outlines.append(outline);
    }
    
    populateOutlineCombo();
    
    // Wybierz wczytany konspekt
    for (int i = 0; i < m_outlineCombo->count(); ++i) {
        if (m_outlineCombo->itemData(i).toString() == outline.id) {
            m_outlineCombo->setCurrentIndex(i);
            break;
        }
    }
    
    return true;
}

void OutlineTab::populateOutlineCombo()
{
    m_outlineCombo->blockSignals(true);
    m_outlineCombo->clear();
    m_outlineCombo->addItem(tr("-- Wybierz konspekt --"), QString());
    
    for (int i = 0; i < m_outlines.size(); ++i) {
        m_outlineCombo->addItem(m_outlines[i].name, m_outlines[i].id);
    }
    
    m_outlineCombo->blockSignals(false);
}

void OutlineTab::populateGameCombo()
{
    m_gameCombo->blockSignals(true);
    m_gameCombo->clear();
    m_gameCombo->addItem(tr("-- Wybierz grę --"), QString());
    m_gameCombo->addItem(tr("Sin Game"), "sin_game");
    m_gameCombo->addItem(tr("Tan Game"), "tan_game");
    m_gameCombo->addItem(tr("Flappy Bird"), "flappy_bird");
    m_gameCombo->addItem(tr("Pong"), "pong");
    m_gameCombo->addItem(tr("Arkanoid"), "arkanoid");
    m_gameCombo->addItem(tr("Mario"), "mario");
    m_gameCombo->addItem(tr("Galaga"), "galaga");
    m_gameCombo->addItem(tr("Archer"), "archer");
    m_gameCombo->addItem(tr("Scorched Earth"), "scorched_earth");
    m_gameCombo->addItem(tr("Car Race"), "car_race");

    // Nie pokazuj jako dostępnych gier, których implementacje nie są jeszcze
    // dołączone do aplikacji. Identyfikatory pozostają rozpoznawalne przy
    // odczycie starszych konspektów, ale użytkownik nie może ich wybrać.
    const int unavailableIndexes[] = {7, 8, 9, 10};
    if (auto* model = qobject_cast<QStandardItemModel*>(m_gameCombo->model())) {
        for (const int index : unavailableIndexes) {
            if (QStandardItem* item = model->item(index)) {
                item->setEnabled(false);
                item->setText(item->text() + tr(" (w przygotowaniu)"));
            }
        }
    }
    m_gameCombo->blockSignals(false);
}

void OutlineTab::onOutlineSelected(int index)
{
    if (index <= 0 || index > m_outlines.size()) {
        m_currentOutline = OutlineData();
        m_currentOutlineIndex = -1;
        
        // Wyczyść wyświetlanie
        m_outlineNameLabel->setText(tr("Nazwa: -"));
        m_outlineDescriptionLabel->setText(tr("Opis: -"));
        if (m_clinicalProtocolBrowser)
            m_clinicalProtocolBrowser->setHtml(tr("<p>Wybierz konspekt, aby wyświetlić protokół.</p>"));
        m_outlineAuthorLabel->setText(tr("Autor: -"));
        m_outlineDurationLabel->setText(tr("Czas: -"));
        m_outlineExerciseCountLabel->setText(tr("Ćwiczenia: 0"));
        
        m_startButton->setEnabled(false);
        return;
    }
    
    m_currentOutlineIndex = index - 1;
    m_currentOutline = m_outlines[m_currentOutlineIndex];
    
    // Wyświetl informacje o konspekcie
    m_outlineNameLabel->setText(tr("Nazwa: %1").arg(m_currentOutline.name));
    m_outlineDescriptionLabel->setText(m_currentOutline.description);
    if (m_clinicalProtocolBrowser) {
        if (!m_currentOutline.clinicalProtocolHtml.isEmpty())
            m_clinicalProtocolBrowser->setHtml(m_currentOutline.clinicalProtocolHtml);
        else
            m_clinicalProtocolBrowser->setHtml(tr("<p>Ten konspekt nie zawiera rozszerzonego protokołu klinicznego.</p>"));
    }
    m_outlineAuthorLabel->setText(tr("Autor: %1").arg(
        m_currentOutline.author.isEmpty() ? tr("Nieznany") : m_currentOutline.author));
    m_outlineDurationLabel->setText(tr("Czas: %1 min").arg(
        m_currentOutline.estimatedDurationMinutes));
    m_outlineExerciseCountLabel->setText(tr("Ćwiczenia: %1").arg(
        m_currentOutline.exercises.size()));
    
    m_startButton->setEnabled(!m_currentOutline.exercises.isEmpty());
    
    emit outlineSelected(m_currentOutline.id);
    
    // Reset do pierwszego ćwiczenia
    setCurrentExerciseIndex(0);
}

void OutlineTab::setCurrentExerciseIndex(int index)
{
    if (m_currentOutline.exercises.isEmpty()) {
        m_currentExerciseIndex = -1;
        return;
    }
    
    if (index < 0) {
        index = 0;
    }
    if (index >= m_currentOutline.exercises.size()) {
        index = m_currentOutline.exercises.size() - 1;
    }
    
    m_currentExerciseIndex = index;
    updateExerciseDisplay();
    updateProgressIndicators();
}

void OutlineTab::updateExerciseDisplay()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        m_exerciseNameLabel->setText(tr("Brak ćwiczenia"));
        m_exerciseDescriptionLabel->setText(tr("Wybierz konspekt aby zobaczyć ćwiczenia."));
        m_exerciseImageLabel->setText(tr("Brak obrazu"));
        m_exerciseImageLabel->setPixmap(QPixmap());
        return;
    }
    
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    
    m_exerciseNameLabel->setText(exercise.name);
    m_exerciseDescriptionLabel->setText(exercise.description);
    
    // Załaduj obraz jeśli istnieje
    if (!exercise.imagePath.isEmpty()) {
        QPixmap pixmap(exercise.imagePath);
        if (!pixmap.isNull()) {
            m_exerciseImageLabel->setPixmap(pixmap.scaled(
                m_exerciseImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_exerciseImageLabel->setText(tr("Nie można załadować obrazu"));
        }
    } else {
        m_exerciseImageLabel->setText(tr("Brak obrazu"));
        m_exerciseImageLabel->setPixmap(QPixmap());
    }
    
    // Ustaw grę w combo jeśli przypisana
    if (!exercise.gameId.isEmpty()) {
        int gameIndex = m_gameCombo->findData(exercise.gameId);
        if (gameIndex >= 0) {
            m_gameCombo->setCurrentIndex(gameIndex);
        }
    }
    
    emit exerciseChanged(m_currentExerciseIndex, exercise);
}

void OutlineTab::updateProgressIndicators()
{
    if (m_currentOutline.exercises.isEmpty()) {
        m_sessionProgress->setValue(0);
        m_currentExerciseLabel->setText(tr("Ćwiczenie: - / -"));
        return;
    }
    
    int total = m_currentOutline.exercises.size();
    int current = m_currentExerciseIndex + 1;
    
    int progressPercent = (m_completedExercises * 100) / total;
    m_sessionProgress->setValue(progressPercent);
    
    m_currentExerciseLabel->setText(tr("Ćwiczenie: %1 / %2").arg(current).arg(total));
    
    // Aktualizuj dostępność przycisków nawigacji
    m_prevButton->setEnabled(m_currentExerciseIndex > 0 && m_isSessionActive && !m_isPaused);
    m_nextButton->setEnabled(m_currentExerciseIndex < total - 1 && m_isSessionActive && !m_isPaused);
}

void OutlineTab::onStartSessionClicked()
{
    if (!validateCurrentSetup()) {
        return;
    }
    
    m_isSessionActive = true;
    m_isPaused = false;
    m_completedExercises = 0;
    m_exerciseScores.clear();
    m_exerciseScores.resize(m_currentOutline.exercises.size());
    std::fill(m_exerciseScores.begin(), m_exerciseScores.end(), 0);
    m_sessionStartTime = QDateTime::currentMSecsSinceEpoch();
    
    // Zablokuj wybór konspektu podczas sesji
    m_outlineCombo->setEnabled(false);
    m_loadOutlineButton->setEnabled(false);
    
    // Włącz kontrolki sesji
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_pauseResumeButton->setEnabled(true);
    m_pauseResumeButton->setText(tr("❚❚ Pauza"));
    
    // Rozpocznij pierwsze ćwiczenie
    setCurrentExerciseIndex(0);
    
    // Uruchom timer sesji
    m_sessionTimer->start(1000); // aktualizacja co sekundę
    
    emit sessionStarted(m_currentOutline.id);
    
    // Automatycznie uruchom grę dla pierwszego ćwiczenia
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    if (!exercise.gameId.isEmpty()) {
        emit requestGameStart(exercise.gameId, exercise);
    }
}

void OutlineTab::onStopSessionClicked()
{
    endSession();
}

void OutlineTab::onPauseResumeClicked()
{
    if (!m_isSessionActive) {
        return;
    }
    
    m_isPaused = !m_isPaused;
    
    if (m_isPaused) {
        m_pauseResumeButton->setText(tr("▶ Wznów"));
        m_sessionTimer->stop();
        
        // Pauzuj grę jeśli istnieje
        if (m_currentGame) {
            m_currentGame->pause();
        }
    } else {
        m_pauseResumeButton->setText(tr("❚❚ Pauza"));
        m_sessionTimer->start(1000);
        
        // Wznów grę jeśli istnieje
        if (m_currentGame) {
            m_currentGame->resume();
        }
    }
}

void OutlineTab::onPreviousExerciseClicked()
{
    if (m_currentExerciseIndex > 0) {
        setCurrentExerciseIndex(m_currentExerciseIndex - 1);
    }
}

void OutlineTab::onNextExerciseClicked()
{
    if (m_currentExerciseIndex < m_currentOutline.exercises.size() - 1) {
        finishCurrentExercise();
        moveToNextExercise();
    }
}

void OutlineTab::onGameSelectionChanged(int index)
{
    QString gameId = m_gameCombo->itemData(index).toString();
    m_configureGameButton->setEnabled(index > 0);
    
    // Jeśli trwa sesja, zapisz wybór gry dla bieżącego ćwiczenia
    if (m_isSessionActive && m_currentExerciseIndex >= 0 && 
        m_currentExerciseIndex < m_currentOutline.exercises.size()) {
        m_currentOutline.exercises[m_currentExerciseIndex].gameId = gameId;
    }
}

void OutlineTab::onVideoPlayerButtonClicked()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        return;
    }
    
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    
    if (exercise.videoPath.isEmpty()) {
        QMessageBox::information(this, tr("Brak wideo"),
                                tr("Nie ma filmu instruktażowego dla tego ćwiczenia."));
        return;
    }
    
    // Spróbuj otworzyć plik wideo
    QUrl url = QUrl::fromLocalFile(exercise.videoPath);
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, tr("Błąd"),
                            tr("Nie można otworzyć pliku wideo."));
    }
}

void OutlineTab::onSessionTimerTick()
{
    if (!m_isSessionActive || m_isPaused) {
        return;
    }
    
    // Aktualizuj czas sesji (można dodać wyświetlanie czasu)
    // qint64 elapsed = (QDateTime::currentMSecsSinceEpoch() - m_sessionStartTime) / 1000;
    // Można dodać label do wyświetlania czasu
}

void OutlineTab::startSession()
{
    onStartSessionClicked();
}

void OutlineTab::stopSession()
{
    onStopSessionClicked();
}

void OutlineTab::pauseSession()
{
    if (m_isSessionActive && !m_isPaused) {
        onPauseResumeClicked();
    }
}

void OutlineTab::resumeSession()
{
    if (m_isSessionActive && m_isPaused) {
        onPauseResumeClicked();
    }
}

void OutlineTab::nextExercise()
{
    onNextExerciseClicked();
}

void OutlineTab::previousExercise()
{
    onPreviousExerciseClicked();
}

void OutlineTab::setGameForCurrentExercise(const QString& gameId)
{
    int index = m_gameCombo->findData(gameId);
    if (index >= 0) {
        m_gameCombo->setCurrentIndex(index);
    }
}

bool OutlineTab::validateCurrentSetup()
{
    if (m_currentOutline.exercises.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"),
                            tr("Wybrany konspekt nie zawiera ćwiczeń."));
        return false;
    }
    
    // Sprawdź czy wszystkie ćwiczenia mają przypisaną grę (opcjonalne)
    for (int i = 0; i < m_currentOutline.exercises.size(); ++i) {
        if (m_currentOutline.exercises[i].gameId.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Brak gry"),
                tr("Ćwiczenie '%1' nie ma przypisanej gry. Czy na pewno kontynuować?")
                  .arg(m_currentOutline.exercises[i].name),
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::No) {
                return false;
            }
        }
    }
    
    return true;
}

void OutlineTab::finishCurrentExercise()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        return;
    }
    
    m_completedExercises++;
    
    emit exerciseCompleted(m_currentExerciseIndex, m_exerciseScores[m_currentExerciseIndex]);
}

void OutlineTab::moveToNextExercise()
{
    int nextIndex = m_currentExerciseIndex + 1;
    
    if (nextIndex >= m_currentOutline.exercises.size()) {
        // Koniec sesji
        endSession();
        return;
    }
    
    setCurrentExerciseIndex(nextIndex);
    
    // Automatycznie uruchom grę dla następnego ćwiczenia
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    if (!exercise.gameId.isEmpty()) {
        emit requestGameStart(exercise.gameId, exercise);
    }
}

void OutlineTab::endSession()
{
    m_isSessionActive = false;
    m_isPaused = false;
    
    m_sessionTimer->stop();
    
    // Przywróć dostępność wyboru konspektu
    m_outlineCombo->setEnabled(true);
    m_loadOutlineButton->setEnabled(true);
    
    // Zaktualizuj stan przycisków
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_pauseResumeButton->setEnabled(false);
    m_prevButton->setEnabled(false);
    m_nextButton->setEnabled(false);
    
    emit sessionEnded(m_completedExercises);
    
    // Wyczyść obszar gry
    m_gameStack->setCurrentWidget(m_gamePlaceholder);
    m_currentGame = nullptr;
}

void OutlineTab::onGameFinished(int score)
{
    // Zapisz wynik
    if (m_currentExerciseIndex >= 0 && 
        m_currentExerciseIndex < m_exerciseScores.size()) {
        m_exerciseScores[m_currentExerciseIndex] = score;
    }
    
    emit gameFinished(score);
    
    // Automatycznie przejdź do następnego ćwiczenia
    if (m_isSessionActive && !m_isPaused) {
        finishCurrentExercise();
        moveToNextExercise();
    }
}

bool OutlineTab::isSessionActive() const
{
    return m_isSessionActive;
}

int OutlineTab::outlineCount() const
{
    return m_outlines.size();
}

QVector<OutlineData> OutlineTab::getOutlines() const
{
    return m_outlines;
}

// ==================== Serial Communication Methods ====================

void OutlineTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    // MainWindow owns serial acquisition. OutlineTab consumes only the shared
    // calibrated MeasurementEngine stream and never subscribes to raw serial data.
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    m_serialPort = std::move(serial);
}

void OutlineTab::setCurrentPatientContext(const QString& patientUuid, const QString& displayName)
{
    m_currentPatientUuid = patientUuid.trimmed();
    if (m_currentPatientUuid.isEmpty()) {
        if (m_patientContextLabel)
            m_patientContextLabel->setText(tr("Pacjent: nie wybrano"));
        m_currentEpisodeUuid.clear();
        refreshRehabilitationEpisodes();
        refreshRehabilitationAssessments();
        refreshRehabilitationGoals();
        refreshRehabilitationSession();
        return;
    }
    const QString safeName = displayName.trimmed().isEmpty()
        ? tr("Pacjent bez nazwy") : displayName.trimmed();
    if (m_patientContextLabel)
        m_patientContextLabel->setText(tr("Pacjent: %1").arg(safeName));
    refreshRehabilitationEpisodes();
}

void OutlineTab::setResourceRepository(
    std::shared_ptr<biofeedback::data::JsonResourceRepository> repository)
{
    m_resourceRepository = std::move(repository);
    refreshRehabilitationEpisodes();
}

void OutlineTab::refreshRehabilitationEpisodes()
{
    if (!m_episodeSelector) return;
    const QSignalBlocker blocker(m_episodeSelector);
    m_episodeSelector->clear();
    m_currentEpisodeUuid.clear();
    const bool canUseRepository = m_resourceRepository && !m_currentPatientUuid.isEmpty();
    m_episodeSelector->setEnabled(canUseRepository);
    m_episodeDiagnosisEdit->setEnabled(canUseRepository);
    m_episodePathwayCombo->setEnabled(canUseRepository);
    m_createEpisodeButton->setEnabled(canUseRepository);
    if (!canUseRepository) {
        if (m_episodeStatusLabel)
            m_episodeStatusLabel->setText(tr("Wybierz pacjenta, aby zarządzać epizodami."));
        return;
    }
    QString error;
    const QJsonArray episodes = m_resourceRepository->list(
        QStringLiteral("rehabilitation-episodes"), &error);
    if (!error.isEmpty()) {
        m_episodeStatusLabel->setText(tr("Nie udało się odczytać epizodów rehabilitacji."));
        m_episodeSelector->setEnabled(false);
        m_createEpisodeButton->setEnabled(false);
        return;
    }
    for (const QJsonValue& value : episodes) {
        if (!value.isObject()) continue;
        const QJsonObject episode = value.toObject();
        if (episode.value(QStringLiteral("patient_id")).toString().compare(
                m_currentPatientUuid, Qt::CaseInsensitive) != 0) continue;
        const QString diagnosis = episode.value(QStringLiteral("diagnosis_code")).toString();
        const QString pathway = episode.value(QStringLiteral("care_pathway")).toString();
        const QString label = diagnosis.isEmpty() ? pathway : diagnosis + QStringLiteral(" — ") + pathway;
        m_episodeSelector->addItem(label, episode.value(QStringLiteral("id")).toString());
    }
    if (m_episodeSelector->count() > 0) {
        m_episodeSelector->setCurrentIndex(0);
        m_currentEpisodeUuid = m_episodeSelector->currentData().toString();
        m_episodeStatusLabel->setText(tr("Wybrano istniejący epizod. Możesz utworzyć kolejny."));
    } else {
        m_episodeStatusLabel->setText(tr("Brak epizodów. Wprowadź rozpoznanie i utwórz epizod."));
    }
    refreshRehabilitationAssessments();
    refreshRehabilitationGoals();
    refreshRehabilitationSession();
    refreshRehabilitationObservations();
}

void OutlineTab::createRehabilitationEpisode()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty()) {
        QMessageBox::warning(this, tr("Epizod rehabilitacji"),
                             tr("Najpierw wybierz pacjenta."));
        return;
    }
    const QString diagnosis = m_episodeDiagnosisEdit->text().trimmed();
    if (diagnosis.isEmpty()) {
        QMessageBox::warning(this, tr("Epizod rehabilitacji"),
                             tr("Wprowadź rozpoznanie lub problem kliniczny."));
        m_episodeDiagnosisEdit->setFocus();
        return;
    }
    const QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("diagnosis_code"), diagnosis},
        {QStringLiteral("care_pathway"), m_episodePathwayCombo->currentData().toString()},
        {QStringLiteral("status"), QStringLiteral("active")}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-episodes"),
                                      payload, created, &error)) {
        QMessageBox::critical(this, tr("Epizod rehabilitacji"),
                              tr("Nie udało się zapisać epizodu. Kod błędu: %1").arg(error));
        return;
    }
    m_episodeDiagnosisEdit->clear();
    refreshRehabilitationEpisodes();
    const QString createdId = created.value(QStringLiteral("id")).toString();
    const int index = m_episodeSelector->findData(createdId);
    if (index >= 0) {
        m_episodeSelector->setCurrentIndex(index);
        m_currentEpisodeUuid = createdId;
    }
    m_episodeStatusLabel->setText(tr("Epizod zapisany w repozytorium."));
}

void OutlineTab::refreshRehabilitationAssessments()
{
    const bool canSave = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                         !m_currentEpisodeUuid.isEmpty();
    if (m_saveAssessmentButton) m_saveAssessmentButton->setEnabled(canSave);
    for (QLineEdit* field : {m_assessmentPainNrsEdit, m_assessmentRomEdit,
                             m_assessmentStrengthEdit, m_assessmentSwellingEdit,
                             m_assessmentFunctionalTestEdit}) {
        if (field) field->setEnabled(canSave);
    }
    if (!m_assessmentHistoryBrowser) return;
    m_assessmentHistoryBrowser->clear();
    if (!canSave) {
        if (m_assessmentStatusLabel)
            m_assessmentStatusLabel->setText(tr("Wybierz pacjenta i epizod, aby zapisać ocenę."));
        return;
    }

    QString error;
    const QJsonArray assessments = m_resourceRepository->list(
        QStringLiteral("rehabilitation-assessments"), &error);
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie udało się odczytać historii ocen."));
        m_saveAssessmentButton->setEnabled(false);
        return;
    }
    QStringList entries;
    for (const QJsonValue& value : assessments) {
        if (!value.isObject()) continue;
        const QJsonObject item = value.toObject();
        if (item.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            item.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        QStringList fields;
        if (item.contains(QStringLiteral("pain_nrs")))
            fields << tr("NRS: %1").arg(item.value(QStringLiteral("pain_nrs")).toInt());
        if (item.contains(QStringLiteral("rom_degrees")))
            fields << tr("ROM: %1°").arg(item.value(QStringLiteral("rom_degrees")).toDouble());
        if (item.contains(QStringLiteral("strength_mrc")))
            fields << tr("MRC: %1/5").arg(item.value(QStringLiteral("strength_mrc")).toDouble());
        if (item.contains(QStringLiteral("swelling_cm")))
            fields << tr("Obwód: %1 cm").arg(item.value(QStringLiteral("swelling_cm")).toDouble());
        if (item.contains(QStringLiteral("functional_test")))
            fields << item.value(QStringLiteral("functional_test")).toString();
        entries << QStringLiteral("• %1 — %2")
                       .arg(item.value(QStringLiteral("assessed_at")).toString(),
                            fields.join(QStringLiteral("; ")));
    }
    m_assessmentHistoryBrowser->setPlainText(entries.isEmpty()
        ? tr("Brak zapisanych ocen dla tego epizodu.") : entries.join(QStringLiteral("\n")));
    m_assessmentStatusLabel->setText(tr("Historia ocen: %1").arg(entries.size()));
}

void OutlineTab::createRehabilitationAssessment()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() ||
        m_currentEpisodeUuid.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Wybierz pacjenta i epizod rehabilitacji."));
        return;
    }

    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("assessment_type"), QStringLiteral("baseline")},
        {QStringLiteral("assessed_at"),
         QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    };
    auto parseOptionalNumber = [this, &payload](QLineEdit* field, const QString& key,
                                      double minimum, double maximum,
                                      bool integerOnly, QString& error) -> bool {
        const QString text = field->text().trimmed();
        if (text.isEmpty()) return true; // Missing measurement is not zero.
        bool ok = false;
        const double number = text.toDouble(&ok);
        if (!ok || !std::isfinite(number) || number < minimum || number > maximum ||
            (integerOnly && std::floor(number) != number)) {
            error = tr("Nieprawidłowa wartość pola: %1").arg(key);
            return false;
        }
        payload.insert(key, integerOnly ? QJsonValue(static_cast<int>(number))
                                           : QJsonValue(number));
        return true;
    };

    QString validationError;
    if (!parseOptionalNumber(m_assessmentPainNrsEdit, QStringLiteral("pain_nrs"), 0, 10, true, validationError) ||
        !parseOptionalNumber(m_assessmentRomEdit, QStringLiteral("rom_degrees"), 0, 360, false, validationError) ||
        !parseOptionalNumber(m_assessmentStrengthEdit, QStringLiteral("strength_mrc"), 0, 5, true, validationError) ||
        !parseOptionalNumber(m_assessmentSwellingEdit, QStringLiteral("swelling_cm"), 0, 100, false, validationError)) {
        m_assessmentStatusLabel->setText(validationError);
        return;
    }
    const QString functionalTest = m_assessmentFunctionalTestEdit->text().trimmed();
    if (!functionalTest.isEmpty())
        payload.insert(QStringLiteral("functional_test"), functionalTest.left(500));
    if (payload.size() <= 4) {
        m_assessmentStatusLabel->setText(tr("Wprowadź co najmniej jeden oceniony parametr."));
        return;
    }

    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-assessments"),
                                      payload, created, &error)) {
        m_assessmentStatusLabel->setText(tr("Nie udało się zapisać oceny: %1").arg(error));
        return;
    }
    m_assessmentPainNrsEdit->clear();
    m_assessmentRomEdit->clear();
    m_assessmentStrengthEdit->clear();
    m_assessmentSwellingEdit->clear();
    m_assessmentFunctionalTestEdit->clear();
    m_assessmentStatusLabel->setText(tr("Ocena początkowa została trwale zapisana."));
    refreshRehabilitationAssessments();
}

void OutlineTab::refreshRehabilitationGoals()
{
    const bool canUse = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                        !m_currentEpisodeUuid.isEmpty();
    if (m_createGoalButton) m_createGoalButton->setEnabled(canUse);
    for (QLineEdit* field : {m_goalTitleEdit, m_goalMetricEdit, m_goalBaselineEdit,
                             m_goalTargetEdit, m_goalUnitEdit, m_goalTargetDateEdit})
        if (field) field->setEnabled(canUse);
    if (!m_goalHistoryBrowser) return;
    m_goalHistoryBrowser->clear();
    if (!canUse) {
        m_goalStatusLabel->setText(tr("Wybierz pacjenta i epizod, aby zarządzać celami."));
        return;
    }
    QString error;
    const QJsonArray goals = m_resourceRepository->list(QStringLiteral("rehabilitation-goals"), &error);
    if (!error.isEmpty()) {
        m_goalStatusLabel->setText(tr("Nie udało się odczytać celów terapeutycznych."));
        m_createGoalButton->setEnabled(false);
        return;
    }
    QStringList entries;
    for (const QJsonValue& value : goals) {
        if (!value.isObject()) continue;
        const QJsonObject goal = value.toObject();
        if (goal.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            goal.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        entries << QStringLiteral("%1 — %2: %3 %4 (cel: %5 %4; termin: %6; status: %7)")
            .arg(goal.value(QStringLiteral("title")).toString(),
                 goal.value(QStringLiteral("metric_id")).toString(),
                 QString::number(goal.value(QStringLiteral("baseline_value")).toDouble()),
                 goal.value(QStringLiteral("unit")).toString(),
                 QString::number(goal.value(QStringLiteral("target_value")).toDouble()),
                 goal.value(QStringLiteral("target_date")).toString().isEmpty()
                    ? tr("nie określono") : goal.value(QStringLiteral("target_date")).toString(),
                 goal.value(QStringLiteral("status")).toString());
    }
    m_goalHistoryBrowser->setPlainText(entries.isEmpty()
        ? tr("Brak zapisanych celów dla tego epizodu.") : entries.join(QStringLiteral("\n")));
    m_goalStatusLabel->setText(tr("Zapisane cele: %1").arg(entries.size()));
}

void OutlineTab::createRehabilitationGoal()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() || m_currentEpisodeUuid.isEmpty()) {
        m_goalStatusLabel->setText(tr("Wybierz pacjenta i epizod rehabilitacji."));
        return;
    }
    const QString title = m_goalTitleEdit->text().trimmed();
    const QString metric = m_goalMetricEdit->text().trimmed();
    const QString unit = m_goalUnitEdit->text().trimmed();
    bool baselineOk = false, targetOk = false;
    const double baseline = m_goalBaselineEdit->text().trimmed().toDouble(&baselineOk);
    const double target = m_goalTargetEdit->text().trimmed().toDouble(&targetOk);
    if (title.isEmpty() || title.size() > 200 || metric.isEmpty() || metric.size() > 120 ||
        unit.isEmpty() || unit.size() > 32 || !baselineOk || !targetOk ||
        !std::isfinite(baseline) || !std::isfinite(target)) {
        m_goalStatusLabel->setText(tr("Uzupełnij cel, wskaźnik, wartości liczbowe i jednostkę."));
        return;
    }
    const QString targetDate = m_goalTargetDateEdit->text().trimmed();
    if (!targetDate.isEmpty()) {
        const QDate parsed = QDate::fromString(targetDate, Qt::ISODate);
        if (!parsed.isValid() || parsed.toString(Qt::ISODate) != targetDate) {
            m_goalStatusLabel->setText(tr("Termin musi mieć format RRRR-MM-DD."));
            return;
        }
    }
    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("title"), title},
        {QStringLiteral("metric_id"), metric},
        {QStringLiteral("baseline_value"), baseline},
        {QStringLiteral("target_value"), target},
        {QStringLiteral("unit"), unit},
        {QStringLiteral("status"), QStringLiteral("active")},
        {QStringLiteral("target_date"), targetDate}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-goals"), payload, created, &error)) {
        m_goalStatusLabel->setText(tr("Nie udało się zapisać celu: %1").arg(error));
        return;
    }
    m_goalTitleEdit->clear();
    m_goalMetricEdit->clear();
    m_goalBaselineEdit->clear();
    m_goalTargetEdit->clear();
    m_goalUnitEdit->clear();
    m_goalTargetDateEdit->clear();
    m_goalStatusLabel->setText(tr("Cel terapeutyczny zapisany w repozytorium."));
    refreshRehabilitationGoals();
}

void OutlineTab::refreshRehabilitationSession()
{
    const bool hasContext = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                            !m_currentEpisodeUuid.isEmpty();
    m_currentRehabilitationSessionUuid.clear();
    if (m_sessionPainBeforeEdit) m_sessionPainBeforeEdit->clear();
    if (m_sessionPainAfterEdit) m_sessionPainAfterEdit->clear();
    if (m_sessionPain24hEdit) m_sessionPain24hEdit->clear();
    if (!hasContext) {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Wybierz pacjenta i epizod."));
        return;
    }
    QString error;
    const QJsonArray sessions = m_resourceRepository->list(
        QStringLiteral("rehabilitation-sessions"), &error);
    if (!error.isEmpty()) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się odczytać sesji rehabilitacyjnych."));
        return;
    }
    QJsonObject current;
    QString latestStartedAt;
    for (const QJsonValue& value : sessions) {
        if (!value.isObject()) continue;
        const QJsonObject candidate = value.toObject();
        if (candidate.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            candidate.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        const QString status = candidate.value(QStringLiteral("session_status")).toString();
        if (status != QStringLiteral("active") && status != QStringLiteral("awaiting_24h"))
            continue;
        const QString startedAt = candidate.value(QStringLiteral("started_at")).toString();
        if (current.isEmpty() || startedAt > latestStartedAt) {
            current = candidate;
            latestStartedAt = startedAt;
        }
    }
    if (current.isEmpty()) {
        m_startRehabilitationSessionButton->setEnabled(true);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(true);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Brak otwartej sesji. Wprowadź ból początkowy i rozpocznij sesję."));
        return;
    }
    m_currentRehabilitationSessionUuid = current.value(QStringLiteral("id")).toString();
    const QString status = current.value(QStringLiteral("session_status")).toString();
    if (status == QStringLiteral("active")) {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(true);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(true);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Sesja trwa. Zapisano ból przed sesją: %1/10.")
            .arg(current.value(QStringLiteral("pre_session_pain_nrs")).toInt()));
    } else {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(true);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(true);
        m_rehabilitationSessionStatusLabel->setText(tr("Sesja zakończona. Oczekuje na kontrolę bólu po 24 godzinach."));
    }
}

void OutlineTab::startRehabilitationSession()
{
    bool ok = false;
    const int pain = m_sessionPainBeforeEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból przed sesją jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("session_type"), QStringLiteral("treatment")},
        {QStringLiteral("session_status"), QStringLiteral("active")},
        {QStringLiteral("started_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("pre_session_pain_nrs"), pain}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-sessions"), payload, created, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się rozpocząć sesji: %1").arg(error));
        return;
    }
    m_sessionPainBeforeEdit->clear();
    refreshRehabilitationSession();
}

void OutlineTab::finishRehabilitationSession()
{
    bool ok = false;
    const int pain = m_sessionPainAfterEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból po sesji jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject current;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
                                   m_currentRehabilitationSessionUuid, current, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się odczytać sesji: %1").arg(error));
        return;
    }
    current.insert(QStringLiteral("post_session_pain_nrs"), pain);
    current.insert(QStringLiteral("ended_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    current.insert(QStringLiteral("session_status"), QStringLiteral("awaiting_24h"));
    QJsonObject updated;
    if (!m_resourceRepository->update(QStringLiteral("rehabilitation-sessions"),
            m_currentRehabilitationSessionUuid, current.value(QStringLiteral("version")).toInt(),
            current, updated, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się zakończyć sesji: %1").arg(error));
        return;
    }
    m_sessionPainAfterEdit->clear();
    refreshRehabilitationSession();
}

void OutlineTab::recordRehabilitation24HourFollowUp()
{
    bool ok = false;
    const int pain = m_sessionPain24hEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból po 24 godzinach jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject current;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
                                   m_currentRehabilitationSessionUuid, current, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się odczytać sesji: %1").arg(error));
        return;
    }
    current.insert(QStringLiteral("pain_at_24h_nrs"), pain);
    current.insert(QStringLiteral("follow_up_24h_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    current.insert(QStringLiteral("session_status"), QStringLiteral("completed"));
    QJsonObject updated;
    if (!m_resourceRepository->update(QStringLiteral("rehabilitation-sessions"),
            m_currentRehabilitationSessionUuid, current.value(QStringLiteral("version")).toInt(),
            current, updated, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się zapisać kontroli: %1").arg(error));
        return;
    }
    m_sessionPain24hEdit->clear();
    m_currentRehabilitationSessionUuid.clear();
    refreshRehabilitationSession();
}

void OutlineTab::setMeasurementEngine(
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine)
{
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    m_measurementEngine = std::move(engine);
    if (!m_measurementEngine) return;

    m_measurementCallbackToken = m_measurementEngine->subscribe(
        [this](const biofeedback::measurement::MeasurementSample& sample) {
            if (!sample.isValid() || sample.unit != "N" || !std::isfinite(sample.value))
                return;
            const auto timestamp = QDateTime::fromMSecsSinceEpoch(sample.timestampUnixMs);
            QMetaObject::invokeMethod(this, [this, value = std::abs(sample.value), timestamp]() {
                if (m_forceGraph)
                    m_forceGraph->addDataPoint(value, timestamp);
            }, Qt::QueuedConnection);
        });
}

bool OutlineTab::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}


} // namespace tab
