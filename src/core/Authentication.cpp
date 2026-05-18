/**
 * @file Authentication.cpp
 * @brief Implementacja systemu autentykacji użytkowników
 * 
 * Ten plik zawiera implementację mechanizmów autentykacji, zarządzania użytkownikami
 * oraz szyfrowania haseł. Kod zawiera rozbudowane mechanizmy debugowania, obsługi 
 * błędów, wyjątków oraz logowania operacji.
 * 
 * @author AI Assistant
 * @date 2025
 */

#include "core/Authentication.hpp"
#include "core/ConfigManager.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <mutex>
#include <cstdlib>

// ============================================================================
// FLAGI DEBUGOWANIA - można włączać/wyłączać poszczególne moduły
// ============================================================================
#ifndef DEBUG_AUTHENTICATION
    #define DEBUG_AUTHENTICATION 1          ///< Główna flaga debugowania autentykacji
#endif

#ifndef DEBUG_USER_OPERATIONS
    #define DEBUG_USER_OPERATIONS 1         ///< Flag dla operacji na użytkownikach
#endif

#ifndef DEBUG_PASSWORD_ENCRYPTION
    #define DEBUG_PASSWORD_ENCRYPTION 1     ///< Flag dla szyfrowania haseł
#endif

#ifndef DEBUG_JSON_PARSING
    #define DEBUG_JSON_PARSING 1            ///< Flag dla parsowania JSON
#endif

#ifndef DEBUG_FILE_OPERATIONS
    #define DEBUG_FILE_OPERATIONS 1         ///< Flag dla operacji na plikach
#endif

// ============================================================================
// MAKRA POMOCNICZE DO LOGOWANIA I DEBUGOWANIA
// ============================================================================

/**
 * @brief Makro do logowania komunikatów debugowych
 */
#define AUTH_DEBUG(level, msg) \
    do { \
        if (DEBUG_AUTHENTICATION && level <= DEBUG_AUTHENTICATION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[DEBUG][AUTH][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

/**
 * @brief Makro do logowania błędów
 */
#define AUTH_ERROR(msg) \
    do { \
        auto now = std::chrono::system_clock::now(); \
        auto time = std::chrono::system_clock::to_time_t(now); \
        std::stringstream ss; \
        ss << "[ERROR][AUTH][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
           << __FUNCTION__ << ": " << msg << std::endl; \
        std::cerr << ss.str(); \
    } while(0)

/**
 * @brief Makro do logowania ostrzeżeń
 */
#define AUTH_WARN(msg) \
    do { \
        if (DEBUG_AUTHENTICATION) { \
            auto now = std::chrono::system_clock::now(); \
            auto time = std::chrono::system_clock::to_time_t(now); \
            std::stringstream ss; \
            ss << "[WARN][AUTH][" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "] " \
               << __FUNCTION__ << ": " << msg << std::endl; \
            std::cerr << ss.str(); \
        } \
    } while(0)

namespace biofeedback {
namespace {

// ============================================================================
// FUNKCJE POMOCNICZE - ESCAPE/UNESCAPE JSON
// ============================================================================

/**
 * @brief Escapuje znaki specjalne w stringu dla formatu JSON
 * 
 * @param input String do escapowania
 * @return String z escapowanymi znakami specjalnymi
 * 
 * Gentle code: funkcja jest odporna na puste inputy i znaki nie-ASCII
 */
std::string escapeJsonString(const std::string& input)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Escapowanie stringa JSON, długość wejścia: " << input.length());
    #endif
    
    // Walidacja wejścia - gentle code
    if (input.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Pusty input - zwracanie pustego stringa");
        #endif
        return "";
    }
    
    std::string output;
    
    try {
        output.reserve(input.size());

        for (const char c : input) {
            switch (c) {
                case '"': output += "\\\""; break;
                case '\\': output += "\\\\"; break;
                case '\b': output += "\\b"; break;
                case '\f': output += "\\f"; break;
                case '\n': output += "\\n"; break;
                case '\r': output += "\\r"; break;
                case '\t': output += "\\t"; break;
                default: output += c; break;
            }
        }

        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Escapowanie zakończone, długość wyjścia: " << output.length());
        #endif

        return output;
        
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas escapowania JSON: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla escapowania JSON");
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas escapowania JSON: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas escapowania JSON");
    }
}

/**
 * @brief Odwrotnie escapuje znaki specjalne z formatu JSON
 * 
 * @param input String z escapowanymi znakami
 * @return String z przywróconymi oryginalnymi znakami
 * 
 * Gentle code: funkcja ignoruje niepoprawne sekwencje escapowania
 */
std::string unescapeJsonString(const std::string& input)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Unescapowanie stringa JSON, długość wejścia: " << input.length());
    #endif
    
    // Walidacja wejścia - gentle code
    if (input.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Pusty input - zwracanie pustego stringa");
        #endif
        return "";
    }
    
    std::string output;
    
    try {
        output.reserve(input.size());

        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '\\' && i + 1 < input.size()) {
                switch (input[i + 1]) {
                    case '"': output += '"'; ++i; break;
                    case '\\': output += '\\'; ++i; break;
                    case 'b': output += '\b'; ++i; break;
                    case 'f': output += '\f'; ++i; break;
                    case 'n': output += '\n'; ++i; break;
                    case 'r': output += '\r'; ++i; break;
                    case 't': output += '\t'; ++i; break;
                    default: 
                        // Nieznana sekwencja escapowania - zachowaj oryginał (gentle code)
                        #if DEBUG_JSON_PARSING
                        AUTH_WARN("Nieznana sekwencja escapowania: \\" << input[i+1] << " - zachowanie oryginału");
                        #endif
                        output += input[i]; 
                        break;
                }
            } else {
                output += input[i];
            }
        }

        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Unescapowanie zakończone, długość wyjścia: " << output.length());
        #endif

        return output;
        
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas unescapowania JSON: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla unescapowania JSON");
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas unescapowania JSON: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas unescapowania JSON");
    }
}

/**
 * @brief Znajduje koniec stringa JSON z uwzględnieniem escapowania
 * 
 * @param text Tekst JSON do przeszukania
 * @param start Pozycja startowa (pierwszy znak po otwierającym cudzysłowie)
 * @return Pozycja zamykającego cudzysłowu lub npos jeśli nie znaleziono
 */
size_t findStringTerminator(const std::string& text, const size_t start)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Szukanie końca stringa od pozycji: " << start);
    #endif
    
    try {
        for (size_t i = start; i < text.size(); ++i) {
            if (text[i] != '"') {
                continue;
            }

            size_t backslashes = 0;
            for (size_t j = i; j > 0 && text[j - 1] == '\\'; --j) {
                ++backslashes;
            }

            if (backslashes % 2 == 0) {
                #if DEBUG_JSON_PARSING
                AUTH_DEBUG(3, "Znaleziono koniec stringa na pozycji: " << i);
                #endif
                return i;
            }
        }

        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(2, "Nie znaleziono końca stringa - zwracanie npos");
        #endif
        return std::string::npos;
        
    } catch (const std::exception& e) {
        AUTH_ERROR("Błąd podczas szukania terminatora stringa: " << e.what());
        return std::string::npos;  // Gentle code: zwracamy npos przy błędzie
    }
}

