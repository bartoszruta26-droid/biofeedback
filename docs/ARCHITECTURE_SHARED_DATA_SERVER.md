# 🧱 Wspólny serwer danych — architektura i plan migracji

> 📘 Weryfikacja kodu: 2026-11-09 • Bazowy SHA: `28272ea153864747d947038a3c0e853f9c1864ec`  
> Status: plikowe repozytorium i API CRUD wdrożone; MariaDB i pełny strumień zdarzeń pozostają planem.

## 1. Cel i zakres

Dokument określa granice między magazynem danych, akwizycją sprzętową, serwerem API i klientami. Oddziela rozwiązania obecne w kodzie od architektury docelowej.

## 2. Stan architektury w bieżącym kodzie

### Magazyn danych i API

`JsonResourceRepository` przechowuje sześć kolekcji plikowych: `patients`, `measurements`, `training-sessions`, `training-results`, `game-sessions` i `game-results`. Używa UUID, kontroli wersji, zapisu atomowego `QSaveFile`, limitu 16 MiB na rekord i uprawnień katalogu ograniczonych do właściciela. Szyfrowanie rekordów zależy od `encryption.enabled`; gdy jest włączone, brak klucza powoduje odmowę inicjalizacji magazynu.

Samodzielny proces `biofeedback_api` uruchamia serwer HTTP na `127.0.0.1` (domyślnie port 8765). Aplikacja Desktop ma własną integrację repozytorium i serwera w procesie aplikacji. Dostęp zdalny wymaga terminacji TLS przed serwerem; surowego HTTP nie należy wystawiać do sieci publicznej.

### Akwizycja

Desktop i TUI otwierają port szeregowy lokalnie i korzystają z adaptera `MeasurementSerialAdapter` oraz lokalnego `MeasurementEngine`. Samodzielny serwer API może niezależnie otworzyć jedno skonfigurowane urządzenie Arduino Nano HX711 przez `AcquisitionSerialDeviceAdapter`, `DeviceService` i `AcquisitionService`, jeśli ustawiono `BIOFEEDBACK_SERIAL_PORT`.

Serwerowa akwizycja jest ograniczona do jednej aktywnej sesji i identyfikatora zarejestrowanego urządzenia. Samo uruchomienie serwera bez `BIOFEEDBACK_SERIAL_PORT` pozostawia sprzętową akwizycję wyłączoną.

### Klienci

| Klient | Dostęp do danych | Tor sprzętowy |
|---|---|---|
| Desktop Qt | Lokalna instancja wspólnego repozytorium i API | Lokalny port szeregowy |
| TUI | API: lista pacjentów, zapis archiwów | Lokalny port szeregowy |
| WebUI | HTTP API: CRUD pacjentów, odczyt archiwów, ręczne wpisy treningu i gry | Brak |
| Android | HTTPS API: CRUD pacjentów, odczyt archiwów | Brak |

Nie jest prawdą, że wszystkie pięć klientów pobiera próbki przez jeden zdalny strumień API. WebUI i Android nie mają strumienia pomiarowego. Desktop i TUI korzystają obecnie z lokalnego transportu szeregowego.

## 3. Kontrakt API wdrożony obecnie

Wersjonowanie ścieżek: `/api/v1`.

| Metoda | Ścieżka | Funkcja |
|---|---|---|
| GET | `/healthz` | Publiczny test dostępności |
| GET | `/api/v1/status` | Publiczne możliwości i ograniczenia |
| POST | `/api/v1/auth/login` | Logowanie i wydanie tokenu Bearer |
| GET | `/api/v1/auth/me` | Tożsamość i rola |
| POST | `/api/v1/auth/logout` | Unieważnienie sesji |
| GET | `/api/v1/devices` | Lista urządzeń — tylko administrator |
| GET | `/api/v1/acquisition/session` | Stan aktywnej sesji |
| POST | `/api/v1/acquisition/session` | Rozpoczęcie sesji |
| DELETE | `/api/v1/acquisition/session` | Zatrzymanie sesji |
| GET | `/api/v1/acquisition/samples?after_sequence=N` | Odczyt ograniczonej historii próbek |
| GET/POST | `/api/v1/{collection}` | Lista / utworzenie rekordu |
| GET/PUT/DELETE | `/api/v1/{collection}/{id}` | Odczyt / aktualizacja / usunięcie rekordu |

