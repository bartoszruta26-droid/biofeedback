#ifndef GAMES_TANGAME_HPP
#define GAMES_TANGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QString>
#include <memory>

class QResizeEvent;

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

    void setForceCalibration(double minimum, double maximum);
    double forceCalibrationMinimum() const;
    double forceCalibrationMaximum() const;
    double normalizedForce() const;
    void setAutomaticDifficulty(bool enabled);
    bool automaticDifficulty() const;
    void setTangentSharpness(double sharpness);
    double tangentSharpness() const;
    void update(double deltaTime) override;

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

protected:
    void initialize() override;
    void resizeEvent(QResizeEvent* event) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

private:
    void calculateTargetY(double x);
    void updateSpatialFrequency();
    void updateAutomaticDifficulty(double deltaTime);
    QColor getDifficultyColor() const;

    double m_currentForce;
    double m_calibrationMinimum;
    double m_calibrationMaximum;
    double m_lastRawForce;
    double m_tangentSharpness;
    double m_targetY;
    double m_phase;
    double m_cursorY;
    bool m_isHit;
    bool m_nearAsymptote;

    int m_difficultyLevel;
    double m_scrollSpeed;
    double m_tolerance;
    double m_frequency;
    double m_inputAge;
    double m_scoreFraction;
    double m_trackingSeconds;
    double m_sessionSeconds;
    double m_timeInTarget;
    double m_accuracyWindowSeconds;
    double m_accuracyWindowHitSeconds;
    bool m_automaticDifficulty;
    int m_combo;
    QString m_feedbackText;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;

};

} // namespace games

#endif // GAMES_TANGAME_HPP
