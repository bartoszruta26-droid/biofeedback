#ifndef TAB_OUTLINETAB_HPP
#define TAB_OUTLINETAB_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QProgressBar>

namespace games {
    class GameEngine;
}

namespace tab {

/**
 * @brief Struktura przechowująca dane pojedynczego ćwiczenia w konspekcie
 */
struct ExerciseData {
    QString id;                 // Unikalny identyfikator ćwiczenia
    QString name;               // Nazwa ćwiczenia
    QString description;        // Opis ćwiczenia
    QString imagePath;          // Ścieżka do obrazu/podglądu
    QString videoPath;          // Ścieżka do filmu instruktażowego
    QString gameId;             // ID gry przypisanej do ćwiczenia
    int durationSeconds;        // Czas trwania ćwiczenia w sekundach
    int targetRepetitions;      // Docelowa liczba powtórzeń
    QString difficulty;         // Poziom trudności (easy/medium/hard)
    
    ExerciseData() : durationSeconds(0), targetRepetitions(0) {}
};

/**
 * @brief Struktura przechowująca dane całego konspektu
 */
struct OutlineData {
    QString id;                     // Unikalny identyfikator konspektu
    QString name;                   // Nazwa konspektu
    QString description;            // Opis konspektu
    QString author;                 // Autor konspektu
    QString category;               // Kategoria (np. rehabilitacja, trening)
    QVector<ExerciseData> exercises; // Lista ćwiczeń w konspekcie
    int estimatedDurationMinutes;   // Szacowany czas trwania całego konspektu
    
    OutlineData() : estimatedDurationMinutes(0) {}
};

/**
 * @class OutlineTab
 * @brief Widget zakładki do zarządzania i uruchamiania konspektów ćwiczeń
 * 
 * OutlineTab umożliwia:
 * - Wybór konspektu z listy dostępnych plików JSON
 * - Przeglądanie ćwiczeń w wybranym konspekcie
 * - Wyświetlanie nazwy, opisu, obrazu i filmu dla każdego ćwiczenia
 * - Przypisywanie gier biofeedback do ćwiczeń
 * - Uruchamianie konspektu - sekwencyjne wykonywanie ćwiczeń z grami
 * - Automatyczne przechodzenie do następnego ćwiczenia po ukończeniu gry
 * - Śledzenie postępu w konspekcie
 */
class OutlineTab : public QWidget
{
    Q_OBJECT

public:
    explicit OutlineTab(QWidget *parent = nullptr);
    ~OutlineTab() override;

    /**
     * @brief Ładuje dostępne konspekty z katalogu
     * @param directory Katalog z plikami JSON konspektów
     */
    void loadOutlines(const QString& directory);

    /**
     * @brief Ładuje konkretny konspekt z pliku JSON
     * @param filename Pełna ścieżka do pliku JSON
     * @return true jeśli ładowanie powiodło się
     */
    bool loadOutlineFromFile(const QString& filename);

    /**
     * @brief Pobiera liczbę dostępnych konspektów
     */
    int outlineCount() const;

    /**
     * @brief Pobiera listę dostępnych konspektów
     */
    QVector<OutlineData> getOutlines() const;

    /**
     * @brief Sprawdza czy konspekt jest aktualnie aktywny (trwa sesja)
     */
    bool isSessionActive() const;

    /**
     * @brief Rozpoczyna sesję z wybranym konspektem
     */
    void startSession();

    /**
     * @brief Zatrzymuje bieżącą sesję
     */
    void stopSession();

    /**
     * @brief Pauzuje bieżącą sesję
     */
    void pauseSession();

    /**
     * @brief Wznawia sesję z pauzy
     */
    void resumeSession();

    /**
     * @brief Przechodzi do następnego ćwiczenia ręcznie
     */
    void nextExercise();

    /**
     * @brief Przechodzi do poprzedniego ćwiczenia
     */
    void previousExercise();

    /**
     * @brief Ustawia grę dla bieżącego ćwiczenia
     * @param gameId ID gry do przypisania
     */
    void setGameForCurrentExercise(const QString& gameId);

signals:
    /**
     * @brief Sygnał emitowany gdy wybrano inny konspekt
     * @param outlineId ID wybranego konspektu
     */
    void outlineSelected(const QString& outlineId);

    /**
     * @brief Sygnał emitowany gdy rozpoczęto sesję
     * @param outlineId ID konspektu
     */
    void sessionStarted(const QString& outlineId);

    /**
     * @brief Sygnał emitowany gdy zakończono sesję
     * @param completedExercises Liczba ukończonych ćwiczeń
     */
    void sessionEnded(int completedExercises);

    /**
     * @brief Sygnał emitowany przy zmianie ćwiczenia
     * @param exerciseIndex Indeks текущего ćwiczenia
     * @param exerciseData Dane ćwiczenia
     */
    void exerciseChanged(int exerciseIndex, const ExerciseData& exerciseData);

