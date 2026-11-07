#include "core/Authentication.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <iterator>

namespace {

const char* kUsersFile = "/tmp/biofeedback_auth_regression_users.json";
const char* kLegacyUsersFile = "/tmp/biofeedback_auth_regression_legacy_users.json";

void writeUsersJson(const char* path)
{
    std::ofstream out(path);
    out << "{\n"
           "  \"users\": [\n"
           "    {\n"
           "      \"username\": \"admin\",\n"
           "      \"password\": \"admin\",\n"
           "      \"role\": \"administrator\",\n"
           "      \"encrypted\": false\n"
           "    },\n"
           "    {\n"
           "      \"username\": \"doctor\\\"x\",\n"
           "      \"password\": \"p\\\\a\\\"ss\",\n"
           "      \"role\": \"therapist\",\n"
           "      \"encrypted\": false\n"
           "    },\n"
           "    {\n"
           "      \"username\": \"brace-user\",\n"
           "      \"password\": \"token-with-}-inside\",\n"
           "      \"role\": \"tester\",\n"
           "      \"encrypted\": false\n"
           "    }\n"
           "  ]\n"
           "}\n";
}

void writeFlaggedPlaintextJson(const char* path)
{
    std::ofstream out(path);
    out << "{\n"
           "  \"users\": [{\n"
           "    \"username\": \"legacy-flagged\",\n"
           "    \"password\": \"old-secret\",\n"
           "    \"role\": \"tester\",\n"
           "    \"encrypted\": true\n"
           "  }]\n"
           "}\n";
}

void expect(bool condition, const std::string& message)
{
    if (!condition) throw std::runtime_error(message);
}

std::string readAll(const char* path)
{
    std::ifstream in(path);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

} // namespace

int main()
{
    try {
        writeUsersJson(kUsersFile);

        biofeedback::Authentication auth(kUsersFile);
        expect(auth.loadUsers(), "loadUsers() powinno się udać");

        // A rejected key must not mutate or persist any password record.
        expect(auth.encryptPasswordsIfNeeded("short") == 0,
               "krótki klucz powinien przerwać migrację");
        expect(readAll(kUsersFile).find(R"("password": "admin")") != std::string::npos,
               "odrzucona migracja nie może zmienić pliku użytkowników");

        const int migrated = auth.encryptPasswordsIfNeeded("biofeedback-test-key-2026");
        expect(migrated == 3, "migracja powinna zahashować wszystkie trzy hasła");
        const std::string migratedJson = readAll(kUsersFile);
        expect(migratedJson.find(R"("password": "pbkdf2_sha256$600000$)") != std::string::npos,
               "hasło powinno być zapisane jako PBKDF2-HMAC-SHA256");
        expect(migratedJson.find(R"("password": "admin")") == std::string::npos,
               "plik nie może zawierać jawnego hasła admin");

        expect(auth.login("admin", "admin"), "logowanie zahashowanego admin/admin powinno działać");
        expect(auth.isLoggedIn(), "po udanym logowaniu user powinien być zalogowany");
        expect(!auth.login("admin", "zle_haslo"), "logowanie ze złym hasłem powinno się nie udać");
        expect(!auth.isLoggedIn(), "po nieudanym logowaniu sesja powinna zostać wyczyszczona");
        expect(auth.login("doctor\"x", "p\\a\"ss"),
               "logowanie użytkownika z escapowanymi znakami powinno działać");
        expect(auth.getCurrentUsername() == "doctor\"x", "odczyt username z escapowanym cudzysłowem");
        expect(auth.login("brace-user", "token-with-}-inside"),
               "logowanie użytkownika z '}' w haśle powinno działać");

        auth.logout();
        expect(auth.loadUsers(), "ponowne loadUsers() po logout powinno działać");
        expect(!auth.isLoggedIn(), "loadUsers powinno resetować sesję");
        expect(auth.login("admin", "admin"), "logowanie po ponownym wczytaniu PBKDF2 powinno działać");

        // Successful login to a legacy plaintext account must upgrade the
        // stored record to a salted one-way hash before returning success.
        writeUsersJson(kLegacyUsersFile);
        biofeedback::Authentication legacyAuth(kLegacyUsersFile);
        expect(legacyAuth.loadUsers(), "wczytanie danych legacy powinno się udać");
        expect(legacyAuth.login("admin", "admin"), "logowanie legacy powinno zadziałać");
        expect(readAll(kLegacyUsersFile).find(R"("password": "pbkdf2_sha256$600000$)") != std::string::npos,
               "udane logowanie legacy musi trwale podnieść hasło do PBKDF2");

        // The former fail-open migration could set encrypted=true while
        // retaining plaintext. A successful exact match must upgrade it too.
        writeFlaggedPlaintextJson(kLegacyUsersFile);
        biofeedback::Authentication flaggedLegacyAuth(kLegacyUsersFile);
        expect(flaggedLegacyAuth.loadUsers(), "wczytanie starego rekordu z błędną flagą powinno się udać");
        expect(flaggedLegacyAuth.login("legacy-flagged", "old-secret"),
               "stary rekord plaintext z encrypted=true powinien zostać zmigrowany");
        expect(readAll(kLegacyUsersFile).find(R"("password": "pbkdf2_sha256$600000$)") != std::string::npos,
               "rekord plaintext z błędną flagą musi zostać zapisany jako PBKDF2");

        std::remove(kUsersFile);
        std::remove(kLegacyUsersFile);
        std::cout << "OK: auth_regression" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAIL: " << ex.what() << std::endl;
        std::remove(kUsersFile);
        std::remove(kLegacyUsersFile);
        return 1;
    }
}
