#ifndef DATA_MANAGER_HPP
#define DATA_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <mutex>
#include <memory>
#include <stdexcept>
#include <atomic>
#include "Encryption.hpp"

// ============================================================================
// DEBUG CONFIGURATION FLAGS
// ============================================================================

/// Enable detailed debug logging for all data operations
constexpr bool DEBUG_DATA_MANAGER_ENABLED = true;

/// Enable verbose JSON parsing debug output
constexpr bool DEBUG_JSON_PARSING_ENABLED = false;

/// Enable encryption/decryption operation logging
constexpr bool DEBUG_ENCRYPTION_OPS_ENABLED = true;

/// Enable file I/O operation debugging
constexpr bool DEBUG_FILE_IO_ENABLED = true;

/// Maximum number of retry attempts for file operations
constexpr int MAX_FILE_RETRY_COUNT = 3;

/// Delay between retries in milliseconds
constexpr int FILE_RETRY_DELAY_MS = 50;

/// Maximum allowed file size (10 MB) to prevent memory issues
constexpr size_t MAX_FILE_SIZE_BYTES = 10 * 1024 * 1024;

/**
 * @brief Poziomy ważności logów dla DataManager
 */
enum class DataManagerLogLevel {
    VERBOSE = 0,   ///< Szczegółowe informacje debugowania
    DEBUG = 1,     ///< Informacje debugowania
    INFO = 2,      ///< Informacje ogólne
    WARNING = 3,   ///< Ostrzeżenia
    ERROR = 4,     ///< Błędy
    CRITICAL = 5   ///< Błędy krytyczne
};

/**
 * @brief Statystyki operacji DataManager
 */
struct DataManagerStatistics {
    std::atomic<size_t> filesRead{0};           ///< Liczba odczytanych plików
    std::atomic<size_t> filesWritten{0};        ///< Liczba zapisanych plików
    std::atomic<size_t> encryptionOps{0};       ///< Liczba operacji szyfrowania
    std::atomic<size_t> decryptionOps{0};       ///< Liczba operacji odszyfrowywania
    std::atomic<size_t> jsonParseOps{0};        ///< Liczba operacji parsowania JSON
    std::atomic<size_t> jsonSerializeOps{0};    ///< Liczba operacji serializacji JSON
    std::atomic<size_t> errors{0};              ///< Liczba błędów
    std::atomic<size_t> retries{0};             ///< Liczba ponowień operacji
    
    /**
     * @brief Resetuje wszystkie statystyki
     */
    void reset() {
        filesRead = 0;
        filesWritten = 0;
        encryptionOps = 0;
        decryptionOps = 0;
        jsonParseOps = 0;
        jsonSerializeOps = 0;
        errors = 0;
        retries = 0;
    }
    
    /**
     * @brief Generuje podsumowanie statystyk
     * @return String z podsumowaniem
     */
    std::string getSummary() const;
};

// Struktura danych pacjenta
struct PatientData {
    std::string id;
    std::string firstName;
    std::string lastName;
    std::string pesel;
    std::string birthDate;
    std::string gender;
    std::string phoneNumber;
    std::string email;
    std::string address;
    std::string medicalHistory;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static PatientData fromJson(const std::string& json);
};

// Struktura pojedynczego ćwiczenia
struct Exercise {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    int sets;
    int reps;
    double weight;
    std::string notes;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static Exercise fromJson(const std::string& json);
};

// Struktura wyniku ćwiczenia (treningu)
struct ExerciseResult {
    std::string id;
    std::string patientId;
    std::string exerciseId;
    std::string date;
    int completedSets;
    int completedReps;
    double actualWeight;
    std::string rating; // np. "łatwe", "średnie", "trudne"
    std::string notes;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static ExerciseResult fromJson(const std::string& json);
};

// Struktura konspektu treningowego
struct TrainingPlan {
    std::string id;
    std::string patientId;
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    std::vector<Exercise> exercises;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static TrainingPlan fromJson(const std::string& json);
};

class DataManager {
public:
    /**
     * @brief Konstruktor
     */
    DataManager();
    
    /**
     * @brief Destruktor
     */
    ~DataManager();
    
    // ==================== Zarządzanie poziomem logowania ====================
    
    /**
     * @brief Ustawia minimalny poziom logowania
     * @param level Minimalny poziom logów do wyświetlania
     */
    void setMinimumLogLevel(DataManagerLogLevel level);
    
    /**
     * @brief Pobiera aktualny minimalny poziom logowania
     * @return Aktualny poziom logowania
     */
    DataManagerLogLevel getMinimumLogLevel() const;
    
