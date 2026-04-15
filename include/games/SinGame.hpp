#ifndef GAMES_SINGAME_HPP
#define GAMES_SINGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPointF>
#include <QVector>
#include <memory>

namespace games {

/**
 * @class SinGame
 * @brief Gra rehabilitacyjna, w której pacjent steruje punktem poprzez naciąganie taśmy lateksowej
 * 
 * Cel gry:
 * - Pacjent naciąga taśmę lateksową, co jest odczytywane przez tensometr podłączony przez Serial
 * - Wartość siły steruje pozycją pionową kursora
 * - Kursor musi trafić w poruszający się wykres sinusoidy
 * - Poziomy trudności zmieniają margines błędu i prędkość ruchu wykresu
 * 
 * Wymagania sprzętowe:
 * - Arduino Nano z tensometrem
 * - Komunikacja przez port szeregowy
 */
class SinGame : public GameEngine {
    Q_OBJECT

public:
    explicit SinGame(QWidget *parent = nullptr);
    ~SinGame() override;

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
    // Stan gry
    double m_currentForce;      ///< Znormalizowana siła z tensometru (0.0 - 1.0)
    double m_targetY;           ///< Pozycja Y sinusa w punkcie kolizji
    double m_phase;             ///< Faza sinusa (do animacji)
    double m_cursorY;           ///< Pozycja Y kursora gracza (wygładzona)
    bool m_isHit;               ///< Czy gracz trafił w ostatniej klatce
    
    // Parametry trudności
    int m_difficultyLevel;      ///< Aktualny poziom trudności (1-5)
    double m_scrollSpeed;       ///< Prędkość przesuwania wykresu [piksele/s]
    double m_tolerance;         ///< Margines błędu [piksele]
    double m_frequency;         ///< Częstotliwość sinusa
    
    // Konfiguracja Serial
    std::shared_ptr<sensor::SerialCommunication> m_serialPort; ///< Współdzielony wskaźnik do komunikacji
    bool m_ownSerial;                                          ///< Czy gra jest właścicielem obiektu serial
    
    // Stałe fizyczne/logiczne
    const double m_minForce = 0.0;
    const double m_maxForce = 100.0; ///< Zakres siły w jednostkach umownych
    
    // Pomocnicze
    void calculateTargetY(double x);
    QColor getDifficultyColor() const;
    void normalizeForceValue(double rawValue);
};

} // namespace games

#endif // GAMES_SINGAME_HPP
