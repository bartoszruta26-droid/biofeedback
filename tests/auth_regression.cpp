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
