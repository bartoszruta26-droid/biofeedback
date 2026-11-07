# Biofeedback TUI

## Cel
Terminalowy interfejs Biofeedback jest osobnym klientem prezentacyjnym C++17.
Nie zastępuje aplikacji Qt i nie implementuje odrębnej logiki pomiarowej.
Docelowo oba interfejsy korzystają z tych samych usług aplikacyjnych i MeasurementEngine.

## Moduły
- Pulpit — status aplikacji, urządzenia i sesji.
- Pomiary — podgląd pomiaru i parametrów sesji.
- Pacjenci — dostęp do wspólnego repozytorium kart.
- Trening — sesje i ćwiczenia.
- Urządzenia — porty szeregowe, czujniki i kalibracja.
- Diagnostyka — stan usług i komunikaty.
- Ustawienia — konfiguracja interfejsu i urządzeń.

## Sterowanie
- Strzałki góra/dół — wybór modułu.
- Home/End — pierwszy lub ostatni moduł.
- Q lub Esc — wyjście.

Tryby bez interfejsu curses:
- `biofeedback_tui --status` — tekstowy status aplikacji.
- `biofeedback_tui --list-ports` — lista wykrytych nazw portów; nie otwiera połączenia.
- `biofeedback_tui --version` i `--help` — informacje o programie.

## Zakres wersji 0.1
To szkielet nawigacyjny z jawnym statusem demonstracyjnym. Nie odczytuje
czujnika, nie tworzy fikcyjnych kart pacjentów i nie zapisuje danych klinicznych.
Integracja z usługami pomiarowymi będzie realizowana przez wspólną warstwę
aplikacyjną, bez duplikowania logiki w TUI.

## Budowanie
Zainstaluj libncurses-dev, a następnie:
    cmake -S . -B build
    cmake --build build --target biofeedback_tui
    ./build/biofeedback_tui

Jeśli ncurses nie jest dostępne, CMake pomija wyłącznie program TUI.
