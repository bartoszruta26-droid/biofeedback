# Biofeedback TUI

## Cel

Terminalowy interfejs Biofeedback jest osobnym klientem prezentacyjnym C++17.
Korzysta z tego samego źródła komunikacji szeregowej co aplikacja Desktop Qt;
nie tworzy odrębnego parsera danych czujnika.

## Moduły

- Pulpit — status aplikacji, połączenia i ostatniej próbki.
- Pomiary — podgląd wartości z aktywnego strumienia czujnika.
- Pacjenci — informacja o stanie integracji wspólnego repozytorium kart.
- Trening — status sesji treningowej.
- Urządzenia — wykrywanie portów, wybór portu i połączenie/rozłączenie.
- Diagnostyka — stan adaptera i interfejsu.
- Ustawienia — skróty sterowania i parametry transmisji.

## Sterowanie

- Strzałki góra/dół — wybór modułu.
- Enter — otwarcie wybranego modułu.
- W module Urządzenia: strzałki lewo/prawo — wybór portu.
- W module Trening: S — start/stop, P — pauza/wznowienie, + / - — cel siły.
- C — połączenie z wybranym portem przy domyślnej prędkości.
- D — rozłączenie.
- R — ponowne skanowanie portów.
- Q lub Esc — wyjście.

## Tryby bez interfejsu curses

- biofeedback_tui --status — status procesu TUI.
- biofeedback_tui --list-ports — lista portów wykrytych przez adapter systemowy.
- biofeedback_tui --version i --help — informacje o programie.

## Granice funkcjonalne wersji 0.2

TUI otwiera port szeregowy przez wspólny adapter i wyświetla próbki z HX711.
Moduł Pomiary pokazuje siłę w N oraz krótki przebieg ostatnich próbek. Moduł
Trening oferuje sesję 3 x 8 z detekcją powtórzeń opartą na progach aktywacji i
zwolnienia, podglądem serii oraz regulowanym celem siły. Sterowanie: S — start/
stop, P — pauza/wznowienie, + / - — zmiana celu. Dane są przetwarzane w pamięci;
TUI nadal nie tworzy kart pacjentów ani nie zapisuje dokumentacji klinicznej.
Przed treningiem urządzenie musi być podłączone i skalibrowane, a jednostka
kalibracji firmware musi odpowiadać założeniu gramów równoważnych.

## Budowanie

Zainstaluj libncurses-dev, a następnie:

    cmake -S . -B build
    cmake --build build --target biofeedback_tui
    ./build/biofeedback_tui

Jeśli ncurses nie jest dostępne, CMake pomija wyłącznie program TUI.
