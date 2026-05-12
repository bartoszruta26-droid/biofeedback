#ifndef ENCRYPTION_HPP
#define ENCRYPTION_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <functional>
#include <chrono>

/**
 * @namespace EncryptionTypes
 * @brief Typy i struktury pomocnicze dla modułu Encryption
 */
namespace EncryptionTypes {

    /**
     * @enum EncryptionLogLevel
     * @brief Poziomy logowania dla operacji kryptograficznych
     * 
     * Określa szczegółowość komunikatów logowanych podczas operacji
     * szyfrowania, deszyfrowania i generowania kluczy.
     */
    enum class EncryptionLogLevel : int {
        VERBOSE = 0,    ///< Szczegółowe informacje debugowe (każda operacja)
        DEBUG = 1,      ///< Informacje debugowe (główne kroki)
        INFO = 2,       ///< Informacyjne (sukcesy operacji)
        WARNING = 3,    ///< Ostrzeżenia (niekrytyczne problemy)
        ERROR = 4,      ///< Błędy (krytyczne problemy)
        CRITICAL = 5    ///< Błędy krytyczne (awarie systemu)
    };

    /**
     * @struct EncryptionStatistics
     * @brief Statystyki operacji kryptograficznych
     * 
     * Przechowuje liczniki operacji wykonanych przez klasę Encryption.
     * Wszystkie pola są thread-safe dzięki atomic.
     */
    struct EncryptionStatistics {
        std::atomic<uint64_t> encryptOperations{0};      ///< Liczba operacji szyfrowania
        std::atomic<uint64_t> decryptOperations{0};      ///< Liczba operacji deszyfrowania
        std::atomic<uint64_t> keyGenerations{0};         ///< Liczba generowań kluczy
        std::atomic<uint64_t> base64EncodeOps{0};        ///< Liczba kodowań Base64
        std::atomic<uint64_t> base64DecodeOps{0};        ///< Liczba dekodowań Base64
        std::atomic<uint64_t> xorOperations{0};          ///< Liczba operacji XOR
        std::atomic<uint64_t> validationErrors{0};       ///< Liczba błędów walidacji
        std::atomic<uint64_t> memoryErrors{0};           ///< Liczba błędów alokacji pamięci
        std::atomic<uint64_t> otherErrors{0};            ///< Liczba innych błędów
        std::atomic<uint64_t> totalBytesProcessed{0};    ///< Łączna liczba przetworzonych bajtów
        
        /**
         * @brief Konwertuje statystyki do czytelnego stringa
         * @return String z podsumowaniem statystyk
         */
        std::string toString() const;
        
        /**
         * @brief Resetuje wszystkie liczniki do zera
         */
        void reset();
    };

    /**
     * @struct EncryptionResult
     * @brief Wynik operacji kryptograficznej z metadanymi
     * 
     * Struktura zawierająca wynik operacji wraz z informacjami
     * o czasie wykonania, statusie i ewentualnym błędzie.
     */
    struct EncryptionResult {
        std::string data;                    ///< Wynikowe dane (zaszyfrowane/odszyfrowane)
        bool success;                        ///< Czy operacja się powiodła
        std::string errorMessage;            ///< Komunikat błędu (jeśli success == false)
        double executionTimeMs;              ///< Czas wykonania w milisekundach
        size_t inputSize;                    ///< Rozmiar danych wejściowych
        size_t outputSize;                   ///< Rozmiar danych wyjściowych
        
        EncryptionResult() : success(false), executionTimeMs(0.0), inputSize(0), outputSize(0) {}
    };
}

/**
 * @class Encryption
 * @brief Klasa zapewniająca podstawowe funkcje kryptograficzne
 * 
 * Klasa udostępnia statyczne metody do szyfrowania i deszyfrowania danych
 * przy użyciu algorytmu XOR z kodowaniem Base64. Zawiera zaawansowane
 * mechanizmy debugowania, obsługi błędów, logowania i statystyk.
 * 
 * @note Algorytm XOR jest prostym szyfrem i nie powinien być używany
 * do zabezpieczania wrażliwych danych w środowisku produkcyjnym.
 * Do celów medycznych zalecane jest użycie AES lub innych certyfikowanych
 * algorytmów.
 * 
 * @threadsafe Tak, wszystkie metody są thread-safe
 */
class Encryption {
public:
    // ========================================================================
    // KONSTRUKTOR I DESTRUKTOR (usunięte - klasa tylko ze statycznymi metodami)
    // ========================================================================
    
    // Klasa nie może być instancjonowana - tylko metody statyczne
    Encryption() = delete;
    ~Encryption() = delete;
    
    // ========================================================================
    // GŁÓWNE METODY KRYPTOGRAFICZNE
    // ========================================================================
    
