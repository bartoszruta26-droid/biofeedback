#ifndef GAMES_SINGAME_HPP
#define GAMES_SINGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPointF>
#include <QVector>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QString>
#include <memory>

namespace games {

class SinGame : public GameEngine {
    Q_OBJECT

public:
    explicit SinGame(QWidget *parent = nullptr);
    ~SinGame() override;

    QString gameName() const override;
    void initialize() override;
    void update(double deltaTime) override;
    void render(QPainter& painter) override;
    bool handleInput(double value) override;

    void setDifficultyLevel(int level);
    int getDifficultyLevel() const;

    // Input calibration is expressed in the same units supplied by TrainingTab.
    void setForceCalibration(double minimum, double maximum);
    double forceCalibrationMinimum() const;
    double forceCalibrationMaximum() const;

    // 0 = sinusoidal, 1 = triangular, 2 = smooth plateau.
    void setWaveformMode(int mode);
    int waveformMode() const;

    void setAutomaticDifficulty(bool enabled);
    bool automaticDifficulty() const;

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

signals:
    void serialError(const QString& message);
    void serialConnected();
    void serialDisconnected();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onSensorDataReceived(const sensor::SensorData& data);
    void onConnectionStatusChanged(bool connected, const std::string& message);

private:
    double m_currentForce;       // Normalized exercise force, 0.0..1.0
    double m_targetY;
    double m_phase;
    double m_cursorY;
    bool m_isHit;

    int m_difficultyLevel;
    double m_scrollSpeed;        // Wave scroll speed in pixels per second
    double m_tolerance;          // Target-band half-width in pixels
    double m_frequency;          // Wave spatial frequency, radians per pixel
    double m_inputAge;           // Seconds since the last valid sensor sample
    double m_scoreFraction;      // Fractional points accumulated between frames
    double m_comboSeconds;
    int m_combo;
    double m_lastDistance;
    QString m_feedbackText;
    double m_calibrationMinimum;
    double m_calibrationMaximum;
    int m_waveformMode;
    bool m_automaticDifficulty;
    double m_sessionSeconds;
    double m_timeInTarget;
    double m_difficultyCheckSeconds;
    double m_accuracyWindowSeconds;
    double m_accuracyWindowHitSeconds;
    double m_bestStreakSeconds;

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    bool m_ownSerial;

    const double m_minForce = 0.0;
    const double m_maxForce = 100.0; // TrainingTab sends 0..100% of target force.

    void calculateTargetY(double x);
    QColor getDifficultyColor() const;
    void normalizeForceValue(double rawValue);
    double waveformValue(double phase) const;
    void refreshGeometryParameters();
    void updateAutomaticDifficulty(double deltaTime);
};

} // namespace games

#endif // GAMES_SINGAME_HPP
