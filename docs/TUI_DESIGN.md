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
- W module Trening: S — start/stop, P — pauza/wznowienie, + / - — cel siły, [ / ] — serie, , / . — powtórzenia.
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

## Training session controls and history

The training screen supports configuration before starting:
- exercise preset: E cycles through hand grip, wrist flexion, wrist extension and generic exercise;
- target force: plus/minus in 5 N increments;
- series count: [ and ];
- repetitions per series: comma and period;
- rest duration: semicolon decreases and apostrophe increases by 5 seconds (0–600 seconds).

After each completed series, the TUI pauses repetition detection during the configured rest interval. The rest countdown is suspended while the session is paused. Completed repetitions are retained in memory and appended to the local CSV session history when the operator stops the session, exits the application, or completes the plan. The file is stored under the user's Biofeedback XDG application data directory with owner-only permissions.

The Diagnostics screen reports valid and rejected sample counts, the last sample-quality classification, the last repetition's peak and mean force, duration and time-to-peak, and the history-file path. TUI session history is local and unencrypted; it does not include a patient identifier. It is not a substitute for the encrypted patient record workflow.

## HX711 calibration requirement

The TUI uses the shared measurement adapter with calibrated grams as its input. The advanced firmware reports whether its calibration is active. Raw binary frames without a confirmed `CAL:1` status are marked sensor-rejected by the physical-unit pipeline and cannot contribute to force-based repetitions. Calibrate with a known reference mass using `CAL:<grams>` after tare; the current firmware does not persist this flag through power cycles.
