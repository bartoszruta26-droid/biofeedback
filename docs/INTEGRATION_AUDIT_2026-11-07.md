# Audyt integracyjny Biofeedback — 2026-11-07

## Zakres

Audyt wykonano na gałęzi `integration/all-work`, po scaleniu PR #199 (commit `af8b37379b7e46296fd8dc33d604dbe24cca1a1a`). Sprawdzono granice pomiędzy:

- `PatientTab`, `MeasurementTab` i `TrainingTab`,
- `SerialCommunication`, `MeasurementSerialAdapter` i `MeasurementEngine`,
- normalizacją jednostek i obliczeniami biomechanicznymi,
- `RepetitionDetector`, zegarem sesji i grami,
- GUI, TUI, lokalnym HTTP API, testami i workflow GitHub Actions.

## Przepływ danych pomiarowych

W aplikacji desktopowej `MainWindow` współdzieli jeden obiekt `SerialCommunication`. `MeasurementTab` tworzy wspólny `MeasurementEngine` i adapter HX711; `MainWindow` przekazuje ten sam silnik do `TrainingTab`. Adapter normalizuje firmware'owe gramy do kilogramów, a moduły pomiaru i treningu przeliczają wartość masową na niutony dokładnie na granicy obliczeń siły.

## Usterki wykryte i poprawione w PR #200

### 1. Brak przekazania kontekstu pacjenta do MeasurementTab

`PatientTab` emitował `currentPatientChanged`, ale `MainWindow` łączył ten sygnał tylko z `TrainingTab`. `MeasurementTab::onPatientChanged` nie było odbiorcą, więc identyfikator pacjenta w module pomiarowym mógł pozostać pusty, a zapis trafiał do katalogu `unassigned`.

**Poprawka:** dodano połączenie sygnału z `MeasurementTab::onPatientChanged`. Zmiana pacjenta przechodzi teraz przez istniejący mechanizm zamknięcia dziennika, resetu sesji, ustawienia identyfikatora i wczytania historii.

### 2. Timestamp 0 błędnie traktowany jako brak czasu

Zegar urządzenia ma 32 bity i przepełnia się. Wartość 0 jest więc prawidłowym timestampem po przepełnieniu. `MeasurementEngine` odrzucał ją jako `MissingTimestamp`; dodatkowo `MeasurementTab` resetował detektor powtórzeń dla tej wartości. Detektor treningowy już korzystał z arytmetyki bez znaku, która obsługuje przepełnienie.

**Poprawka:** dodano jawne pole `hasTimestamp` do `SensorData` i `MeasurementInput`. Parser oznacza obecność pola czasu w poprawnych ramkach binarnych i tekstowych, adapter zachowuje tę informację, a silnik odróżnia brak timestampu od prawidłowego zera. Usunięto również sentinel `deviceTimestampMs == 0` z toru detekcji powtórzeń w `MeasurementTab`.

## Weryfikacja

Na commicie PR #200 `ae432c2e512fd3cc547ef0227294f0a046c9002a` wszystkie 6 przebiegów CI zakończyło się powodzeniem:

- serial-diagnostics-policy,
- serial-command-validation,
- serial-read-policy,
- serial-write-policy,
- security-regression,
- Linux desktop GUI build.

Linux CI wykonało pełną kompilację aplikacji Qt, CTest, kontrolę zależności, budowę pakietu Debian i test instalacji lokalnej. Security CI wykonało testy regresyjne, kompilację targetów, test lokalnego HTTP API, tryby TUI, kontrolę składni WebUI i CTest.

## Otwarte ryzyka / dalsze prace

1. **Cykl życia callbacków — poprawione w aktualizacji PR #200:** `MeasurementEngine` przechowuje współdzielone wpisy subskrypcji, liczy callbacki będące w toku i `unsubscribe()` czeka na ich zakończenie. Dzięki temu destruktor `MeasurementTab` i `TrainingTab` nie wraca z rejestracji, gdy callback przechwytujący widget nadal działa. Dodano regresję wielowątkową: callback blokuje się, odsubskrybowanie musi poczekać, po czym nowe próbki nie uruchamiają już callbacku.
2. **HTTP API:** obecny lokalny serwer API udostępnia zdrowie/status, natomiast operacje urządzenia i pomiaru pozostają jawnie wyłączone (`CAPABILITY_NOT_AVAILABLE`). To świadome ograniczenie granicy usługi, a nie potwierdzona integracja API z silnikiem pomiarowym.
3. **Walidacja sprzętowa:** CI potwierdza zachowanie programowe i testy regresyjne; nie zastępuje pomiaru na rzeczywistym HX711/Arduino, testu długiego czasu pracy ani walidacji klinicznej.