Kolekcje: `patients`, `measurements`, `training-sessions`, `training-results`, `game-sessions`, `game-results`. Aktualizacja i usunięcie wymagają `expected_version`; konflikt wersji zwraca HTTP 409. Identyfikatory rekordów są UUID. Maksymalny rozmiar treści żądania wynosi 16 MiB.

**Niezaimplementowane:** `POST /api/v1/devices/{id}/connect`, `POST /api/v1/devices/{id}/disconnect`, WebSocket, SSE, zdalne sterowanie portem oraz zakresy uprawnień per pacjent. Metody takie występują w kliencie JavaScript lub w kontrakcie docelowym, ale nie mają odpowiadających im tras serwera.

## 4. Uwierzytelnianie i ograniczenia uprawnień

Token Bearer ma 32 bajty losowości (64 znaki szesnastkowe), wygasa po 30 minutach i jest przechowywany w pamięci serwera. Serwer ogranicza nieudane próby logowania: po pięciu błędach z adresu klienta blokuje kolejne próby na 60 sekund. Limit aktywnych sesji wynosi 64.

Obecnie odczyt i modyfikacja kolekcji, rejestr urządzeń i sterowanie akwizycją wymagają roli `administrator`. Pełne RBAC i zakresy dostępu do danych pacjenta nie są wdrożone. Jest to ograniczenie funkcjonalne, a nie docelowy model kliniczny.

## 5. Retencja próbek i transmisja

Endpoint `/api/v1/acquisition/samples` wykonuje odczyt przez HTTP. Zwraca `retention: "bounded_in_memory"`; domyślna historia silnika ma 512 próbek. Samodzielny `biofeedback_api` nie podłącza trwałego `AcquisitionJournal` ani `CommitHook` do silnika. Próbki nie są więc automatycznie utrwalane i znikają po restarcie procesu. WebSocket/SSE pozostaje niezaimplementowany.

Kolekcja `measurements` jest osobnym trwałym archiwum rekordów JSON. Zapisanie rekordu pomiarowego przez klienta nie oznacza, że każda próbka serwerowego strumienia została automatycznie zarchiwizowana.

## 6. Backend przechowywania

- `storage.patient_backend: "files"` — obecny backend plikowy.
- `storage.patient_backend: "mariadb"` — opcja docelowa, niezaimplementowana.

Wybranie MariaDB powoduje jawny błąd inicjalizacji i brak awaryjnego przełączenia na pliki. Nie wolno opisywać MariaDB jako działającego backendu.

## 7. Następne etapy

1. Wprowadzić trwały dziennik próbek w samodzielnym API i odzyskiwanie numeru sekwencji po restarcie.
2. Dodać kontrolowany strumień zdarzeń WebSocket/SSE albo jawnie wersjonowane odpytywanie z kursorem, wykrywaniem luk i backpressure.
3. Zaimplementować serwerowe endpointy połączenia/rozłączenia urządzeń albo usunąć nieobsługiwane metody z klientów.
4. Wdrożyć RBAC z zakresem pacjentów i audytem operacji.
5. Zaimplementować i przetestować adapter MariaDB.
6. Dodać testy integracyjne obejmujące Desktop, TUI, WebUI i Android, w tym restart serwera, utratę sieci, konflikty wersji i nieuprawniony dostęp.

## 8. Kryteria odbioru

- Każdy zaakceptowany pomiar jest trwale zatwierdzony przed potwierdzeniem i publikacją.
- Restart nie powoduje ponownego użycia identyfikatorów sekwencji ani utraty zatwierdzonych próbek.
- Klient wykrywa lukę w sekwencji i potrafi wznowić pobieranie.
- Żaden klient nie może odczytać ani zmodyfikować rekordu poza przyznanym zakresem.
- API statusu odzwierciedla rzeczywiste endpointy i możliwości uruchomionej instancji.
- Testy kontraktowe obejmują wszystkie wspierane metody i kody błędów.
