#include "data/LegacyXorMigration.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Użycie: biofeedback_legacy_migrate <źródło> <wynik_BFENC> <kopia_legacy>\n"
                  << "Wymagane zmienne środowiskowe: BIOFEEDBACK_LEGACY_XOR_KEY oraz BIOFEEDBACK_ENCRYPTION_KEY\n";
        return 2;
    }
    const char* oldKey = std::getenv("BIOFEEDBACK_LEGACY_XOR_KEY");
    const char* newKey = std::getenv("BIOFEEDBACK_ENCRYPTION_KEY");
    if (!oldKey || !newKey) {
        std::cerr << "Brak wymaganych zmiennych środowiskowych z kluczami. Nie podawaj kluczy jako argumentów procesu.\n";
        return 2;
    }
    LegacyXorMigrationResult result;
    std::string error;
    if (!migrateLegacyXorFile(argv[1], argv[2], argv[3], oldKey, newKey, result, error)) {
        std::cerr << "Migracja nie powiodła się: " << error << "\n";
        return 1;
    }
    std::cout << "Migracja zakończona i zweryfikowana.\n"
              << "Bajty legacy: " << result.inputBytes << "\n"
              << "Bajty tekstu jawnego: " << result.plaintextBytes << "\n"
              << "Kopia oryginału: " << result.backupPath << "\n"
              << "Nowy szyfrogram: " << result.destinationPath << "\n"
              << "Źródło pozostało niezmienione.\n";
    return 0;
}
