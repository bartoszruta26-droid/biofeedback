#include "core/Authentication.hpp"
#include <algorithm>
#include <cstdio>

namespace biofeedback {
namespace {

std::string escapeJsonString(const std::string& input)
{
    std::string output;
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

    return output;
}

std::string unescapeJsonString(const std::string& input)
{
    std::string output;
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
                default: output += input[i]; break;
            }
        } else {
            output += input[i];
        }
    }

    return output;
}

std::string parseJsonStringAt(const std::string& json, const size_t startQuotePos)
{
    if (startQuotePos >= json.size() || json[startQuotePos] != '"') {
        return "";
    }

    std::string raw;
    raw.reserve(32);

    for (size_t i = startQuotePos + 1; i < json.size(); ++i) {
        const char c = json[i];
        if (c == '\\' && i + 1 < json.size()) {
            raw += c;
            raw += json[i + 1];
            ++i;
            continue;
        }
        if (c == '"') {
            return unescapeJsonString(raw);
        }
        raw += c;
    }

    return "";
}

} // namespace

std::string UserData::toJson() const
{
    std::ostringstream json;
    json << "{\n";
    json << "  \"username\": \"" << escapeJsonString(username) << "\",\n";
    json << "  \"password\": \"" << escapeJsonString(password) << "\",\n";
    json << "  \"role\": \"" << escapeJsonString(role) << "\",\n";
    json << "  \"encrypted\": " << (encrypted ? "true" : "false") << "\n";
    json << "}";
    return json.str();
}

UserData UserData::fromJson(const std::string& json)
{
    UserData user{};
    user.encrypted = false;

    auto extractValue = [&](const std::string& key) -> std::string {
        const std::string searchKey = "\"" + key + "\"";
        const size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) {
            return "";
        }

        const size_t colonPos = json.find(':', keyPos + searchKey.size());
        if (colonPos == std::string::npos) {
            return "";
        }

        const size_t quotePos = json.find('"', colonPos + 1);
        if (quotePos == std::string::npos) {
            return "";
        }

        return parseJsonStringAt(json, quotePos);
    };

    user.username = extractValue("username");
    user.password = extractValue("password");
    user.role = extractValue("role");
    user.encrypted = (json.find("\"encrypted\": true") != std::string::npos ||
                     json.find("\"encrypted\":true") != std::string::npos);

    return user;
}

Authentication::Authentication(const std::string& usersFilePath)
    : usersFilePath(usersFilePath)
    , currentUser(nullptr)
{
}

Authentication::~Authentication()
{
}

void Authentication::setEncryptionKey(const std::string& key)
{
    encryptionKey = key;
}

std::string Authentication::extractStringValue(const std::string& json, const std::string& key) const
{
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);

    if (pos == std::string::npos) {
        return "";
    }

    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return "";
    }

    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) {
        return "";
    }

    return parseJsonStringAt(json, pos);
}

bool Authentication::loadUsers()
{
    currentUser = nullptr;

    std::ifstream file(usersFilePath);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku użytkowników: " << usersFilePath << std::endl;
        return false;
    }

    std::ostringstream content;
    content << file.rdbuf();
    std::string jsonContent = content.str();
    file.close();

    // Parsowanie prostego JSON - szukamy tablicy users
    users.clear();

    size_t arrayStart = jsonContent.find("[");
    size_t arrayEnd = jsonContent.rfind("]");

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cerr << "Nieprawidłowy format pliku użytkowników" << std::endl;
        return false;
    }

    std::string arrayContent = jsonContent.substr(arrayStart, arrayEnd - arrayStart + 1);

    // Parsowanie każdego obiektu użytkownika
    size_t objStart = 0;
    while ((objStart = arrayContent.find("{", objStart)) != std::string::npos) {
        size_t objEnd = arrayContent.find("}", objStart);
        if (objEnd == std::string::npos) break;

        std::string userJson = arrayContent.substr(objStart, objEnd - objStart + 1);
        UserData user = UserData::fromJson(userJson);

        if (!user.username.empty()) {
            users.push_back(user);
        }

        objStart = objEnd + 1;
    }

    std::cout << "Wczytano " << users.size() << " użytkowników" << std::endl;
    return true;
}

