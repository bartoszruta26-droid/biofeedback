# Profesjonalny audyt repozytorium Biofeedback

Data: 2026-11-07
Gałąź bazowa: `main`
Commit bazowy: `6b1e84d891ef5cb13ec20edb568f05d349978422`

## Werdykt
**Nie rekomenduje się używania aktualnego stanu `main` do przechowywania rzeczywistych danych pacjentów ani deklarowania gotowości produktu medycznego.** Audyt obejmował statyczny przegląd kodu i konfiguracji oraz odczyt logów GitHub Actions. Nie uruchomiono lokalnego builda, sanitiserów, fuzzingu ani testów fizycznego urządzenia.

## P0 — krytyczne

1. **Jawne domyślne poświadczenia** — `config/users.json` zawiera `admin`, hasło `12345`, `encrypted: false`. Usunąć domyślne konto/hasło, wdrożyć jednorazowy kreator pierwszego konta i wymusić zmianę poświadczeń.
2. **Uwierzytelnianie można wyłączyć** — `config/config.json`: `medical.require_authentication=false`. Kontrolę dostępu wymusić w warstwie dostępu do danych, domyślnie fail-closed.
3. **XOR/Base64 nie jest szyfrowaniem kryptograficznym** — `src/data/Encryption.cpp` wykonuje XOR z cyklicznym kluczem, po czym koduje Base64. W `generateKey()` używa `std::rand()`. To nie zapewnia poufności ani integralności. Zastąpić biblioteką OpenSSL EVP z AEAD (np. AES-256-GCM lub ChaCha20-Poly1305), nonce i tagiem uwierzytelniającym; klucz wyprowadzać z użyciem zatwierdzonego KDF; przygotować wersjonowany format i bezpieczną migrację na kopiach danych.
4. **Trwały magazyn domyślnie zapisuje bez szyfrowania** — DataManager inicjalizuje `encryptionEnabled(false)`, a metody zapisu/odczytu przyjmują domyślnie `encrypt=false`/`decrypt=false`. Wprowadzić obowiązkową politykę szyfrowania dla danych osobowych; eksport jawny wydzielić jako świadomą, kontrolowaną operację.
5. **Podwójna własność obiektów Qt** — `src/core/Application.cpp`: zakładki utrzymywane jako `unique_ptr`, a zarazem dodawane do `QTabWidget` i osadzane w hierarchii rodzic-dziecko Qt. To ryzyko podwójnego zwolnienia/use-after-free. Ustanowić jednego właściciela (preferowane `MainWindow`/`QTabWidget`), a `Application` pozostawić wskaźniki nieposiadające własności.
6. **Awaria kompilacji PR #173** — najnowszy sprawdzony run https://github.com/bartoszruta26-droid/biofeedback/actions/runs/42347363187 zakończył się błędem w `src/gui/MainWindow.cpp`: brak `MeasurementTab::setEngineRepetitionsPerSeries` i `MeasurementTab::setMeasurementPaused`. Testy zostały pominięte, ponieważ przerwała się kompilacja.

## P1 — wysokie

7. **Timeout HX711 mylony z zerem** — `hx711_advanced.ino`: `readHX711Raw()` zwraca 0 przy timeout, a 0 jest także prawidłowym odczytem. Stan brakującej próbki powinien być osobnym statusem; próbka nieważna nie może aktualizować ostatniej wartości ani trafiać do obliczeń.
8. **Reguła udev z globalnym zapisem/odczytem** — `biofeedback_installer.sh`: `MODE="0666"` dla urządzeń VID 2341. Zmienić na dedykowaną grupę i `MODE="0660"`, zawęzić VID/PID.
9. **Logowanie informacji wrażliwych** — `Encryption.cpp` domyślnie włącza debug; kod XOR może logować bajty danych i klucza. Usunąć logowanie sekretów, wartości bajtów i materiału kluczowego; profil produkcyjny ma wyłączać debug.
10. **Zapis plików nie jest atomowy** — `DataManager.cpp` i `ConfigManager.cpp` używają bezpośredniego `std::ofstream` do docelowej ścieżki. Stosować plik tymczasowy w tym samym katalogu, restrykcyjne prawa, flush/fsync i atomowe rename; dodać testy przerwania zapisu.

## P2 — umiarkowane

11. README opisuje niezgodną z rzeczywistą strukturę katalogów (`tabs/`, `docs/` vs `tab/`, `doc/`).
12. `tests/CMakeLists.txt` jest tylko plikiem zastępczym; testy są rozmieszczone w głównym CMake i osobnych plikach. Ujednolicić konfigurację CTest.
13. Opis „medical-grade” w README nie ma w tym audycie dowodu potwierdzającego. Zastąpić deklaracje opisem rzeczywistego przeznaczenia i ograniczeń.
14. CI i testy symulacyjne nie zastępują akceptacji fizycznego Arduino Nano/HX711, kalibracji względem wzorca i oceny niepewności.

## Plan naprawczy

### Etap 0 — bezpieczeństwo i stabilność
- Usunąć poświadczenia domyślne; provisionowanie konta bez haseł w repo.
- Wymusić uwierzytelnianie/autoryzację w warstwie danych.
- Zastąpić XOR AEAD i zaprojektować migrację istniejących danych.
- Wymusić szyfrowanie danych trwałych.
- Naprawić własność obiektów Qt.
- Naprawić błędy kompilacji PR #173 i potwierdzić pełny build GUI.

### Etap 1 — tor pomiarowy
- Rozdzielić timeout, odłączenie czujnika i prawidłową wartość zero.
- Dodać status jakości, jednostki i znaczniki czasu próbek.
- Wersjonować profile kalibracyjne, wiązać z ID urządzenia.
- Testować fragmentację, szum, błędne CRC, utratę synchronizacji i przepełnienia.
- Wykonać udokumentowane testy fizycznego Nano/HX711.

### Etap 2 — trwałość i testy
- Atomowy zapis, kopie zapasowe i próbne odtwarzanie.
- ASan, UBSan, TSan, fuzzing parsera i testy wielowątkowości.
- Osobne zadania CI dla core, GUI, bezpieczeństwa, migracji i parsera.
- Sprawdzić uprawnienia katalogów, logów i plików danych.

### Etap 3 — gotowość produktu
- Zdefiniować przeznaczenie i granice funkcjonalne.
- Zbudować macierz wymagań, ryzyk i testów.
- Przeprowadzić niezależny przegląd bezpieczeństwa i walidację pomiarową.
- Nie deklarować zgodności ani gotowości medycznej bez odpowiednich dowodów.

## Kryteria odbioru
- brak domyślnych poświadczeń;
- wymuszone uwierzytelnianie i autoryzacja;
- AEAD i potwierdzanie integralności; brak XOR jako ochrony danych;
- szyfrowanie danych pacjentów domyślnie;
- zielony pełny build GUI i wszystkie testy CI;
- brak błędów ASan/UBSan/TSan w krytycznych ścieżkach;
- timeout nigdy nie staje się próbką pomiarową;
- test sprzętowy i kalibracyjny udokumentowane;
- jednoznaczna własność Qt i cykl życia wątków;
- instalator ogranicza dostęp do portu do dedykowanej grupy;
- sprawdzone kopie i odtwarzanie.

## Ograniczenia
To przegląd statyczny i analiza dostępnych logów CI. Nie wykonano lokalnej kompilacji, dynamicznej analizy pamięci, fuzzingu ani pomiarów fizycznego sprzętu. Wnioski dotyczą odczytanego SHA gałęzi `main`; wyniki z gałęzi roboczych nie oznaczają wdrożenia zmian na `main`.
