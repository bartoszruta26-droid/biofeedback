# 🔐 Wzmocnienie bezpieczeństwa — gałąź integration/all-work (2026-11-06)

> 📘 Audyt historyczny • Język: polski • Zakres: wskazany commit gałęzi integration/all-work  
> ⚠️ Wyników i statusów z tego dokumentu nie należy przypisywać automatycznie do bieżącej gałęzi main.

## 1. Zakres

Ten zestaw zmian wprowadza haszowanie haseł, szyfrowanie uwierzytelnione, atomowy zapis oraz bezpieczne blokowanie dostępu do kart pacjentów w razie błędu. Nie stanowi certyfikacji bezpieczeństwa rozwiązania produkcyjnego.

## 2. Dane uwierzytelniające kont

- Nowe dane uwierzytelniające są przechowywane w formacie `pbkdf2-sha256$v1$<iterations>$<salt-hex>$<digest-hex>`.
- PBKDF2-HMAC-SHA-256 wykorzystuje 600 000 iteracji, 16-bajtową sól z CSPRNG i 32-bajtowy skrót.
- Weryfikacja hasła wykorzystuje porównanie skrótów o stałym czasie wykonania.
- Starsze konto z hasłem jawnym jest migrowane przy pierwszym poprawnym logowaniu. Konto otrzymuje znacznik `mustChangePassword`; GUI blokuje zakończenie logowania do czasu ustawienia i zapisania nowego hasła o długości co najmniej 12 znaków.
- Tworzenie nowych kont jest ograniczone do uwierzytelnionego administratora i wymaga hasła o długości co najmniej 12 znaków.
- Wymuszona zmiana hasła odrzuca ponowne użycie naruszonego hasła legacy.
- Starsze rekordy haseł szyfrowane odwracalnym XOR są odrzucane. Nie ma automatycznego mechanizmu awaryjnego XOR.
- Sama zmiana kodu nie modyfikuje `config/users.json`. Pierwsze poprawne logowanie legacy zapisuje migrację PBKDF2 atomowo.

## 3. Szyfrowanie danych

- Nowe zaszyfrowane wartości używają prefiksu `BFENC:v1:` i algorytmu AES-256-GCM.
- Ładunek jest kodowaniem Base64 12-bajtowego nonce, 16-bajtowego znacznika uwierzytelniającego i szyfrogramu.
- Dla każdego szyfrowania generowany jest nowy nonce za pomocą OpenSSL `RAND_bytes`.
- Materiał klucza jest przekształcany funkcją SHA-256 do 256-bitowego klucza AES. Aplikacja wymaga zewnętrznego przekazania `BIOFEEDBACK_ENCRYPTION_KEY` o długości co najmniej 32 bajtów. Należy używać losowego sekretu kryptograficznego, a nie hasła użytkownika. Sekretu nie wolno commitować.
- Błąd znacznika uwierzytelniającego, niewłaściwy klucz, niepoprawny ładunek i nieobsługiwany format legacy powodują odmowę dostępu.
- Szyfrogramy XOR nie są migrowane automatycznie. Należy zachować chronioną kopię zapasową, a ewentualną migrację legacy wykonać jako oddzielną, jawnie zatwierdzoną operację offline.

## 4. Trwały zapis plików

- `DataManager` zapisuje do pliku tymczasowego w tym samym katalogu, tworzonego przez `mkstemp`, ogranicza uprawnienia do odczytu/zapisu właściciela, zapisuje wszystkie bajty, wywołuje `fsync` i atomowo zmienia nazwę pliku na docelową.
- Po zmianie nazwy synchronizowany jest katalog nadrzędny, jeśli system to obsługuje.
- Pliki kont uwierzytelniania korzystają z tego samego wzorca.
- Błąd przed zmianą nazwy pozostawia poprzedni plik docelowy bez zmian. Błąd synchronizacji katalogu po zmianie nazwy może zgłosić problem trwałości, mimo że atomowa podmiana już nastąpiła.

## 5. Kartoteki pacjentów i eksport JSON

- Rekordy pacjentów, wyniki ćwiczeń powiązane z pacjentem i plany treningowe zawsze używają AES-GCM; starsze argumenty logiczne nie mogą wyłączyć szyfrowania.
- Odczyt pacjentów bez klucza szyfrowania albo z danymi jawnymi/legacy kończy się odmową dostępu.
- Eksport/import JSON szyfruje i uwierzytelnia ładunek oraz wymaga skonfigurowanego klucza.

## 6. Historyczny stan weryfikacji

Dodano testy regresyjne dla haszowania i weryfikacji PBKDF2, migracji starszego konta i wymuszonej zmiany hasła, tworzenia kont, cyklu AES-GCM, odrzucenia niewłaściwego klucza, manipulacji znacznikiem, odrzucania starszych formatów, bezpiecznego blokowania zapisu pacjentów, szyfrowania wyników ćwiczeń i planów treningowych, szyfrowania eksportu/importu JSON oraz częściowego zapisu wymuszonego przez RLIMIT_FSIZE, który sprawdza zachowanie poprzedniego pliku.

GitHub Actions zakończyło się powodzeniem dla commita `412357e39d960e22d9d34e758fc0cd5a32e8e3dd`: zbudowano cel kompilacji samego `LoginDialog.cpp` i przeszły cztery zestawy CTest (`encryption_regression`, `password_hasher_regression`, `auth_regression`, `data_manager_regression`). Pełny plik wykonywalny GUI Qt nie został zbudowany, ponieważ środowisko CI nie instaluje Qt Charts; zmienioną jednostkę translacji okna logowania skompilowano niezależnie.

Do wykonania pozostają testy sprzętowe, testy penetracyjne, testy awarii procesu oraz testy trwałości właściwe dla systemu plików. Historyczny wynik CI dotyczy wyłącznie wskazanego SHA i nie potwierdza bieżącego stanu gałęzi main.
