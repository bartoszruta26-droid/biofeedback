# Audyt interfejsów Biofeedback

Data audytu: 2026-11-07  
Audytowana gałąź: `integration/all-work`  
Commit bazowy audytu: `cdbb661e6154362aca125cb07091f9cd1085ac15`  
Zakres: Desktop GUI, TUI, Web UI, Android GUI, kontrakty integracyjne i CI.

## Zakres i ograniczenia

Audyt wykonano przez przegląd kodu, dokumentacji, konfiguracji CMake/Gradle i dostępnych wyników GitHub Actions. Nie wykonywano testów na fizycznym Raspberry Pi, Arduino ani czujniku HX711. Wynik CI dla desktopowego celu i TUI nie oznacza walidacji wszystkich zachowań GUI w środowisku graficznym.

## Ocena platform

| Platforma | Stan rozpoznany w kodzie | Główne ograniczenie |
|---|---|---|
| Desktop Qt5 | Rozbudowana aplikacja z zakładkami, logowaniem i obsługą szeregową | MainWindow i Application skupiają zbyt wiele odpowiedzialności; brak wspólnego modelu usług dla pozostałych klientów |
| TUI ncurses | Model siedmiu ekranów, tryby CLI, obsługa adaptera szeregowego i test modelu | Pacjenci/trening/ustawienia nie mają pełnej parytetowej funkcjonalności; logika interfejsu i urządzenia wymaga dalszego rozdzielenia |
| Web UI | HTML/CSS/JS, klient API, testy klienta, responsywny panel | API backendu udostępnia obecnie status/health; pacjenci, sesje, urządzenia i strumień pomiarowy nie są gotowe |
| Android | Kotlin/Jetpack Compose, nawigacja i ekrany demonstracyjne | README jawnie potwierdza brak API, Bluetooth, USB i portu szeregowego; nie potwierdzono uruchomienia workflow Android dla badanego HEAD |

## Główne ustalenia

### P0 — blokujące bezpieczną integrację

1. Brak kompletnej wspólnej warstwy Application Services i stabilnego kontraktu domenowego dostępnego dla wszystkich klientów.
2. Brak pełnego uwierzytelniania i autoryzacji wieloplatformowej. Nie wolno wystawiać prototypowego API statusu jako usługi sieciowej.
3. Ryzyko ujawnienia danych osobowych w logach: w kodzie Application.cpp znaleziono logowanie PESEL oraz imienia i nazwiska przy zdarzeniu dodania pacjenta. Wymagana jest kontrola wszystkich ścieżek logowania i testy regresyjne.

### P1 — istotne dla poprawności i utrzymywalności

4. Nadmierna odpowiedzialność MainWindow oraz Application.
5. Sprzężenie logiki TUI z obsługą urządzeń i duży punkt wejścia TUI.
6. Brak rzeczywistego strumienia pomiarowego WebSocket oraz endpointów wymaganych przez moduły Web UI.
7. Android pozostaje demonstracyjnym klientem bez komunikacji z backendem.
8. Brak wspólnego cyklu życia sesji, obsługi błędów i stanu połączeń.
9. Nierówny poziom testów platformowych; brak testów instrumentacyjnych Android i testów E2E Web/Desktop.

### P2 — jakość i spójność

10. Niejednolity system komponentów, ikon i motywów.
11. Brak kompletnej macierzy dostępności i responsywności.
12. Dokumentacja nie rozróżniała dostatecznie funkcji docelowych od zaimplementowanych.

## Zasady napraw

- Nie zmieniać gałęzi `main`.
- Każda niezależna poprawka P0/P1 otrzymuje własną gałąź, commit i PR do `integration/all-work`.
- Przed każdą operacją zapisu ponownie pobrać HEAD gałęzi bazowej.
- Aktualizować ref wyłącznie z oczekiwanym SHA (bez wymuszonego nadpisania).
- Nie scalać PR automatycznie.
- Nie oznaczać testu jako zaliczonego bez rzeczywistego wyniku CI.
- Nie włączać sterowania sprzętem przez API bez uwierzytelniania, autoryzacji, walidacji wejścia, limitów i bezpiecznego modelu współbieżności.
- Każda zmiana danych pacjentów wymaga testów migracji, kopii i odtwarzania.

## Weryfikacja bazowa

Dla badanego commit `cdbb661e` potwierdzono sukces workflow `security-regression` oraz `WebUI API client`. Workflow security regresji obejmuje kompilację wybranych celów C++, TUI, API, smoke testy HTTP, kontrolę trybów TUI i CTest. Workflow Web UI sprawdza składnię JS i testy klienta. Nie znaleziono uruchomienia workflow `android-ui-build` dla tego HEAD. Nie jest to dowód, że Android się nie buduje — oznacza brak potwierdzenia.

## Priorytety

P0-1: usunięcie danych osobowych z logów i test regresyjny.  
P0-2: projekt i implementacja wspólnego Application Services/kontraktu API.  
P0-3: model uwierzytelniania i autoryzacji API.  
P1-1: wydzielenie usług urządzeń i pomiarów od UI.  
P1-2: klient Web UI i strumień pomiarowy po wdrożeniu bezpiecznego API.  
P1-3: klient Android po ustabilizowaniu kontraktu API.  
P1-4: refaktoryzacja TUI i Desktop z zachowaniem kompatybilności.

## Kryteria zamknięcia audytu

Każdy problem ma wskazany plik/komponent, test reprodukujący, poprawkę, wynik CI i kryterium odbioru. Funkcjonalny parytet platform ocenia się na podstawie rzeczywistych operacji, a nie samej obecności ekranu.
