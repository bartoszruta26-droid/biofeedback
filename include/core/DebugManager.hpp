#ifndef CORE_DEBUGMANAGER_HPP
#define CORE_DEBUGMANAGER_HPP

#include <QObject>
#include <QString>
#include <QMutex>
#include <QDateTime>
#include <memory>
#include <functional>
#include <map>
#include <atomic>

namespace core {

/**
 * @brief Poziomy ważności wiadomości debugowych
 * 
 * Umożliwia filtrowanie i kategoryzację komunikatów
 */
enum class DebugLevel {
    VERBOSE = 0,    ///< Szczegółowe informacje diagnostyczne
    DEBUG = 1,      ///< Informacje debugowe dla programistów
    INFO = 2,       ///< Informacyjne wiadomości o stanie systemu
    WARNING = 3,    ///< Ostrzeżenia o potencjalnych problemach
    ERROR = 4,      ///< Błędy które nie przerywają działania
    CRITICAL = 5    ///< Krytyczne błędy wymagające natychmiastowej uwagi
};

/**
 * @brief Struktura przechowująca pojedynczą wiadomość debugową
 */
struct DebugMessage {
    QString message;        ///< Treść wiadomości
    QString type;           ///< Typ wiadomości (INFO, ERROR, itp.)
    DebugLevel level;       ///< Poziom ważności
    QDateTime timestamp;    ///< Czas wystąpienia
    QString source;         ///< Źródło wiadomości (nazwa modułu/funkcji)
    int threadId;           ///< ID wątku który wygenerował wiadomość
    
