# 🧱 Wspólny serwer danych — architektura i plan migracji

> 📘 Dokument techniczny • Język: polski • Status: projekt architektury z częściowo wdrożonymi elementami  
> ⚠️ Opis stanu implementacji należy zawsze porównywać z bieżącym kodem i testami.

## 🧭 Cel i zakres

Dokument określa docelowy podział odpowiedzialności między serwerem danych, akwizycją sprzętową i klientami aplikacji. Oddziela decyzje architektoniczne od aktualnego stanu implementacji.

## 1. Decyzje architektoniczne

- Karty pacjentów są przechowywane w plikach albo docelowo w MariaDB. PostgreSQL nie jest wymaganiem projektu.
- Wybór magazynu kart pacjentów jest konfigurowany po stronie serwera, a nie niezależnie w każdym kliencie.
- Magazyn plikowy jest wspieraną opcją docelową. Rekordy muszą być szyfrowane w spoczynku, zapisywane atomowo, chronione uprawnieniami systemu plików i adresowane niejawymi identyfikatorami zamiast numerem PESEL lub nazwiskiem.
- MariaDB jest alternatywnym backendem. Implementacja musi używać zapytań parametryzowanych, transakcji, migracji schematu, uprawnień minimalnych, szyfrowanego transportu przy połączeniach zdalnych oraz jawnych procedur kopii i odtwarzania.
- Pomiary, sesje i wyniki treningowe oraz sesje i wyniki gier należą do wspólnej usługi danych. Akwizycja z urządzeń pozostaje odrębną odpowiedzialnością.
- Desktop Linux, Desktop Windows, WebUI, TUI i Android są równorzędnymi klientami. Korzystają z wersjonowanego API i nie odczytują bezpośrednio plików ani bazy danych serwera.
- Funkcje anten, elektrod i IR-PWM nie należą do zakresu aplikacji Biofeedback.

## 2. Granice odpowiedzialności procesów

### Serwer danych

Odpowiada za:
- repozytorium pacjentów i mapowanie tożsamości;
- repozytorium wyników pomiarów i metadanych;
- plany treningowe, sesje, punkty kontrolne i wyniki;
- sesje gier, punktację i historię wyników;
- uwierzytelnianie i autoryzację na granicach zasobów;
- zdarzenia audytowe, kontrolę wersji, idempotencję zapisu, kopie zapasowe, odtwarzanie i eksport danych.

Docelowo udostępnia:
- REST/HTTPS API do operacji CRUD i odczytu historii;
- WebSocket albo Server-Sent Events do powiadomień o stanie i wynikach;
- punkt health/status informujący o wybranym magazynie i gotowości, bez ujawniania sekretów ani danych pacjentów.

### Akwizycja danych z urządzeń i pomiarów

Odpowiada za:
- komunikację szeregową/sieciową z Arduino Nano, Raspberry Pi Pico i obsługiwanymi czujnikami;
- parsowanie protokołu, kalibrację, normalizację znaczników czasu, walidację i ocenę jakości sygnału;
- dostarczanie próbek na żywo do aplikacji/usługi danych.

Nie jest właścicielem głównej kartoteki pacjentów ani trwałej historii wyników współdzielonej między klientami. Zweryfikowana próbka jest wiązana z pacjentem i sesją w warstwie aplikacyjnej, a następnie zapisywana przez serwer danych.

### Klienci

Desktop Linux, Desktop Windows, WebUI, TUI i Android:
- odczytują i zapisują dane pacjentów, pomiarów, treningów i gier przez API;
- przechowują wyłącznie tymczasowy stan prezentacji i jawnie ograniczone kolejki offline;
- prezentują konflikty i brak połączenia, nie nadpisują po cichu nowszych rekordów;
- nie zawierają poświadczeń bazy danych i nie łączą się bezpośrednio z MariaDB.

## 3. Strategia przechowywania danych

### Plikowy magazyn kart pacjentów

Dla każdego rekordu stosuj niejawny identyfikator w katalogu danych aplikacji. Nie umieszczaj PESEL, nazwiska, daty urodzenia ani innych bezpośrednich identyfikatorów w nazwach plików i katalogów. Każdy rekord szyfruj uwierzytelnionym szyfrowaniem (AES-256-GCM lub równoważną, poddaną przeglądowi konstrukcją AEAD). Klucze przechowuj poza repozytorium i poza plikami pacjentów. Zapisuj dane do ograniczonego uprawnieniami pliku tymczasowego, opróżnij bufory i atomowo zastąp plik docelowy. Przy odczycie sprawdzaj JSON, schemat i tożsamość rekordu. Metadane audytowe przechowuj oddzielnie od treści klinicznej.

