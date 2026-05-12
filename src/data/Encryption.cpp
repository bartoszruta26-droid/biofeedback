/**
 * @file Encryption.cpp
 * @brief Implementacja klasy Encryption - szyfrowanie XOR z kodowaniem Base64
 * 
 * Ten plik zawiera implementację mechanizmów szyfrowania i deszyfrowania danych
 * z wykorzystaniem algorytmu XOR oraz kodowania Base64. Kod zawiera rozbudowane
 * mechanizmy debugowania, obsługi błędów, wyjątków oraz logowania operacji.
 * 
 * @author AI Assistant
 * @date 2025
 */

#include "data/Encryption.hpp"
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

// ============================================================================
// FLAGI DEBUGOWANIA - można włączać/wyłączać poszczególne moduły
// ============================================================================
#ifndef DEBUG_ENCRYPTION
    #define DEBUG_ENCRYPTION 1          ///< Główna flaga debugowania enkrypcji
#endif

#ifndef DEBUG_XOR_OPERATIONS
    #define DEBUG_XOR_OPERATIONS 1      ///< Flag dla operacji XOR
#endif

#ifndef DEBUG_BASE64_OPERATIONS
    #define DEBUG_BASE64_OPERATIONS 1   ///< Flag dla operacji Base64
#endif

#ifndef DEBUG_KEY_GENERATION
    #define DEBUG_KEY_GENERATION 1      ///< Flag dla generowania kluczy
#endif

#ifndef DEBUG_ERROR_HANDLING
    #define DEBUG_ERROR_HANDLING 1      ///< Flag dla obsługi błędów
#endif

// ============================================================================
// MAKRA POMOCNICZE DO LOGOWANIA I DEBUGOWANIA
// ============================================================================

/**
 * @brief Makro do logowania komunikatów debugowych
 * 
 * Wyświetla komunikat tylko gdy odpowiednia flaga debugowania jest włączona.
 * Format: [DEBUG][timestamp][function] message
 */
