# WebUI — rzeczywisty kontrakt integracyjny API i funkcje planowane

> 📘 Weryfikacja kodu: 2026-11-09 • Bazowy SHA: `28272ea153864747d947038a3c0e853f9c1864ec`  
> Ten dokument rozdziela endpointy faktycznie zaimplementowane od funkcji docelowych. Źródłem prawdy jest `src/api/HttpApiServer.cpp`.

## 1. Endpointy zaimplementowane

| Metoda | Ścieżka | Dostęp | Działanie |
|---|---|---|---|
| GET | `/healthz` | Publiczny | Test dostępności |
| GET | `/api/v1/status` | Publiczny | Wersja, tryb, kolekcje, możliwości i ograniczenia |
| POST | `/api/v1/auth/login` | Publiczny | Logowanie i wydanie tokenu Bearer |
| GET | `/api/v1/auth/me` | Zalogowany | Tożsamość, rola i termin wygaśnięcia |
| POST | `/api/v1/auth/logout` | Zalogowany | Unieważnienie bieżącej sesji |
| GET | `/api/v1/devices` | Administrator | Lista urządzeń i ich stanów |
| GET | `/api/v1/acquisition/session` | Administrator | Odczyt stanu aktywnej sesji |
| POST | `/api/v1/acquisition/session` | Administrator | Rozpoczęcie sesji akwizycji |
| DELETE | `/api/v1/acquisition/session` | Administrator | Zatrzymanie sesji po identyfikatorze |
| GET | `/api/v1/acquisition/samples?after_sequence=N` | Administrator | Odczyt próbek z ograniczonej historii w pamięci |
| GET/POST | `/api/v1/{collection}` | Administrator | Lista / utworzenie rekordu |
| GET/PUT/DELETE | `/api/v1/{collection}/{id}` | Administrator | Odczyt / aktualizacja / usunięcie rekordu |

Dozwolone kolekcje:
- `patients`
- `measurements`
- `training-sessions`
- `training-results`
- `game-sessions`
- `game-results`

### Przykład operacji CRUD

Utworzenie rekordu:

```http
POST /api/v1/patients
Authorization: Bearer <token>
Content-Type: application/json

{"firstName":"Jan","lastName":"Kowalski","pesel":"<identyfikator>","medicalHistory":""}
```

Aktualizacja wymaga treści `{"expected_version":1,"record":{...}}`. Usunięcie wymaga `{"expected_version":1}`. Konflikt wersji zwraca HTTP 409. Serwer tworzy UUID i pola `version`, `created_at`, `updated_at`.

Rekord pacjenta wymaga niepustych pól `firstName`, `lastName` i `pesel`; identyfikator ma maksymalnie 32 znaki i musi być unikalny. Ścieżka rekordu używa UUID, nigdy PESEL.

## 2. Sesja akwizycji

Rozpoczęcie sesji wymaga obiektu zawierającego:
- `id` — identyfikator sesji;
- `patient_id` — UUID istniejącego pacjenta;
- `device_id` — identyfikator zarejestrowanego i połączonego urządzenia;
- opcjonalnie `protocol_id`.

Serwer odrzuca żądanie, jeśli repozytorium pacjentów lub rejestr urządzeń są niedostępne, pacjent nie istnieje, urządzenie nie jest zarejestrowane albo nie jest połączone. W danym momencie może działać jedna sesja akwizycji.

Urządzenie Arduino Nano HX711 jest rejestrowane przez samodzielny proces API tylko po jawnym ustawieniu `BIOFEEDBACK_SERIAL_PORT`. Opcjonalne zmienne: `BIOFEEDBACK_SERIAL_BAUD` i `BIOFEEDBACK_DEVICE_ID`. Bez portu sprzętowa akwizycja pozostaje wyłączona.

## 3. Odczyt próbek — ograniczenia retencji