## Status

PR #200 zawiera poprawki routingu pacjenta, semantyki zawijania timestampu oraz synchronizacji cyklu życia callbacków, wraz z regresjami. CI zweryfikowano ponownie na aktualnym HEAD PR po tych zmianach. PR pozostaje skierowany do `integration/all-work`; `main` nie jest celem integracji.


## Audyt silnika gier — PR #202

Audyt kodu wykonano na gałęzi roboczej PR #202 względem integration/all-work. W aktualnej konfiguracji CMake aplikacja desktopowa kompiluje cztery gry: SinGame, PongGame, FlappyBirdGame i MarioGame. Pozostałe nazwy wymienione w PROJECT_STRUCTURE.md (Arkanoid, Galaga, Archer, Scored Earth, Car Race) nie mają obecnie odpowiadających im źródeł w aktywnym zestawie kompilacji.

### GameEngine

Wykryto i poprawiono:
- użycie zegara ściennego do pomiaru czasu klatki — zastąpiono go monotonicznym QElapsedTimer;
- brak jawnej obsługi wywołań wejścia z wątku komunikacji — wejście jest kolejkowane do wątku widgetu;
- brak odrzucania wartości niebędących liczbami — NaN i nieskończoności są odrzucane i zliczane;
- brak implementacji getGameStats() i resetGameStats() — dodano wynik, stan, czas, liczbę klatek i liczbę nieprawidłowych wejść;
- możliwość przepełnienia arytmetyki wyniku — dodawanie punktów jest nasycane do zakresu int;
- niejednoznaczną semantykę start() dla sesji w pauzie — start() nie resetuje już trwającej sesji, a resume() pozostaje odrębną operacją.

### SinGame

- Prędkość wykresu była odejmowana bezpośrednio od fazy, mimo że parametr był opisany w pikselach na sekundę. Zastosowano przeliczenie przez częstotliwość przestrzenną sinusa.
- Wprowadzono histerezę strefy trafienia, aby szum i drżenie wokół granicy tolerancji nie generowały wielokrotnych punktów.
- Punktacja za pojedyncze wejście do strefy jest ograniczona do 1–10 punktów.
- Usunięto emisję pozornego scoreChanged(0) przy samej zmianie poziomu trudności.

### PongGame

- Fizykę podzielono na podkroki o maksymalnym czasie 1/120 s, aby ograniczyć przenikanie piłki przez paletki przy spadku liczby klatek.
- Ograniczono pozycję paletki do poprawnego zakresu również dla małych wysokości widgetu.

### FlappyBirdGame

- Losowanie położenia luki wykonywało się przed wylosowaniem jej rzeczywistej wysokości. Przy większych lukach ich krawędzie mogły wychodzić poza ekran. Teraz najpierw losowany jest rozmiar, a następnie środek z uwzględnieniem promienia ptaka.
- Po zakończeniu gry w aktualizacji ptaka pomijane są dalsze kroki tej samej klatki.
- Setter prędkości gry ignoruje wartości niebędące liczbami.
- Utrzymano ujemny znak siły trzepotania skrzydeł, ponieważ oś Y rośnie w dół ekranu.

### MarioGame

- Generator przeszkód zwiększał czas stałą 0,016 s na klatkę; teraz otrzymuje rzeczywiste deltaTime.
- Ładowanie skoku było mierzone zegarem ściennym, a stan ładowania zerowano w każdej klatce, gdy postać stała na ziemi. Ładowanie jest teraz akumulowane czasem symulacji i zachowuje stan podczas przytrzymania.
- Czas ładowania nie rośnie podczas pauzy.
- Dziura w podłożu była rysowana przed pełnym prostokątem ziemi, który ją zasłaniał. Otwory są rysowane ponownie na wierzchu podłoża.
- Postać może opaść przez lukę; zakończenie następuje po przekroczeniu dolnej granicy ekranu.
- Animacja wzgórz używa czasu symulacji zamiast zegara ściennego.

### Testy i ograniczenia

Dodano game_engine_regression dla cyklu życia, pauzy/wznowienia, wejścia z obcego wątku, odrzucania NaN, statystyk i nasycania wyniku. Pierwszy przebieg CI ujawnił brak src/core/DebugManager.cpp w linkowaniu nowego targetu testowego; dodano brakujące źródło. Kolejny przebieg na aktualnym HEAD jest wymagany przed scaleniem.

Testy automatyczne nie zastępują ręcznej walidacji rozgrywki, testu z rzeczywistym tensometrem ani oceny klinicznej. W szczególności trzeba jeszcze przeprowadzić testy scenariuszowe każdej gry, różnych rozmiarów okna, przerw w próbkowaniu i granic kalibracji.
