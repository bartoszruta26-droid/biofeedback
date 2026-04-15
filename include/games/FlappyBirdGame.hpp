#ifndef GAMES_FLAPPYBIRDGAME_HPP
#define GAMES_FLAPPYBIRDGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QRectF>
#include <QPointF>
#include <vector>
#include <memory>

namespace games {

/**
 * @class FlappyBirdGame
 * @brief Gra typu Flappy Bird sterowana siłą nacisku z tensometru
 * 
 * Cel gry:
 * - Gracz steruje ptakiem poprzez naciąganie taśmy lateksowej
 * - Wartość siły z tensometru steruje prędkością w górę ptaka
 * - Ptak leci między gałęziami drzew (przeszkodami)
 * - Gracz nie może dotknąć gałęzi ani spaść na ziemię
 * - Zdobywa punkty za każde pomyślnie minięcie przeszkody
 * 
 * Wymagania sprzętowe:
 * - Arduino Nano z tensometrem
 * - Komunikacja przez port szeregowy (opcjonalnie)
 * 
 * Funkcjonalności:
 * - Poziomy trudności (1-5) z różną prędkością gry i odstępami między gałęziami
 * - Różne prędkości gry wpływające na dynamikę rozgrywki
 * - Generowanie losowych przeszkód z lukami do przelotu
 */
class FlappyBirdGame : public GameEngine {
    Q_OBJECT

public:
    explicit FlappyBirdGame(QWidget *parent = nullptr);
    ~FlappyBirdGame() override;

    // Implementacja interfejsu GameEngine
    QString gameName() const override;
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter &painter) override;
    bool handleInput(double value) override;

    /**
     * @brief Ustawia poziom trudności gry
     * @param level Poziom trudności (1-5, gdzie 1 najłatwiejszy)
     */
    void setDifficultyLevel(int level);

    /**
     * @brief Pobiera aktualny poziom trudności
     * @return Poziom trudności
     */
    int getDifficultyLevel() const;

    /**
     * @brief Ustawia prędkość gry (mnożnik prędkości przeszkód)
     * @param speed Mnożnik prędkości (0.5 - 2.0)
     */
    void setGameSpeed(float speed);

    /**
     * @brief Pobiera aktualną prędkość gry
     * @return Mnożnik prędkości
     */
    float getGameSpeed() const;

    /**
     * @brief Podłącza obiekt SerialCommunication do gry
     * @param serial Współdzielony wskaźnik do obiektu SerialCommunication
     */
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);

    /**
     * @brief Rozłącza obiekt SerialCommunication
     */
    void disconnectSerial();

    /**
     * @brief Sprawdza czy połączenie szeregowe jest aktywne
     * @return true jeśli połączony
     */
    bool isSerialConnected() const;

signals:
    /**
     * @brief Sygnał emitowany gdy wystąpi błąd komunikacji szeregowej
     * @param message Opis błędu
     */
    void serialError(const QString &message);

    /**
     * @brief Sygnał emitowany gdy połączenie szeregowe zostanie nawiązane
     */
    void serialConnected();

    /**
     * @brief Sygnał emitowany gdy połączenie szeregowe zostanie zerwane
     */
    void serialDisconnected();

private slots:
    /**
     * @brief Callback dla nowych danych z sensora
     * @param data Dane z czujnika
     */
    void onSensorDataReceived(const sensor::SensorData& data);

    /**
     * @brief Callback dla zmian statusu połączenia
     * @param connected Status połączenia
     * @param message Komunikat o statusie
     */
    void onConnectionStatusChanged(bool connected, const std::string& message);

private:
    // Struktura reprezentująca przeszkodę (gałęzie drzewa)
    struct Obstacle {
        float xPosition;          ///< Pozycja X przeszkody
        float gapY;               ///< Pozycja Y luki między gałęziami
        float gapHeight;          ///< Wysokość luki
        bool passed;              ///< Czy ptak już minął tę przeszkodę
        
        Obstacle() : xPosition(0), gapY(0), gapHeight(0), passed(false) {}
    };

    // Stan gry - Ptak
    QPointF m_birdPos;            ///< Pozycja ptaka
    QPointF m_birdVelocity;       ///< Prędkość ptaka
    float m_birdRadius;           ///< Promień ptaka
    float m_gravity;              ///< Siła grawitacji
    float m_flapForce;            ///< Siła odbicia przy naciągu
    
    // Stan gry - Przeszkody
    std::vector<Obstacle> m_obstacles;  ///< Lista aktywnych przeszkód
    float m_obstacleWidth;              ///< Szerokość przeszkody
    float m_obstacleSpawnTimer;         ///< Timer do generowania przeszkód
    float m_obstacleSpawnInterval;      ///< Odstęp czasu między przeszkodami
    float m_obstacleSpeed;              ///< Prędkość ruchu przeszkód
    float m_minGapHeight;               ///< Minimalna wysokość luki
    float m_maxGapHeight;               ///< Maksymalna wysokość luki
    
    // Parametry trudności
    int m_difficultyLevel;      ///< Aktualny poziom trudności (1-5)
    float m_gameSpeed;          ///< Mnożnik prędkości gry
    
    // Konfiguracja Serial
    std::shared_ptr<sensor::SerialCommunication> m_serialPort; ///< Współdzielony wskaźnik do komunikacji
    bool m_ownSerial;                                          ///< Czy gra jest właścicielem obiektu serial
    
    // Stałe fizyczne/logiczne
    const float m_birdBaseRadius = 12.0f;
    const float m_obstacleBaseWidth = 60.0f;
    const double m_minForce = 0.0;
    const double m_maxForce = 100.0; ///< Zakres siły w jednostkach umownych
    
    // Aktualna siła z sensora (znormalizowana 0.0 - 1.0)
    double m_currentForce;
    
    // Licznik punktów za przeszkody
    int m_obstaclesPassed;
    
    // Pomocnicze
    void resetBird();
    void spawnObstacle();
    void normalizeForceValue(double rawValue);
    QColor getDifficultyColor() const;
    void checkCollisions();
    void updateObstacles(double deltaTime);
    void updateBird(double deltaTime);
};

} // namespace games

#endif // GAMES_FLAPPYBIRDGAME_HPP