bool Authentication::saveUsers()
{
    std::ofstream file(usersFilePath);
    if (!file.is_open()) {
        std::cerr << "Nie można zapisać do pliku użytkowników: " << usersFilePath << std::endl;
        return false;
    }

    file << "{\n    \"users\": [\n";

    for (size_t i = 0; i < users.size(); ++i) {
        file << "        " << users[i].toJson();
        if (i < users.size() - 1) {
            file << ",";
        }
        file << "\n";
    }

    file << "    ]\n}\n";
    file.close();

    std::cout << "Zapisano " << users.size() << " użytkowników" << std::endl;
    return true;
}

UserData* Authentication::findUser(const std::string& username)
{
    for (auto& user : users) {
        if (user.username == username) {
            return &user;
        }
    }
    return nullptr;
}

std::string Authentication::encryptPassword(const std::string& password)
{
    if (encryptionKey.empty()) {
        // Domyślny klucz jeśli nie ustawiono
        encryptionKey = "BiofeedbackApp2024SecureKey!";
    }

    try {
        return Encryption::encrypt(password, encryptionKey);
    } catch (const std::exception& e) {
        std::cerr << "Błąd szyfrowania hasła: " << e.what() << std::endl;
        return password;
    }
}

std::string Authentication::decryptPassword(const std::string& encryptedPassword)
{
    if (encryptionKey.empty()) {
        encryptionKey = "BiofeedbackApp2024SecureKey!";
    }

    try {
        return Encryption::decrypt(encryptedPassword, encryptionKey);
    } catch (const std::exception& e) {
        std::cerr << "Błąd deszyfrowania hasła: " << e.what() << std::endl;
        return "";
    }
}

int Authentication::encryptPasswordsIfNeeded(const std::string& key)
{
    int encryptedCount = 0;

    if (key.empty()) {
        std::cerr << "Klucz szyfrujący jest pusty" << std::endl;
        return 0;
    }

    encryptionKey = key;

    for (auto& user : users) {
        if (!user.encrypted && !user.password.empty()) {
            // Zaszyfruj hasło
            user.password = encryptPassword(user.password);
            user.encrypted = true;
            encryptedCount++;
            std::cout << "Zaszyfrowano hasło dla użytkownika: " << user.username << std::endl;
        }
    }

    // Zapisz zaktualizowane dane
    if (encryptedCount > 0) {
        saveUsers();
    }

    return encryptedCount;
}

bool Authentication::login(const std::string& username, const std::string& password)
{
    UserData* user = findUser(username);

    if (user == nullptr) {
        currentUser = nullptr;
        std::cout << "Nie znaleziono użytkownika: " << username << std::endl;
        return false;
    }

    std::string storedPassword;

    if (user->encrypted) {
        // Hasło jest zaszyfrowane - musimy je odszyfrować do porównania
        storedPassword = decryptPassword(user->password);
    } else {
        // Hasło nie jest zaszyfrowane - używamy bezpośrednio
        storedPassword = user->password;
    }

    if (storedPassword == password) {
        currentUser = user;
        std::cout << "Zalogowano użytkownika: " << username << " (rola: " << user->role << ")" << std::endl;
        return true;
    }

    currentUser = nullptr;
    std::cout << "Nieprawidłowe hasło dla użytkownika: " << username << std::endl;
    return false;
}

void Authentication::logout()
{
    currentUser = nullptr;
    std::cout << "Wylogowano użytkownika" << std::endl;
}

bool Authentication::isLoggedIn() const
{
    return currentUser != nullptr;
}

std::string Authentication::getCurrentUsername() const
{
    if (currentUser) {
        return currentUser->username;
    }
    return "";
}

std::string Authentication::getCurrentUserRole() const
{
    if (currentUser) {
        return currentUser->role;
    }
    return "";
}

} // namespace biofeedback
