#ifndef TAB_DEBUGTAB_HPP
#define TAB_DEBUGTAB_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QFont>
#include <QScrollBar>
#include <memory>
#include <mutex>
#include "sensor/SerialCommunication.hpp"

namespace tab {

/**
 * @class DebugTab
 * @brief Osobna zakładka do zbierania informacji debugowych w tle
 * 
 * DebugTab umożliwia:
 * - Podgląd danych z Arduino Nano z HX711 w czasie rzeczywistym
 * - Logowanie zdarzeń systemowych
 * - Monitorowanie połączenia szeregowego
 * - Historię wiadomości z możliwością filtrowania
 * - Eksport logów do pliku
 */
class DebugTab : public QWidget
{
    Q_OBJECT

public:
    explicit DebugTab(QWidget *parent = nullptr);
    ~DebugTab() override;

    /**
     * @brief Inicjalizuje terminal debugowania
     */
    void setupDebugTerminal();

    /**
     * @brief Dodaje wiadomość do terminala debugowania
     * @param message Treść wiadomości
     * @param type Typ wiadomości (INFO, ERROR, WARNING, DATA, SERIAL, ARDUINO)
     */
    void addDebugMessage(const QString& message, const QString& type = "INFO");

    /**
     * @brief Czyści terminal debugowania
     */
    void clearTerminal();

    /**
     * @brief Eksportuje logi do pliku tekstowego
     * @param filename Ścieżka do pliku
     * @return true jeśli eksport się powiódł
     */
    bool exportLogsToFile(const QString& filename);

    /**
     * @brief Ustawia połączenie szeregowe z Arduino
     * @param serial Połączenie szeregowe
     */
    void setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial);

    /**
     * @brief Sprawdza czy Arduino jest podłączone
     * @return true jeśli Arduino Nano z HX711 jest podłączone
     */
    bool isArduinoConnected() const;

    /**
     * @brief Zwraca liczbę wiadomości w logu
     */
    int getMessageCount() const;

    /**
     * @brief Ustawia maksymalną liczbę linii w terminalu
     */
    void setMaxLines(int maxLines);

signals:
    void arduinoDataReceived(const sensor::SensorData& data);
    void debugMessageAdded(const QString& message, const QString& type);
    void connectionStatusChanged(bool connected);

public slots:
    void onSensorDataReceived(const sensor::SensorData& data);
    void onConnectionStatusChanged(bool connected, const QString& message);

private slots:
    void onClearButtonClicked();
    void onExportButtonClicked();
    void onAutoScrollToggled(bool checked);
    void onFilterTypeChanged(int index);

private:
    void setupUI();
    void updateConnectionStatus(bool connected);
    QString formatMessageType(const QString& type) const;
    QColor getMessageColor(const QString& type) const;

    // Komponenty UI
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_controlLayout;
    
    QPushButton* m_btnClear;
    QPushButton* m_btnExport;
    QCheckBox* m_chkAutoScroll;
    QLabel* m_lblConnectionStatus;
    QLabel* m_lblMessageCount;
    
    QTextEdit* m_debugTerminal;
    
    QGroupBox* m_filterBox;
    QHBoxLayout* m_filterLayout;
    QLabel* m_lblFilter;
    QComboBox* m_filterCombo;
    
    QGroupBox* m_arduinoStatusBox;
    QVBoxLayout* m_arduinoStatusLayout;
    QLabel* m_lblPortName;
    QLabel* m_lblBaudRate;
    QLabel* m_lblLastDataTime;
    QLabel* m_lblRawValue;
    QLabel* m_lblCalibratedValue;
    
    // Dane
    std::shared_ptr<sensor::SerialCommunication> m_serialPort;
    sensor::SensorData m_lastSensorData;
    bool m_hasArduinoConnection;
    bool m_autoScroll;
    int m_maxLines;
    int m_messageCount;
    
    QString m_currentPort;
    int m_baudRate;
    QDateTime m_lastDataTime;
    
    std::mutex m_messageMutex;
};

} // namespace tab

#endif // TAB_DEBUGTAB_HPP