    /**
     * @brief Szyfruje dane za pomocą klucza (XOR) i koduje Base64
     * 
     * Metoda przeprowadza pełny proces szyfrowania:
     * 1. Walidacja parametrów wejściowych
     * 2. Konwersja danych do wektora bajtów
     * 3. Szyfrowanie XOR z użyciem podanego klucza
     * 4. Kodowanie wyniku do Base64
     * 5. Logowanie operacji i aktualizacja statystyk
     * 
     * @param data Dane do zaszyfrowania (string tekstowy)
     * @param key Klucz szyfrujący (minimum 4 znaki, zalecane 8+)
     * @return Zaszyfrowane dane zakodowane w Base64
     * 
     * @throws std::invalid_argument Jeśli klucz jest pusty
     * @throws std::runtime_error Jeśli wystąpi błąd alokacji pamięci lub inny krytyczny
     * 
     * @note Dla pustych danych zwracany jest pusty string (gentle code)
     * @note Czas wykonania jest mierzony i dostępny w statystykach
     */
    static std::string encrypt(const std::string& data, const std::string& key);
    
    /**
     * @brief Dekoduje Base64 i odszyfrowuje dane za pomocą klucza (XOR)
     * 
     * Metoda przeprowadza pełny proces deszyfrowania:
     * 1. Walidacja parametrów wejściowych
     * 2. Dekodowanie Base64 do wektora bajtów
     * 3. Deszyfrowanie XOR z użyciem podanego klucza
     * 4. Konwersja wyniku do stringa
     * 5. Logowanie operacji i aktualizacja statystyk
     * 
     * @param encryptedData Dane zaszyfrowane w formacie Base64
     * @param key Klucz szyfrujący (ten sam co użyty do szyfrowania)
     * @return Odszyfrowane dane oryginalne
     * 
     * @throws std::invalid_argument Jeśli klucz jest pusty lub dane są niepoprawne
     * @throws std::runtime_error Jeśli wystąpi błąd dekodowania lub alokacji pamięci
     * 
     * @note Dla pustych danych zwracany jest pusty string (gentle code)
     * @note Operacja jest odwrotna do encrypt() - decrypt(encrypt(data, key), key) == data
     */
    static std::string decrypt(const std::string& encryptedData, const std::string& key);
    
    /**
     * @brief Generuje losowy klucz o podanej długości
     * 
     * Tworzy losowy ciąg znaków ASCII drukowalnych (33-126)
     * o zadanej długości, przeznaczony do użycia jako klucz szyfrujący.
     * 
     * @param length Długość klucza w znakach (domyślnie 32)
     * @return Losowy klucz jako string
     * 
     * @note Dla length = 0 ustawiana jest wartość domyślna 32
     * @note Dla length > 1024 klucz jest ograniczany do 1024 znaków (gentle code)
     * @warning Przed użyciem należy wywołać srand() do inicjalizacji generatora losowego
     * @note Zalecane minimum to 8 znaków dla podstawowego bezpieczeństwa
     */
    static std::string generateKey(size_t length = 32);
    
    // ========================================================================
    // METODY POMOCNICZE BASE64
    // ========================================================================
    
    /**
     * @brief Kodowanie danych binarnych do formatu Base64
     * 
     * Konwertuje wektor bajtów na string zakodowany w Base64 zgodnie z RFC 4648.
     * 
     * @param input Wektor bajtów do zakodowania
     * @return String zakodowany w Base64 z paddingiem '='
     * 
     * @note Puste dane wejściowe zwracają pusty string
     * @note Rozmiar wyniku to około 133% rozmiaru danych wejściowych
     */
    static std::string base64Encode(const std::vector<uint8_t>& input);
    
    /**
     * @brief Dekodowanie danych z formatu Base64 do postaci binarnej
     * 
     * Konwertuje string zakodowany w Base64 z powrotem do wektora bajtów.
     * 
     * @param input String zakodowany w Base64
     * @return Wektor bajtów z odkodowanymi danymi
     * 
     * @throws std::runtime_error Jeśli napotkano niepoprawne znaki Base64
     * @note Niepoprawne znaki są ignorowane (gentle code)
     * @note Padding '=' jest opcjonalny
     */
    static std::vector<uint8_t> base64Decode(const std::string& input);
    
    // ========================================================================
    // METODY STATYSTYCZNE I KONFIGURACYJNE
    // ========================================================================
    
    /**
     * @brief Ustawia minimalny poziom logowania
     * 
     * Kontroluje szczegółowość komunikatów logowanych przez moduł Encryption.
     * 
     * @param level Minimalny poziom logowania (domyślnie INFO)
     * 
     * @note Poziomy: VERBOSE(0), DEBUG(1), INFO(2), WARNING(3), ERROR(4), CRITICAL(5)
     * @threadsafe Tak
     */
    static void setMinimumLogLevel(EncryptionTypes::EncryptionLogLevel level);
    
    /**
     * @brief Pobiera aktualny minimalny poziom logowania
     * 
     * @return Aktualny poziom logowania
     * 
     * @threadsafe Tak
     */
    static EncryptionTypes::EncryptionLogLevel getMinimumLogLevel();
    
