#ifndef GAMES_MARIOGAME_HPP
#define GAMES_MARIOGAME_HPP

#include "games/GameEngine.hpp"
#include <QPainter>
#include <QVector>
#include <QRectF>
#include <QRandomGenerator>

namespace games {

/**
 * @class MarioGame
 * @brief Gra typu "Mario Run" z mechaniką skoku zależną od tensometru
 * 
 * Mario biegnie automatycznie w prawo. Gracz kontroluje wysokość skoku
 * poprzez naciśnięcie i przytrzymanie tensometru - im dłużej przytrzymuje,
 * tym wyższy skok (mechanika ładowania).
 * 
 * Cechy gry:
 * - Styl 4-bitowy (proste kształty, ograniczona paleta kolorów)
 * - Proceduralne generowanie poziomów
 * - Rosnąca trudność z każdym poziomem
 * - Przeszkody: dziury, kolce, platformy, rurki
 */
class MarioGame : public GameEngine
{
    Q_OBJECT

public:
    explicit MarioGame(QWidget *parent = nullptr);
    ~MarioGame() override;

    /**
     * @brief Nazwa gry
     * @return "Mario Run"
     */
    QString gameName() const override;

protected:
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

private:
    /**
     * @brief Struktura reprezentująca gracza (Mario)
     */
    struct Player {
        QRectF body;           // Ciało gracza
        double velocityY;      // Prędkość pionowa
        bool isJumping;        // Czy gracz skacze
        bool isOnGround;       // Czy gracz jest na ziemi
        double chargeLevel;    // Poziom naładowania skoku (0.0 - 1.0)
        bool isCharging;       // Czy gracz ładuje skok
        int frame;             // Klatka animacji biegu
        double frameTimer;     // Timer animacji
    };

    /**
     * @brief Struktura reprezentująca przeszkodę
     */
    struct Obstacle {
        QRectF rect;           // Prostokąt przeszkody
        enum Type { SPIKE, PIPE, PLATFORM, GAP } type;  // Typ przeszkody
        bool passed;           // Czy gracz minął przeszkodę
    };

    /**
     * @brief Struktura reprezentująca chmurkę (dekoracja)
     */
    struct Cloud {
        QPointF pos;           // Pozycja chmurki
        double speed;          // Szybkość ruchu
        int size;              // Rozmiar chmurki
    };

    // Inicjalizacja gracza
    void initPlayer();

    // Aktualizacja fizyki gracza
    void updatePlayer(double deltaTime);

    // Generowanie przeszkód
    void generateObstacles();

    // Aktualizacja przeszkód
    void updateObstacles(double deltaTime);

    // Wykrywanie kolizji
    bool checkCollision(const Player& player, const Obstacle& obstacle);

    // Renderowanie gracza (styl 4-bitowy)
    void renderPlayer(QPainter& painter, const Player& player);

    // Renderowanie przeszkód (styl 4-bitowy)
    void renderObstacles(QPainter& painter);

    // Renderowanie tła (styl 4-bitowy)
    void renderBackground(QPainter& painter);

    // Renderowanie UI (pasek ładowania, wynik, poziom)
    void renderUI(QPainter& painter);

    // Reset poziomu
    void resetLevel();

    // Zwiększanie trudności
    void increaseDifficulty();

    // Stałe fizyczne
    static constexpr double GRAVITY = 800.0;         // Przyspieszenie grawitacyjne
    static constexpr double JUMP_FORCE = 650.0;      // Siła skoku
    static constexpr double MAX_CHARGE_TIME = 1.5;   // Maksymalny czas ładowania (sekundy)
    static constexpr double PLAYER_SPEED = 200.0;    // Prędkość biegu Mario
    static constexpr double GROUND_Y_RATIO = 0.75;   // Pozycja ziemi (75% wysokości)

    // Stałe graficzne - paleta 4-bitowa (16 kolorów NES)
    static constexpr int BIT_DEPTH = 4;              // 4 bity = 16 kolorów
    
    // Kolory w stylu NES
    const QColor COLOR_SKY = QColor(0x6B, 0x8C, 0xFF);      // Jasnoniebieski
    const QColor COLOR_GROUND = QColor(0xC8, 0x4C, 0x0C);   // Brązowy
    const QColor COLOR_MARIO_RED = QColor(0xFC, 0x38, 0x38); // Czerwony (czapka, koszulka)
    const QColor COLOR_MARIO_SKIN = QColor(0xFC, 0x98, 0x38); // Kolor skóry
    const QColor COLOR_MARIO_BROWN = QColor(0x80, 0x4C, 0x0C); // Brązowy (włosy, buty)
    const QColor COLOR_SPIKE = QColor(0xA0, 0xA0, 0xA0);    // Szary (kolce)
    const QColor COLOR_PIPE = QColor(0x00, 0xC8, 0x00);     // Zielony (rurki)
    const QColor COLOR_CLOUD = QColor(0xFC, 0xFC, 0xFC);    // Biały (chmury)
    const QColor COLOR_CHARGE_EMPTY = QColor(0x80, 0x80, 0x80); // Szary (pusty pasek)
    const QColor COLOR_CHARGE_FULL = QColor(0xFC, 0x98, 0x38);  // Pomarańczowy (pełny pasek)

    // Stan gry
    Player m_player;
    QVector<Obstacle> m_obstacles;
    QVector<Cloud> m_clouds;
    
    // Poziomy
    int m_currentLevel;
    int m_obstaclesPassed;
    int m_obstaclesToNextLevel;
    
    // Czas od ostatniego wygenerowania przeszkody
    double m_timeSinceLastObstacle;
    double m_obstacleSpawnInterval;
    
    // Tensometr
    double m_sensorThreshold;      // Próg aktywacji
    double m_chargeStartTime;      // Czas rozpoczęcia ładowania
};

} // namespace games

#endif // GAMES_MARIOGAME_HPP
