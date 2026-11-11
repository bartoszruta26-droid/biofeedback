#include <iostream>
#include "core/Authentication.hpp"

int main() {
    std::cout << "=== Test systemu logowania ===" << std::endl;
    
    // Utwórz obiekt Authentication
    biofeedback::Authentication auth("config/users.json");
    
    // Wczytaj użytkowników
    std::cout << "\n1. Wczytywanie użytkowników..." << std::endl;
    if (!auth.loadUsers()) {
        std::cerr << "Błąd wczytywania użytkowników!" << std::endl;
        return 1;
    }
    
    // Sprawdź czy hasła są zaszyfrowane i zaszyfruj jeśli nie
    std::cout << "\n2. Sprawdzanie szyfrowania haseł..." << std::endl;
    std::string encryptionKey = "BiofeedbackApp2024SecureKey!";
    int encryptedCount = auth.encryptPasswordsIfNeeded(encryptionKey);
    std::cout << "Zaszyfrowano " << encryptedCount << " haseł" << std::endl;
    
    // Spróbuj zalogować się jako admin
    std::cout << "\n3. Próba logowania jako admin/admin..." << std::endl;
    if (auth.login("admin", "admin")) {
        std::cout << "   Zalogowano pomyślnie!" << std::endl;
        std::cout << "   Użytkownik: " << auth.getCurrentUsername() << std::endl;
        std::cout << "   Rola: " << auth.getCurrentUserRole() << std::endl;
        
        // Wyloguj
        auth.logout();
        std::cout << "   Wylogowano." << std::endl;
    } else {
        std::cout << "   Logowanie nieudane!" << std::endl;
    }
    
    // Spróbuj zalogować się z błędnym hasłem
    std::cout << "\n4. Próba logowania z błędnym hasłem..." << std::endl;
    if (!auth.login("admin", "wrongpassword")) {
        std::cout << "   Oczekiwany błąd logowania - poprawnie!" << std::endl;
    }
    
    std::cout << "\n=== Test zakończony ===" << std::endl;
    return 0;
}
