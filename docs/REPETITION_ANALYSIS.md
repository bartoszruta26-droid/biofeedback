# Analiza powtórzeń i serii

## Zakres

Moduł biofeedback::training::RepetitionDetector segmentuje strumień siły na pojedyncze powtórzenia, a następnie wylicza statystyki serii. Algorytm działa niezależnie od Qt, dzięki czemu jest testowany jako biblioteka C++17.

## Jednostki wejściowe

Firmware HX711 w obecnym protokole raportuje skalibrowaną wartość masy równoważnej w gramach. Zakładka treningu przelicza ją na siłę:

F[N] = m[g] × 9,80665 / 1000

Wartości celu w planie treningowym są wyrażone w niutonach. Przed użyciem sprzętu trzeba potwierdzić, że współczynnik kalibracji firmware rzeczywiście zwraca gramy równoważne. Nie wolno traktować surowych zliczeń ADC jako niutonów.

## Segmentacja powtórzenia

1. Początek: siła osiąga próg aktywacji.
2. Rejestracja: próbki aktualizują maksimum, czas do maksimum, całkę siła-czas i licznik próbek.
3. Koniec: siła spada poniżej progu zwolnienia.
4. Walidacja: zdarzenie jest akceptowane, jeśli trwa co najmniej 250 ms.
5. Odrzucenie: niepoprawna próbka, powtórzony znacznik czasu, przerwa między próbkami powyżej 500 ms lub czas trwania powyżej 30 s zeruje niekompletne zdarzenie.

Domyślne progi z histerezą:

- aktywacja: max(2 N, 20% siły docelowej);
- zwolnienie: 50% progu aktywacji (czyli zwykle 10% celu).

Histereza zapobiega wielokrotnemu zliczaniu jednego ruchu w pobliżu progu. Przerwa w strumieniu powoduje odrzucenie niekompletnego powtórzenia; detektor czeka na nową krawędź aktywacji.

## Parametry pojedynczego powtórzenia

| Parametr | Definicja |
|---|---|
| Siła szczytowa | Maksymalna siła w zarejestrowanym powtórzeniu |
| Siła średnia | Całka trapezowa siły po czasie / czas powtórzenia |
| Czas trwania | Od przekroczenia progu aktywacji do zejścia poniżej progu zwolnienia |
| Czas do szczytu | Czas od początku powtórzenia do próbki z siłą maksymalną |
| Liczba próbek | Liczba poprawnych próbek wykorzystanych w zdarzeniu |
| Osiągnięcie celu | Czy siła szczytowa osiągnęła lub przekroczyła cel ćwiczenia |

## Parametry serii

Po zarejestrowaniu zaplanowanej liczby powtórzeń seria zostaje zamknięta. Obliczane są:

- liczba powtórzeń;
- średnia i maksymalna siła szczytowa;
- średni czas trwania;
- średni czas do szczytu;
- odsetek powtórzeń osiągających cel;
- współczynnik zmienności siły szczytowej (CV = odchylenie standardowe / średnia × 100%).

Wartość consistencyScore w aplikacji jest roboczym wskaźnikiem 0–100 równym max(0, 100 − CV%). Nie jest to zwalidowana skala kliniczna.

## Przejścia treningu

- Powtórzenie kończy się po detekcji zwolnienia.
- Po wymaganej liczbie powtórzeń zamykana jest seria i emitowany jest sygnał seriesCompleted.
- Po wymaganej liczbie serii zamykane jest ćwiczenie.
- Między seriami obowiązuje odpoczynek skonfigurowany dla ćwiczenia; między ćwiczeniami stosowane jest obecnie 120 s.
- Wyniki są zachowywane w strukturze ExerciseStats i eksportowane do CSV w układzie jeden wiersz na powtórzenie.

## Ograniczenia i odbiór sprzętowy

Algorytm rozpoznaje cykl narastania i zwolnienia siły. Nie rozdziela jeszcze fazy koncentrycznej i ekscentrycznej, nie ocenia toru ruchu ani jakości techniki ćwiczenia. Ruch izometryczny bez zejścia poniżej progu zwolnienia nie zostanie policzony jako powtórzenie. Próg aktywacji jest na razie domyślny i powinien być weryfikowany dla rodzaju ćwiczenia oraz czujnika.

Testy syntetyczne obejmują histerezę, minimalny czas, niepoprawne próbki, utratę próbek, przepełnienie 32-bitowego zegara urządzenia i statystyki serii. Ostateczny odbiór wymaga porównania z obserwacją rzeczywistych powtórzeń oraz wzorcem siły po kalibracji HX711. Oprogramowanie nie jest samo w sobie wyrobem medycznym ani zwalidowanym narzędziem diagnostycznym.

## Desktop analysis and session history

The desktop Training tab now exposes a dedicated analytics panel:
- peak-force trace across repetitions, separated by exercise;
- series-level table with mean and maximum peak force, average repetition duration, target achievement and peak-force coefficient of variation;
- repetition-level table with peak/mean force, duration, time-to-peak and sample count.

The operator can explicitly save a session summary to the local history. The history file is stored with owner-only filesystem permissions. When a patient is selected, the history file is stored in that patient's application data directory; otherwise it uses the unassigned application history. The history JSON contains aggregate training metrics and does not embed the patient's name or PESEL. It is not encrypted by this widget; configure the application's existing encryption facilities and protect the operating-system account and backups accordingly.

The Training tab instantiates the selected Pong, Flappy Bird, Mario Run or sinusoidal game. Sensor force is mapped to a 0–100 game input relative to the exercise target force. Game points are kept separate from sensor-derived repetition counts and are displayed in the session analytics.
