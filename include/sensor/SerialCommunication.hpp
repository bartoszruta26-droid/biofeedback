#ifndef SENSOR_SERIALCOMMUNICATION_HPP
#define SENSOR_SERIALCOMMUNICATION_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <functional>
#include <memory>
#include <map>

namespace sensor {

/**
 * @brief Struktura reprezentująca dane z czujnika
 */
struct SensorData {
    uint32_t timestamp;      ///< Timestamp w milisekundach
    int32_t value;           ///< Wartość surowa z czujnika
    double calibratedValue;  ///< Wartość po kalibracji
    uint8_t crc;             ///< Suma kontrolna CRC
    bool isValid;            ///< Flagi poprawności danych
    
    SensorData() : timestamp(0), value(0), calibratedValue(0.0), crc(0), isValid(false) {}
};

/**
 * @brief Informacje o zidentyfikowanym Arduino
 */
struct ArduinoInfo {
    std::string deviceName;      ///< Nazwa urządzenia (np. "Arduino Nano")
    std::string firmwareVersion; ///< Wersja firmware
    std::string serialNumber;    ///< Numer seryjny (jeśli dostępny)
    uint32_t boardId;            ///< ID płyty
    bool isConnected;            ///< Status połączenia
    
    ArduinoInfo() : boardId(0), isConnected(false) {}
};

/**
 * @brief Konfiguracja portu szeregowego
 */
struct SerialConfig {
    std::string portName;        ///< Nazwa portu (np. "/dev/ttyUSB0", "COM3")
    int baudRate;                ///< Prędkość transmisji (domyślnie 9600)
    int dataBits;                ///< Liczba bitów danych (domyślnie 8)
    char parity;                 ///< Parity ('N', 'E', 'O')
    float stopBits;              ///< Bity stopu (1.0, 1.5, 2.0)
    int timeout;                 ///< Timeout w milisekundach
    
    SerialConfig() : baudRate(9600), dataBits(8), parity('N'), 
                     stopBits(1.0f), timeout(1000) {}
};

/**
 * @brief Klasa obsługująca komunikację szeregową z Arduino Nano
 * 
 * Funkcjonalności:
 * - Komunikacja przez port szeregowy z Arduino
 * - Obliczanie i weryfikacja CRC
 * - Watchdog dla monitorowania połączenia
 * - Obsługa timestampów
 * - Kalibracja wartości z czujnika
 * - Identyfikacja Arduino i portu połączenia
 */
class SerialCommunication {
public:
    /**
     * @brief Typ funkcji callback dla nowych danych
     */
    using DataCallback = std::function<void(const SensorData&)>;
    
    /**
     * @brief Typ funkcji callback dla statusu połączenia
     */
    using ConnectionCallback = std::function<void(bool connected, const std::string& message)>;

    /**
     * @brief Konstruktor
     */
    SerialCommunication();
    
    /**
     * @brief Destruktor
     */
    ~SerialCommunication();

    // ==================== Zarządzanie połączeniem ====================
    
    /**
     * @brief Nawiązuje połączenie z określonym portem
     * @param config Konfiguracja portu szeregowego
     * @return true jeśli połączenie nawiązane pomyślnie
     */
    bool connect(const SerialConfig& config);
    
    /**
     * @brief Nawiązuje połączenie z portem o podanej nazwie i prędkości
     * @param portName Nazwa portu (np. "/dev/ttyUSB0", "COM3")
     * @param baudRate Prędkość transmisji (domyślnie 115200)
     * @return true jeśli połączenie nawiązane pomyślnie
     */
    bool connect(const std::string& portName, int baudRate = DEFAULT_BAUD_RATE);
    
    /**
     * @brief Zamyka połączenie szeregowe
     */
    void disconnect();
    
    /**
     * @brief Sprawdza czy połączenie jest aktywne
     * @return true jeśli połączony
     */
    bool isConnected() const;
    
    /**
     * @brief Pobiera nazwę aktualnego portu
     * @return Nazwa portu lub pusty string jeśli niepołączony
     */
    std::string getPortName() const;

    // ==================== Identyfikacja ====================
    
    /**
     * @brief Skanuje dostępne porty szeregowe w systemie
     * @return Lista dostępnych portów
     */
    static std::vector<std::string> scanAvailablePorts();
    
    /**
     * @brief Identyfikuje urządzenie Arduino na podłączonym porcie
     * @param timeout Timeout w milisekundach na odpowiedź
     * @return Informacje o Arduino
     */
    ArduinoInfo identifyArduino(int timeout = 2000);
    
    /**
     * @brief Automatycznie wykrywa i łączy się z Arduino
     * @param maxAttempts Maksymalna liczba prób na portach
     * @return true jeśli znaleziono i połączono z Arduino
     */
    bool autoConnect(int maxAttempts = 10);
    
    /**
     * @brief Pobiera informacje o zidentyfikowanym Arduino
     * @return Informacje o Arduino
     */
    ArduinoInfo getArduinoInfo() const;

    // ==================== Odczyt danych ====================
    
    /**
     * @brief Odczytuje dane z czujnika (blokujące)
     * @param timeout Timeout w milisekundach
     * @return Dane z czujnika
     */
    SensorData readData(int timeout = 1000);
    
    /**
     * @brief Odczytuje dane z czujnika (nieblokujące)
     * @param data Wyjściowe dane z czujnika
     * @return true jeśli dane zostały odczytane
     */
    bool tryReadData(SensorData& data);
    
    /**
     * @brief Rejestruje callback dla nowych danych
     * @param callback Funkcja wywoływana przy nowych danych
     */
    void setDataCallback(DataCallback callback);
    
