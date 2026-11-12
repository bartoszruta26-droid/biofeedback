#ifndef GAMES_GAMEENGINE_HPP
#define GAMES_GAMEENGINE_HPP

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>
#include <atomic>
#include "core/DebugManager.hpp"

namespace games {

/**
 * @class GameEngine
 * @brief Abstrakcyjna klasa bazowa dla wszystkich gier w aplikacji Biofeedback
 * 
 * GameEngine definiuje wspólny interfejs dla wszystkich gier:
 * - Zarządzanie stanem gry (start, stop, pauza)
 * - Obsługa sygnałów wejściowych z sensorów
 * - Renderowanie widoku gry
 * - Zliczanie wyników i statystyk
 * 
 * Każda gra dziedzicząca po GameEngine musi zaimplementować:
 * - initialize() - inicjalizacja specyficzna dla gry
 * - update() - aktualizacja logiki gry w każdej klatce
 * - render() - renderowanie stanu gry
 * - handleInput() - obsługa wejścia z sensora
 */
class GameEngine : public QWidget
{
    Q_OBJECT

public:
    explicit GameEngine(QWidget *parent = nullptr);
    ~GameEngine() override;

    /**
     * @brief Rozpoczyna grę
     * Inicjalizuje stan gry i uruchamia pętlę gry
     */
    virtual void start();

    /**
     * @brief Zatrzymuje grę
     * Zatrzymuje pętlę gry i czyści zasoby
     */
    virtual void stop();

    /**
     * @brief Pauzuje grę
     * Tymczasowo zatrzymuje aktualizację gry
     */
    virtual void pause();

    /**
     * @brief Wznawia grę z pauzy
     */
    virtual void resume();

    /**
     * @brief Sprawdza czy gra jest aktywna
     * @return true jeśli gra trwa
     */
    bool isRunning() const;

    /**
     * @brief Sprawdza czy gra jest spauzowana
     * @return true jeśli gra jest spauzowana
     */
    bool isPaused() const;

    /**
     * @brief Pobiera aktualny wynik
     * @return Aktualny wynik gracza
     */
    int getScore() const;

    /**
     * @brief Pobiera nazwę gry
     * @return Nazwa gry jako QString
     */
    virtual QString gameName() const = 0;

    /**
     * @brief Resetuje stan gry do początkowego
     */
    virtual void reset();

    /**
     * @brief Pobiera statystyki gry jako string
     * @return String zawierający statystyki (wynik, czas gry, błędy)
     */
    QString getGameStats() const;

    /**
     * @brief Resetuje liczniki statystyk
     */
    void resetGameStats();

signals:
    /**
     * @brief Sygnał emitowany gdy gra się rozpoczyna
     */
    void gameStarted();

    /**
     * @brief Sygnał emitowany gdy gra się kończy
     * @param score Końcowy wynik
     */
    void gameEnded(int score);

    /**
     * @brief Sygnał emitowany gdy wynik ulega zmianie
     * @param score Nowy wynik
     */
    void scoreChanged(int score);

    /**
     * @brief Sygnał emitowany gdy gra jest pauzowana
     */
    void gamePaused();

    /**
     * @brief Sygnał emitowany gdy gra jest wznawiana
     */
    void gameResumed();

public slots:
    /**
     * @brief Główna pętla aktualizacji gry
     * Wywoływana okresowo przez QTimer
     */
    virtual void gameLoop();

    /**
     * @brief Obsługuje dane wejściowe z sensora
     * @param value Wartość odczytana z sensora (np. siła nacisku)
     */
    virtual void handleSensorInput(double value);

protected:
    /**
     * @brief Inicjalizuje specyficzne elementy gry
     * Do zaimplementowania w klasach pochodnych
     */
    virtual void initialize() = 0;

    /**
     * @brief Aktualizuje logikę gry
     * Do zaimplementowania w klasach pochodnych
     * @param deltaTime Czas jaki upłynął od ostatniej klatki (w sekundach)
     */
    virtual void update(double deltaTime) = 0;

    /**
     * @brief Renderuje stan gry
     * Do zaimplementowania w klasach pochodnych
     * @param painter Obiekt QPainter używany do rysowania
     */
    virtual void render(QPainter& painter) = 0;

    // Called by the presentation layer with the fraction remaining between
    // fixed simulation states. Games opt in by interpolating their own state.
    virtual void renderInterpolated(QPainter& painter, double alpha);

    static constexpr double kFixedSimulationStepSeconds = 1.0 / 240.0;
    // Standard accumulator interpolation blends adjacent 240 Hz simulation
    // states smoothly at the presentation cadence, without jumping ahead.
    double renderInterpolationAlpha() const { return m_renderInterpolationAlpha; }

    /**
     * @brief Obsługuje wejście specyficzne dla gry
     * Do zaimplementowania w klasach pochodnych
     * @param value Wartość wejściowa
     * @return true jeśli wejście zostało obsłużone
     */
    virtual bool handleInput(double value) = 0;

    /**
     * @brief Ustawia wynik gry
     * @param score Nowy wynik
     */
    void setScore(int score);

    /**
     * @brief Zwiększa wynik o podaną wartość
     * @param points Liczba punktów do dodania
     */
    void addScore(int points);

    /**
     * @brief Paint event - wywoływany przy repaint
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Resize event - wywoływany przy zmianie rozmiaru
     */
    void resizeEvent(QResizeEvent* event) override;

protected:
    // Stan gry
    bool m_isRunning;
    bool m_isPaused;
    
    // Wynik
    int m_score;
    
    // Timer do pętli gry
    QTimer* m_gameTimer;
    
    // Monotonic timestamp of the last simulation pump, in nanoseconds.
    qint64 m_lastFrameTime;
    QElapsedTimer m_monotonicClock;
    qint64 m_accumulatedSessionMs;
    double m_simulationAccumulator;
    double m_renderInterpolationAlpha;
    
    quint64 m_frameCount;
    quint64 m_invalidInputCount;
    std::atomic<quint64> m_pendingInvalidInputCount{0};

    // Coalesce high-rate worker-thread sensor callbacks into one GUI dispatch.
    // The generation invalidates callbacks queued before stop/reset.
    std::atomic<double> m_pendingSensorValue{0.0};
    std::atomic_bool m_pendingSensorValueAvailable{false};
    std::atomic_bool m_sensorDispatchQueued{false};
    std::atomic<quint64> m_sensorDispatchGeneration{0};

    // Wartość wejściowa z sensora
    double m_sensorValue;
};

} // namespace games

#endif // GAMES_GAMEENGINE_HPP
