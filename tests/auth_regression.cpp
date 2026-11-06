#include "core/Authentication.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

const char* kUsersFile = "/tmp/biofeedback_auth_regression_users.json";

void writeUsersJson()
{
    std::ofstream out(kUsersFile);
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

void expect(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main()
{
    try {
        writeUsersJson();

        biofeedback::Authentication auth(kUsersFile);
        expect(auth.loadUsers(), "loadUsers() powinno się udać");

        expect(auth.login("admin", "admin"), "pierwsze logowanie admin/admin powinno się udać");
        expect(auth.isLoggedIn(), "po udanym logowaniu user powinien być zalogowany");

        expect(!auth.login("admin", "zle_haslo"), "logowanie ze złym hasłem powinno się nie udać");
        expect(!auth.isLoggedIn(), "po nieudanym logowaniu sesja powinna zostać wyczyszczona");

        expect(auth.login("doctor\"x", "p\\a\"ss"), "logowanie użytkownika z escapowanymi znakami powinno działać");
        expect(auth.getCurrentUsername() == "doctor\"x", "odczyt username z escapowanym cudzysłowem");
        expect(auth.login("brace-user", "token-with-}-inside"), "logowanie użytkownika z '}' w haśle powinno działać");

        // Invalid key must fail closed without mutating user records.
        expect(auth.encryptPasswordsIfNeeded("short") == 0,
               "krótki klucz powinien przerwać migrację bez zapisu");
        expect(auth.login("admin", "admin"),
               "odrzucona migracja nie może zmienić istniejącego hasła");

        const int migrated = auth.encryptPasswordsIfNeeded("biofeedback-test-key-2026");
        expect(migrated == 3, "migracja powinna zaszyfrować wszystkie trzy hasła");
        std::ifstream encryptedFile(kUsersFile);
        const std::string encryptedJson((std::istreambuf_iterator<char>(encryptedFile)),
                                        std::istreambuf_iterator<char>());
        expect(encryptedJson.find("\\"encrypted\\": true") != std::string::npos,
               "zapis powinien oznaczać hasła jako zaszyfrowane");
        expect(encryptedJson.find("\\"password\\": \\"admin\\"") == std::string::npos,
               "plik nie może zawierać jawnego hasła admin");
        auth.logout();
        expect(auth.login("admin", "admin"),
               "logowanie po migracji haseł powinno działać");

        auth.logout();
        expect(auth.loadUsers(), "ponowne loadUsers() po logout powinno działać");
        expect(!auth.isLoggedIn(), "loadUsers powinno resetować sesję");

        std::remove(kUsersFile);
        std::cout << "OK: auth_regression" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "FAIL: " << ex.what() << std::endl;
        std::remove(kUsersFile);
        return 1;
    }
}