#define ENCRYPT_DEBUG(level, msg) \
    do { \
        if (DEBUG_ENCRYPTION && level <= DEBUG_ENCRYPTION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[DEBUG][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

/**
 * @brief Makro do logowania błędów
 * 
 * Zawsze wyświetla komunikaty błędów niezależnie od flag debugowania.
 * Format: [ERROR][timestamp][function] message
 */
#define ENCRYPT_ERROR(msg) \
    do { \
        auto now = std::chrono::system_clock::now(); \
        auto time = std::chrono::system_clock::to_time_t(now); \
        std::stringstream ss; \
        ss << "[ERROR][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
           << __FUNCTION__ << ": " << msg << std::endl; \
        std::cerr << ss.str(); \
    } while(0)

/**
 * @brief Makro do logowania ostrzeżeń
 * 
 * Wyświetla komunikaty ostrzeżeń dla niekrytycznych problemów.
 */
#define ENCRYPT_WARN(msg) \
    do { \
        if (DEBUG_ENCRYPTION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[WARN][ENCRYPTION][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

// ============================================================================
// STAŁE I ZMIENNE GLOBALNE
// ============================================================================

/**
 * @brief Tabela kodowania Base64
 * 
 * Zawiera wszystkie znaki używane w kodowaniu Base64 w poprawnej kolejności.
 * Jest to standardowa tabela zgodna z RFC 4648.
 */
static const std::string base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// ============================================================================
// INICJALIZACJA ZMIENNYCH STATYCZNYCH KLASY ENCRYPTION
// ============================================================================

EncryptionTypes::EncryptionLogLevel Encryption::m_minimumLogLevel = EncryptionTypes::EncryptionLogLevel::INFO;
bool Encryption::m_consoleOutputEnabled = true;
EncryptionTypes::EncryptionStatistics Encryption::m_statistics;
std::mutex Encryption::m_logMutex;
std::function<void(const std::string&)> Encryption::m_errorCallback;
std::function<void(const std::string&)> Encryption::m_criticalErrorCallback;

// ============================================================================
// IMPLEMENTACJA METOD STATYSTYCZNYCH I KONFIGURACYJNYCH
// ============================================================================

void Encryption::setMinimumLogLevel(EncryptionTypes::EncryptionLogLevel level) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_minimumLogLevel = level;
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, 
               "Poziom logowania ustawiony na: " + logLevelToString(level));
}

EncryptionTypes::EncryptionLogLevel Encryption::getMinimumLogLevel() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    return m_minimumLogLevel;
}

void Encryption::setConsoleOutputEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_consoleOutputEnabled = enabled;
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__,
               "Wyjście na konsolę " + std::string(enabled ? "włączone" : "wyłączone"));
}

bool Encryption::isConsoleOutputEnabled() {
    std::lock_guard<std::mutex> lock(m_logMutex);
    return m_consoleOutputEnabled;
}

EncryptionTypes::EncryptionStatistics Encryption::getStatistics() {
    // Tworzenie kopii statystyk z atomic load dla każdego pola
    EncryptionTypes::EncryptionStatistics stats;
    stats.encryptOperations.store(m_statistics.encryptOperations.load());
    stats.decryptOperations.store(m_statistics.decryptOperations.load());
    stats.keyGenerations.store(m_statistics.keyGenerations.load());
    stats.base64EncodeOps.store(m_statistics.base64EncodeOps.load());
    stats.base64DecodeOps.store(m_statistics.base64DecodeOps.load());
    stats.xorOperations.store(m_statistics.xorOperations.load());
    stats.validationErrors.store(m_statistics.validationErrors.load());
    stats.memoryErrors.store(m_statistics.memoryErrors.load());
    stats.otherErrors.store(m_statistics.otherErrors.load());
    stats.totalBytesProcessed.store(m_statistics.totalBytesProcessed.load());
    return stats;
}


std::string Encryption::getStatisticsSummary() {
    std::stringstream ss;
    ss << "=== Statystyki Encryption ===" << std::endl;
    ss << "Operacje szyfrowania: " << m_statistics.encryptOperations.load() << std::endl;
    ss << "Operacje deszyfrowania: " << m_statistics.decryptOperations.load() << std::endl;
    ss << "Generowania kluczy: " << m_statistics.keyGenerations.load() << std::endl;
    ss << "Kodowań Base64: " << m_statistics.base64EncodeOps.load() << std::endl;
    ss << "Dekodowań Base64: " << m_statistics.base64DecodeOps.load() << std::endl;
    ss << "Operacji XOR: " << m_statistics.xorOperations.load() << std::endl;
    ss << "Błędów walidacji: " << m_statistics.validationErrors.load() << std::endl;
    ss << "Błędów pamięci: " << m_statistics.memoryErrors.load() << std::endl;
    ss << "Innych błędów: " << m_statistics.otherErrors.load() << std::endl;
    ss << "Przetworzonych bajtów: " << m_statistics.totalBytesProcessed.load() << std::endl;
    return ss.str();
}

void Encryption::setErrorCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_errorCallback = callback;
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, "Callback błędów zarejestrowany");
}

void Encryption::setCriticalErrorCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(m_logMutex);
    m_criticalErrorCallback = callback;
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, "Callback błędów krytycznych zarejestrowany");
}

// ============================================================================
// IMPLEMENTACJA METOD PRYWATNYCH - LOGOWANIE
// ============================================================================

void Encryption::logMessage(EncryptionTypes::EncryptionLogLevel level, 
                           const std::string& function, 
                           const std::string& message) {
    // Sprawdzenie czy poziom jest wystarczający
    if (level < m_minimumLogLevel) {
        return;
    }
    
    // Sprawdzenie czy wyjście na konsolę jest włączone
    if (!m_consoleOutputEnabled) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_logMutex);
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "[" << logLevelToString(level) << "][ENCRYPTION][" 
       << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] "
       << function << ": " << message << std::endl;
    
    if (level >= EncryptionTypes::EncryptionLogLevel::ERROR) {
        std::cerr << ss.str();
    } else {
        std::cout << ss.str();
    }
    
    // Wywołanie callbacka dla błędów krytycznych
    if (level == EncryptionTypes::EncryptionLogLevel::CRITICAL && m_criticalErrorCallback) {
        try {
            m_criticalErrorCallback(message);
        } catch (...) {
            // Gentle code: ignorujemy błędy w callbacku
        }
    } else if (level >= EncryptionTypes::EncryptionLogLevel::ERROR && m_errorCallback) {
        try {
            m_errorCallback(message);
        } catch (...) {
            // Gentle code: ignorujemy błędy w callbacku
        }
    }
}

