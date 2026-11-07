# Plan rozwoju interfejsów Biofeedback

## Założenia
W development lokalne pliki konfiguracyjne i dane testowe mogą pozostawać nieszyfrowane. Priorytetem są funkcjonalność, ergonomia, spójność UX, stabilność i łatwość rozwoju. Nie jest to deklaracja gotowości produkcyjnej.

Wszystkie interfejsy korzystają z jednego kontraktu aplikacyjnego, jednego MeasurementEngine i wspólnego modelu sesji. Nie implementujemy algorytmów pomiarowych oddzielnie w każdym kliencie.

## Desktop GUI — Qt/C++
- Dashboard: urządzenie, pacjent, sesja, pomiar i status.
- Wykres na żywo: pauza, wznowienie, autoskalowanie, wybór zakresu.
- Karty: Pacjenci, Pomiary, Trening, Gry, Historia, Raporty, Urządzenia, Diagnostyka, Ustawienia.
- Kreator połączenia i kalibracji czujnika.
- Edytor protokołów treningowych i ćwiczeń.
- Eksport CSV/JSON i podgląd raportu.
- Układy responsywne, skróty klawiszowe, dostępność, motyw jasny/ciemny.
- Jawne stany: brak urządzenia, rozłączenie, brak próbki, błąd, pomiar, pauza.

## TUI — terminal
- Status urządzenia, sesji i pomiaru.
- Lista portów, połączenie i rozłączenie.
- Diagnostyka protokołu i odczyt próbek.
- Prowadzony kreator kalibracji.
- Start/pauza/wznowienie/stop.
- Statystyki, logi, ostatnie błędy i konfiguracja.
- Obsługa klawiatury, małych terminali i trybu bez koloru.
- Osobny tryb uruchamiania, np. biofeedback --tui.
- TUI korzysta z usług aplikacyjnych; nie otwiera portu szeregowego samodzielnie.

## WebUI
- Responsywny dashboard dla desktopu, tabletu i telefonu.
- Urządzenia, pacjenci, sesje, treningi, historia i raporty.
- Pomiar na żywo przez WebSocket.
- Ustawienia i diagnostyka.
- Wersjonowane lokalne API HTTP/JSON + WebSocket.
- Backend korzysta z tego samego MeasurementEngine i usług aplikacyjnych co Desktop.

## Android App GUI
- Natywny Android: Kotlin + Jetpack Compose.
- Dashboard urządzeń i połączenia.
- Pomiar na żywo i podstawowe metryki.
- Sterowanie sesją ze stanami i potwierdzeniem.
- Historia, raporty, ustawienia.
- Układy telefon/tablet.
- Klient wersjonowanego API i WebSocket; bez duplikowania logiki pomiarowej.

## Wspólna architektura
1. Sensor Adapter — wykrywanie, połączenie i transport.
2. Protocol Parser — dekodowanie ramek i walidacja.
3. MeasurementEngine — walidacja próbek, normalizacja i metryki.
4. Application Services — pacjenci, sesje, treningi, urządzenia, raporty.
5. Repository — lokalne pliki i dane rozwojowe.
6. API Gateway — HTTP/JSON + WebSocket.
7. Presentation Adapters — Qt, TUI, WebUI, Android.

Zasady: jedno źródło prawdy; jawny stan urządzenia i pomiaru; brak blokującego I/O w GUI; praca długotrwała w usługach/workerach; aktualizacje UI przez bezpieczne mechanizmy wątkowe; wspólne jednostki, nazwy i statusy; ekrany mają stany ładowania, puste, błędu, braku połączenia i sukcesu.

## Priorytety
### P0 — fundament Desktop
- MainWindow jest jedynym właścicielem zakładek Qt.
- Application przechowuje wyłącznie nieposiadające własności wskaźniki.
- Nie tworzyć drugiego QTabWidget ani nie zastępować centralnego widgetu.
- Lokalny development nie wymaga klucza szyfrowania.
- Pełna kompilacja GUI i uruchomienie bez fizycznego urządzenia.
- Test cyklu życia MainWindow i zakładek.

### P1 — funkcjonalny Desktop
- Spójna nawigacja i dashboard.
- Wykres, panel sterowania i wybór urządzenia.
- Tryb symulacji.
- Pełny przepływ pacjent → sesja → pomiar → zapis → raport.

### P2 — TUI
- Tryb terminalowy i integracja z usługami aplikacyjnymi.
- Testy w pseudoterminalu.

### P3 — API i WebUI
- Wersjonowane API HTTP i WebSocket.
- Responsywny frontend i testy kontraktowe.

### P4 — Android
- Kotlin/Compose, klient API, dashboard, live measurement, history i settings.
- Testy jednostkowe, UI i integracyjne.

## Kryteria akceptacji
- Wszystkie interfejsy pokazują te same wartości i jednostki tej samej sesji.
- Start/pauza/wznowienie/stop mają jednoznaczne przejścia stanów.
- Rozłączenie urządzenia nie zawiesza GUI ani nie generuje fikcyjnych próbek.
- Desktop uruchamia się w trybie symulacji bez sprzętu.
- TUI działa bez serwera graficznego.
- WebUI i Android korzystają z wersjonowanego API.
- Testy obejmują kontrakty, stany i błędy połączenia.
