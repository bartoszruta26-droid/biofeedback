# Profesjonalny audyt repozytorium Biofeedback

Data audytu: 2026-11-07
Audytowana baza: main
Commit bazowy: 6b1e84d891ef5cb13ec20edb568f05d349978422
Zakres: przegląd statyczny kodu i konfiguracji, struktura projektu, testy oraz ostatnie wyniki GitHub Actions.
Ograniczenie: audyt zdalny przez GitHub API; nie uruchomiono lokalnie kompilacji, testów sprzętowych ani dynamicznej analizy sanitiserami. Wnioski oparte na odczytanych plikach i logach CI.

## 1. Podsumowanie wykonawcze

Stan: **NIE GOTOWY DO UŻYCIA PRODUKCYJNEGO Z DANYMI PACJENTÓW**.

Najwyższe ryzyka:
1. W repozytorium znajduje się domyślne konto administratora z hasłem jawnym `admin/12345` (`config/users.json`).
2. Konfiguracja pozwala wyłączyć uwierzytelnianie (`config/config.json: medical.require_authentication=false`).
3. `src/data/Encryption.cpp` implementuje szyfrowanie cyklicznym XOR i Base64, a nie deklarowany w konfiguracji AES-256-CBC/PBKDF2. XOR nie zapewnia poufności ani integralności.
4. `DataManager` domyślnie przyjmuje `encrypt=false` dla zapisu i `decrypt=false` dla odczytu.
5. `Application.cpp` utrzymuje `unique_ptr` do zakładek jednocześnie przekazując je do hierarchii rodzic-dziecko Qt. To podwójna własność i ryzyko podwójnego zwolnienia obiektów.
6. Najnowszy odczytany run CI dla PR #173 kończy się błędem kompilacji GUI: `MainWindow.cpp` odwołuje się do `MeasurementTab::setEngineRepetitionsPerSeries` i `MeasurementTab::setMeasurementPaused`, których kompilator nie znajduje.
7. Firmware HX711 używa wartości 0 zarówno jako wyniku timeoutu, jak i wartości liczbowej; w konsekwencji utrata próbki może zostać pomylona z poprawnym odczytem.
8. Instalator tworzy regułę udev z `MODE="0666"`, dając wszystkim lokalnym użytkownikom dostęp do pasującego urządzenia szeregowego.
9. Moduł szyfrowania loguje szczegóły operacji oraz bajty danych/klucza w trybie debug; plik zawiera również generator klucza używający `std::rand()`.

## 2. Ustalenia szczegółowe

### P0 — bezpieczeństwo danych i dostęp

**P0-01. Jawne konto administratora**
- Plik: `config/users.json`
- Stan: `admin`, `password=12345`, `encrypted=false`.
- Ryzyko: łatwe przejęcie konta w instalacji używającej pliku domyślnego.
- Zalecenie: usunąć stałe poświadczenia z repozytorium; utworzyć jednorazowy kreator pierwszego konta; wymagać unikatowego, silnego hasła i wymusić jego zmianę; nie wprowadzać haseł testowych do produkcyjnej konfiguracji.

**P0-02. Uwierzytelnianie wyłączalne w konfiguracji**
- Plik: `config/config.json`
- Ustawienie: `medical.require_authentication=false`.
- Ryzyko: kontrola dostępu zależy od konfigurowalnego przełącznika.
- Zalecenie: wymusić kontrolę dostępu w warstwie usług/danych, nie wyłącznie w GUI; domyślnie fail-closed; testy prób dostępu anonimowego do wszystkich operacji wrażliwych.

**P0-03. XOR przedstawiany jako szyfrowanie**
- Plik: `src/data/Encryption.cpp`
- Potwierdzenie: `encrypt()` i `decrypt()` wywołują `xorCipher()`, po czym stosowane jest Base64. Klucz używany jest cyklicznie.
- `generateKey()` bazuje na `std::rand()`.
- Ryzyko: brak kryptograficznej poufności, brak uwierzytelnienia szyfrogramu, możliwość manipulacji danymi, przewidywalne klucze.
- Zalecenie: zastąpić mechanizm OpenSSL EVP AES-256-GCM lub ChaCha20-Poly1305; wyprowadzać klucz z hasła przy użyciu Argon2id (preferowane) albo scrypt/PBKDF2 z parametrami zatwierdzonymi dla docelowego środowiska; losować salt i nonce CSPRNG; walidować tag przed zwróceniem plaintextu; wersjonować format koperty kryptograficznej; przeprowadzić migrację danych na kopiach i nie usuwać oryginałów przed walidacją.