std::string Encryption::logLevelToString(EncryptionTypes::EncryptionLogLevel level) {
    switch (level) {
        case EncryptionTypes::EncryptionLogLevel::VERBOSE: return "VERBOSE";
        case EncryptionTypes::EncryptionLogLevel::DEBUG: return "DEBUG";
        case EncryptionTypes::EncryptionLogLevel::INFO: return "INFO";
        case EncryptionTypes::EncryptionLogLevel::WARNING: return "WARNING";
        case EncryptionTypes::EncryptionLogLevel::ERROR: return "ERROR";
        case EncryptionTypes::EncryptionLogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

EncryptionTypes::EncryptionLogLevel Encryption::stringToLogLevel(const std::string& levelStr) {
    if (levelStr == "VERBOSE") return EncryptionTypes::EncryptionLogLevel::VERBOSE;
    if (levelStr == "DEBUG") return EncryptionTypes::EncryptionLogLevel::DEBUG;
    if (levelStr == "INFO") return EncryptionTypes::EncryptionLogLevel::INFO;
    if (levelStr == "WARNING") return EncryptionTypes::EncryptionLogLevel::WARNING;
    if (levelStr == "ERROR") return EncryptionTypes::EncryptionLogLevel::ERROR;
    if (levelStr == "CRITICAL") return EncryptionTypes::EncryptionLogLevel::CRITICAL;
    throw std::invalid_argument("Nieznany poziom logowania: " + levelStr);
}

// Implementacja metod EncryptionStatistics
std::string EncryptionTypes::EncryptionStatistics::toString() const {
    std::stringstream ss;
    ss << "Encrypt: " << encryptOperations.load() 
       << ", Decrypt: " << decryptOperations.load()
       << ", Keys: " << keyGenerations.load()
       << ", B64Enc: " << base64EncodeOps.load()
       << ", B64Dec: " << base64DecodeOps.load()
       << ", XOR: " << xorOperations.load()
       << ", ValErr: " << validationErrors.load()
       << ", MemErr: " << memoryErrors.load()
       << ", OthErr: " << otherErrors.load()
       << ", Bytes: " << totalBytesProcessed.load();
    return ss.str();
}

void EncryptionTypes::EncryptionStatistics::reset() {
    encryptOperations.store(0);
    decryptOperations.store(0);
    keyGenerations.store(0);
    base64EncodeOps.store(0);
    base64DecodeOps.store(0);
    xorOperations.store(0);
    validationErrors.store(0);
    memoryErrors.store(0);
    otherErrors.store(0);
    totalBytesProcessed.store(0);
}
std::string Encryption::encrypt(const std::string& data, const std::string& key) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Rozpoczęcie procesu szyfrowania
    logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, 
               "Rozpoczynanie szyfrowania danych");
    logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
               "Długość danych wejściowych: " + std::to_string(data.length()) + " bajtów");
    logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
               "Długość klucza: " + std::to_string(key.length()) + " bajtów");
    
    // Walidacja parametrów wejściowych - gentle code: bezpieczne sprawdzenie
    try {
        if (key.empty()) {
            m_statistics.validationErrors.fetch_add(1);
            logMessage(EncryptionTypes::EncryptionLogLevel::ERROR, __FUNCTION__, 
                      "Próba szyfrowania z pustym kluczem - zgłaszanie wyjątku");
            throw std::invalid_argument("Klucz szyfrujący nie może być pusty");
        }
        
        if (data.empty()) {
            logMessage(EncryptionTypes::EncryptionLogLevel::WARNING, __FUNCTION__, 
                      "Szyfrowanie pustych danych - zwracanie pustego wyniku");
            return "";  // Gentle code: zwracamy pusty string zamiast błędu
        }
        
        // Dodatkowa walidacja długości klucza (zbyt krótki klucz może być niebezpieczny)
        if (key.length() < MIN_KEY_LENGTH) {
            logMessage(EncryptionTypes::EncryptionLogLevel::WARNING, __FUNCTION__, 
                      "Klucz jest bardzo krótki (" + std::to_string(key.length()) 
                      + " znaków). Zalecany klucz minimum " + std::to_string(MIN_KEY_LENGTH) + " znaków.");
        }
        
        // Sprawdzenie maksymalnego rozmiaru danych
        if (data.length() > MAX_DATA_SIZE) {
            m_statistics.validationErrors.fetch_add(1);
            logMessage(EncryptionTypes::EncryptionLogLevel::ERROR, __FUNCTION__,
                      "Dane przekraczają maksymalny rozmiar (" + std::to_string(MAX_DATA_SIZE) + " bajtów)");
            throw std::invalid_argument("Dane zbyt duże do zaszyfrowania");
        }
        
    } catch (const std::invalid_argument& e) {
        logMessage(EncryptionTypes::EncryptionLogLevel::ERROR, __FUNCTION__, 
                  "Błąd walidacji parametrów: " + std::string(e.what()));
        throw;  // Propagacja wyjątku dalej
    } catch (const std::exception& e) {
        m_statistics.otherErrors.fetch_add(1);
        logMessage(EncryptionTypes::EncryptionLogLevel::CRITICAL, __FUNCTION__, 
                  "Nieoczekiwany błąd podczas walidacji: " + std::string(e.what()));
        throw std::runtime_error("Krytyczny błąd walidacji parametrów szyfrowania");
    }
    
    std::vector<uint8_t> dataBytes;
    std::vector<uint8_t> keyBytes;
    std::vector<uint8_t> encryptedBytes;
    
    try {
        // Konwersja danych i klucza do wektorów bajtów
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__, 
                  "Konwersja danych do wektora bajtów");
        dataBytes.assign(data.begin(), data.end());
        
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__, 
                  "Konwersja klucza do wektora bajtów");
        keyBytes.assign(key.begin(), key.end());
        
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
                  "Liczba bajtów danych: " + std::to_string(dataBytes.size()));
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
                  "Liczba bajtów klucza: " + std::to_string(keyBytes.size()));
        
        // Szyfrowanie XOR
        logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, 
                  "Wykonywanie szyfrowania XOR");
        encryptedBytes = xorCipher(dataBytes, keyBytes);
        
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
                  "Szyfrowanie zakończone, wynik: " + std::to_string(encryptedBytes.size()) + " bajtów");
        
        // Kodowanie Base64
        logMessage(EncryptionTypes::EncryptionLogLevel::DEBUG, __FUNCTION__, 
                  "Kodowanie wyników do Base64");
        std::string result = base64Encode(encryptedBytes);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        double execTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        
        // Aktualizacja statystyk
        m_statistics.encryptOperations.fetch_add(1);
        m_statistics.totalBytesProcessed.fetch_add(data.length());
        
        logMessage(EncryptionTypes::EncryptionLogLevel::INFO, __FUNCTION__, 
                  "Szyfrowanie zakończone sukcesem (czas: " + std::to_string(execTimeMs) + " ms)");
        logMessage(EncryptionTypes::EncryptionLogLevel::VERBOSE, __FUNCTION__,
                  "Długość zaszyfrowanych danych: " + std::to_string(result.length()) + " znaków");
        
        return result;
        
    } catch (const std::bad_alloc& e) {
        m_statistics.memoryErrors.fetch_add(1);
        logMessage(EncryptionTypes::EncryptionLogLevel::CRITICAL, __FUNCTION__, 
                  "Błąd alokacji pamięci podczas szyfrowania: " + std::string(e.what()));
        throw std::runtime_error("Nie udało się zaalokować pamięci dla operacji szyfrowania");
    } catch (const std::exception& e) {
        m_statistics.otherErrors.fetch_add(1);
        logMessage(EncryptionTypes::EncryptionLogLevel::CRITICAL, __FUNCTION__, 
                  "Nieoczekiwany błąd podczas szyfrowania: " + std::string(e.what()));
        throw std::runtime_error("Krytyczny błąd podczas operacji szyfrowania");
    }
}
std::string Encryption::decrypt(const std::string& encryptedData, const std::string& key) {
    // Rozpoczęcie procesu deszyfrowania
    ENCRYPT_DEBUG(1, "Rozpoczynanie deszyfrowania danych");
    ENCRYPT_DEBUG(2, "Długość zaszyfrowanych danych: " << encryptedData.length() << " znaków");
    ENCRYPT_DEBUG(2, "Długość klucza: " << key.length() << " bajtów");
    
    // Walidacja parametrów wejściowych
    try {
        if (key.empty()) {
            ENCRYPT_ERROR("Próba deszyfrowania z pustym kluczem - zgłaszanie wyjątku");
            throw std::invalid_argument("Klucz szyfrujący nie może być pusty");
        }
        
        if (encryptedData.empty()) {
            ENCRYPT_WARN("Deszyfrowanie pustych danych - zwracanie pustego wyniku");
            return "";  // Gentle code: zwracamy pusty string zamiast błędu
        }
        
        // Walidacja formatu Base64 (podstawowa)
        if (encryptedData.length() % 4 != 0) {
            ENCRYPT_WARN("Dane nie są poprawnie wyrównane dla Base64 (długość: " 
                        << encryptedData.length() << "). Spróbuję kontynuować.");
        }
        
    } catch (const std::invalid_argument& e) {
        ENCRYPT_ERROR("Błąd walidacji parametrów: " << e.what());
        throw;
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas walidacji: " << e.what());
        throw std::runtime_error("Krytyczny błąd walidacji parametrów deszyfrowania");
    }
    
    std::vector<uint8_t> encryptedBytes;
    std::vector<uint8_t> keyBytes;
    std::vector<uint8_t> decryptedBytes;
    
    try {
        // Dekodowanie Base64
        ENCRYPT_DEBUG(2, "Dekodowanie danych z Base64");
        encryptedBytes = base64Decode(encryptedData);
        
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(3, "Dekodowanie Base64 zakończone: " << encryptedBytes.size() << " bajtów");
        #endif
        
        // Konwersja klucza do wektora bajtów
        ENCRYPT_DEBUG(3, "Konwersja klucza do wektora bajtów");
        keyBytes.assign(key.begin(), key.end());
        
        // Deszyfrowanie XOR (ta sama operacja co szyfrowanie)
        ENCRYPT_DEBUG(2, "Wykonywanie deszyfrowania XOR");
        decryptedBytes = xorCipher(encryptedBytes, keyBytes);
        
        #if DEBUG_XOR_OPERATIONS
        ENCRYPT_DEBUG(3, "Deszyfrowanie zakończone: " << decryptedBytes.size() << " bajtów");
        #endif
        
        // Konwersja do stringa
        ENCRYPT_DEBUG(2, "Konwersja wyniku do stringa");
        std::string result(decryptedBytes.begin(), decryptedBytes.end());
        
        ENCRYPT_DEBUG(1, "Deszyfrowanie zakończone sukcesem");
        ENCRYPT_DEBUG(2, "Długość odszyfrowanych danych: " << result.length() << " znaków");
        
        return result;
        
    } catch (const std::invalid_argument& e) {
        ENCRYPT_ERROR("Błąd niepoprawnych danych zaszyfrowanych: " << e.what());
        throw std::invalid_argument("Niepoprawny format danych zaszyfrowanych");
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas deszyfrowania: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla operacji deszyfrowania");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas deszyfrowania: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas operacji deszyfrowania");
    }
}

