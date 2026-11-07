# Biofeedback WebUI

Responsywny interfejs webowy dla systemu Biofeedback. Wersja 0.2 zawiera
nawigację modułową z obsługą adresów URL, tryb jasny/ciemny oraz klienta
wspólnego API REST/WebSocket.

## Uruchomienie

Z katalogu głównego repozytorium:

    python3 -m http.server 8080 --directory platforms/webui

Następnie otwórz http://localhost:8080.

## Klient API

`api-client.js` udostępnia `window.BiofeedbackApi.createApiClient()`:
- status i lista urządzeń,
- połączenie i rozłączenie urządzenia,
- sesje pomiarowe i repozytorium pacjentów,
- subskrypcje WebSocket dla pomiarów i zdarzeń,
- ujednolicone błędy HTTP, sieci i niepoprawnej odpowiedzi.

Klient domyślnie używa tego samego pochodzenia (same-origin). Interfejs
wykonuje kontrolne żądanie `GET /api/v1/status`; brak serwera jest jawnie
pokazywany jako stan niedostępności. Nie ma trybu zastępczego z fikcyjnymi
danymi. Polecenia urządzeń nie są wysyłane automatycznie.

## Ważne ograniczenie wersji 0.2

Serwer API i adapter sprzętowy nie są jeszcze zaimplementowane. Klient
definiuje warstwę integracyjną, ale nie zastępuje backendu ani
MeasurementEngine. Nie generuje fikcyjnych odczytów, kart pacjentów ani
wyników terapii.

Kontrakt integracyjny opisano w `API_CONTRACT.md`.
Testy klienta: `node --test platforms/webui/api-client.test.js`.