    /**
     * @brief Rejestruje callback dla zmian statusu połączenia
     * @param callback Funkcja wywoływana przy zmianie statusu
     */
    void setConnectionCallback(ConnectionCallback callback);
    
    /**
     * @brief Rozpoczyna asynchroniczne czytanie danych
     */
    void startAsyncReading();
    
    /**
     * @brief Zatrzymuje asynchroniczne czytanie danych
     */
    void stopAsyncReading();

    // ==================== CRC ====================
    
    /**
     * @brief Oblicza CRC8 dla danych
     * @param data Bufor danych
     * @param length Długość danych
     * @return Obliczona wartość CRC
     */
    static uint8_t calculateCRC8(const uint8_t* data, size_t length);
    
    /**
     * @brief Weryfikuje CRC dla otrzymanych danych
     * @param data Bufor danych (wraz z CRC na końcu)
     * @param length Długość danych (wraz z CRC)
     * @return true jeśli CRC jest poprawne
     */
    static bool verifyCRC8(const uint8_t* data, size_t length);
    
    /**
     * @brief Ustawia typ algorytmu CRC
     * @param polynomial Wielomian CRC (domyślnie CRC-8-CCITT)
     */
    void setCRCPolynomial(uint8_t polynomial);

    // ==================== Watchdog ====================
    
    /**
     * @brief Konfiguruje watchdog połączenia
     * @param timeoutMs Timeout w milisekundach bez aktywności
     * @param enabled Czy watchdog ma być aktywny
     */
    void configureWatchdog(int timeoutMs, bool enabled = true);
    
    /**
     * @brief Resetuje licznik watchdoga
     */
    void resetWatchdog();
    
    /**
     * @brief Sprawdza status watchdoga
     * @return true jeśli watchdog nie zgłosił timeoutu
     */
    bool isWatchdogOK() const;
    
    /**
     * @brief Pobiera czas od ostatniej aktywności
     * @return Czas w milisekundach
     */
    int getTimeSinceLastActivity() const;

    // ==================== Kalibracja ====================
    
    /**
     * @brief Kalibruje czujnik (tare)
     * @param samples Liczba próbek do uśrednienia
     * @return Wartość tare
     */
    double tare(int samples = 10);
    
    /**
     * @brief Ustawia punkt kalibracyjny
     * @param rawValue Surowa wartość z czujnika
     * @param knownValue Znana wartość wzorcowa
     */
    void setCalibrationPoint(int32_t rawValue, double knownValue);
    
    /**
     * @brief Kalibruje czujnik używając dwóch punktów
     * @param rawZero Surowa wartość dla zera
     * @param rawSpan Surowa wartość dla znanej wagi
     * @param knownWeight Znana waga w jednostkach użytkownika
     * @return true jeśli kalibracja pomyślna
     */
    bool calibrate(int32_t rawZero, int32_t rawSpan, double knownWeight);
    
    /**
     * @brief Konwertuje surową wartość na skalibrowaną
     * @param rawValue Surowa wartość
     * @return Skalibrowana wartość
     */
    double convertToCalibrated(int32_t rawValue) const;
    
    /**
     * @brief Pobiera współczynnik kalibracji
     * @return Współczynnik skalowania
     */
    double getCalibrationScale() const;
    
    /**
     * @brief Pobiera offset kalibracji
     * @return Offset
     */
    int32_t getCalibrationOffset() const;
    
    /**
     * @brief Czyści kalibrację (przywraca domyślne wartości)
     */
    void resetCalibration();

    // ==================== Timestamp ====================
    
    /**
     * @brief Pobiera aktualny timestamp w milisekundach
     * @return Timestamp
     */
    static uint32_t getCurrentTimestamp();
    
    /**
     * @brief Ustawia synchronizację czasu z Arduino
     * @param arduinoTimestamp Timestamp z Arduino
     */
    void syncTimestamp(uint32_t arduinoTimestamp);
    
    /**
     * @brief Pobiera offset czasu między hostem a Arduino
     * @return Offset w milisekundach
     */
    int32_t getTimestampOffset() const;

    // ==================== Wysyłanie komend ====================
    
    /**
     * @brief Wysyła komendę do Arduino
     * @param command Komenda jako string
     * @param waitForResponse Czy czekać na odpowiedź
     * @return Odpowiedź z Arduino (pusta jeśli nie oczekiwano)
     */
    std::string sendCommand(const std::string& command, bool waitForResponse = true);
    
    /**
     * @brief Wysyła dane binarne do Arduino
     * @param data Bufor danych
     * @param length Długość danych
     * @return true jeśli wysłano pomyślnie
     */
    bool sendRawData(const uint8_t* data, size_t length);

    // ==================== Utility ====================
    
    /**
     * @brief Ustawia poziom logowania
     * @param verbose true dla szczegółowego logowania
     */
    void setVerbose(bool verbose);
    
    /**
     * @brief Pobiera ostatni błąd
     * @return Opis błędu
     */
    std::string getLastError() const;

private:
    // Implementacja wewnętrzna (wskaźnik do impl PImpl)
    class Impl;
    std::unique_ptr<Impl> m_impl;
    
    // Metody pomocnicze
    bool openPort(const std::string& portName, int baudRate);
    void closePort();
    int readFromPort(uint8_t* buffer, size_t maxLength, int timeout);
    bool writeToPort(const uint8_t* data, size_t length);
    SensorData parseSensorData(const std::vector<uint8_t>& rawData);
    void updateWatchdog();
};

} // namespace sensor

#endif // SENSOR_SERIALCOMMUNICATION_HPP