/**
 * @brief Ekstrahuje wartość pola string z JSON
 * 
 * @param jsonText Pełny tekst JSON
 * @param key Nazwa klucza do ekstrakcji
 * @return Wartość pola lub pusty string jeśli nie znaleziono
 */
std::string extractJsonStringField(const std::string& jsonText, const std::string& key)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Ekstrakcja pola string: \"" << key << "\"");
    #endif
    
    // Walidacja wejścia - gentle code
    if (jsonText.empty() || key.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_WARN("Pusty jsonText lub key - zwracanie pustego stringa");
        #endif
        return "";
    }
    
    try {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = jsonText.find(searchKey);
        if (keyPos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nie znaleziono klucza: " << key);
            #endif
            return "";
        }

        const size_t colonPos = jsonText.find(':', keyPos + searchKey.size());
        if (colonPos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nie znaleziono dwukropka po kluczu: " << key);
            #endif
            return "";
        }

        const size_t quotePos = jsonText.find('"', colonPos + 1);
        if (quotePos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nie znaleziono otwierającego cudzysłowu wartości");
            #endif
            return "";
        }

        const size_t endQuotePos = findStringTerminator(jsonText, quotePos + 1);
        if (endQuotePos == std::string::npos || endQuotePos <= quotePos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nieprawidłowy zakres stringa wartości");
            #endif
            return "";
        }

        std::string result = unescapeJsonString(jsonText.substr(quotePos + 1, endQuotePos - quotePos - 1));
        
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Ekstrakcja zakończona sukcesem, długość: " << result.length());
        #endif
        
        return result;
        
    } catch (const std::out_of_range& e) {
        AUTH_ERROR("Błąd zakresu podczas ekstrakcji pola: " << e.what());
        return "";  // Gentle code: zwracamy pusty string przy błędzie
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas ekstrakcji pola: " << e.what());
        return "";  // Gentle code: zwracamy pusty string przy błędzie
    }
}

