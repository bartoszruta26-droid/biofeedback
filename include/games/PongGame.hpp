#ifndef GAMES_PONGGAME_HPP
#define GAMES_PONGGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QRectF>
#include <QPointF>
#include <memory>

namespace games {

/**
 * @class PongGame
 * @brief Klasyczna gra Pong sterowana siłą nacisku z tensometru
 * 
 * Cel gry:
 * - Gracz steruje lewą paletką poprzez naciąganie taśmy lateksowej
 * - Wartość siły z tensometru steruje pozycją pionową paletki
 * - Prawa paletka jest sterowana przez komputer (AI)
 * - Piłka odbija się od paletki i krawędzi ekranu
 * - Gracz zdobywa punkty gdy piłka minie paletkę przeciwnika
 * 
 * Wymagania sprzętowe:
 * - Arduino Nano z tensometrem
 * - Komunikacja przez port szeregowy (opcjonalnie)
 */
class PongGame : public GameEngine {
    Q_OBJECT

public:
    explicit PongGame(QWidget *parent = nullptr);
    ~PongGame() override;

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
    // Stan gry - Paletki
    QRectF m_leftPaddle;    ///< Paletka gracza (lewa strona)
    QRectF m_rightPaddle;   ///< Paletka komputera (prawa strona)
    float m_leftPaddleY;    ///< Pozycja Y paletki gracza
    float m_rightPaddleY;   ///< Pozycja Y paletki komputera
    
    // Stan gry - Piłka
    QPointF m_ballPos;      ///< Pozycja piłki
    QPointF m_ballVel;      ///< Wektor prędkości piłki
    float m_ballRadius;     ///< Promień piłki
    
    // Stan gry - Punkty
    int m_playerScore;      ///< Wynik gracza
    int m_aiScore;          ///< Wynik komputera
    
    // Parametry trudności
    int m_difficultyLevel;  ///< Aktualny poziom trudności (1-5)
    float m_aiSpeed;        ///< Prędkość AI [piksele/s]
    float m_ballSpeed;      ///< Prędkość piłki [piksele/s]
    
    // Konfiguracja Serial
    std::shared_ptr<sensor::SerialCommunication> m_serialPort; ///< Współdzielony wskaźnik do komunikacji
    bool m_ownSerial;                                          ///< Czy gra jest właścicielem obiektu serial
    
    // Stałe fizyczne/logiczne
    const float m_paddleWidth = 15.0f;
    const float m_paddleHeight = 80.0f;
    const float m_paddleMargin = 20.0f;
    const double m_minForce = 0.0;
    const double m_maxForce = 100.0; ///< Zakres siły w jednostkach umownych
    
    // Aktualna siła z sensora (znormalizowana 0.0 - 1.0)
    double m_currentForce;
    
    // Pomocnicze
    void resetBall();
    void normalizeForceValue(double rawValue);
    QColor getDifficultyColor() const;
    void updateAIPaddle(double deltaTime);
    void checkCollisions();
};

} // namespace games

#endif // GAMES_PONGGAME_HPP