### Magazyn MariaDB

Implementuj go za tym samym interfejsem repozytorium co magazyn plikowy. Używaj migracji i jawnego wersjonowania schematu. Wymagane encje logiczne obejmują: patients, measurements, training_plans, training_sessions, training_results, game_sessions, game_results, users/roles, audit_events i idempotency_keys. W miarę możliwości przechowuj identyfikatory pacjentów oddzielnie od treści klinicznej. Wymuszaj klucze obce, indeksy, ograniczenia unikalności i transakcje.

### Konfiguracja

Backend jest wybierany w `config/config.json`:
- `storage.patient_backend: "files"` — lokalny magazyn plikowy kart pacjentów (obecne ustawienie domyślne);
- `storage.patient_backend: "mariadb"` — docelowy magazyn MariaDB.

Ustawienia połączenia MariaDB znajdują się w `storage.mariadb`: `host`, `port`, `database`, `username`, `password_env`, `tls` i `connect_timeout_seconds`. Hasło jest dostarczane przez wskazaną zmienną środowiskową (domyślnie `BIOFEEDBACK_MARIADB_PASSWORD`), nigdy przez JSON ani repozytorium.

**Stan implementacji:** Magazyn plikowy jest obecnie podłączony do aplikacji Desktop. Adapter repozytorium MariaDB i integracja sterownika nie są jeszcze zaimplementowane. Wybranie `mariadb` powoduje jawny błąd uruchomienia Desktop i nie przełącza automatycznie aplikacji na pliki.

## 4. Kontrakt API

Wersjonowanie endpointów: `/api/v1`. Kolekcje CRUD:
- `patients`
- `measurements`
- `training-sessions`
- `training-results`
- `game-sessions`
- `game-results`

Każda kolekcja udostępnia `GET /{collection}` i `POST /{collection}`; pojedynczy rekord obsługuje `GET /{collection}/{id}`, `PUT /{collection}/{id}` i `DELETE /{collection}/{id}`. Identyfikatory tworzy serwer jako UUID. Rekord zawiera `id`, `version`, `created_at` i `updated_at`. Aktualizacja i usunięcie wymagają `expected_version`; konflikt wersji zwraca HTTP 409. Ciało rekordu jest ograniczone do 16 MiB.

Do czasu wdrożenia zakresów RBAC wszystkie operacje na kolekcjach wymagają aktywnej sesji użytkownika z rolą `administrator`. Nie jest to docelowy model uprawnień klinicznych. Wymagane są zakresy per rola, pacjent i relacja terapeuta–pacjent, rejestr audytowy z aktorem i wynikiem operacji oraz klucze idempotencji. Nie wolno logować danych pacjentów ani sekretów.

Lista kolekcji jest ograniczona do jawnej listy dozwolonych nazw; identyfikatory nie są ścieżkami ani numerami PESEL. Repozytorium sprawdza format JSON, limit rozmiaru, wersję rekordu i integralność identyfikatora. Walidacja domenowa jednostek, znaczników czasu i relacji pacjent/sesja musi być rozszerzana wraz z integracją producentów danych. WebSocket/SSE dla zdarzeń pozostaje niezaimplementowany.

## 5. Kolejność migracji

1. Naprawić i ustabilizować testy regresyjne bieżącej gałęzi main. Zachować istniejące pliki pacjentów i obsługę importu/eksportu CSV.
2. Zdefiniować interfejsy repozytoriów i wspólne DTO domenowe pacjenta, pomiaru, treningu i gry. Dodać testy kontraktowe niezależne od backendu.
3. Zaimplementować plikowe repozytorium pacjentów zgodne z istniejącą polityką szyfrowania; dodać bezpieczne operacje CRUD, zapis atomowy, kontrolę wersji i testy migracji/importu.
4. Wydzielić proces serwera danych i wstrzykiwać repozytoria do HTTP API. Domyślnie pozostawić nasłuch wyłącznie na loopback; dostęp zdalny wymaga HTTPS/reverse proxy i jawnej konfiguracji.
5. Dodać repozytoria i trasy API pomiarów, treningów i gier. Akwizycję urządzeń utrzymać oddzielnie, a zweryfikowane wyniki sesji przekazywać przez usługę aplikacyjną.
6. Dodać adapter MariaDB i migracje schematu. Uruchamiać ten sam zestaw testów kontraktowych dla backendu plikowego i MariaDB.
7. Migrować najpierw Desktop Linux i TUI, następnie WebUI i Android, a potem Desktop Windows. Nie usuwać lokalnych ścieżek legacy przed wykazaniem zgodności danych i możliwości wycofania.
8. Dodać testy wieloklientowe obejmujące tworzenie i edycję pacjenta, odczyt pomiarów, treningów i wyników gier z różnych klientów.
9. Dodać CI Windows (MSVC build/test/package), testy instrumentacyjne Androida, testy integracji WebUI z serwerem, testy kopii/odtwarzania, współbieżności i bezpieczeństwa.

