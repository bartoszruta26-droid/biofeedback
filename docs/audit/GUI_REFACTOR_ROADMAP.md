# Plan naprawczy P0/P1

Każdy punkt ma być osobnym PR do `integration/all-work`. Nie scalać automatycznie. Przed utworzeniem gałęzi i przed aktualizacją ref ponownie sprawdzić HEAD.

## P0

### P0-1 — anonimizacja logów pacjentów
- Zakres: wszystkie logi, debug terminal, obsługa zdarzeń pacjentów.
- Testy: regresja potwierdzająca brak PESEL/imienia/nazwiska.
- Odbiór: dane osobowe nie występują w logach aplikacyjnych.

### P0-2 — wspólne Application Services
- Zakres: interfejsy usług i adapter dla istniejących funkcji; bez jednoczesnego przepisywania wszystkich klientów.
- Testy: jednostkowe usług i testy zgodności zachowania Desktop/TUI.
- Odbiór: UI nie zawiera logiki domenowej ani bezpośredniego dostępu do transportu.

### P0-3 — uwierzytelnianie i autoryzacja API
- Zakres: model tożsamości, role, sesje, middleware, ochrona komend.
- Testy: brak tokena, token błędny/wygasły, rola niewystarczająca, odwołanie sesji.
- Odbiór: każda operacja wrażliwa wymaga uprawnień; domyślnie API pozostaje loopback.

## P1

### P1-1 — właściciel urządzenia i pomiarów
- Zakres: pojedynczy DeviceService i MeasurementService.
- Testy: jedna subskrypcja portu, rozłączenie, timeout, błędna ramka, CRC, kalibracja.
- Odbiór: UI konsumują ten sam strumień próbek.

### P1-2 — Web API i zdarzenia
- Zakres: endpointy pacjentów/sesji/urządzeń oraz strumień dopiero po P0-3.
- Testy: kontrakt API, uprawnienia, reconnect, backpressure.
- Odbiór: brak danych demonstracyjnych w aktywnych pomiarach.

### P1-3 — Android
- Zakres: klient API, bezpieczne przechowywanie poświadczeń, stany offline i cykl życia.
- Testy: kompilacja Gradle, testy ViewModel, instrumentacyjne.
- Odbiór: Android pokazuje rzeczywisty stan backendu i jawnie sygnalizuje offline.

### P1-4 — refaktoryzacja Desktop/TUI
- Zakres: wydzielanie komponentów i kontrolerów małymi krokami.
- Testy: regresje nawigacji, pomiaru, zamykania i odłączania urządzenia.
- Odbiór: bez zmiany istniejących przepływów użytkownika.

## Bramki jakości

- Build i testy wymagane dla dotkniętej platformy.
- Testy bezpieczeństwa i prywatności dla zmian P0.
- Testy kontraktowe przy zmianie DTO/API.
- PR opisuje ryzyko, migrację, ograniczenia i sposób wycofania.
