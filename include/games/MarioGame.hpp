#ifndef GAMES_MARIOGAME_HPP
#define GAMES_MARIOGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPainter>
#include <QVector>
#include <QRectF>
#include <QRandomGenerator>
#include <memory>

class QResizeEvent;
class QPaintEvent;
class QKeyEvent;

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

    // Read-only gameplay diagnostics for regression tests and telemetry.
    double playerVerticalVelocity() const;
    bool playerOnGround() const;
    bool playerCharging() const;
    double jumpCharge() const;
    int activeObstacleCount() const;
    int activeEnemyCount() const;
    int goombaCount() const;
    int piranhaPlantCount() const;
    int currentLevel() const;
    int livesRemaining() const;
    int coinsCollected() const;
    bool doubleJumpAvailable() const;
    double invulnerabilityRemaining() const;

protected:
    void initialize() override;
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
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
        bool doubleJumpAvailable; // Czy dostępny jest dodatkowy skok w powietrzu
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

    struct Enemy {
        QRectF rect;
        enum Type { GOOMBA, PIRANHA_PLANT } type;
        double phase;
        bool alive;
    };

    struct Collectible {
        QRectF rect;
        enum Type { COIN, STAR } type;
        double phase;
    };

    // Inicjalizacja gracza
    void initPlayer();

    // Aktualizacja fizyki gracza
    void updatePlayer(double deltaTime);

    // Generowanie przeszkód
    void generateObstacles(double deltaTime);

    // Aktualizacja przeszkód
    void updateObstacles(double deltaTime);
    void updateCollectibles(double deltaTime);
    void generateEnemies(double deltaTime);
    void updateEnemies(double deltaTime);
    void renderEnemies(QPainter& painter);
    void renderClouds(QPainter& painter);
    void renderParallaxBackground(QPainter& painter);
    void renderCollectibles(QPainter& painter);
    void launchJump(double force);
    void applyDamage();

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
    static constexpr int STARTING_LIVES = 3;
    static constexpr double HIT_PROTECTION_TIME = 1.6;
    static constexpr double STAR_POWER_TIME = 5.0;
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
    QVector<Collectible> m_collectibles;
    QVector<Enemy> m_enemies;
    double m_enemySpawnTimer = 0.0;
    QVector<Cloud> m_clouds;
    
    // Poziomy
    int m_currentLevel;
    int m_obstaclesPassed;
    int m_obstaclesToNextLevel;
    int m_lives = STARTING_LIVES;
    int m_coinsCollected = 0;
    int m_coinStreak = 0;
    double m_invulnerabilityRemaining = 0.0;
    bool m_keyboardJumpHeld = false;
    
    // Czas od ostatniego wygenerowania przeszkody
    double m_timeSinceLastObstacle;
    double m_obstacleSpawnInterval;
    
    // Tensometr
    double m_sensorThreshold;      // Próg aktywacji (skala 0–100)
    bool m_sensorPressed = false;  // Histereza wejścia tensometru
    
    // Komunikacja szeregowa z Arduino Nano HX711
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    bool m_ownSerial;  // Czy gra jest właścicielem obiektu SerialCommunication
    
public:
    /**
     * @brief Podłącza obiekt SerialCommunication do gry
     * @param serial Współdzielony wskaźnik do obiektu SerialCommunication
     */
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    
    /**
     * @brief Sprawdza czy połączenie szeregowe jest aktywne
     * @return true jeśli Arduino Nano z HX711 jest podłączone
     */
    bool isSerialConnected() const;
    
private:
    /**
     * @brief Odłącza się od sensora
     */
    void cleanupSensorConnection();
};

} // namespace games

#endif // GAMES_MARIOGAME_HPP
