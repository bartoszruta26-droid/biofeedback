#include "core/Authentication.hpp"
#include <algorithm>
#include <cctype>
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

size_t findStringTerminator(const std::string& text, const size_t start)
{
    for (size_t i = start; i < text.size(); ++i) {
        if (text[i] != '"') {
            continue;
        }

        size_t backslashes = 0;
        for (size_t j = i; j > 0 && text[j - 1] == '\\'; --j) {
            ++backslashes;
        }

        if (backslashes % 2 == 0) {
            return i;
        }
    }

    return std::string::npos;
}

std::string extractJsonStringField(const std::string& jsonText, const std::string& key)
{
    const std::string searchKey = "\"" + key + "\"";
    const size_t keyPos = jsonText.find(searchKey);
    if (keyPos == std::string::npos) {
        return "";
    }

    const size_t colonPos = jsonText.find(':', keyPos + searchKey.size());
    if (colonPos == std::string::npos) {
        return "";
    }

    const size_t quotePos = jsonText.find('"', colonPos + 1);
    if (quotePos == std::string::npos) {
        return "";
    }

    const size_t endQuotePos = findStringTerminator(jsonText, quotePos + 1);
    if (endQuotePos == std::string::npos || endQuotePos <= quotePos) {
        return "";
    }

    return unescapeJsonString(jsonText.substr(quotePos + 1, endQuotePos - quotePos - 1));
}

bool extractJsonBoolField(const std::string& jsonText, const std::string& key, const bool defaultValue)
{
    const std::string searchKey = "\"" + key + "\"";
    const size_t keyPos = jsonText.find(searchKey);
    if (keyPos == std::string::npos) {
        return defaultValue;
    }

    size_t pos = jsonText.find(':', keyPos + searchKey.size());
    if (pos == std::string::npos) {
        return defaultValue;
    }

    ++pos;
    while (pos < jsonText.size() && std::isspace(static_cast<unsigned char>(jsonText[pos]))) {
        ++pos;
    }

    if (jsonText.compare(pos, 4, "true") == 0) {
        return true;
    }
    if (jsonText.compare(pos, 5, "false") == 0) {
        return false;
    }

    return defaultValue;
}

std::string findUsersArray(const std::string& jsonContent)
{
    const std::string key = "\"users\"";
    const size_t keyPos = jsonContent.find(key);
    if (keyPos == std::string::npos) {
        return "";
    }

    const size_t bracketStart = jsonContent.find('[', keyPos + key.size());
    if (bracketStart == std::string::npos) {
        return "";
    }

    bool inString = false;
    int depth = 0;
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
                return jsonContent.substr(bracketStart, i - bracketStart + 1);
            }
        }
    }

    return "";
}

std::vector<std::string> splitJsonObjectsFromArray(const std::string& arrayJson)
{
    std::vector<std::string> objects;

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
                objStart = std::string::npos;
            }
        }
    }

    return objects;
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

UserData UserData::fromJson(const std::string& jsonText)
{
    UserData user{};
    user.username = extractJsonStringField(jsonText, "username");
    user.password = extractJsonStringField(jsonText, "password");
    user.role = extractJsonStringField(jsonText, "role");
    user.encrypted = extractJsonBoolField(jsonText, "encrypted", false);
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

std::string Authentication::extractStringValue(const std::string& jsonContent, const std::string& key) const
{
    return extractJsonStringField(jsonContent, key);
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

    users.clear();

    const std::string usersArray = findUsersArray(jsonContent);
    if (usersArray.empty()) {
        std::cerr << "Nieprawidłowy format pliku użytkowników (brak tablicy users)" << std::endl;
        return false;
    }

    const auto objects = splitJsonObjectsFromArray(usersArray);
    for (const auto& objectJson : objects) {
        UserData user = UserData::fromJson(objectJson);
        if (!user.username.empty()) {
            users.push_back(user);
        }
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
