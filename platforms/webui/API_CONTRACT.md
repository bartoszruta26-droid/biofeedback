# WebUI — kontrakt integracyjny API

Docelowa komunikacja powinna korzystać ze wspólnej warstwy usług aplikacyjnych,
a nie implementować logiki pomiarowej w przeglądarce.

## HTTP

- GET /api/v1/status — status aplikacji, wersja, tryb pracy.
- GET /api/v1/devices — lista urządzeń i stan połączenia.
- POST /api/v1/devices/{id}/connect — żądanie połączenia.
- POST /api/v1/devices/{id}/disconnect — rozłączenie.
- GET /api/v1/measurements/sessions — historia sesji.
- POST /api/v1/measurements/sessions — utworzenie sesji.
- GET /api/v1/patients — lista pacjentów zgodnie z uprawnieniami.
- GET /api/v1/patients/{id} — szczegóły karty.

## WebSocket

- /ws/measurements — strumień próbek pomiarowych.
- /ws/events — zdarzenia urządzeń i stan usług.

## Zasady

- API musi rozróżniać brak danych, brak urządzenia i błąd pomiaru.
- Próbki powinny zawierać znacznik czasu, jednostkę, źródło i status jakości.
- Polecenia sprzętowe wymagają potwierdzenia stanu adaptera.
- WebUI nie może prezentować danych demonstracyjnych jako danych klinicznych.
- Kontrakt jest propozycją architektoniczną; serwer API nie jest jeszcze
  zaimplementowany.