**P0-04. Domyślne wyłączenie szyfrowania**
- Pliki: `include/data/DataManager.hpp`, `src/data/DataManager.cpp`.
- Metody zapisu i odczytu pacjentów, ćwiczeń, wyników i planów przyjmują domyślnie `encrypt=false` / `decrypt=false`. Konstruktor DataManager inicjalizuje `encryptionEnabled(false)`.
- Ryzyko: wywołanie standardowego API może zapisać dane w jawnej postaci mimo posiadania klucza.
- Zalecenie: niejawny wybór szyfrowania zastąpić typem/konfiguracją polityki danych; w produkcji odmówić zapisu danych osobowych bez aktywnego szyfrowania; oddzielić jawny eksport kontrolowany od trwałego magazynu.

**P0-05. Niewłaściwa własność obiektów Qt**
- Plik: `src/core/Application.cpp`, funkcja `createMainWindow()` i `shutdown()`.
- Potwierdzenie: zakładki tworzone są jako `unique_ptr` z rodzicem `m_mainWindow.get()`, a następnie dodawane do `QTabWidget`; kod utrzymuje posiadanie przez `unique_ptr` i równocześnie polega na usuwaniu przez Qt.
- Ryzyko: podwójne usunięcie, use-after-free, błędy zamykania.
- Zalecenie: jeden właściciel. Preferowane: zakładki tworzy i posiada MainWindow/QTabWidget; Application trzyma wyłącznie nieposiadające wskaźniki/Qt guarded pointers. Zapewnić kolejność zatrzymania wątków i callbacków przed usunięciem GUI.

### P1 — pomiary i niezawodność

**P1-01. Timeout HX711 nieodróżnialny od zera**
- Plik: `hx711_advanced.ino`.
- Potwierdzenie: `readHX711Raw()` zwraca 0 po timeout; `readHX711Averaged()` także zwraca 0 w takim przypadku; kod aktualizacji ignoruje 0 jako odczyt.
- Ryzyko: brak danych jest reprezentowany w ten sam sposób co prawidłowa wartość zerowa; stare `current_weight` może być ponownie wysyłane z nowym znacznikiem czasu.
- Zalecenie: wynik strukturalny `{status, raw, timestamp}`; jawne stany timeout/odłączenie/błąd; brak transmisji próbki nieważnej; testy przepełnienia czasu i utraty gotowości czujnika.

**P1-02. Zbyt szerokie uprawnienia portu**
- Plik: `biofeedback_installer.sh`, reguła udev.
- Potwierdzenie: `MODE="0666"` dla urządzeń z vendor ID 2341.
- Ryzyko: każdy lokalny użytkownik może otworzyć pasujący port; zakres dopasowania jest szeroki.
- Zalecenie: dedykowana grupa (np. `dialout`), `MODE="0660"`, precyzyjniejsze dopasowanie VID/PID i identyfikatorów urządzenia.

**P1-03. Logowanie danych kryptograficznych**
- Pliki: `src/data/Encryption.cpp`, `src/data/DataManager.cpp`, `src/core/Authentication.cpp`.
- Potwierdzenie: debug XOR może wypisywać wartości bajtów danych i klucza; poziomy debug są włączone domyślnie w makrach; logowane są też długości haseł i szczegóły kryptograficzne.
- Ryzyko: ujawnianie sekretów lub danych w terminalu/CI/logach oraz niepotrzebne zwiększanie powierzchni wycieku.
- Zalecenie: usunąć logowanie bajtów/kluczy/sekretów; logi produkcyjne wyłącznie z bezpiecznymi identyfikatorami korelacyjnymi; wyłączyć debug w konfiguracji produkcyjnej.

**P1-04. Zapis plików nieatomowy**
- Pliki: `src/data/DataManager.cpp`, `src/core/ConfigManager.cpp`.
- Potwierdzenie: zapis przez bezpośrednie `std::ofstream` do docelowej ścieżki.
- Ryzyko: przerwanie procesu/zasilania może pozostawić plik ucięty; brak atomowej podmiany i jawnej strategii fsync.
- Zalecenie: plik tymczasowy w tym samym katalogu, restrykcyjne prawa, flush/fsync, atomowy rename, kontrola błędów i kopia poprzedniej wersji; testy awarii w każdym etapie.

**P1-05. Bezpieczeństwo generatora kluczy**
- Plik: `src/data/Encryption.cpp`.
- Potwierdzenie: `std::rand() % 93`.
- Ryzyko: generator nie jest kryptograficznie bezpieczny i może tworzyć przewidywalne klucze.
- Zalecenie: OpenSSL `RAND_bytes`/EVP_RAND, kontrola błędów i bezpieczne czyszczenie buforów sekretów.

### P2 — architektura i utrzymanie

- CMake ma ścieżkę warunkową: GUI budowane tylko gdy Qt5 komponenty są dostępne. CI powinno jawnie zawierać pełny build GUI.
- `tests/CMakeLists.txt` jest plikiem zastępczym; część testów jest deklarowana w głównym CMake, ale struktura testowania jest rozproszona.
- README zawiera niezgodną strukturę katalogów (np. `tabs/`, `docs/`, gdy rzeczywisty kod używa `tab/`, `doc/`).
- README określa program jako „medical-grade”, ale jednocześnie zastrzega, że to oprogramowanie R&D i wymaga właściwego dopuszczenia przed użyciem medycznym. Należy usunąć nieuzasadnione deklaracje jakości regulacyjnej.
- Brak potwierdzenia fizycznych testów Arduino Nano/HX711. Test parsera i symulator nie zastępują testu sprzętu, kalibracji oraz niepewności pomiaru.

