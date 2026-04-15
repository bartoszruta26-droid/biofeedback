#ifndef CORE_AUTHENTICATION_HPP
#define CORE_AUTHENTICATION_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "data/Encryption.hpp"
#include "core/ConfigManager.hpp"

namespace biofeedback {

// Struktura danych użytkownika
struct UserData {
    std::string username;
    std::string password;
    std::string role;
    bool encrypted;
    
    // Konwersja do JSON
    std::string toJson() const {
        std::ostringstream json;
        json << "{\n";
        json << "  \"username\": \"" << username << "\",\n";
        json << "  \"password\": \"" << password << "\",\n";
        json << "  \"role\": \"" << role << "\",\n";
        json << "  \"encrypted\": " << (encrypted ? "true" : "false") << "\n";
        json << "}";
        return json.str();
    }
    
    // Parsowanie z JSON
    static UserData fromJson(const std::string& json) {
        UserData user;
        
        // Proste parsowanie JSON bez zewnętrznych bibliotek
        size_t pos;
        
        pos = json.find("\"username\"");
        if (pos != std::string::npos) {
            pos = json.find(':', pos);
            pos = json.find('"', pos + 1);
            size_t endPos = json.find('"', pos + 1);
            if (endPos != std::string::npos) {
                user.username = json.substr(pos + 1, endPos - pos - 1);
            }
        }
        
        pos = json.find("\"password\"");
        if (pos != std::string::npos) {
            pos = json.find(':', pos);
            pos = json.find('"', pos + 1);
            size_t endPos = json.find('"', pos + 1);
            if (endPos != std::string::npos) {
                user.password = json.substr(pos + 1, endPos - pos - 1);
            }
        }
        
        pos = json.find("\"role\"");
        if (pos != std::string::npos) {
            pos = json.find(':', pos);
            pos = json.find('"', pos + 1);
            size_t endPos = json.find('"', pos + 1);
            if (endPos != std::string::npos) {
                user.role = json.substr(pos + 1, endPos - pos - 1);
            }
        }
        
        user.encrypted = (json.find("\"encrypted\": true") != std::string::npos ||
                         json.find("\"encrypted\":true") != std::string::npos);
        
        return user;
    }
};

/**
 * @class Authentication
 * @brief Klasa zarządzająca uwierzytelnianiem użytkowników
 * 
 * Authentication zapewnia:
 * - Wczytywanie danych użytkowników z pliku JSON
 * - Szyfrowanie haseł jeśli nie są zaszyfrowane
 * - Weryfikację loginu i hasła
 * - Zarządzanie sesją użytkownika
 */
class Authentication {
public:
    /**
     * @brief Konstruktor klasy Authentication
     * @param usersFilePath Ścieżka do pliku z danymi użytkowników
     */
    explicit Authentication(const std::string& usersFilePath = "config/users.json");
    
    /**
     * @brief Destruktor
     */
    ~Authentication();
    
    /**
     * @brief Wczytuje dane użytkowników z pliku JSON
     * @return true jeśli wczytywanie powiodło się
     */
    bool loadUsers();
    
    /**
     * @brief Zapisuje dane użytkowników do pliku JSON
     * @return true jeśli zapis powiódł się
     */
    bool saveUsers();
    
    /**
     * @brief Loguje użytkownika
     * @param username Nazwa użytkownika
     * @param password Hasło
     * @return true jeśli logowanie powiodło się
     */
    bool login(const std::string& username, const std::string& password);
    
    /**
     * @brief Wylogowuje bieżącego użytkownika
     */
    void logout();
    
    /**
     * @brief Sprawdza czy użytkownik jest zalogowany
     * @return true jeśli użytkownik jest zalogowany
     */
    bool isLoggedIn() const;
    
    /**
     * @brief Pobiera nazwę zalogowanego użytkownika
     * @return Nazwa użytkownika lub pusty string jeśli nie zalogowano
     */
    std::string getCurrentUsername() const;
    
    /**
     * @brief Pobiera rolę zalogowanego użytkownika
     * @return Rola użytkownika lub pusty string jeśli nie zalogowano
     */
    std::string getCurrentUserRole() const;
    
    /**
     * @brief Szyfruje hasła użytkowników które nie są zaszyfrowane
     * @param encryptionKey Klucz szyfrujący
     * @return liczba zaszyfrowanych haseł
     */
    int encryptPasswordsIfNeeded(const std::string& encryptionKey);
    
    /**
     * @brief Ustawia klucz szyfrujący
     * @param key Klucz szyfrujący
     */
    void setEncryptionKey(const std::string& key);

private:
    std::string usersFilePath;
    std::vector<UserData> users;
    std::string encryptionKey;
    UserData* currentUser;
    
    /**
     * @brief Wyszukuje użytkownika po nazwie
     * @param username Nazwa użytkownika
     * @return Wskaźnik na UserData lub nullptr jeśli nie znaleziono
     */
    UserData* findUser(const std::string& username);
    
    /**
     * @brief Szyfruje pojedyncze hasło
     * @param password Hasło do zaszyfrowania
     * @return Zaszyfrowane hasło
     */
    std::string encryptPassword(const std::string& password);
    
    /**
     * @brief Deszyfruje pojedyncze hasło
     * @param encryptedPassword Zaszyfrowane hasło
     * @return Odszyfrowane hasło
     */
    std::string decryptPassword(const std::string& encryptedPassword);
    
    /**
     * @brief Prosta implementacja extractStringValue dla parsera JSON
     */
    std::string extractStringValue(const std::string& json, const std::string& key) const;
};

} // namespace biofeedback

#endif // CORE_AUTHENTICATION_HPP