    /**
     * @brief Sygnał emitowany gdy ukończono pojedyncze ćwiczenie
     * @param exerciseIndex Indeks ukończonego ćwiczenia
     * @param score Wynik z gry
     */
    void exerciseCompleted(int exerciseIndex, int score);

    /**
     * @brief Sygnał emitowany gdy żądano uruchomienia gry
     * @param gameId ID gry do uruchomienia
     * @param exerciseData Dane ćwiczenia kontekstowego
     */
    void requestGameStart(const QString& gameId, const ExerciseData& exerciseData);

    /**
     * @brief Sygnał emitowany gdy gra została zakończona
     * @param score Wynik końcowy
     */
    void gameFinished(int score);

public slots:
    /**
     * @brief Obsługuje zakończenie gry z zewnątrz
     * @param score Wynik gry
     */
    void onGameFinished(int score);

private slots:
    void onOutlineSelected(int index);
    void onStartSessionClicked();
    void onStopSessionClicked();
    void onPauseResumeClicked();
    void onNextExerciseClicked();
    void onPreviousExerciseClicked();
    void onGameSelectionChanged(int index);
    void onVideoPlayerButtonClicked();
    void updateExerciseDisplay();
    void onSessionTimerTick();

private:
    void setupUI();
    void setupOutlineSelector();
    void setupExerciseView();
    void setupControlPanel();
    void setupGameArea();
    void createConnections();
    
    void loadOutlineFiles(const QString& directory);
    bool parseOutlineFile(const QString& filename, OutlineData& outline);
    void populateOutlineCombo();
    void populateGameCombo();
    void updateProgressIndicators();
    void showExerciseDetails(int index);
    void setCurrentExerciseIndex(int index);
    bool validateCurrentSetup();
    void finishCurrentExercise();
    void moveToNextExercise();
    void endSession();

    // Główne widgety
    QVBoxLayout* m_mainLayout;
    
    // Sekcja wyboru konspektu
    QGroupBox* m_outlineGroup;
    QHBoxLayout* m_outlineSelectorLayout;
    QLabel* m_outlineLabel;
    QComboBox* m_outlineCombo;
    QPushButton* m_loadOutlineButton;
    
    // Panel informacji o konspekcie
    QGroupBox* m_outlineInfoGroup;
    QLabel* m_outlineNameLabel;
    QLabel* m_outlineDescriptionLabel;
    QLabel* m_outlineAuthorLabel;
    QLabel* m_outlineDurationLabel;
    QLabel* m_outlineExerciseCountLabel;
    
    // Widok ćwiczenia
    QGroupBox* m_exerciseGroup;
    QVBoxLayout* m_exerciseLayout;
    
    // Informacje o ćwiczeniu
    QWidget* m_exerciseInfoWidget;
    QVBoxLayout* m_exerciseInfoLayout;
    QLabel* m_exerciseNameLabel;
    QLabel* m_exerciseDescriptionLabel;
    
    // Multimedia
    QHBoxLayout* m_mediaLayout;
    QLabel* m_exerciseImageLabel;
    QWidget* m_videoWidget;
    QPushButton* m_playVideoButton;
    
    // Wybór gry
    QGroupBox* m_gameSelectionGroup;
    QHBoxLayout* m_gameSelectionLayout;
    QLabel* m_gameLabel;
    QComboBox* m_gameCombo;
    QPushButton* m_configureGameButton;
    
    // Panel sterowania sesją
    QGroupBox* m_controlGroup;
    QHBoxLayout* m_controlLayout;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_pauseResumeButton;
    QPushButton* m_prevButton;
    QPushButton* m_nextButton;
    
    // Pasek postępu
    QGroupBox* m_progressGroup;
    QHBoxLayout* m_progressLayout;
    QLabel* m_progressLabel;
    QProgressBar* m_sessionProgress;
    QLabel* m_currentExerciseLabel;
    
    // Obszar gry (embedowana gra)
    QGroupBox* m_gameAreaGroup;
    QVBoxLayout* m_gameAreaLayout;
    QStackedWidget* m_gameStack;
    QWidget* m_gamePlaceholder;
    
    // Timer sesji
    QTimer* m_sessionTimer;
    
    // Dane
    QVector<OutlineData> m_outlines;
    OutlineData m_currentOutline;
    int m_currentOutlineIndex;
    int m_currentExerciseIndex;
    bool m_isSessionActive;
    bool m_isPaused;
    
    // Stan sesji
    int m_completedExercises;
    QVector<int> m_exerciseScores;
    qint64 m_sessionStartTime;
    qint64 m_exerciseStartTime;
    
    // Domyślny katalog z konspektami
    QString m_outlinesDirectory;
    
    // Aktualnie uruchomiona gra
    games::GameEngine* m_currentGame;
};

} // namespace tab

#endif // TAB_OUTLINETAB_HPP
