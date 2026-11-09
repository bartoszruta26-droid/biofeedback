# ⌨️ Biofeedback — projekt interfejsu terminalowego TUI

> 📘 Dokumentacja projektu • Język: polski • Nazwy techniczne, identyfikatory API i symbole kodu pozostają oryginalne.

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

## Sterowanie sesją treningową i historia

Ekran treningu umożliwia konfigurację przed rozpoczęciem sesji:
- zestaw ćwiczeń: klawisz E przełącza chwyt dłoni, zgięcie nadgarstka, wyprost nadgarstka i ćwiczenie ogólne;
- target force: plus/minus in 5 N increments;
- series count: [ and ];
- liczba powtórzeń w serii: klawisze przecinka i kropki;
- czas odpoczynku: średnik zmniejsza, a apostrof zwiększa wartość o 5 sekund (zakres 0–600 sekund).

Po zakończeniu każdej serii TUI wstrzymuje wykrywanie powtórzeń na skonfigurowany czas odpoczynku. Odliczanie odpoczynku jest zawieszone podczas pauzy sesji. Zakończone powtórzenia pozostają w pamięci i są wysyłane jako jeden zasób `training-sessions` do uwierzytelnionego wspólnego API, gdy operator zatrzyma sesję, zamknie aplikację lub zrealizuje cały plan. Rekord obejmuje ćwiczenie, cel, metadane serii i powtórzeń, statystyki siły oraz liczbę próbek dla każdego powtórzenia. Nieudane wysłanie do API jest widoczne w stanie treningu; TUI nie zapisuje po cichu danych zastępczo do oddzielnego lokalnego pliku historii.

Ekran Pacjenci odczytuje przez API wspólną kolekcję `patients` i wyświetla rekordy zwrócone przez serwer. Ekran Diagnostyka prezentuje liczbę próbek poprawnych i odrzuconych, ostatnią klasyfikację jakości próbki oraz siłę szczytową i średnią, czas trwania i czas do maksimum ostatniego powtórzenia. W opisywanej wersji TUI nie wybiera jeszcze pacjenta, dlatego nie dołącza `patient_id` do sesji treningowej. Powiązanie pacjenta z sesją i pełna historia wyników treningowych pozostają kolejnymi etapami integracji.

## Wymagania kalibracji HX711

TUI korzysta ze wspólnego adaptera pomiarowego, którego wejściem są skalibrowane gramy. Zaawansowane firmware informuje, czy kalibracja jest aktywna. Surowe ramki binarne bez potwierdzonego statusu `CAL:1` są oznaczane przez tor jednostek fizycznych jako odrzucone przez czujnik i nie mogą uczestniczyć w wykrywaniu powtórzeń opartych na sile. Po tarowaniu wykonaj kalibrację znaną masą za pomocą `CAL:<grams>`; obecne firmware nie zachowuje tego znacznika po zaniku zasilania.
