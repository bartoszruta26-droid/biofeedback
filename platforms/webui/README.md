# Biofeedback WebUI

Responsywny interfejs webowy dla systemu Biofeedback. Wersja 0.2 zawiera
nawigację modułową z obsługą adresów URL, tryb jasny/ciemny oraz klienta
wspólnego API REST/WebSocket.

## Uruchomienie

Z katalogu głównego repozytorium:

    python3 -m http.server 8080 --directory platforms/webui

Następnie otwórz http://localhost:8080.

## Klient API i widoki danych

`api-client.js` udostępnia `window.BiofeedbackApi.createApiClient()`:
- logowanie Bearer i wylogowanie;
- CRUD pacjentów, pomiarów, sesji/wyników treningowych oraz sesji/wyników gier;
- kontrolę wersji rekordów przy aktualizacji i usuwaniu;
- ujednolicone błędy HTTP, sieci i niepoprawnej odpowiedzi.

Po zalogowaniu strony Pacjenci, Pomiary i Trening pobierają dane z tych samych
kolekcji API. Karty pacjentów można dodawać, edytować i usuwać; pozostałe strony
wyświetlają archiwum pomiarów, treningów i gier. Klient domyślnie używa tego
samego pochodzenia (same-origin). Brak serwera i brak uprawnień są jawnie
pokazywane. Nie ma trybu zastępczego z fikcyjnymi danymi. WebSocket i
strumieniowanie pomiarów czasu rzeczywistego nie są jeszcze dostępne.

## Ważne ograniczenie wersji 0.2

Serwer API i adapter sprzętowy nie są jeszcze zaimplementowane. Klient
definiuje warstwę integracyjną, ale nie zastępuje backendu ani
MeasurementEngine. Nie generuje fikcyjnych odczytów, kart pacjentów ani
wyników terapii.

Kontrakt integracyjny opisano w `API_CONTRACT.md`.
Testy klienta: `node --test platforms/webui/api-client.test.js`.