    DebugMessage() : level(DebugLevel::INFO), threadId(0) {}
    DebugMessage(const QString& msg, const QString& t, DebugLevel l = DebugLevel::INFO,
                 const QString& src = QString(), int tid = 0)
        : message(msg), type(t), level(l), timestamp(QDateTime::currentDateTime()),
          source(src), threadId(tid) {}
};

/**
 * @class DebugManager
 * @brief Centralny menedżer wiadomości debugowych działający jako singleton
 * 
 * DebugManager umożliwia:
 * - Rejestrację callbacka do DebugTab
 * - Wysyłanie wiadomości debugowych z dowolnego miejsca w aplikacji
 * - Thread-safe operacje dzięki mutexowi
 * - Filtrowanie wiadomości po poziomie ważności
 * - Statystyki i liczniki wiadomości
 * - Zapis do pliku logowego
 * 
 * @section FEATURES Główne funkcjonalności
 * - Singleton pattern zapewniający jedną instancję w całej aplikacji
 * - Wsparcie dla wielu poziomów logowania (VERBOSE do CRITICAL)
 * - Thread-safe dzięki QMutex
 * - Możliwość rejestrowania źródeł wiadomości
 * - Liczniki statystyczne dla każdego poziomu
 * - Callbacki dla zewnętrznych odbiorców (DebugTab, terminal)
 * - Sygnały Qt dla integracji z GUI
 * 
 * @section THREAD_SAFETY Bezpieczeństwo wątkowe
 * Wszystkie publiczne metody są thread-safe i mogą być wywoływane
 * z wielu wątków jednocześnie. Wewnętrzny mutex zapewnia serializację
 * dostępu do współdzielonych danych.
 * 
 * @section ERROR_HANDLING Obsługa błędów
 * - Metody nie rzucają wyjątków na zewnątrz
 * - Błędy wewnętrzne są logowane na stderr
 * - Invalid input jest obsługiwany gracefully z default wartościami
 * 
 * Tylko DebugTab i command line terminal zbierają dane debugowe.
 * Inne zakładki nie mają własnych terminali debugowych.
 */
class DebugManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Pobiera instancję singletona DebugManager
     * @return Referencja do instancji DebugManager
     * 
     * Thread-safe implementacja singletona używająca local static variable
     * (Meyers' Singleton pattern - safe od C++11)
     */
    static DebugManager& instance();

    /**
     * @brief Rejestruje callback do wysyłania wiadomości debugowych
     * @param callback Funkcja przyjmująca message i type
     * 
     * Thread-safe: używa QMutexLocker do ochrony zasobów
     */
    void registerDebugCallback(std::function<void(const QString&, const QString&)> callback);

    /**
     * @brief Odłącza zarejestrowany callback
     * 
     * Thread-safe: używa QMutexLocker do ochrony zasobów
     */
    void unregisterDebugCallback();

    /**
     * @brief Wysyła wiadomość debugową do DebugTab
     * @param message Treść wiadomości
     * @param type Typ wiadomości (INFO, ERROR, WARNING, DATA, SERIAL, ARDUINO)
     * 
     * Thread-safe metoda wysyłająca wiadomość do wszystkich zarejestrowanych
     * odbiorców (callback + sygnał Qt). Jeśli nie ma callbacka, wypisuje
     * na stderr jako fallback.
     * 
     * @note Automatycznie dodaje timestamp i informacje o źródle
     */
    void sendDebugMessage(const QString& message, const QString& type = "INFO");

    /**
     * @brief Wysyła wiadomość z określonym poziomem ważności
     * @param message Treść wiadomości
     * @param level Poziom ważności (DEBUG, INFO, WARNING, ERROR, CRITICAL)
     * @param source Źródło wiadomości (nazwa modułu/funkcji)
     * 
     * Umożliwia precyzyjną kontrolę nad ważnością komunikatów
     * i późniejsze filtrowanie w DebugTab.
     */
    void sendDebugMessage(const QString& message, DebugLevel level, const QString& source = QString());

    /**
     * @brief Wysyła wiadomość z określonym poziomem ważności i typem
     * @param message Treść wiadomości
     * @param level Poziom ważności (DEBUG, INFO, WARNING, ERROR, CRITICAL)
     * @param source Źródło wiadomości (nazwa modułu/funkcji)
     * @param type Typ wiadomości (np. DATA, SERIAL, ARDUINO) - zachowywany oryginalny typ
     * 
     * Umożliwia precyzyjną kontrolę nad ważnością komunikatów
     * i późniejsze filtrowanie w DebugTab z zachowaniem oryginalnego typu.
     */
    void sendDebugMessage(const QString& message, DebugLevel level, const QString& source, const QString& type);

    /**
     * @brief Sprawdza czy callback jest zarejestrowany
     * @return true jeśli callback jest ustawiony
     * 
     * Thread-safe: używa QMutexLocker do ochrony zasobów
     */
    bool hasCallback() const;

    /**
     * @brief Ustawia minimalny poziom logowania
     * @param level Minimalny poziom wiadomości które będą przetwarzane
     * 
     * Wiadomości poniżej tego poziomu będą ignorowane.
     * Domyślnie: DebugLevel::DEBUG
     */
    void setMinimumLogLevel(DebugLevel level);

    /**
     * @brief Pobiera aktualny minimalny poziom logowania
     * @return Aktualny minimalny poziom
     */
    DebugLevel minimumLogLevel() const;

    /**
     * @brief Włącza/wyłącza zapis do konsoli
     * @param enabled true aby włączyć wyjście na stderr
     */
    void setConsoleOutputEnabled(bool enabled);

    /**
     * @brief Sprawdza czy wyjście konsolowe jest włączone
     * @return true jeśli konsola jest aktywna
     */
    bool isConsoleOutputEnabled() const;

    /**
     * @brief Czyści historię wiadomości (jeśli przechowywana)
     * 
     * Thread-safe operacja
     */
    void clearMessageHistory();

    /**
     * @brief Pobiera liczbę wiadomości danego typu
     * @param type Typ wiadomości do policzenia
     * @return Liczba wiadomości tego typu
     * 
     * Thread-safe: używa QMutexLocker do ochrony zasobów
     */
    int getMessageCount(const QString& type) const;

    /**
     * @brief Pobiera całkowitą liczbę wiadomości
     * @return Łączna liczba wszystkich wiadomości
     * 
     * Thread-safe: używa QMutexLocker do ochrony zasobów
     */
    int getTotalMessageCount() const;

    /**
     * @brief Resetuje wszystkie liczniki statystyczne
     * 
     * Thread-safe operacja
     */
    void resetStatistics();

    /**
     * @brief Pobiera statystyki jako sformatowany string
     * @return String zawierający podsumowanie statystyk
     * 
     * Przydatne do eksportu lub wyświetlania w UI
     */
    QString getStatisticsSummary() const;

signals:
    /**
     * @brief Sygnał emitowany gdy wiadomość debugowa została wysłana
     * @param message Treść wiadomości
     * @param type Typ wiadomości
     * 
     * Emitowany dla każdej wiadomości która przeszła filtr poziomu
     */
    void debugMessageSent(const QString& message, const QString& type);

    /**
     * @brief Sygnał emitowany przy krytycznym błędzie
     * @param message Opis błędu
     * @param source Źródło błędu
     * 
     * Umożliwia specjalną obsługę krytycznych sytuacji w UI
     */
    void criticalErrorOccurred(const QString& message, const QString& source);

    /**
     * @brief Sygnał emitowany gdy statystyki zostaną zresetowane
     */
    void statisticsReset();

private:
    /**
     * @brief Konstruktor prywatny dla singletona
     * 
     * Inicjalizuje wszystkie pola domyślnymi wartościami
     */
    DebugManager();

    /**
     * @brief Destruktor
     * 
     * Czyści zasoby i unregisteruje callbacki
     */
    ~DebugManager() override;

    // Zapobieganie kopiowaniu (singleton)
    DebugManager(const DebugManager&) = delete;
    DebugManager& operator=(const DebugManager&) = delete;

    /**
     * @brief Konwertuje DebugLevel na string
     * @param level Poziom do konwersji
     * @return String reprezentujący poziom
     */
    static QString levelToString(DebugLevel level);

    /**
     * @brief Internalna metoda logująca z pełnymi informacjami
     * @param msg Struktura wiadomości
     * 
     * Wykonuje właściwe logowanie z wszystkimi zabezpieczeniami
     */
    void logMessageInternal(const DebugMessage& msg);

    std::function<void(const QString&, const QString&)> m_debugCallback;
    mutable QMutex m_mutex;
    
    DebugLevel m_minimumLogLevel;           ///< Minimalny poziom logowania
    bool m_consoleOutputEnabled;            ///< Czy wyjście na konsolę jest włączone
    
    // Statystyki
    std::map<QString, std::atomic<int>> m_messageCounts;  ///< Liczniki per typ
    std::atomic<int> m_totalMessageCount;   ///< Całkowity licznik wiadomości
    std::atomic<int> m_criticalCount;       ///< Licznik krytycznych błędów
};

} // namespace core

#endif // CORE_DEBUGMANAGER_HPP