/**
 * @brief Ekstrahuje wartość pola bool z JSON
 * 
 * @param jsonText Pełny tekst JSON
 * @param key Nazwa klucza do ekstrakcji
 * @param defaultValue Wartość domyślna jeśli klucz nie istnieje
 * @return Wartość pola lub defaultValue jeśli nie znaleziono
 */
bool extractJsonBoolField(const std::string& jsonText, const std::string& key, const bool defaultValue)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Ekstrakcja pola bool: \"" << key << "\", default: " << defaultValue);
    #endif
    
    // Walidacja wejścia - gentle code
    if (jsonText.empty() || key.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_WARN("Pusty jsonText lub key - zwracanie default: " << defaultValue);
        #endif
        return defaultValue;
    }
    
    try {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = jsonText.find(searchKey);
        if (keyPos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nie znaleziono klucza - zwracanie default");
            #endif
            return defaultValue;
        }

        size_t pos = jsonText.find(':', keyPos + searchKey.size());
        if (pos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Nie znaleziono dwukropka - zwracanie default");
            #endif
            return defaultValue;
        }

        ++pos;
        while (pos < jsonText.size() && std::isspace(static_cast<unsigned char>(jsonText[pos]))) {
            ++pos;
        }

        if (jsonText.compare(pos, 4, "true") == 0) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Znaleziono wartość: true");
            #endif
            return true;
        }
        if (jsonText.compare(pos, 5, "false") == 0) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(3, "Znaleziono wartość: false");
            #endif
            return false;
        }

        #if DEBUG_JSON_PARSING
        AUTH_WARN("Nieznana wartość bool - zwracanie default");
        #endif
        return defaultValue;
        
    } catch (const std::out_of_range& e) {
        AUTH_ERROR("Błąd zakresu podczas ekstrakcji bool: " << e.what());
        return defaultValue;  // Gentle code
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas ekstrakcji bool: " << e.what());
        return defaultValue;  // Gentle code
    }
}

/**
 * @brief Znajduje tablicę "users" w JSON
 * 
 * @param jsonContent Zawartość JSON
 * @return Fragment JSON z tablicą users lub pusty string jeśli nie znaleziono
 */
std::string findUsersArray(const std::string& jsonContent)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Szukanie tablicy users w JSON");
    #endif
    
    // Walidacja wejścia - gentle code
    if (jsonContent.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_WARN("Pusta zawartość JSON");
        #endif
        return "";
    }
    
    try {
        const std::string key = "\"users\"";
        const size_t keyPos = jsonContent.find(key);
        if (keyPos == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(2, "Nie znaleziono klucza \"users\"");
            #endif
            return "";
        }

        const size_t bracketStart = jsonContent.find('[', keyPos + key.size());
        if (bracketStart == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(2, "Nie znaleziono otwierającego nawiasu tablicy");
            #endif
            return "";
        }

        bool inString = false;
        int depth = 0;
        size_t bracketEnd = std::string::npos;
        
        for (size_t i = bracketStart; i < jsonContent.size(); ++i) {
            const char c = jsonContent[i];

            if (c == '"') {
                size_t backslashes = 0;
                for (size_t j = i; j > 0 && jsonContent[j - 1] == '\\'; --j) {
                    ++backslashes;
                }
                if (backslashes % 2 == 0) {
                    inString = !inString;
                }
            }

            if (inString) {
                continue;
            }

            if (c == '[') {
                ++depth;
            } else if (c == ']') {
                --depth;
                if (depth == 0) {
                    bracketEnd = i;
                    break;
                }
            }
        }
        
        if (bracketEnd == std::string::npos) {
            #if DEBUG_JSON_PARSING
            AUTH_DEBUG(2, "Nie znaleziono zamykającego nawiasu tablicy");
            #endif
            return "";
        }

        std::string result = jsonContent.substr(bracketStart, bracketEnd - bracketStart + 1);
        
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Znaleziono tablicę users, długość: " << result.length());
        #endif
        
        return result;
        
    } catch (const std::out_of_range& e) {
        AUTH_ERROR("Błąd zakresu podczas szukania tablicy users: " << e.what());
        return "";  // Gentle code
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas szukania tablicy users: " << e.what());
        return "";  // Gentle code
    }
}

/**
 * @brief Dzieli tablicę JSON na pojedyncze obiekty
 * 
 * @param arrayJson Tablica JSON jako string
 * @return Wektor obiektów JSON
 */