std::string Encryption::generateKey(size_t length) {
    ENCRYPT_DEBUG(1, "Generowanie nowego klucza o długości: " << length << " znaków");
    
    // Walidacja parametru length
    try {
        if (length == 0) {
            ENCRYPT_WARN("Próba wygenerowania klucza o zerowej długości - ustawiam domyślną wartość 32");
            length = 32;  // Gentle code: ustawienie rozsądnej wartości domyślnej
        }
        
        if (length > 1024) {
            ENCRYPT_WARN("Żądana długość klucza (" << length 
                        << ") jest bardzo duża. Ograniczam do 1024 znaków.");
            length = 1024;  // Gentle code: ograniczenie maksymalnej długości
        }
        
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Błąd podczas walidacji długości klucza: " << e.what());
        throw std::runtime_error("Krytyczny błąd walidacji parametru długości klucza");
    }
    
    std::string key;
    
    try {
        key.reserve(length);  // Rezerwacja pamięci z wyprzedzeniem dla wydajności
        
        #if DEBUG_KEY_GENERATION
        ENCRYPT_DEBUG(2, "Pamięć zarezerwowana dla klucza: " << key.capacity() << " znaków");
        ENCRYPT_DEBUG(3, "Rozpoczynanie pętli generowania znaków");
        #endif
        
        for (size_t i = 0; i < length; ++i) {
            // Generowanie losowego znaku z zakresu ASCII drukowalnego (33-126)
            // Zakres 93 znaków: od '!' (33) do '~' (126)
            char randomChar = static_cast<char>(33 + (std::rand() % 93));
            key += randomChar;
            
            #if DEBUG_KEY_GENERATION && DEBUG_ENCRYPTION >= 3
            if (i < 10 || i % 100 == 0) {  // Loguj tylko pierwsze 10 i co 100 znak
                ENCRYPT_DEBUG(3, "Wygenerowano znak #" << i << ": '" << randomChar 
                            << "' (ASCII: " << static_cast<int>(randomChar) << ")");
            }
            #endif
        }
        
        ENCRYPT_DEBUG(1, "Klucz wygenerowany pomyślnie");
        ENCRYPT_DEBUG(2, "Rzeczywista długość klucza: " << key.length() << " znaków");
        
        // Ostrzeżenie jeśli rand() nie został zainicjalizowany (słaba losowość)
        // Uwaga: użytkownik powinien wywołać srand() przed pierwszym użyciem
        ENCRYPT_DEBUG(3, "Uwaga: Upewnij się, że srand() zostało wywołane przed użyciem tej funkcji");
        
        return key;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas generowania klucza: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla generowanego klucza");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas generowania klucza: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas generowania klucza");
    }
}

