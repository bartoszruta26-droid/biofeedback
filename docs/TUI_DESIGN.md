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
- C — połączenie z wybranym portem przy domyślnej prędkości.
- D — rozłączenie.
- R — ponowne skanowanie portów.
- Q lub Esc — wyjście.

## Tryby bez interfejsu curses

- biofeedback_tui --status — status procesu TUI.
- biofeedback_tui --list-ports — lista portów wykrytych przez adapter systemowy.
- biofeedback_tui --version i --help — informacje o programie.

## Granice funkcjonalne wersji 0.2

TUI może otworzyć port szeregowy, uruchomić wspólny adapter odczytu i
wyświetlać ostatnią poprawną próbkę. Nie tworzy kart pacjentów, nie zapisuje
danych klinicznych i nie uruchamia jeszcze sesji treningowych. Wspólne
repozytorium pacjentów oraz usługi sesji będą integrowane w kolejnych etapach.

## Budowanie

Zainstaluj libncurses-dev, a następnie:

    cmake -S . -B build
    cmake --build build --target biofeedback_tui
    ./build/biofeedback_tui

Jeśli ncurses nie jest dostępne, CMake pomija wyłącznie program TUI.