std::vector<std::string> splitJsonObjectsFromArray(const std::string& arrayJson)
{
    #if DEBUG_JSON_PARSING
    AUTH_DEBUG(3, "Dzielenie tablicy JSON na obiekty, długość: " << arrayJson.length());
    #endif
    
    std::vector<std::string> objects;
    
    // Walidacja wejścia - gentle code
    if (arrayJson.empty()) {
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(3, "Pusta tablica - zwracanie pustego wektora");
        #endif
        return objects;
    }
    
    try {
        bool inString = false;
        int depth = 0;
        size_t objStart = std::string::npos;

        for (size_t i = 0; i < arrayJson.size(); ++i) {
            const char c = arrayJson[i];

            if (c == '"') {
                size_t backslashes = 0;
                for (size_t j = i; j > 0 && arrayJson[j - 1] == '\\'; --j) {
                    ++backslashes;
                }
                if (backslashes % 2 == 0) {
                    inString = !inString;
                }
            }

            if (inString) {
                continue;
            }

            if (c == '{') {
                if (depth == 0) {
                    objStart = i;
                }
                ++depth;
                continue;
            }

            if (c == '}') {
                --depth;
                if (depth == 0 && objStart != std::string::npos) {
                    objects.push_back(arrayJson.substr(objStart, i - objStart + 1));
                    #if DEBUG_JSON_PARSING
                    AUTH_DEBUG(3, "Dodano obiekt #" << objects.size() << ", długość: " << objects.back().length());
                    #endif
                    objStart = std::string::npos;
                }
            }
        }
        
        #if DEBUG_JSON_PARSING
        AUTH_DEBUG(2, "Podział zakończony, liczba obiektów: " << objects.size());
        #endif

        return objects;
        
    } catch (const std::out_of_range& e) {
        AUTH_ERROR("Błąd zakresu podczas dzielenia tablicy JSON: " << e.what());
        return objects;  // Gentle code: zwracamy co mamy
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas dzielenia tablicy JSON: " << e.what());
        return objects;  // Gentle code
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas dzielenia tablicy JSON: " << e.what());
        return objects;  // Gentle code
    }
}

} // namespace

// ============================================================================
// IMPLEMENTACJA KLASY UserData
// ============================================================================

/**
 * @brief Konwertuje dane użytkownika do formatu JSON
 * 
 * @return String z reprezentacją JSON danych użytkownika
 */
std::string UserData::toJson() const
{
    #if DEBUG_USER_OPERATIONS
    // Security: Do not log raw username - use redacted form
    AUTH_DEBUG(3, "Konwersja UserData do JSON dla użytkownika: [REDACTED] (length: " << username.length() << ")");
    #endif
    
    try {
        std::ostringstream json;
        json << "{\n";
        json << "  \"username\": \"" << escapeJsonString(username) << "\",\n";
        json << "  \"password\": \"" << escapeJsonString(password) << "\",\n";
        json << "  \"role\": \"" << escapeJsonString(role) << "\",\n";
        json << "  \"encrypted\": " << (encrypted ? "true" : "false") << "\n";
        json << "}";
        
        #if DEBUG_USER_OPERATIONS
        AUTH_DEBUG(3, "Konwersja do JSON zakończona sukcesem");
        #endif
        
        return json.str();
        
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas konwersji do JSON: " << e.what());
        throw std::runtime_error("Nie udało się zaalokować pamięci dla konwersji UserData do JSON");
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas konwersji do JSON: " << e.what());
        throw std::runtime_error("Krytyczny błąd podczas konwersji UserData do JSON");
    }
}

/**
 * @brief Parsuje dane użytkownika z formatu JSON
 * 
 * @param jsonText String z danymi JSON
 * @return Obiekt UserData z parsowanymi danymi
 * 
 * Gentle code: zwraca pustego UserData jeśli parsowanie się nie powiedzie
 */
UserData UserData::fromJson(const std::string& jsonText)
{
    #if DEBUG_USER_OPERATIONS
    AUTH_DEBUG(3, "Parsowanie UserData z JSON");
    #endif
    
    // Walidacja wejścia - gentle code
    if (jsonText.empty()) {
        #if DEBUG_USER_OPERATIONS
        AUTH_WARN("Pusty jsonText - zwracanie pustego UserData");
        #endif
        return UserData{};
    }
    
    try {
        UserData user{};
        user.username = extractJsonStringField(jsonText, "username");
        user.password = extractJsonStringField(jsonText, "password");
        user.role = extractJsonStringField(jsonText, "role");
        user.encrypted = extractJsonBoolField(jsonText, "encrypted", false);
        
        #if DEBUG_USER_OPERATIONS
        // Security: Do not log raw username - use redacted form
        AUTH_DEBUG(3, "Parsowanie zakończone, username: [REDACTED] (length: " << user.username.length() << "), role: " << user.role);
        #endif
        
        return user;
        
    } catch (const std::exception& e) {
        AUTH_ERROR("Błąd podczas parsowania UserData z JSON: " << e.what());
        return UserData{};  // Gentle code: zwracamy pustego UserData
    }
}

