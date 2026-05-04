#include "tab/DebugTab.hpp"
#include "core/DebugManager.hpp"
#include "sensor/SerialCommunication.hpp"
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QFont>
#include <QScrollBar>
#include <QMessageBox>
#include <QThread>
#include <iostream>
#include <mutex>

namespace tab {

DebugTab::DebugTab(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_btnClear(nullptr)
    , m_btnExport(nullptr)
    , m_chkAutoScroll(nullptr)
    , m_lblConnectionStatus(nullptr)
    , m_lblMessageCount(nullptr)
    , m_debugTerminal(nullptr)
    , m_filterBox(nullptr)
    , m_filterLayout(nullptr)
    , m_lblFilter(nullptr)
    , m_filterCombo(nullptr)
    , m_arduinoStatusBox(nullptr)
    , m_arduinoStatusLayout(nullptr)
    , m_lblPortName(nullptr)
    , m_lblBaudRate(nullptr)
    , m_lblLastDataTime(nullptr)
    , m_lblRawValue(nullptr)
    , m_lblCalibratedValue(nullptr)
    , m_serialPort(nullptr)
    , m_hasArduinoConnection(false)
    , m_autoScroll(true)
    , m_maxLines(1000)
    , m_messageCount(0)
    , m_baudRate(9600)
{
    setupUI();
    setupDebugTerminal();
    registerAsDebugReceiver();
}

DebugTab::~DebugTab() override
{
    // Odłącz callback przy niszczeniu
    core::DebugManager::instance().unregisterDebugCallback();
}

void DebugTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // ============================================
    // Panel kontrolny (góra)
    // ============================================
    m_controlLayout = new QHBoxLayout();
    
    m_btnClear = new QPushButton(tr("🗑️ Wyczyść"), this);
    m_btnClear->setMaximumWidth(120);
    connect(m_btnClear, &QPushButton::clicked, this, &DebugTab::onClearButtonClicked);
    
    m_btnExport = new QPushButton(tr("💾 Eksportuj logi"), this);
    m_btnExport->setMaximumWidth(150);
    connect(m_btnExport, &QPushButton::clicked, this, &DebugTab::onExportButtonClicked);
    
    m_chkAutoScroll = new QCheckBox(tr("📜 Auto-przewijanie"), this);
    m_chkAutoScroll->setChecked(m_autoScroll);
    connect(m_chkAutoScroll, &QCheckBox::toggled, this, &DebugTab::onAutoScrollToggled);
    
    m_lblConnectionStatus = new QLabel(tr("🔴 Rozłączony"), this);
    m_lblConnectionStatus->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    
    m_lblMessageCount = new QLabel(tr("Wiadomości: 0"), this);
    
    m_controlLayout->addWidget(m_btnClear);
    m_controlLayout->addWidget(m_btnExport);
    m_controlLayout->addWidget(m_chkAutoScroll);
    m_controlLayout->addStretch();
    m_controlLayout->addWidget(m_lblConnectionStatus);
    m_controlLayout->addWidget(m_lblMessageCount);
    
    m_mainLayout->addLayout(m_controlLayout);
    
    // ============================================
    // Filtr wiadomości
    // ============================================
    m_filterBox = new QGroupBox(tr("🔍 Filtr wiadomości"), this);
    m_filterLayout = new QHBoxLayout();
    
    m_lblFilter = new QLabel(tr("Pokaż:"), this);
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem("Wszystkie", "ALL");
    m_filterCombo->addItem("INFO", "INFO");
    m_filterCombo->addItem("ERROR", "ERROR");
    m_filterCombo->addItem("WARNING", "WARNING");
    m_filterCombo->addItem("DATA", "DATA");
    m_filterCombo->addItem("SERIAL", "SERIAL");
    m_filterCombo->addItem("ARDUINO", "ARDUINO");
    
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &DebugTab::onFilterTypeChanged);
    
    m_filterLayout->addWidget(m_lblFilter);
    m_filterLayout->addWidget(m_filterCombo);
    m_filterLayout->addStretch();
    m_filterBox->setLayout(m_filterLayout);
    
