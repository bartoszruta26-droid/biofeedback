#ifndef GAMES_SINGAME_HPP
#define GAMES_SINGAME_HPP

#include "games/GameEngine.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QPointF>
#include <QVector>
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

    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);
    void disconnectSerial();
    bool isSerialConnected() const;

signals:
    void serialError(const QString& message);
    void serialConnected();
    void serialDisconnected();

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

    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    bool m_ownSerial;

    const double m_minForce = 0.0;
    const double m_maxForce = 100.0; // TrainingTab sends 0..100% of target force.

    void calculateTargetY(double x);
    QColor getDifficultyColor() const;
    void normalizeForceValue(double rawValue);
};

} // namespace games

#endif // GAMES_SINGAME_HPP