// ============================================================================
// Authentication - Implementation
// ============================================================================

/**
 * @brief Konstruktor klasy Authentication
 * 
 * @param usersFilePath Ścieżka do pliku z danymi użytkowników
 */
Authentication::Authentication(const std::string& usersFilePath)
    : usersFilePath(usersFilePath)
{
    AUTH_DEBUG(1, "Tworzenie instancji Authentication");
    AUTH_DEBUG(2, "Ścieżka do pliku użytkowników: " << usersFilePath);
    
    // Walidacja parametru - gentle code
    if (usersFilePath.empty()) {
        AUTH_WARN("Pusta ścieżka do pliku użytkowników");
    }
}

/**
 * @brief Destruktor klasy Authentication
 */
Authentication::~Authentication()
{
    AUTH_DEBUG(2, "Niszczenie instancji Authentication");
    
    // Bezpieczne czyszczenie wrażliwych danych
    currentUser = nullptr;
    encryptionKey.clear();
}

/**
 * @brief Wczytuje użytkowników z pliku
 * 
 * @return true jeśli wczytywanie powiodło się, false w przeciwnym razie
 */
bool Authentication::loadUsers()
{
    AUTH_DEBUG(1, "Rozpoczynanie wczytywania użytkowników z pliku");
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        currentUser = nullptr;

        // Sprawdzenie czy ścieżka nie jest pusta
        if (usersFilePath.empty()) {
            AUTH_ERROR("Pusta ścieżka do pliku użytkowników");
            return false;
        }

        std::ifstream file(usersFilePath);
        if (!file.is_open()) {
            AUTH_ERROR("Nie można otworzyć pliku użytkowników: " << usersFilePath);
            return false;
        }
        
        #if DEBUG_FILE_OPERATIONS
        AUTH_DEBUG(2, "Plik otwarty sukcesem: " << usersFilePath);
        #endif

        std::ostringstream content;
        content << file.rdbuf();
        std::string jsonContent = content.str();
        file.close();
        
        #if DEBUG_FILE_OPERATIONS
        AUTH_DEBUG(2, "Wczytano zawartość pliku, rozmiar: " << jsonContent.length() << " bajtów");
        #endif

        users.clear();

        const std::string usersArray = findUsersArray(jsonContent);
        if (usersArray.empty()) {
            AUTH_ERROR("Nieprawidłowy format pliku użytkowników (brak tablicy users)");
            return false;
        }

        const auto objects = splitJsonObjectsFromArray(usersArray);
        size_t loadedCount = 0;
        
        for (const auto& objectJson : objects) {
            try {
                UserData user = UserData::fromJson(objectJson);
                if (!user.username.empty()) {
                    users.push_back(user);
                    loadedCount++;
                    #if DEBUG_USER_OPERATIONS
                    // Security: Do not log raw username - use redacted form
                    AUTH_DEBUG(3, "Wczytano użytkownika: [REDACTED] (length: " << user.username.length() << ")");
                    #endif
                } else {
                    #if DEBUG_USER_OPERATIONS
                    AUTH_WARN("Pominięto użytkownika z pustym username");
                    #endif
                }
            } catch (const std::exception& e) {
                AUTH_WARN("Błąd podczas parsowania pojedynczego użytkownika: " << e.what());
                // Kontynuujemy z następnym użytkownikiem (gentle code)
            }
        }

        AUTH_DEBUG(1, "Wczytano " << loadedCount << " użytkowników");
        return loadedCount > 0 || users.empty();  // Sukces jeśli coś wczytano lub plik był pusty
        
    } catch (const std::ios_base::failure& e) {
        AUTH_ERROR("Błąd I/O podczas wczytywania pliku: " << e.what());
        return false;
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas wczytywania użytkowników: " << e.what());
        return false;
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas wczytywania użytkowników: " << e.what());
        return false;
    }
}

/**
 * @brief Zapisuje użytkowników do pliku
 * 
 * @return true jeśli zapisywanie powiodło się, false w przeciwnym razie
 */