    m_mainLayout->addWidget(m_filterBox);
    
    // ============================================
    // Terminal debugowania (główna część)
    // ============================================
    m_debugTerminal = new QTextEdit(this);
    m_debugTerminal->setReadOnly(true);
    m_debugTerminal->setFont(QFont("Courier New", 9));
    m_debugTerminal->setStyleSheet(
        "QTextEdit { "
        "    background-color: #1e1e1e; "
        "    color: #d4d4d4; "
        "    border: 1px solid #3e3e3e; "
        "    border-radius: 5px; "
        "} "
        "QTextEdit:focus { "
        "    border: 1px solid #5e5e5e; "
        "}"
    );
    m_debugTerminal->setPlaceholderText(tr("Terminal debugowania - dane z Arduino Nano z HX711 i zdarzenia systemowe"));
    
    m_mainLayout->addWidget(m_debugTerminal, 1); // Stretch factor 1
    
    // ============================================
    // Status Arduino (dół)
    // ============================================
    m_arduinoStatusBox = new QGroupBox(tr("📡 Status Arduino Nano z HX711"), this);
    m_arduinoStatusLayout = new QVBoxLayout();
    
    m_lblPortName = new QLabel(tr("Port: ---"), this);
    m_lblBaudRate = new QLabel(tr("Baud rate: ---"), this);
    m_lblLastDataTime = new QLabel(tr("Ostatnie dane: ---"), this);
    m_lblRawValue = new QLabel(tr("Wartość surowa: ---"), this);
    m_lblCalibratedValue = new QLabel(tr("Wartość skalibrowana: ---"), this);
    
    m_arduinoStatusLayout->addWidget(m_lblPortName);
    m_arduinoStatusLayout->addWidget(m_lblBaudRate);
    m_arduinoStatusLayout->addWidget(m_lblLastDataTime);
    m_arduinoStatusLayout->addWidget(m_lblRawValue);
    m_arduinoStatusLayout->addWidget(m_lblCalibratedValue);
    m_arduinoStatusBox->setLayout(m_arduinoStatusLayout);
    
    m_mainLayout->addWidget(m_arduinoStatusBox);
    
    setLayout(m_mainLayout);
}

void DebugTab::setupDebugTerminal()
{
    addDebugMessage("=== Debug Terminal Tab Initialized ===", "INFO");
    addDebugMessage(QString("Debug tab opened at %1").arg(QDateTime::currentDateTime().toString()), "INFO");
    addDebugMessage("Oczekiwanie na dane z Arduino Nano z HX711...", "SERIAL");
}