// ============================================================================
// IMPLEMENTACJA METOD PRYWATNYCH
// ============================================================================

std::string Encryption::base64Encode(const std::vector<uint8_t>& input) {
    #if DEBUG_BASE64_OPERATIONS
    ENCRYPT_DEBUG(2, "Rozpoczynanie kodowania Base64");
    ENCRYPT_DEBUG(3, "Liczba bajtów wejściowych: " << input.size());
    #endif
    
    // Walidacja danych wejściowych
    if (input.empty()) {
        ENCRYPT_DEBUG(3, "Puste dane wejściowe - zwracanie pustego stringa");
        return "";  // Gentle code: puste dane -> pusty wynik
    }
    
    std::string encoded;
    
    try {
        // Rezerwacja pamięci z wyprzedzeniem (Base64 zwiększa rozmiar o ~33%)
        encoded.reserve(((input.size() + 2) / 3) * 4);
        
        int val = 0;
        int valb = -6;
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t charCount = 0;
        #endif
        
        for (uint8_t c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                charCount++;
                #endif
            }
        }
        
        if (valb > -6) {
            encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
            #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
            charCount++;
            #endif
        }
        
        // Dodawanie paddingu (znaki '=')
        size_t paddingNeeded = (4 - (encoded.size() % 4)) % 4;
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(3, "Dodawanie " << paddingNeeded << " znaków paddingu '='");
        #endif
        
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t finalCharCount = charCount;
        #endif
        
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(2, "Kodowanie Base64 zakończone");
        ENCRYPT_DEBUG(3, "Liczba zakodowanych znaków: " << encoded.length());
        #if DEBUG_ENCRYPTION >= 3
        ENCRYPT_DEBUG(3, "Znaki danych: " << finalCharCount << ", Padding: " << paddingNeeded);
        #endif
        #endif
        
        return encoded;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla kodowania Base64");
    } catch (const std::out_of_range& e) {
        ENCRYPT_ERROR("Błąd indeksu podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Błąd indeksu przy dostępie do tabeli Base64");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas kodowania Base64: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas kodowania Base64");
    }
}

