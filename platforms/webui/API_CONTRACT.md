# WebUI — kontrakt integracyjny API

Docelowa komunikacja korzysta ze wspólnej warstwy usług aplikacyjnych.
Przeglądarka nie implementuje parsera czujnika, kalibracji ani logiki sesji.

## HTTP v1

| Metoda | Ścieżka | Cel |
|---|---|---|
| GET | /api/v1/status | Publiczny status usługi, wersja i gotowość |
| POST | /api/v1/auth/login | Logowanie; zwraca losowy token Bearer |
| GET | /api/v1/auth/me | Tożsamość i rola aktywnej sesji |
| POST | /api/v1/auth/logout | Unieważnienie tokenu bieżącej sesji |
| GET | /api/v1/devices | Lista urządzeń i stan adapterów |
| POST | /api/v1/devices/{id}/connect | Żądanie połączenia |
| POST | /api/v1/devices/{id}/disconnect | Żądanie rozłączenia |
| GET | /api/v1/measurements/sessions | Historia sesji według uprawnień |
| POST | /api/v1/measurements/sessions | Utworzenie sesji |
| GET | /api/v1/patients | Lista pacjentów według uprawnień |
| GET | /api/v1/patients/{id} | Szczegóły karty według uprawnień |

### Przykładowe odpowiedzi

Status usługi:

```json
{
  "status": "ready",
  "version": "1.0.0",
  "mode": "desktop",
  "timestamp": "2026-11-07T10:00:00Z"
}
```

Urządzenie:

```json
{
  "id": "sensor-01",
  "name": "Czujnik siły",
  "state": "connected",
  "transport": "serial",
  "last_seen": "2026-11-07T10:00:00Z"
}
```

Próbka pomiarowa:

```json
{
  "session_id": "session-uuid",
  "timestamp": "2026-11-07T10:00:00.125Z",
  "value": 12.4,
  "unit": "N",
  "source": "sensor-01",
  "quality": "valid"
}
```

## WebSocket

- /ws/measurements — strumień próbek pomiarowych.
- /ws/events — zdarzenia urządzeń, sesji i stanu usług.

Wiadomości są obiektami JSON. Klient musi obsługiwać niepoprawne
wiadomości, zamknięcie połączenia i brak serwera bez generowania danych
zastępczych.

## Błędy i stany

- 400 — niepoprawne żądanie lub parametry.
- 401 — brak uwierzytelnienia.
- 403 — brak uprawnień do zasobu lub operacji.
- 404 — nieznane urządzenie, pacjent lub sesja.
- 409 — konflikt stanu, np. urządzenie już połączone.
- 422 — poprawne składniowo żądanie, którego nie można wykonać.
- 503 — usługa lub adapter niedostępny.

Odpowiedź błędu powinna zawierać stabilny kod i bezpieczny komunikat,
np. `{"code":"DEVICE_BUSY","message":"Urządzenie jest zajęte"}`.

## Bezpieczeństwo i integralność

- Logowanie API korzysta z tego samego magazynu kont i weryfikacji haseł co aplikacja Qt.
- Token Bearer ma 256 bitów losowości, jest przechowywany wyłącznie w pamięci serwera i wygasa po 30 minutach.
- Serwer ogranicza próby logowania: po pięciu błędnych próbach z adresu klienta blokuje kolejne próby na 60 sekund.
- WebUI przechowuje token wyłącznie w pamięci klienta; nie zapisuje go w localStorage ani w adresie URL.
- Endpoint /api/v1/status i /healthz są publiczne i nie zwracają danych pacjentów.
- Endpointy danych, sesji i sterowania urządzeniami pozostają wyłączone do czasu wdrożenia kontroli ról i integracji usług.
- API musi egzekwować uwierzytelnianie i autoryzację po stronie serwera.
- Dane pacjentów są dostępne wyłącznie zgodnie z rolą i zakresem uprawnień.
- Nie wolno umieszczać danych osobowych ani sekretów w adresach URL i logach.
- Polecenia sprzętowe wymagają potwierdzenia stanu adaptera.
- Próbki muszą zawierać czas, jednostkę, źródło i status jakości.
- Należy rozróżniać brak danych, brak urządzenia i błąd pomiaru.
- WebUI nie może prezentować danych demonstracyjnych jako klinicznych.
- Wdrożenie produkcyjne wymaga HTTPS/WSS i jawnej polityki CORS.

## Stan implementacji

Klient JavaScript jest zaimplementowany w `api-client.js` i pokryty testami
Node.js. Serwer `biofeedback_api` udostępnia `GET /healthz`,
`GET /api/v1/status`, `POST /api/v1/auth/login`,
`GET /api/v1/auth/me` oraz `POST /api/v1/auth/logout`.
Sesje są przechowywane w pamięci procesu i wygasają po 30 minutach.
Domyślnie serwer nasłuchuje wyłącznie na `127.0.0.1:8765`.

Endpointy urządzeń, pacjentów i sesji oraz kanały WebSocket są nadal
zarezerwowane, lecz niewdrożone. Samo zwrócenie roli przez endpoint
tożsamości nie oznacza jeszcze kompletnej autoryzacji zasobów. Desktop GUI
używa lokalnego okna logowania; integracja jego sesji z API, a także
integracja TUI i Androida z tokenem API, pozostają kolejnymi etapami.
Nie wystawiać serwera poza loopback bez HTTPS, kontroli pochodzenia,
ochrony CSRF/CORS i pełnej autoryzacji ról. Nie traktować go jako gotowego
interfejsu klinicznego.

Uruchomienie:

    cmake -S . -B build
    cmake --build build --target biofeedback_api
    ./build/biofeedback_api

Przykłady JSON powyżej są schematem kontraktu; rzeczywista odpowiedź statusu
jest generowana przez lokalny serwer.