bool Authentication::saveUsers()
{
    AUTH_DEBUG(1, "Rozpoczynanie zapisywania użytkowników do pliku");
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    AUTH_DEBUG(2, "Liczba użytkowników do zapisu: " << users.size());
    
    try {
        // Sprawdzenie czy ścieżka nie jest pusta
        if (usersFilePath.empty()) {
            AUTH_ERROR("Pusta ścieżka do pliku użytkowników");
            return false;
        }

        std::ofstream file(usersFilePath);
        if (!file.is_open()) {
            AUTH_ERROR("Nie można zapisać do pliku użytkowników: " << usersFilePath);
            return false;
        }
        
        #if DEBUG_FILE_OPERATIONS
        AUTH_DEBUG(2, "Plik otwarty do zapisu: " << usersFilePath);
        #endif

        file << "{\n    \"users\": [\n";

        for (size_t i = 0; i < users.size(); ++i) {
            try {
                file << "        " << users[i].toJson();
                if (i < users.size() - 1) {
                    file << ",";
                }
                file << "\n";
            } catch (const std::exception& e) {
                AUTH_WARN("Błąd podczas zapisywania użytkownika #" << i << ": " << e.what());
                // Kontynuujemy z następnym użytkownikiem (gentle code)
            }
        }

        file << "    ]\n}\n";
        
        if (!file.good()) {
            AUTH_ERROR("Błąd podczas zapisywania do pliku");
            file.close();
            return false;
        }
        
        file.close();

        AUTH_DEBUG(1, "Zapisano " << users.size() << " użytkowników");
        return true;
        
    } catch (const std::ios_base::failure& e) {
        AUTH_ERROR("Błąd I/O podczas zapisywania pliku: " << e.what());
        return false;
    } catch (const std::bad_alloc& e) {
        AUTH_ERROR("Błąd alokacji pamięci podczas zapisywania użytkowników: " << e.what());
        return false;
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas zapisywania użytkowników: " << e.what());
        return false;
    }
}

/**
 * @brief Szuka użytkownika po nazwie
 * 
 * @param username Nazwa użytkownika do znalezienia
 * @return Wskaźnik do UserData lub nullptr jeśli nie znaleziono
 */
UserData* Authentication::findUser(const std::string& username)
{
    #if DEBUG_USER_OPERATIONS
    // Security: Do not log raw username - use redacted form
    AUTH_DEBUG(3, "Szukanie użytkownika: [REDACTED] (length: " << username.length() << ")");
    #endif
    
    // Walidacja wejścia - gentle code
    if (username.empty()) {
        AUTH_WARN("Pusta nazwa użytkownika");
        return nullptr;
    }
    
    try {
        for (auto& user : users) {
            if (user.username == username) {
                #if DEBUG_USER_OPERATIONS
                // Security: Do not log found username - just indicate success
                AUTH_DEBUG(3, "Znaleziono użytkownika: [REDACTED]");
                #endif
                return &user;
            }
        }
        
        #if DEBUG_USER_OPERATIONS
        // Security: Do not log which username was not found
        AUTH_DEBUG(3, "Nie znaleziono użytkownika: [REDACTED]");
        #endif
        return nullptr;
        
    } catch (const std::exception& e) {
        AUTH_ERROR("Błąd podczas szukania użytkownika: " << e.what());
        return nullptr;  // Gentle code
    }
}

/**
 * @brief Szyfruje hasło za pomocą klucza
 * 
 * @param password Hasło do zaszyfrowania
 * @return Zaszyfrowane hasło lub oryginalne jeśli błąd
 */
std::string Authentication::encryptPassword(const std::string& password)
{
    #if DEBUG_PASSWORD_ENCRYPTION
    AUTH_DEBUG(2, "Szyfrowanie hasła, długość: " << password.length());
    #endif
    
    // Gentle code: obsługa pustego hasła
    if (password.empty()) {
        AUTH_WARN("Próba szyfrowania pustego hasła");
        return "";
    }
    
    try {
        if (encryptionKey.empty()) {
            // Pobranie klucza z zmiennej środowiskowej lub ConfigManager
            const char* envKey = std::getenv("BIOFEEDBACK_ENCRYPTION_KEY");
            if (envKey && std::string(envKey).length() >= 16) {
                encryptionKey = std::string(envKey);
                AUTH_DEBUG(2, "Użyto klucza z zmiennej środowiskowej BIOFEEDBACK_ENCRYPTION_KEY");
            } else {
                // Jeśli brak zmiennej środowiskowej, użyj klucza z ConfigManager
                auto& config = core::ConfigManager::instance();
                encryptionKey = config.getEncryptionKey();
                
                if (encryptionKey.empty() || encryptionKey.length() < 16) {
                    AUTH_ERROR("Brak poprawnego klucza szyfrującego. Konfiguracja wymaga ustawienia klucza min. 16 znaków.");
                    throw std::runtime_error("Niepoprawny klucz szyfrujący - wymagany klucz min. 16 znaków");
                }
                AUTH_DEBUG(2, "Użyto klucza z ConfigManager");
            }
        }

        std::string result = Encryption::encrypt(password, encryptionKey);
        
        #if DEBUG_PASSWORD_ENCRYPTION
        AUTH_DEBUG(2, "Szyfrowanie hasła zakończone sukcesem");
        #endif
        
        return result;
        
    } catch (const std::invalid_argument& e) {
        AUTH_ERROR("Błąd argumentu podczas szyfrowania hasła: " << e.what());
        return password;  // Gentle code: zwracamy oryginał
    } catch (const std::exception& e) {
        AUTH_ERROR("Błąd szyfrowania hasła: " << e.what());
        return password;  // Gentle code: zwracamy oryginał zamiast rzucać wyjątek
    }
}