std::vector<uint8_t> Encryption::base64Decode(const std::string& input) {
    #if DEBUG_BASE64_OPERATIONS
    ENCRYPT_DEBUG(2, "Rozpoczynanie dekodowania Base64");
    ENCRYPT_DEBUG(3, "Długość danych wejściowych: " << input.length() << " znaków");
    #endif
    
    // Walidacja danych wejściowych
    if (input.empty()) {
        ENCRYPT_DEBUG(3, "Puste dane wejściowe - zwracanie pustego wektora");
        return std::vector<uint8_t>();  // Gentle code: puste dane -> pusty wektor
    }
    
    std::vector<uint8_t> decoded;
    std::vector<int> T(256, -1);
    
    try {
        // Inicjalizacja tabeli dekodującej
        ENCRYPT_DEBUG(3, "Inicjalizacja tabeli dekodującej Base64");
        for (int i = 0; i < 64; i++) {
            T[base64_chars[i]] = i;
        }
        
        #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t byteCount = 0;
        size_t ignoredChars = 0;
        #endif
        
        int val = 0;
        int valb = -8;
        
        for (char c : input) {
            if (T[c] == -1) {
                // Ignoruj znaki inne niż Base64 (w tym '=', spacje, nowe linie)
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                ignoredChars++;
                #endif
                continue;
            }
            
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                uint8_t decodedByte = static_cast<uint8_t>((val >> valb) & 0xFF);
                decoded.push_back(decodedByte);
                valb -= 8;
                
                #if DEBUG_BASE64_OPERATIONS && DEBUG_ENCRYPTION >= 3
                byteCount++;
                if (byteCount <= 10 || byteCount % 100 == 0) {
                    ENCRYPT_DEBUG(3, "Zdekodowano bajt #" << (byteCount-1) 
                                << ": 0x" << std::hex << static_cast<int>(decodedByte) << std::dec);
                }
                #endif
            }
        }
        
        #if DEBUG_BASE64_OPERATIONS
        ENCRYPT_DEBUG(2, "Dekodowanie Base64 zakończone");
        ENCRYPT_DEBUG(3, "Liczba zdekodowanych bajtów: " << decoded.size());
        #if DEBUG_ENCRYPTION >= 3
        ENCRYPT_DEBUG(3, "Zignorowane znaki: " << ignoredChars);
        #endif
        #endif
        
        return decoded;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla dekodowania Base64");
    } catch (const std::out_of_range& e) {
        ENCRYPT_ERROR("Błąd indeksu podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Błąd indeksu przy dostępie do tabeli dekodującej");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas dekodowania Base64: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas dekodowania Base64");
    }
}

