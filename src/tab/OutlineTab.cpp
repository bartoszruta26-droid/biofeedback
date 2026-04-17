#include "tab/OutlineTab.hpp"
#include "games/GameEngine.hpp"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>

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
{
    setupUI();
    createConnections();
    
    // Domyślnie załaduj konspekty z katalogu
    loadOutlines(m_outlinesDirectory);
}

OutlineTab::~OutlineTab()
{
    if (m_isSessionActive) {
        stopSession();
    }
}

void OutlineTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    setupOutlineSelector();
    setupExerciseView();
    setupControlPanel();
    setupGameArea();
}

void OutlineTab::setupOutlineSelector()
{
    // Sekcja wyboru konspektu
    m_outlineGroup = new QGroupBox(tr("Wybór Konspektu"), this);
    m_outlineSelectorLayout = new QHBoxLayout();
    
    m_outlineLabel = new QLabel(tr("Konspekt:"), this);
    m_outlineCombo = new QComboBox(this);
    m_outlineCombo->setMinimumWidth(300);
    m_outlineCombo->addItem(tr("-- Wybierz konspekt --"), QString());
    
    m_loadOutlineButton = new QPushButton(tr("Wczytaj..."), this);
    m_loadOutlineButton->setToolTip(tr("Wczytaj plik JSON konspektu"));
    
    m_outlineSelectorLayout->addWidget(m_outlineLabel);
    m_outlineSelectorLayout->addWidget(m_outlineCombo, 1);
    m_outlineSelectorLayout->addWidget(m_loadOutlineButton);
    
    m_outlineGroup->setLayout(m_outlineSelectorLayout);
    m_mainLayout->addWidget(m_outlineGroup);
    
    // Panel informacji o konspekcie
    m_outlineInfoGroup = new QGroupBox(tr("Informacje o Konspekcie"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout();
    
    m_outlineNameLabel = new QLabel(tr("Nazwa: -"), this);
    m_outlineNameLabel->setFont(QFont("Sans Serif", 12, QFont::Bold));
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
    m_mainLayout->addWidget(m_outlineInfoGroup);
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
    m_exerciseNameLabel->setFont(QFont("Sans Serif", 14, QFont::Bold));
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
    m_exerciseLayout->addWidget(m_gameSelectionGroup);
    
    m_exerciseGroup->setLayout(m_exerciseLayout);
    m_mainLayout->addWidget(m_exerciseGroup);
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
    m_mainLayout->addWidget(m_controlGroup);
    
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
    m_mainLayout->addWidget(m_progressGroup);
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
    m_mainLayout->addWidget(m_gameAreaGroup);
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
    m_gameCombo->addItem(tr("Scored Earth"), "scored_earth");
    m_gameCombo->addItem(tr("Car Race"), "car_race");
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
    qint64 elapsed = (QDateTime::currentMSecsSinceEpoch() - m_sessionStartTime) / 1000;
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

} // namespace tab