/**
 * @brief Deszyfruje hasło za pomocą klucza
 * 
 * @param encryptedPassword Zaszyfrowane hasło
 * @return Odszyfrowane hasło lub pusty string jeśli błąd
 */
std::string Authentication::decryptPassword(const std::string& encryptedPassword)
{
    #if DEBUG_PASSWORD_ENCRYPTION
    AUTH_DEBUG(2, "Deszyfrowanie hasła");
    #endif
    
    // Gentle code: obsługa pustego hasła
    if (encryptedPassword.empty()) {
        AUTH_WARN("Próba deszyfrowania pustego hasła");
        return "";
    }
    
    try {
        if (encryptionKey.empty()) {
            // Pobranie klucza z zmiennej środowiskowej lub ConfigManager
            const char* envKey = std::getenv("BIOFEEDBACK_ENCRYPTION_KEY");
            if (envKey && std::string(envKey).length() >= 16) {
                encryptionKey = std::string(envKey);
                AUTH_DEBUG(2, "Użyto klucza z zmiennej środowiskowej BIOFEEDBACK_ENCRYPTION_KEY");
            } else {
                // Jeśli brak zmiennej środowiskowej, użyj klucza z ConfigManager
                auto& config = core::ConfigManager::instance();
                encryptionKey = config.getEncryptionKey();
                
                if (encryptionKey.empty() || encryptionKey.length() < 16) {
                    AUTH_ERROR("Brak poprawnego klucza szyfrującego. Konfiguracja wymaga ustawienia klucza min. 16 znaków.");
                    throw std::runtime_error("Niepoprawny klucz szyfrujący - wymagany klucz min. 16 znaków");
                }
                AUTH_DEBUG(2, "Użyto klucza z ConfigManager");
            }
        }

        std::string result = Encryption::decrypt(encryptedPassword, encryptionKey);
        
        #if DEBUG_PASSWORD_ENCRYPTION
        AUTH_DEBUG(2, "Deszyfrowanie hasła zakończone sukcesem");
        #endif
        
        return result;
        
    } catch (const std::invalid_argument& e) {
        AUTH_ERROR("Błąd argumentu podczas deszyfrowania hasła: " << e.what());
        return "";  // Gentle code
    } catch (const std::exception& e) {
        AUTH_ERROR("Błąd deszyfrowania hasła: " << e.what());
        return "";  // Gentle code: zwracamy pusty string zamiast rzucać wyjątek
    }
}

/**
 * @brief Szyfruje hasła wszystkich nieszyfrowanych użytkowników
 * 
 * @param key Klucz szyfrujący
 * @return Liczba zaszyfrowanych haseł
 */
int Authentication::encryptPasswordsIfNeeded(const std::string& key)
{
    AUTH_DEBUG(1, "Rozpoczynanie szyfrowania haseł");
    
    // Walidacja parametru
    if (key.empty()) {
        AUTH_ERROR("Klucz szyfrujący jest pusty");
        return 0;
    }
    
    int encryptedCount = 0;
    
    try {
        encryptionKey = key;
        AUTH_DEBUG(2, "Ustawiono klucz szyfrujący");

        for (auto& user : users) {
            try {
                if (!user.encrypted && !user.password.empty()) {
                    // Zaszyfruj hasło
                    user.password = encryptPassword(user.password);
                    user.encrypted = true;
                    encryptedCount++;
                    
                    #if DEBUG_PASSWORD_ENCRYPTION
                    // Security: Do not log raw username - use redacted form
                    AUTH_DEBUG(2, "Zaszyfrowano hasło dla użytkownika: [REDACTED] (length: " << user.username.length() << ")");
                    #endif
                }
            } catch (const std::exception& e) {
                // Security: Do not log raw username in error message
                AUTH_WARN("Błąd podczas szyfrowania hasła dla użytkownika: [REDACTED]: " << e.what());
                // Kontynuujemy z następnym użytkownikiem (gentle code)
            }
        }

        // Zapisz zaktualizowane dane
        if (encryptedCount > 0) {
            AUTH_DEBUG(2, "Zapisywanie zaktualizowanych danych użytkowników");
            saveUsers();
        }

        AUTH_DEBUG(1, "Zaszyfrowano " << encryptedCount << " haseł");
        return encryptedCount;
        
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas szyfrowania haseł: " << e.what());
        return encryptedCount;  // Zwracamy to co udało się zrobić
    }
}