void DebugTab::addDebugMessage(const QString& message, const QString& type)
{
    std::lock_guard<std::mutex> lock(m_messageMutex);
    
    if (!m_debugTerminal) return;
    
    // Sprawdź filtr
    QString currentFilter = m_filterCombo->currentData().toString();
    if (currentFilter != "ALL" && currentFilter != type) {
        // Wiadomość nie spełnia filtra, ale nadal ją dodajemy do licznika
        m_messageCount++;
        m_lblMessageCount->setText(QString("Wiadomości: %1").arg(m_messageCount));
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString formattedMessage;
    
    // Kolorowanie w zależności od typu wiadomości
    if (type == "ERROR") {
        formattedMessage = QString("<span style=\"color: #ff6b6b; font-weight: bold;\">[%1] [%2] %3</span><br>").arg(timestamp, type, message);
    } else if (type == "WARNING") {
        formattedMessage = QString("<span style=\"color: #ffd93d; font-weight: bold;\">[%1] [%2] %3</span><br>").arg(timestamp, type, message);
    } else if (type == "DATA") {
        formattedMessage = QString("<span style=\"color: #6bcb77;\">[%1] [DATA] %2</span><br>").arg(timestamp, message);
    } else if (type == "SERIAL") {
        formattedMessage = QString("<span style=\"color: #4d96ff;\">[%1] [SERIAL] %2</span><br>").arg(timestamp, message);
    } else if (type == "ARDUINO") {
        formattedMessage = QString("<span style=\"color: #ff9f43; font-weight: bold;\">[%1] [ARDUINO] %2</span><br>").arg(timestamp, message);
    } else {
        formattedMessage = QString("<span style=\"color: #c4c4c4;\">[%1] [INFO] %2</span><br>").arg(timestamp, message);
    }
    
    m_debugTerminal->append(formattedMessage);
    
    // Ogranicz liczbę linii
    QStringList lines = m_debugTerminal->toPlainText().split("\n");
    while (lines.size() > m_maxLines) {
        lines.removeFirst();
    }
    
    // Wyczyść i przywróć ograniczoną zawartość
    m_debugTerminal->clear();
    m_debugTerminal->append(lines.join("\n"));
    
    // Przewiń do ostatniej linii jeśli auto-scroll jest włączony
    if (m_autoScroll) {
        m_debugTerminal->verticalScrollBar()->setValue(m_debugTerminal->verticalScrollBar()->maximum());
    }
    
    m_messageCount++;
    m_lblMessageCount->setText(QString("Wiadomości: %1").arg(m_messageCount));
    
    emit debugMessageAdded(message, type);
}

void DebugTab::clearTerminal()
{
    std::lock_guard<std::mutex> lock(m_messageMutex);
    
    if (m_debugTerminal) {
        m_debugTerminal->clear();
        m_messageCount = 0;
        m_lblMessageCount->setText("Wiadomości: 0");
        addDebugMessage("Terminal wyczyszczony", "INFO");
    }
}

bool DebugTab::exportLogsToFile(const QString& filename)
{
    if (!m_debugTerminal) return false;
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Błąd eksportu"), 
                             tr("Nie można zapisać pliku: %1").arg(file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    out << "=== Debug Log Export ===\n";
    out << "Date: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    out << "Total messages: " << m_messageCount << "\n\n";
    out << m_debugTerminal->toPlainText();
    
    file.close();
    
    addDebugMessage(QString("Wyeksportowano logi do: %1").arg(filename), "INFO");
    return true;
}

void DebugTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    m_serialPort = serial;
    
    if (m_serialPort) {
        // Połącz sygnały SerialCommunication z slotami DebugTab
        connect(m_serialPort.get(), &sensor::SerialCommunication::sensorDataReady,
                this, &DebugTab::onSensorDataReceived, Qt::QueuedConnection);
        
        connect(m_serialPort.get(), &sensor::SerialCommunication::connectionStatusChanged,
                this, &DebugTab::onConnectionStatusChanged, Qt::QueuedConnection);
        
        addDebugMessage("Połączenie z SerialCommunication nawiązane", "SERIAL");
    }
}

bool DebugTab::isArduinoConnected() const
{
    return m_hasArduinoConnection;
}

int DebugTab::getMessageCount() const
{
    return m_messageCount;
}

void DebugTab::setMaxLines(int maxLines)
{
    m_maxLines = maxLines;
}

void DebugTab::registerAsDebugReceiver()
{
    // Rejestrujemy lambda jako callback do DebugManager
    // Dzięki temu wszystkie wiadomości z innych zakładek trafią do DebugTab
    auto debugCallback = [this](const QString& message, const QString& type) {
        // Wywołujemy addDebugMessage w głównym wątku GUI
        if (QThread::currentThread() != this->thread()) {
            QMetaObject::invokeMethod(this, "addDebugMessage", 
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, message), 
                                      Q_ARG(QString, type));
        } else {
            addDebugMessage(message, type);
        }
    };
    
    core::DebugManager::instance().registerDebugCallback(debugCallback);
    addDebugMessage("DebugTab registered as central debug receiver", "INFO");
}

