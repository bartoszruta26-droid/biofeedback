# WebUI — kontrakt integracyjny API

Docelowa komunikacja korzysta ze wspólnej warstwy usług aplikacyjnych.
Przeglądarka nie implementuje parsera czujnika, kalibracji ani logiki sesji.

## HTTP v1

| Metoda | Ścieżka | Cel |
|---|---|---|
| GET | /api/v1/status | Status usługi, wersja, tryb i gotowość |
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
Node.js. W repozytorium znajduje się także `biofeedback_api`, lokalny serwer
Qt Network udostępniający `GET /healthz` oraz `GET /api/v1/status`.
Domyślnie nasłuchuje wyłącznie na `127.0.0.1:8765`.

Pozostałe endpointy (urządzenia, pacjenci, sesje) i kanały WebSocket są
zarezerwowane w kontrakcie, ale nie są jeszcze wdrożone. Serwer zwraca
jawny błąd dla nieobsługiwanych operacji. Nie należy wystawiać go poza
interfejs lokalny ani traktować jako gotowego interfejsu klinicznego.

Uruchomienie:

    cmake -S . -B build
    cmake --build build --target biofeedback_api
    ./build/biofeedback_api

Przykłady JSON powyżej są schematem kontraktu; rzeczywista odpowiedź statusu
jest generowana przez lokalny serwer.