/**
 * @brief Loguje użytkownika
 * 
 * @param username Nazwa użytkownika
 * @param password Hasło
 * @return true jeśli logowanie powiodło się, false w przeciwnym razie
 */
bool Authentication::login(const std::string& username, const std::string& password)
{
    // Security: Do not log raw username - use redacted form with length only
    AUTH_DEBUG(1, "Próba logowania użytkownika: [REDACTED] (length: " << username.length() << ")");
    
    // Walidacja parametrów - gentle code
    if (username.empty()) {
        AUTH_WARN("Pusta nazwa użytkownika");
        return false;
    }
    
    // Uwaga: puste hasło może być dozwolone w niektórych systemach
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    try {
        UserData* user = findUser(username);

        if (user == nullptr) {
            currentUser = nullptr;
            // Security: Do not log the username that was not found
            AUTH_DEBUG(2, "Nie znaleziono użytkownika: [REDACTED]");
            return false;
        }

        std::string storedPassword;

        try {
            if (user->encrypted) {
                // Hasło jest zaszyfrowane - musimy je odszyfrować do porównania
                #if DEBUG_PASSWORD_ENCRYPTION
                AUTH_DEBUG(3, "Hasło jest zaszyfrowane - deszyfrowanie");
                #endif
                storedPassword = decryptPassword(user->password);
            } else {
                // Hasło nie jest zaszyfrowane - używamy bezpośrednio
                #if DEBUG_PASSWORD_ENCRYPTION
                AUTH_DEBUG(3, "Hasło nie jest zaszyfrowane");
                #endif
                storedPassword = user->password;
            }
        } catch (const std::exception& e) {
            AUTH_ERROR("Błąd podczas deszyfrowania hasła: " << e.what());
            currentUser = nullptr;
            return false;
        }

        if (storedPassword == password) {
            currentUser = user;
            // Security: Log success without exposing username, only role
            AUTH_DEBUG(1, "Zalogowano użytkownika: [REDACTED] (rola: " << user->role << ")");
            return true;
        }

        currentUser = nullptr;
        // Security: Do not reveal which username had invalid password
        AUTH_DEBUG(2, "Nieprawidłowe hasło dla użytkownika: [REDACTED]");
        return false;
        
    } catch (const std::exception& e) {
        AUTH_ERROR("Nieoczekiwany błąd podczas logowania: " << e.what());
        currentUser = nullptr;
        return false;
    }
}

/**
 * @brief Wylogowuje aktualnego użytkownika
 */
void Authentication::logout()
{
    AUTH_DEBUG(1, "Wylogowywanie użytkownika");
    
    std::lock_guard<std::mutex> lock(m_mutex);
    currentUser = nullptr;
    
    #if DEBUG_USER_OPERATIONS
    AUTH_DEBUG(2, "Użytkownik wylogowany");
    #endif
}

/**
 * @brief Sprawdza czy użytkownik jest zalogowany
 * 
 * @return true jeśli użytkownik jest zalogowany, false w przeciwnym razie
 */
bool Authentication::isLoggedIn() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return currentUser != nullptr;
}

/**
 * @brief Pobiera nazwę aktualnie zalogowanego użytkownika
 * 
 * @return Nazwa użytkownika lub pusty string jeśli nikt nie jest zalogowany
 */
std::string Authentication::getCurrentUsername() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (currentUser) {
        return currentUser->username;
    }
    return "";
}

/**
 * @brief Pobiera rolę aktualnie zalogowanego użytkownika
 * 
 * @return Rola użytkownika lub pusty string jeśli nikt nie jest zalogowany
 */
std::string Authentication::getCurrentUserRole() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (currentUser) {
        return currentUser->role;
    }
    return "";
}

/**
 * @brief Ustawia klucz szyfrujący
 * 
 * @param key Klucz szyfrujący
 */
void Authentication::setEncryptionKey(const std::string& key)
{
    AUTH_DEBUG(2, "Ustawianie klucza szyfrującego");
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Walidacja parametru - gentle code
    if (key.empty()) {
        AUTH_WARN("Próba ustawienia pustego klucza szyfrującego");
    }
    
    encryptionKey = key;
    
    #if DEBUG_PASSWORD_ENCRYPTION
    AUTH_DEBUG(3, "Klucz szyfrujący ustawiony");
    #endif
}

} // namespace biofeedback