    /**
     * @brief Włącza/wyłącza wyjście na konsolę
     * @param enabled true aby włączyć, false aby wyłączyć
     */
    void setConsoleOutputEnabled(bool enabled);
    
    /**
     * @brief Sprawdza czy wyjście na konsolę jest włączone
     * @return true jeśli wyjście na konsolę jest aktywne
     */
    bool isConsoleOutputEnabled() const;
    
    // ==================== Statystyki ====================
    
    /**
     * @brief Pobiera statystyki operacji
     * @return Referencja do statystyk
     */
    const DataManagerStatistics& getStatistics() const;
    
    /**
     * @brief Resetuje wszystkie statystyki
     */
    void resetStatistics();
    
    /**
     * @brief Generuje podsumowanie statystyk
     * @return String z podsumowaniem statystyk
     */
    std::string getStatisticsSummary() const;
    
    // ==================== Zarządzanie kluczem szyfrującym ====================
    /**
     * @brief Ustawia klucz szyfrujący
     * @param key Klucz szyfrujący
     */
    void setEncryptionKey(const std::string& key);
    
    /**
     * @brief Czyści klucz szyfrujący
     */
    void clearEncryptionKey();
    
    /**
     * @brief Sprawdza czy szyfrowanie jest włączone
     * @return true jeśli szyfrowanie jest aktywne
     */
    bool isEncryptionEnabled() const;
    
    // ==================== Zapis i odczyt danych pacjenta ====================
    /**
     * @brief Zapisuje dane pacjenta do pliku
     * @param patient Dane pacjenta
     * @param filename Nazwa pliku
     * @param encrypt Czy szyfrować dane (domyślnie false)
     * @return true jeśli zapis pomyślny
     */
    bool savePatientData(const PatientData& patient, const std::string& filename, bool encrypt = false);
    
    /**
     * @brief Wczytuje dane pacjenta z pliku
     * @param filename Nazwa pliku
     * @param decrypt Czy odszyfrowywać dane (domyślnie false)
     * @return Dane pacjenta (puste jeśli błąd)
     */
    PatientData loadPatientData(const std::string& filename, bool decrypt = false);
    
    // ==================== Zapis i odczyt listy ćwiczeń ====================
    /**
     * @brief Zapisuje listę ćwiczeń do pliku
     * @param exercises Lista ćwiczeń
     * @param filename Nazwa pliku
     * @param encrypt Czy szyfrować dane (domyślnie false)
     * @return true jeśli zapis pomyślny
     */
    bool saveExercises(const std::vector<Exercise>& exercises, const std::string& filename, bool encrypt = false);
    
    /**
     * @brief Wczytuje listę ćwiczeń z pliku
     * @param filename Nazwa pliku
     * @param decrypt Czy odszyfrowywać dane (domyślnie false)
     * @return Lista ćwiczeń (pusta jeśli błąd)
     */
    std::vector<Exercise> loadExercises(const std::string& filename, bool decrypt = false);
    
    // ==================== Zapis i odczyt wyników ćwiczeń ====================
    /**
     * @brief Zapisuje wyniki ćwiczeń do pliku
     * @param results Lista wyników
     * @param filename Nazwa pliku
     * @param encrypt Czy szyfrować dane (domyślnie false)
     * @return true jeśli zapis pomyślny
     */
    bool saveExerciseResults(const std::vector<ExerciseResult>& results, const std::string& filename, bool encrypt = false);
    
    /**
     * @brief Wczytuje wyniki ćwiczeń z pliku
     * @param filename Nazwa pliku
     * @param decrypt Czy odszyfrowywać dane (domyślnie false)
     * @return Lista wyników (pusta jeśli błąd)
     */
    std::vector<ExerciseResult> loadExerciseResults(const std::string& filename, bool decrypt = false);
    
    // ==================== Zapis i odczyt konspektu treningowego ====================
    /**
     * @brief Zapisuje konspekt treningowy do pliku
     * @param plan Konspekt treningowy
     * @param filename Nazwa pliku
     * @param encrypt Czy szyfrować dane (domyślnie false)
     * @return true jeśli zapis pomyślny
     */
    bool saveTrainingPlan(const TrainingPlan& plan, const std::string& filename, bool encrypt = false);
    
    /**
     * @brief Wczytuje konspekt treningowy z pliku
     * @param filename Nazwa pliku
     * @param decrypt Czy odszyfrowywać dane (domyślnie false)
     * @return Konspekt treningowy (pusty jeśli błąd)
     */
    TrainingPlan loadTrainingPlan(const std::string& filename, bool decrypt = false);
    
