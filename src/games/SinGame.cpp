#include "games/SinGame.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QMath>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>

namespace games {

SinGame::SinGame(QWidget *parent)
    : GameEngine(parent)
    , m_currentForce(0.0)
    , m_targetY(0.0)
    , m_phase(0.0)
    , m_cursorY(0.0)
    , m_isHit(false)
    , m_difficultyLevel(1)
    , m_scrollSpeed(100.0) // piksele na sekundę
    , m_tolerance(40.0)    // piksele tolerancji
    , m_frequency(0.02)
    , m_serialPort(nullptr)
    , m_ownSerial(false)
{
}

SinGame::~SinGame() {
    disconnectSerial();
}

QString SinGame::gameName() const {
    return "Sinus Force Trainer";
}

void SinGame::initialize() {
    setDifficultyLevel(1); // Domyślny poziom
    m_currentForce = 0.0;
    m_cursorY = height() / 2.0;
    m_phase = 0.0;
    m_isHit = false;
    
    // Jeśli mamy podłączonego seriala, upewnijmy się że callbacki są ustawione
    if (m_serialPort) {
        m_serialPort->setDataCallback([this](const sensor::SensorData& data) {
            // Wywołanie slotu w wątku głównym Qt
            QMetaObject::invokeMethod(this, "onSensorDataReceived", 
                                     Qt::QueuedConnection,
                                     Q_ARG(sensor::SensorData, data));
        });
        
        m_serialPort->setConnectionCallback([this](bool connected, const std::string& message) {
            QMetaObject::invokeMethod(this, "onConnectionStatusChanged",
                                     Qt::QueuedConnection,
                                     Q_ARG(bool, connected),
                                     Q_ARG(std::string, message));
        });
    }
}

void SinGame::setDifficultyLevel(int level) {
    m_difficultyLevel = qBound(1, level, 5);
    
    // Skalowanie trudności
    // Poziom 1: wolno, duża tolerancja
    // Poziom 5: szybko, mała tolerancja
    m_scrollSpeed = 80.0 + (m_difficultyLevel * 40.0); 
    m_tolerance = qMax(10.0, 50.0 - (m_difficultyLevel * 8.0)); 
    m_frequency = 0.015 + (m_difficultyLevel * 0.005);
    
    emit scoreChanged(0); // Reset punktów przy zmianie poziomu
}

int SinGame::getDifficultyLevel() const {
    return m_difficultyLevel;
}

void SinGame::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial) {
    // Rozłącz poprzednie jeśli istnieje i byliśmy właścicielem
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
    }
    
    m_serialPort = serial;
    m_ownSerial = false; // Nie jesteśmy właścicielem, tylko użytkownikiem
    
    if (m_serialPort && m_serialPort->isConnected()) {
        emit serialConnected();
    }
}

void SinGame::disconnectSerial() {
    if (m_ownSerial && m_serialPort) {
        m_serialPort->stopAsyncReading();
        m_serialPort->disconnect();
        m_serialPort = nullptr;
        m_ownSerial = false;
        emit serialDisconnected();
    } else if (m_serialPort) {
        // Tylko wyczyść wskaźnik, nie rozłączaj bo nie jesteśmy właścicielem
        m_serialPort = nullptr;
        emit serialDisconnected();
    }
}

bool SinGame::isSerialConnected() const {
    return m_serialPort && m_serialPort->isConnected();
}

void SinGame::onSensorDataReceived(const sensor::SensorData& data) {
    if (!data.isValid) {
        return;
    }
    
    // Użyj skalibrowanej wartości z sensora
    normalizeForceValue(data.calibratedValue);
}

void SinGame::onConnectionStatusChanged(bool connected, const std::string& message) {
    if (connected) {
        qDebug() << "SinGame: Połączono z sensorem:" << QString::fromStdString(message);
        emit serialConnected();
    } else {
        qDebug() << "SinGame: Rozłączono z sensorem:" << QString::fromStdString(message);
        emit serialError(QString::fromStdString("Rozłączono: " + message));
        emit serialDisconnected();
    }
}

void SinGame::normalizeForceValue(double rawValue) {
    // Normalizacja siły do zakresu 0.0 - 1.0
    // Zakładamy, że rawValue jest w zakresie 0-m_maxForce (np. 0-100)
    double normalizedForce = rawValue / m_maxForce;
    m_currentForce = qBound(0.0, normalizedForce, 1.0);
}

void SinGame::handleInput(double value) {
    // Ta metoda jest wywoływana przez GameEngine::handleSensorInput
    // Wartość jest już przekazana przez callback seriala, ale obsługujemy też ręczne wejście
    normalizeForceValue(value);
}

void SinGame::calculateTargetY(double x) {
    // Obliczamy Y sinusa dla danej pozycji X względem fazy
    // Środek ekranu to 0, góra to -height/2, dół to +height/2
    double amplitude = height() * 0.35; // Amplituda zajmuje 70% wysokości
    double centerY = height() / 2.0;
    
    m_targetY = centerY + std::sin(x * m_frequency + m_phase) * amplitude;
}