void DebugTab::onSensorDataReceived(const sensor::SensorData& data)
{
    if (data.isValid) {
        m_lastSensorData = data;
        m_lastDataTime = QDateTime::currentDateTime();
        
        // Aktualizuj UI statusu Arduino
        updateConnectionStatus(true);
        
        // Dodaj wiadomość do terminala z danymi z Arduino
        QString arduinoMsg = QString("RAW=%1 | CALIBRATED=%.2fN | Timestamp=%2")
            .arg(data.value)
            .arg(data.calibratedValue)
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
        
        addDebugMessage(arduinoMsg, "ARDUINO");
        
        // Emituj sygnał dla innych komponentów
        emit arduinoDataReceived(data);
    } else {
        addDebugMessage("Otrzymano nieprawidłowe dane z Arduino", "WARNING");
    }
}

void DebugTab::onConnectionStatusChanged(bool connected, const QString& message)
{
    m_hasArduinoConnection = connected;
    
    if (connected) {
        m_currentPort = m_serialPort ? m_serialPort->getPortName() : "Unknown";
        m_baudRate = m_serialPort ? m_serialPort->getBaudRate() : 0;
        
        m_lblPortName->setText(QString("Port: %1").arg(m_currentPort));
        m_lblBaudRate->setText(QString("Baud rate: %1").arg(m_baudRate));
        
        addDebugMessage(QString("Połączono z Arduino: %1 @ %2 baud").arg(m_currentPort).arg(m_baudRate), "SERIAL");
        addDebugMessage(message, "INFO");
    } else {
        m_lblPortName->setText("Port: ---");
        m_lblBaudRate->setText("Baud rate: ---");
        m_lblLastDataTime->setText("Ostatnie dane: ---");
        m_lblRawValue->setText("Wartość surowa: ---");
        m_lblCalibratedValue->setText("Wartość skalibrowana: ---");
        
        addDebugMessage(QString("Rozłączono z Arduino: %1").arg(message), "WARNING");
    }
    
    updateConnectionStatus(connected);
    emit connectionStatusChanged(connected);
}

void DebugTab::onClearButtonClicked()
{
    clearTerminal();
}

void DebugTab::onExportButtonClicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        tr("Eksportuj logi debugowania"),
        QString(),
        tr("Pliki tekstowe (*.txt);;Wszystkie pliki (*)")
    );
    
    if (!filename.isEmpty()) {
        exportLogsToFile(filename);
    }
}

void DebugTab::onAutoScrollToggled(bool checked)
{
    m_autoScroll = checked;
    addDebugMessage(QString("Auto-przewijanie %1").arg(checked ? "włączone" : "wyłączone"), "INFO");
}

void DebugTab::onFilterTypeChanged(int /*index*/)
{
    QString filter = m_filterCombo->currentData().toString();
    addDebugMessage(QString("Filtr ustawiony na: %1").arg(filter), "INFO");
    
    // Odśwież widok terminala
    // (wiadomości są filtrowane w addDebugMessage)
}

void DebugTab::updateConnectionStatus(bool connected)
{
    if (connected) {
        m_lblConnectionStatus->setText(tr("🟢 Połączony"));
        m_lblConnectionStatus->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        
        // Aktualizuj ostatnie dane
        if (m_lastSensorData.isValid) {
            m_lblLastDataTime->setText(QString("Ostatnie dane: %1").arg(m_lastDataTime.toString("HH:mm:ss")));
            m_lblRawValue->setText(QString("Wartość surowa: %1").arg(m_lastSensorData.value));
            m_lblCalibratedValue->setText(QString("Wartość skalibrowana: %.2f N").arg(m_lastSensorData.calibratedValue));
        }
    } else {
        m_lblConnectionStatus->setText(tr("🔴 Rozłączony"));
        m_lblConnectionStatus->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
}

QString DebugTab::formatMessageType(const QString& type) const
{
    return QString("[%1]").arg(type);
}

QColor DebugTab::getMessageColor(const QString& type) const
{
    if (type == "ERROR") return QColor(255, 107, 107);
    if (type == "WARNING") return QColor(255, 217, 61);
    if (type == "DATA") return QColor(107, 203, 119);
    if (type == "SERIAL") return QColor(77, 150, 255);
    if (type == "ARDUINO") return QColor(255, 159, 67);
    return QColor(196, 196, 196);
}

} // namespace tab