## 6. Kryteria odbioru

- Jeden kanoniczny rekord pacjenta jest spójnie widoczny dla wszystkich uprawnionych klientów.
- Plikowy i MariaDB backend kart pacjentów przechodzą identyczne testy kontraktowe repozytorium.
- W trybie plikowym rekordy pacjentów są szyfrowane w spoczynku; sekrety nie występują w kodzie, logach ani odpowiedziach API.
- Nieudany zapis nie pozostawia uciętego ani uszkodzonego rekordu pacjenta.
- Równoczesne edycje powodują wykrywalny konflikt zamiast cichej utraty danych.
- Wyniki pomiarów, treningów i gier są powiązane z właściwym pacjentem/sesją i przetrwają restart serwera.
- Autoryzacja klientów jest testowana dla dostępu dozwolonego i zabronionego.
- Odtworzenie kopii przywraca rekordy i zachowuje identyfikatory oraz relacje.
- CI raportuje każdą platformę oddzielnie; platformy niezbudowane lub nieprzetestowane nie są oznaczane jako zaliczone.

## 7. Aktualny stan implementacji opisany w przeglądzie

- Dodano `JsonResourceRepository` dla sześciu kolekcji: patients, measurements, training-sessions, training-results, game-sessions i game-results.
- Magazyn plikowy używa UUID, atomowego zapisu `QSaveFile`, limitu 16 MiB, kontroli wersji i uprawnień owner-only. Gdy `encryption.enabled=true`, rekordy są szyfrowane AES-256-GCM; brak klucza powoduje odmowę inicjalizacji.
- Desktop i wbudowany serwer HTTP współdzielą tę samą instancję repozytorium. Zakładka Pacjenci zapisuje i odczytuje karty; sesje pomiarowe są archiwizowane wraz z próbkami, sesje treningowe wraz z wynikami ćwiczeń, a wyniki gier trafiają do kolekcji game-sessions i game-results. Stare pliki pacjentów są migrowane bez usuwania źródeł migracji.
- API ma CRUD kolekcji, sesje Bearer, kontrolę wersji i tymczasową blokadę dostępu do roli administratora. Dla pomiarów, treningów i gier walidowany jest patient_id, jeżeli został podany; wyniki treningowe mogą wskazywać tylko istniejącą sesję treningową, a patient_id wyniku i sesji nie mogą być różne. Usunięcie pacjenta z rekordami zależnymi oraz sesji treningowej z wynikami jest blokowane (HTTP 409). Pełne RBAC, audyt operacji i WebSocket/SSE pozostają do wdrożenia.
- WebUI obsługuje CRUD kart pacjentów oraz odczyt archiwów pomiarów, treningów i gier. Android obsługuje CRUD kart pacjentów, odczyt sześciu kolekcji API (w tym sesji gier) oraz bezpośredni odczyt pojedynczych rekordów. Ekrany Androida pozostają w trybie podglądu: aplikacja nie prowadzi jeszcze akwizycji ani tworzenia rzeczywistych sesji z czujnika, treningu i gry.
- TUI loguje się przez API, odczytuje listę pacjentów, zapisuje sesje pomiarowe przy rozłączeniu czujnika lub zamknięciu aplikacji oraz zapisuje sesje i wyniki treningowe. TUI nie implementuje obecnie silnika gier ani zapisu wyników gier.
- MariaDB pozostaje niewdrożona; wybranie jej jako backendu pacjentów nie powoduje fallbacku do plików.

Ten opis nie oznacza pełnej synchronizacji pięciu interfejsów. Wymagane są dalsze integracje WebUI i Androida dla pomiarów/treningów/gier, archiwizacja gier z TUI, pełne RBAC, testy wieloklientowe i testy kopii/odtwarzania. Wyniki CI należy przypisywać wyłącznie do konkretnego SHA; sam kod repozytorium nie stanowi dowodu poprawności klinicznej ani kompletności bezpieczeństwa.