std::vector<uint8_t> Encryption::xorCipher(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key) {
    #if DEBUG_XOR_OPERATIONS
    ENCRYPT_DEBUG(2, "Rozpoczynanie operacji XOR Cipher");
    ENCRYPT_DEBUG(3, "Długość danych: " << data.size() << " bajtów");
    ENCRYPT_DEBUG(3, "Długość klucza: " << key.size() << " bajtów");
    #endif
    
    // Walidacja danych wejściowych
    if (data.empty()) {
        ENCRYPT_DEBUG(3, "Puste dane wejściowe - zwracanie pustego wektora");
        return std::vector<uint8_t>();  // Gentle code: puste dane -> pusty wynik
    }
    
    if (key.empty()) {
        ENCRYPT_ERROR("Próba wykonania XOR z pustym kluczem");
        throw std::invalid_argument("Klucz nie może być pusty w operacji XOR");
    }
    
    std::vector<uint8_t> result;
    
    try {
        // Rezerwacja pamięci z wyprzedzeniem
        result.resize(data.size());
        
        #if DEBUG_XOR_OPERATIONS && DEBUG_ENCRYPTION >= 3
        size_t loggedOps = 0;
        #endif
        
        for (size_t i = 0; i < data.size(); ++i) {
            // Operacja XOR z cyklicznym użyciem klucza
            result[i] = data[i] ^ key[i % key.size()];
            
            #if DEBUG_XOR_OPERATIONS && DEBUG_ENCRYPTION >= 3
            loggedOps++;
            if (loggedOps <= 5 || loggedOps % 50 == 0) {
                ENCRYPT_DEBUG(3, "XOR operacja #" << i 
                            << ": data[0x" << std::hex << static_cast<int>(data[i]) << "]"
                            << " ^ key[0x" << static_cast<int>(key[i % key.size()]) << "]"
                            << " = 0x" << static_cast<int>(result[i]) << std::dec);
            }
            #endif
        }
        
        #if DEBUG_XOR_OPERATIONS
        ENCRYPT_DEBUG(2, "Operacja XOR Cipher zakończona");
        ENCRYPT_DEBUG(3, "Liczba przetworzonych bajtów: " << result.size());
        #if DEBUG_ENCRYPTION >= 3
        ENCRYPT_DEBUG(3, "Liczba zalogowanych operacji: " << loggedOps);
        #endif
        #endif
        
        return result;
        
    } catch (const std::bad_alloc& e) {
        ENCRYPT_ERROR("Błąd alokacji pamięci podczas operacji XOR: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla operacji XOR");
    } catch (const std::out_of_range& e) {
        ENCRYPT_ERROR("Błąd indeksu podczas operacji XOR: " << e.what());
        throw std::runtime_error("Błąd indeksu przy dostępie do danych/klucza");
    } catch (const std::exception& e) {
        ENCRYPT_ERROR("Nieoczekiwany błąd podczas operacji XOR: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas operacji XOR Cipher");
    }
}