    /**
     * @brief Włącza/wyłącza wyjście logów na konsolę
     * 
     * @param enabled true aby włączyć, false aby wyłączyć
     * 
     * @threadsafe Tak
     */
    static void setConsoleOutputEnabled(bool enabled);
    
    /**
     * @brief Sprawdza czy wyjście na konsolę jest włączone
     * 
     * @return true jeśli logi są wypisywane na konsolę
     * 
     * @threadsafe Tak
     */
    static bool isConsoleOutputEnabled();
    
    /**
     * @brief Pobiera aktualne statystyki operacji
     * 
     * @return Kopia aktualnych statystyk
     * 
     * @threadsafe Tak
     */
    static EncryptionTypes::EncryptionStatistics getStatistics();
    
    /**
     * @brief Resetuje wszystkie statystyki do zera
     * 
     * @threadsafe Tak
     */
    static void resetStatistics();
    
    /**
     * @brief Pobiera podsumowanie statystyk jako string
     * 
     * @return Sformatowany string ze statystykami
     * 
     * @threadsafe Tak
     */
    static std::string getStatisticsSummary();
    
    /**
     * @brief Rejestruje callback do powiadomień o błędach
     * 
     * Callback jest wywoływany przy każdym błędzie krytycznym.
     * 
     * @param callback Funkcja przyjmująca komunikat błędu (string)
     * 
     * @threadsafe Tak
     */
    static void setErrorCallback(std::function<void(const std::string&)> callback);
    
    /**
     * @brief Rejestruje callback do powiadomień o błędach krytycznych
     * 
     * Callback jest wywoływany tylko przy błędach CRITICAL.
     * 
     * @param callback Funkcja przyjmująca komunikat błędu (string)
     * 
     * @threadsafe Tak
     */
    static void setCriticalErrorCallback(std::function<void(const std::string&)> callback);

private:
    // ========================================================================
    // METODY PRYWATNE
    // ========================================================================
    
    /**
     * @brief Prosty algorytm XOR do szyfrowania/deszyfrowania
     * 
     * Wykonuje operację XOR na danych z cyklicznie powtarzanym kluczem.
     * Ta sama operacja służy zarówno do szyfrowania jak i deszyfrowania.
     * 
     * @param data Dane do przetworzenia
     * @param key Klucz do operacji XOR
     * @return Wynik operacji XOR
     * 
     * @throws std::invalid_argument Jeśli klucz jest pusty
     * @throws std::runtime_error Jeśli wystąpi błąd alokacji pamięci
     */
    static std::vector<uint8_t> xorCipher(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key);
    
    /**
     * @brief Loguje komunikat z odpowiednim poziomem ważności
     * 
     * @param level Poziom ważności komunikatu
     * @param function Nazwa funkcji wywołującej
     * @param message Treść komunikatu
     * 
     * @threadsafe Tak
     */
    static void logMessage(EncryptionTypes::EncryptionLogLevel level, 
                          const std::string& function, 
                          const std::string& message);
    
    /**
     * @brief Konwertuje poziom logowania na string
     * 
     * @param level Poziom logowania
     * @return String reprezentujący poziom (np. "DEBUG", "ERROR")
     */
    static std::string logLevelToString(EncryptionTypes::EncryptionLogLevel level);
    
    /**
     * @brief Konwertuje string na poziom logowania
     * 
     * @param levelStr String z nazwą poziomu
     * @return Odpowiedni poziom logowania
     * @throws std::invalid_argument Jeśli string nie jest rozpoznany
     */
    static EncryptionTypes::EncryptionLogLevel stringToLogLevel(const std::string& levelStr);
    
    // ========================================================================
    // ZMIENNE STATYCZNE PRYWATNE
    // ========================================================================
    
    static EncryptionTypes::EncryptionLogLevel m_minimumLogLevel;   ///< Minimalny poziom logowania
    static bool m_consoleOutputEnabled;                              ///< Czy logi idą na konsolę
    static EncryptionTypes::EncryptionStatistics m_statistics;       ///< Statystyki operacji
    static std::mutex m_logMutex;                                    ///< Mutex do logowania
    static std::function<void(const std::string&)> m_errorCallback;  ///< Callback błędów
    static std::function<void(const std::string&)> m_criticalErrorCallback;  ///< Callback błędów krytycznych
    
    /// Stałe konfiguracyjne
    static constexpr size_t MIN_KEY_LENGTH = 4;                      ///< Minimalna długość klucza
    static constexpr size_t MAX_KEY_LENGTH = 1024;                   ///< Maksymalna długość klucza
    static constexpr size_t DEFAULT_KEY_LENGTH = 32;                 ///< Domyślna długość klucza
    static constexpr size_t MAX_DATA_SIZE = 100 * 1024 * 1024;       ///< Max 100MB danych
};

#endif // ENCRYPTION_HPP
