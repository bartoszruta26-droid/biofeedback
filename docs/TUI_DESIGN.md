# ⌨️ Biofeedback — projekt i stan interfejsu terminalowego TUI

> 📘 Weryfikacja kodu: 2026-11-09 • Zakres: `src/tui/main.cpp`, `src/tui/TuiModel.cpp`, `MeasurementSerialAdapter`, `HttpApiClient`  
> Status: działający klient terminalowy z ograniczonym zakresem funkcji; bez pełnego parytetu z Desktop.

## 1. Cel i granice odpowiedzialności

TUI jest osobnym klientem C++17 opartym na ncurses. Loguje się do wspólnego API w celu odczytu kart pacjentów i zapisu archiwów, ale obsługuje lokalny port szeregowy bezpośrednio przez `SerialCommunication` i `MeasurementSerialAdapter`. Nie korzysta obecnie z serwerowej `AcquisitionService` do pozyskiwania próbek.

## 2. Moduły

- **Pulpit** — stan połączenia i ostatnia próbka.
- **Pomiary** — podgląd próbek z lokalnego adaptera; próbki są buforowane do archiwizacji.
- **Pacjenci** — lista pobrana z API i wybór pacjenta do przypisania archiwum.
- **Trening** — lokalna detekcja powtórzeń, serie, cel siły, pauza i odpoczynek.
- **Urządzenia** — skanowanie portów, wybór, połączenie i rozłączenie lokalnego portu szeregowego.
- **Diagnostyka** — liczby próbek poprawnych/odrzuconych i parametry ostatniego powtórzenia.
- **Ustawienia** — skróty i stała domyślna prędkość transmisji.

## 3. Przepływ danych i archiwizacja

1. Po uruchomieniu TUI loguje użytkownika przez API i pobiera listę kart z kolekcji `patients`.
2. W module Pacjenci operator wybiera kartę strzałkami lewo/prawo. Wybór jest opcjonalny; interfejs jawnie dopuszcza archiwizację bez przypisania pacjenta.
3. Port szeregowy jest otwierany lokalnie przez `SerialCommunication`. Adapter przekazuje próbki do lokalnej instancji `MeasurementEngine`.
4. Próbki w jednostce skalibrowanej są normalizowane z gramów do kilogramów, a następnie siła do obliczeń treningowych jest wyliczana jako masa × 9,80665 m/s².
5. Archiwum pomiarowe zawiera próbki i jest wysyłane do kolekcji `measurements` przy rozłączeniu urządzenia lub zamknięciu TUI. Bufor sesji jest ograniczony do 100 000 próbek.
6. Archiwum treningowe jest wysyłane do `training-sessions`, a podsumowanie do `training-results`. Zapis zawiera `patient_id`, jeśli operator wybrał pacjenta; w przeciwnym razie pole jest puste.
7. Nieudany zapis pomiaru przywraca próbki do bufora w pamięci. Nie jest to trwała kolejka offline: awaria procesu przed ponowną próbą może utracić niezapisane dane. Nieudany zapis treningu jest komunikowany w statusie, ale nie ma automatycznego mechanizmu ponawiania po restarcie.

## 4. Sterowanie

- Strzałki góra/dół — wybór modułu.
- Enter — otwarcie wybranego modułu.
- W module Pacjenci: strzałki lewo/prawo — wybór pacjenta.
- W module Urządzenia: strzałki lewo/prawo — wybór portu; C — połączenie; D — rozłączenie; R — ponowne skanowanie.
- W module Trening: S — start/stop; P — pauza/wznowienie; E — wybór zestawu ćwiczeń; + / - — cel siły; [ / ] — liczba serii; , / . — liczba powtórzeń; ; / ' — czas odpoczynku.
- Q lub Esc — wyjście.

Domyślny plan treningowy to 3 serie po 8 powtórzeń, cel 50 N i odpoczynek 60 sekund. Zakresy regulacji wynikają z implementacji w `src/tui/main.cpp`.

## 5. Ograniczenia

- TUI nie tworzy ani nie edytuje kart pacjentów.
- TUI nie implementuje silnika gier ani nie zapisuje wyników gier.
- Wybór pacjenta nie jest obowiązkowy, więc operator może zapisać archiwum bez przypisania do osoby.
- Połączenie sprzętowe jest lokalne; TUI nie uruchamia ani nie zatrzymuje serwerowej sesji akwizycji.
- Bufor pomiarów i historia ostatnich próbek są przechowywane w pamięci procesu; archiwizacja następuje dopiero przy rozłączeniu lub wyjściu.
- Kalibracja nie jest wykonywana z poziomu opisanego TUI; tor wymaga poprawnie skalibrowanych danych firmware oznaczonych `CAL:1`.

## 6. Tryby bez interfejsu curses

- `biofeedback_tui --status` — status procesu TUI.
- `biofeedback_tui --list-ports` — lista portów wykrytych przez adapter systemowy.
- `biofeedback_tui --version` i `--help` — informacje o programie.

## 7. Budowanie

Zainstaluj `libncurses-dev`, a następnie:

```bash
cmake -S . -B build
cmake --build build --target biofeedback_tui
./build/biofeedback_tui
```

Jeśli ncurses nie jest dostępne, CMake pomija wyłącznie program TUI.

## 8. Weryfikacja

W repozytorium znajdują się testy `tui_model_regression`, `measurement_engine_regression`, `repetition_detector_regression`, `training_session_clock_regression` oraz testy integracji akwizycji i protokołu. Testy automatyczne nie potwierdzają poprawności kalibracji fizycznego czujnika ani trwałości niezapisanych buforów TUI.
