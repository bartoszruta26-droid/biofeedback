# 💪 Analiza powtórzeń i serii

> 📘 Dokumentacja projektu • Język: polski • Nazwy techniczne, identyfikatory API i symbole kodu pozostają oryginalne.

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

## Analiza w Desktop i historia sesji

The desktop Training tab now exposes a dedicated analytics panel:
- peak-force trace across repetitions, separated by exercise;
- series-level table with mean and maximum peak force, average repetition duration, target achievement and peak-force coefficient of variation;
- tabela powtórzeń zawierająca siłę szczytową i średnią, czas trwania, czas do osiągnięcia maksimum oraz liczbę próbek.

Operator może jawnie zapisać podsumowanie sesji w historii lokalnej. Plik historii ma uprawnienia systemu plików ograniczone do właściciela. Po wybraniu pacjenta historia jest zapisywana w katalogu danych aplikacji przypisanym do tego pacjenta; w przeciwnym razie trafia do historii nieprzypisanej. Plik JSON zawiera podsumowanie sesji z wersją schematu oraz zagnieżdżone rekordy ćwiczeń, serii i powtórzeń (znaczniki czasu, czas trwania, czas do maksimum, siłę średnią/szczytową, liczbę próbek i punkty gry). Nie zapisuje imienia, nazwiska ani numeru PESEL pacjenta. Ten widżet nie szyfruje historii; należy skonfigurować mechanizmy szyfrowania aplikacji oraz odpowiednio chronić konto systemowe i kopie zapasowe.

Zakładka Trening tworzy wybraną grę: Pong, Flappy Bird, Mario Run albo grę sinusoidalną. Siła z czujnika jest przeliczana na wejście gry w zakresie 0–100 względem siły docelowej ćwiczenia. Punkty gry są oddzielone od liczby powtórzeń wyznaczanej z czujnika i prezentowane osobno w analizie sesji.

## Rekordy powiązane z pacjentem i granice prywatności

Kartoteki pacjentów są ponownie wczytywane z lokalnego katalogu danych aplikacji podczas uruchomienia i zapisywane przez DataManager po dodaniu lub edycji. Plik JSON pacjenta podlega ustawieniu szyfrowania aplikacji (`encryption.enabled`) i jest zapisywany z uprawnieniami ograniczonymi do właściciela. Nazwa katalogu wykorzystuje skrót SHA-256 zamiast jawnego numeru PESEL; jest to ukrycie/pseudonimizacja ścieżki, a nie anonimizacja.

Wybór pacjenta przełącza widok historii sesji Desktop na katalog danych tego pacjenta. Zmiana pacjenta jest zablokowana podczas aktywnej sesji treningowej, aby zapobiec przypadkowemu przypisaniu danych do niewłaściwej osoby. JSON historii sesji jest zapisywany wyłącznie na wyraźne żądanie; ma uprawnienia ograniczone do właściciela, ale widżet analityczny go nie szyfruje. Uprawnienia systemu plików ani skrócona nazwa katalogu nie zastępują szyfrowanych kopii zapasowych i właściwej kontroli dostępu.

## Kalibracja HX711 i poprawność ramek szeregowych

Zaawansowane firmware Arduino przesyła binarną ramkę surową chronioną CRC oraz rekord tekstowy zawierający skalibrowaną masę w gramach. Rekord tekstowy zawiera jawny znacznik `CAL:1` albo `CAL:0`. Ramka binarna może być poprawna jako surowe dane czujnika, a jednocześnie nie nadawać się do obliczeń siły lub ruchu. Pomiar, trening i gry Desktop wymagają `hasCalibratedValue=true`; diagnostyczny zapis RAW pozostaje oddzielny.

Firmware rozpoczyna pracę z `is_calibrated=false`. Kalibrację należy wykonać po tarowaniu, używając znanej masy odniesienia i komendy szeregowej `CAL:<known_weight_in_grams>` (np. `CAL:500` dla masy 500 g). W obecnym firmware znacznik kalibracji jest ulotny i trzeba go ponownie ustawić po resecie firmware lub zaniku zasilania. Pomyślny test regresyjny parsera obejmuje odpowiedzi binarne z tekstowymi, odpowiedzi wyłącznie tekstowe ze statusem kalibracji oraz jawnie niekalibrowane.

Interfejs przelicza skalibrowane gramy na niutony, używając standardowego przyspieszenia ziemskiego (9.80665 m/s²). Prezentacja w kgf jest wyłącznie konwersją wyświetlania; parametry ruchu pozostają zapisane w jednostkach SI. Programowa kontrola protokołu nie weryfikuje fizycznego czujnika tensometrycznego, geometrii mocowania, dokładności kalibracji, histerezy, liniowości ani powtarzalności. Wymagają one kontrolowanych obciążeń odniesienia i sprzętowych testów odbiorczych.