## 3. CI — potwierdzona awaria PR #173

Run: https://github.com/bartoszruta26-droid/biofeedback/actions/runs/42347363187
Job: `core-regressions`
Wynik: failure podczas `cmake --build`; etap uruchomienia testów został pominięty.
Błędy:
- `src/gui/MainWindow.cpp:403`: `MeasurementTab::setEngineRepetitionsPerSeries` — brak deklaracji w `MeasurementTab`.
- `src/gui/MainWindow.cpp:411`: `MeasurementTab::setMeasurementPaused` — brak deklaracji w `MeasurementTab`.

To jest blokada kompilacji GUI. Nie wolno uznać testów regresyjnych za zaliczone, ponieważ nie uruchomiły się po przerwaniu kompilacji.

## 4. Stan repozytorium i ograniczenia audytu

- Repo: `bartoszruta26-droid/biofeedback`, publiczne.
- Gałąź bazowa: `main`.
- SHA bazowy: `6b1e84d891ef5cb13ec20edb568f05d349978422`.
- Odczytano kod głównych modułów, firmware, konfigurację, instalator, CMake, testy i log CI.
- Nie uruchomiono kompilacji w lokalnym środowisku tego audytu, testów sanitiserami, fuzzingu ani pomiarów na fizycznym urządzeniu. Te działania pozostają warunkiem odbioru technicznego.

## 5. Plan naprawczy

### Faza 0 — blokady przed użyciem z danymi pacjentów
1. Usunąć domyślne konto/hasło z repozytorium i zapewnić bezpieczne provisionowanie.
2. Wymusić uwierzytelnianie i autoryzację w warstwie dostępu do danych.
3. Wstrzymać użycie XOR jako ochrony danych; zaprojektować nowy format AEAD i migrację.
4. Wymusić szyfrowanie trwałego magazynu pacjentów.
5. Ujednolicić własność obiektów Qt.
6. Naprawić błędy kompilacji PR #173 i uzyskać zielony build GUI.

### Faza 1 — wiarygodność toru pomiarowego
1. Rozdzielić timeout od prawidłowej wartości zero.
2. Wprowadzić walidację próbek, jakości, jednostek i czasu.
3. Zweryfikować kalibrację względem wzorców i z wersjonowaniem profilu.
4. Dodać parser fuzzing, testy utraty synchronizacji, szumu, fragmentacji, błędnego CRC.
5. Przeprowadzić testy urządzenia Nano/HX711 i zapisać wyniki.

### Faza 2 — integralność i odzyskiwanie
1. Atomowy zapis plików i kopie zapasowe z weryfikacją.
2. Testy odtwarzania, uszkodzeń i przerw zasilania.
3. Audyt logów i uprawnień plików.
4. Testy wielowątkowości, ASan, UBSan, TSan i fuzzing.
5. Uporządkować CI: osobne zadania core, GUI, security, parser, migracja i hardware acceptance.

### Faza 3 — gotowość produktu
1. Określić przeznaczenie i granice funkcjonalne oprogramowania.
2. Zbudować macierz wymagań, ryzyk i testów.
3. Uporządkować dokumentację i instrukcje wdrożeniowe.
4. Wykonać niezależny przegląd bezpieczeństwa i walidację kliniczno-pomiarową.
5. Wstrzymać deklaracje „medical-grade” do uzyskania odpowiednich dowodów zgodności.

## 6. Kryteria odbioru

Audyt można uznać za zamknięty dopiero, gdy:
- nie ma domyślnych poświadczeń;
- wszystkie dane wrażliwe są domyślnie szyfrowane i integralność jest weryfikowana;
- nie ma użycia XOR jako ochrony kryptograficznej;
- pełny build GUI i wszystkie testy CI przechodzą;
- testy jednostkowe, integracyjne oraz migracyjne przechodzą;
- brak błędów ASan/UBSan/TSan w zakresie krytycznych ścieżek;
- timeout czujnika nie jest zapisywany jako próbka;
- test sprzętowy HX711 i kalibracja są udokumentowane;
- własność obiektów Qt i cykl życia wątków są jednoznaczne;
- instalator nie nadaje globalnych uprawnień do portów;
- kopia zapasowa i odtwarzanie są przetestowane.

---
**Werdykt:** repozytorium ma wartościowy kierunek architektoniczny i istotną bazę funkcjonalną, ale w aktualnym stanie gałęzi `main` nie powinno być używane do przechowywania rzeczywistych danych pacjentów ani przedstawiane jako gotowy produkt medyczny.
