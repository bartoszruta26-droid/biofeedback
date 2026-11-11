#ifndef GAMES_TANGAME_HPP
#define GAMES_TANGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <memory>

namespace games {

class TanGame final : public GameEngine
{
    Q_OBJECT

public:
    explicit TanGame(QWidget *parent = nullptr);
    ~TanGame() override = default;

    QString gameName() const override;

    void setDifficultyLevel(int level);
    int getDifficultyLevel() const;

    // Public for parity with SinGame and deterministic regression coverage.
    void update(double deltaTime) override;

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

protected:
    void initialize() override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

private:
    void calculateTargetY(double x);
    QColor getDifficultyColor() const;

    double m_currentForce;
    double m_targetY;
    double m_phase;
    double m_cursorY;
    bool m_isHit;

    int m_difficultyLevel;
    double m_scrollSpeed; // piksele na sekundę
    double m_tolerance;   // tolerancja trafienia w pikselach
    double m_frequency;   // radiany na piksel

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;

    static constexpr double MIN_FORCE = 0.0;
    static constexpr double MAX_FORCE = 100.0;
};

} // namespace games

#endif // GAMES_TANGAME_HPP