    // ==================== Operacje na plikach ====================
    /**
     * @brief Sprawdza czy plik istnieje
     * @param filename Nazwa pliku
     * @return true jeśli plik istnieje
     */
    bool fileExists(const std::string& filename) const;
    
    /**
     * @brief Usuwa plik
     * @param filename Nazwa pliku do usunięcia
     * @return true jeśli usunięcie pomyślne
     */
    bool deleteFile(const std::string& filename);
    
    /**
     * @brief Listuje pliki w katalogu
     * @param directory Ścieżka do katalogu (domyślnie ".")
     * @return Lista nazw plików
     */
    std::vector<std::string> listFiles(const std::string& directory = ".") const;
    
    // ==================== Eksport/Import nieszyfrowany JSON ====================
    /**
     * @brief Eksportuje dane do pliku JSON
     * @param data Dane do zapisu
     * @param filename Nazwa pliku
     * @return true jeśli zapis pomyślny
     */
    bool exportToJson(const std::string& data, const std::string& filename);
    
    /**
     * @brief Importuje dane z pliku JSON
     * @param filename Nazwa pliku
     * @return Zawartość pliku jako string
     */
    std::string importFromJson(const std::string& filename);

    // ==================== Pomocnicze metody do parsowania JSON ====================
    /**
     * @brief Ekstrahuje wartość string z JSON
     * @param json String JSON
     * @param key Klucz do wyszukania
     * @return Wartość string lub pusty string jeśli nie znaleziono
     */
    std::string extractStringValue(const std::string& json, const std::string& key) const;
    
    /**
     * @brief Ekstrahuje wartość int z JSON
     * @param json String JSON
     * @param key Klucz do wyszukania
     * @return Wartość int lub 0 jeśli nie znaleziono
     */
    int extractIntValue(const std::string& json, const std::string& key) const;
    
    /**
     * @brief Ekstrahuje wartość double z JSON
     * @param json String JSON
     * @param key Klucz do wyszukania
     * @return Wartość double lub 0.0 jeśli nie znaleziono
     */
    double extractDoubleValue(const std::string& json, const std::string& key) const;
    
    /**
     * @brief Escapuje string do formatu JSON
     * @param input String do escapowania
     * @return Escapowany string
     */
    std::string escapeJsonString(const std::string& input) const;
    
    /**
     * @brief Unescapuje string z formatu JSON
     * @param input String w formacie JSON
     * @return Unescapowany string
     */
    std::string unescapeJsonString(const std::string& input) const;

private:
    std::string encryptionKey;
    bool encryptionEnabled;
    
    // ========================================================================
    // LOGGING & DEBUG CONFIGURATION
    // ========================================================================
    DataManagerLogLevel minimumLogLevel{DataManagerLogLevel::DEBUG};  ///< Minimalny poziom logowania
    bool consoleOutputEnabled{true};                                   ///< Czy wyjście na konsolę jest włączone
    mutable std::mutex logMutex;                                       ///< Mutex dla thread-safe logowania
    DataManagerStatistics statistics;                                  ///< Statystyki operacji
    
    // ========================================================================
    // HELPER METHODS
    // ========================================================================
    
    /**
     * @brief Pomocnicza metoda do operacji na plikach
     * @param filename Nazwa pliku
     * @return Zawartość pliku lub pusty string jeśli błąd
     */
    std::string readFile(const std::string& filename) const;
    
    /**
     * @brief Zapisuje content do pliku
     * @param filename Nazwa pliku
     * @param content Zawartość do zapisu
     * @return true jeśli zapis pomyślny
     */
    bool writeFile(const std::string& filename, const std::string& content);
    
    /**
     * @brief Loguje wiadomość z określonym poziomem ważności
     * @param level Poziom ważności
     * @param message Wiadomość do zalogowania
     * @param source Źródło wiadomości (nazwa funkcji/metody)
     */
    void logMessage(DataManagerLogLevel level, const std::string& message, 
                    const std::string& source = "DataManager") const;
    
    /**
     * @brief Konwertuje poziom logowania na string
     * @param level Poziom logowania
     * @return String reprezentujący poziom
     */
    static std::string logLevelToString(DataManagerLogLevel level);
    
    /**
     * @brief Konwertuje string na poziom logowania
     * @param level String z poziomem logowania
     * @return Odpowiedni poziom logowania
     */
    static DataManagerLogLevel stringToLogLevel(const std::string& level);
    
};

#endif // DATA_MANAGER_HPP