void SinGame::update(double deltaTime) {
    if (!isRunning()) return;

    // Aktualizacja fazy sinusa (ruch wykresu w lewo)
    m_phase -= m_scrollSpeed * deltaTime;
    
    // Pozycja X "bramki" sprawdzającej trafienie (stała w prawo od środka)
    // Gracz celuje w linię pionową na 75% szerokości ekranu
    double checkX = width() * 0.75;
    
    // Oblicz gdzie powinien być sinus w punkcie sprawdzania
    calculateTargetY(checkX);
    
    // Oblicz pozycję kursora gracza na podstawie siły
    // Siła 0 -> dół ekranu, Siła 1 -> góra ekranu (naciąganie taśmy podnosi punkt)
    double cursorY = height() - (m_currentForce * height());
    
    // Prosty filtr wygładzający dla kursora (żeby nie skakał)
    m_cursorY = m_cursorY * 0.8 + cursorY * 0.2;
    
    // Detekcja kolizji
    // Sprawdzamy czy kursor jest w pobliżu linii sinusa w punkcie checkX
    double distance = std::abs(m_cursorY - m_targetY);
    
    if (distance < m_tolerance) {
        if (!m_isHit) {
            m_isHit = true;
            // Punktacja: im mniejszy dystans, tym więcej punktów
            int points = static_cast<int>((m_tolerance - distance) * 10);
            addScore(points);
        }
    } else {
        m_isHit = false;
    }
}

void SinGame::render(QPainter &painter) {
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width();
    int h = height();
    double centerY = h / 2.0;
    double amplitude = h * 0.35;
    
    // 1. Tło
    painter.fillRect(rect(), QColor(20, 20, 30));
    
    // 2. Rysowanie Sinusoidy
    QPen sinPen(getDifficultyColor(), 3);
    painter.setPen(sinPen);
    QPainterPath sinPath;
    
    bool first = true;
    for (int x = 0; x <= w; ++x) {
        // Odwracamy logikę fazowania, żeby rysować całą linię poprawnie
        double val = std::sin(x * m_frequency + m_phase);
        double y = centerY + val * amplitude;
        
        if (first) {
            sinPath.moveTo(x, y);
            first = false;
        } else {
            sinPath.lineTo(x, y);
        }
    }
    painter.drawPath(sinPath);
    
    // 3. Strefa celu (wyznacznik gdzie celować)
    double checkX = w * 0.75;
    painter.setPen(QPen(Qt::white, 1, Qt::DashLine));
    painter.drawLine(checkX, 0, checkX, h);
    
    // Tekst informacyjny
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(10, 20, "Celuj w linię przerywaną");
    painter.drawText(10, 40, QString("Siła: %1%").arg(static_cast<int>(m_currentForce * 100)));
    painter.drawText(10, 60, QString("Tolerancja: %1 px").arg(static_cast<int>(m_tolerance)));
    painter.drawText(10, 80, QString("Poziom: %1").arg(m_difficultyLevel));
    
    // 4. Rysowanie Kursora Gracza
    // Kursor znajduje się na linii checkX, ale na wysokości m_cursorY
    double currentCursorY = m_cursorY;
    
    // Wizualizacja kursora (kółko)
    painter.setBrush(QBrush(Qt::cyan));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(checkX, currentCursorY), 10, 10);
    
    // Wizualizacja strefy trafienia (jeśli blisko)
    if (m_isHit) {
        painter.setBrush(QBrush(Qt::green));
        painter.drawEllipse(QPointF(checkX, m_targetY), 15, 15);
        
        painter.setPen(Qt::green);
        painter.drawText(checkX + 20, currentCursorY, "TRAFFIENIE!");
    } else {
        // Pokaż gdzie jest cel (małe kółko na sinusoide w miejscu przecięcia)
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(QPointF(checkX, m_targetY), 5, 5);
    }
    
    // 5. Pasek siły (UI pomocnicze)
    int barW = 20;
    int barH = h - 40;
    int barX = w - 30;
    int barY = 20;
    
    painter.setPen(Qt::white);
    painter.drawRect(barX, barY, barW, barH);
    
    // Wypełnienie paska
    int fillH = static_cast<int>(m_currentForce * barH);
    painter.setBrush(QBrush(Qt::yellow));
    painter.drawRect(barX, barY + barH - fillH, barW, fillH);
    
    // 6. Status połączenia serial
    if (m_serialPort && !m_serialPort->isConnected()) {
        painter.setPen(QPen(Qt::red, 2));
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.drawText(w/2 - 100, 30, "BRAK POŁĄCZENIA Z SENSOREM");
    }
}

QColor SinGame::getDifficultyColor() const {
    switch (m_difficultyLevel) {
        case 1: return Qt::green;
        case 2: return Qt::cyan;
        case 3: return Qt::yellow;
        case 4: return Qt::orange;
        default: return Qt::red;
    }
}

} // namespace games