`GET /api/v1/acquisition/samples?after_sequence=N` zwraca m.in.:
- `items` — próbki o numerze większym niż `after_sequence`;
- `count`;
- `after_sequence`;
- `next_sequence`;
- `retention: "bounded_in_memory"`;
- opcjonalnie `session_id` i `device_id`.

Domyślna historia silnika obejmuje 512 próbek. W samodzielnym `biofeedback_api` nie jest obecnie podłączony trwały dziennik próbek ani `CommitHook`. Historia jest tracona po restarcie procesu, a sekwencja zaczyna się ponownie od wartości początkowej. Ten endpoint nie jest archiwum klinicznym ani strumieniem push czasu rzeczywistego.

## 4. Endpointy i funkcje NIEZAIMPLEMENTOWANE

Poniższe funkcje są wymienione w kliencie JavaScript lub w starszym opisie docelowym, ale nie mają działających tras serwera:

| Metoda / kanał | Ścieżka | Stan |
|---|---|---|
| POST | `/api/v1/devices/{id}/connect` | Niezaimplementowane |
| POST | `/api/v1/devices/{id}/disconnect` | Niezaimplementowane |
| WebSocket | `/ws/measurements` | Niezaimplementowane |
| WebSocket | `/ws/events` | Niezaimplementowane |
| SSE | — | Niezaimplementowane |
| RBAC per rola/pacjent | — | Niezaimplementowane |

Metody `connectDevice`, `disconnectDevice`, `subscribeMeasurements` i `subscribeEvents` w `platforms/webui/api-client.js` nie potwierdzają dostępności odpowiednich funkcji po stronie serwera.

## 5. Uwierzytelnianie i bezpieczeństwo

- Token Bearer powstaje z 32 losowych bajtów, ma 64 znaki szesnastkowe i wygasa po 30 minutach.
- Serwer przechowuje sesje w pamięci; limit wynosi 64 aktywne sesje.
- Po pięciu nieudanych próbach logowania z danego adresu klienta następuje blokada na 60 sekund.
- Wszystkie operacje na kolekcjach, rejestrze urządzeń i sesji akwizycji wymagają obecnie roli `administrator`.
- Serwer API domyślnie nasłuchuje na `127.0.0.1:8765`; sam serwer nie zapewnia TLS.
- Android wymaga adresu HTTPS. Połączenie zdalne wymaga terminacji TLS przez reverse proxy/bramę.
- Publiczne `/healthz` i `/api/v1/status` nie zwracają danych pacjentów.
- Nie wystawiaj surowego HTTP poza loopback.

## 6. Status odpowiedzi API

Endpoint `/api/v1/status` aktualnie zwraca wersję `0.3.0`, tryb `shared-data-api`, kolekcje, możliwości i ograniczenia. Wskazuje m.in. ograniczenia `device_control`, `measurement_stream`, `role_scoped_authorization`, `websocket_events` i `mariadb_adapter`.

Uwaga: `QCoreApplication::applicationVersion()` w `src/api/main.cpp` jest ustawione na `0.2.0`, podczas gdy odpowiedź statusu ma `0.3.0`. To rozbieżność wersjonowania do usunięcia w kodzie.

## 7. Wymagane dalsze prace

1. Trwałe zatwierdzanie próbek i odzyskiwanie sekwencji po restarcie.
2. Wybór oraz wdrożenie transportu strumieniowego (WebSocket/SSE) lub stabilnego odpytywania z wykrywaniem luk.
3. Dodanie rzeczywistych tras łączenia/rozłączania urządzeń albo usunięcie nieobsługiwanych metod z klienta.
4. Wdrożenie RBAC i kontroli dostępu do danych pacjentów.
5. Testy integracyjne WebUI–API, Android–API oraz testy restartu, utraty sieci i konfliktów wersji.

## 8. Testy

`node --test platforms/webui/api-client.test.js` weryfikuje klienta JavaScript z kontrolowanymi odpowiedziami. Nie jest to test kompletności endpointów backendu ani test end-to-end z urządzeniem.
