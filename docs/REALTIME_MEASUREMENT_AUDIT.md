# Audyt toru pomiarowego czasu rzeczywistego

Data audytu: 2026-11-07  
Zakres: wykres, akwizycja HX711/port szeregowy, walidacja próbek, zegary, detekcja powtórzeń, obliczenia biomechaniczne, archiwizacja i prezentacja.

## 1. Przepływ danych

```text
HX711 / Arduino
  -> SerialCommunication callback
  -> MeasurementSerialAdapter
  -> MeasurementEngine (walidacja, jednostka kanoniczna, jakość, historia RAM)
  -> MeasurementTab callback (kolejkowany do wątku GUI)
  -> RepetitionDetector
  -> statystyki powtórzenia / serii
  -> ForcePlotWidget + tabela próbek + tabela statystyk
  -> eksport JSON / CSV
```

Ważne rozróżnienie: wykres zakładki MeasurementTab jest realizowany przez `ForcePlotWidget`; `gui::GraphWidget` nie jest obecnie używany przez tę zakładkę.

## 2. Zmiany wykonane w PR #199

- Usunięto synchroniczne komunikaty diagnostyczne wykonywane dla każdej próbki w ścieżce odbioru i aktualizacji wykresu.
- Zegar czasu sesji oparto na `QElapsedTimer`, aby korekta zegara systemowego nie powodowała cofania osi czasu i odrzucania kolejnych próbek.
- Ograniczono bufory robocze UI do 10 000 próbek; tabela nadal pokazuje ostatnie 100 próbek.
- Skala wykresu siły jest przeliczana z aktualnie widocznego okna, a nie z historycznego maksimum, które już opuściło okno.
- Odrzucane są wartości niefinitywne i próbki o czasie cofniętym względem poprzedniego punktu wykresu.
- Poprawiono `seriesSpeed`: liczba powtórzeń jest dzielona przez rzeczywisty czas od początku pierwszego do końca ostatniego powtórzenia. Poprzedni wzór odejmował czasy trwania dwóch pojedynczych powtórzeń, co nie wyznacza czasu serii.
- Dodano jawne czasy początku i końca powtórzenia do statystyk oraz JSON/CSV. Import CSV nadal obsługuje wcześniejszy układ 14 kolumn.
- Eksport JSON w wersji 2 obejmuje pełny strumień zarejestrowanych próbek (czas hosta, czas urządzenia, czas sesji, wartość, jednostka, surowy odczyt ADC, źródło i jakość). Można zapisać także sesję, w której nie zakończono jeszcze powtórzenia.
- Archiwum zachowuje także skalibrowaną masę w kg obok wartości siły w N i surowego ADC. Import JSON odtwarza próbki na wykresie i w tabeli, zachowuje tryb RAW/skalibrowany oraz kontynuuje oś czasu od ostatniego zapisanego znacznika czasu.

## 3. Integracja 65 parametrów — wdrożony etap

- Model `BiomechanicalParams` zawiera teraz 65 uporządkowanych nazw, jednostek i statusów dostępności.
- Statusy są odrębne od wartości liczbowej: `Available`, `InsufficientData`, `RequiresCalibration`, `RequiresAdditionalSensor`, `RequiresHistory`, `InvalidInput`. Wartość 0 może być prawidłowym wynikiem; brak danych nie jest już utożsamiany z zerem.
- `MeasurementTab` przekazuje rzeczywiste próbki z `RepetitionDetector` do `Formulas`, oblicza parametry powtórzenia i bieżącej serii oraz wyświetla tabelę 65 parametrów z jednostką i statusem. Tabela ma wybór serii, a wartości sztywności i masy są jawnie konfigurowane, zapisywane w sesji i ponownie stosowane przy imporcie; zmiana pacjenta czyści masę poprzedniej osoby.
- JSON zapisuje wyniki i statusy dla każdego powtórzenia i serii, a także wejściowe znaczniki czasu, siły i próbki powtórzenia, co pozwala odtworzyć i ponownie przeliczyć wyniki po imporcie. Wersja schematu 3 zapisuje również sztywność układu sprężystego i masę pacjenta; oba parametry mają domyślnie stan „brak kalibracji/danych” i mogą być jawnie ustawione w panelu.
- Naprawiono agregację mocy szczytowej: maksimum jest wybierane spośród mocy poszczególnych powtórzeń, a nie obliczane z maksimum siły z całej serii i próbek ostatniego powtórzenia.
- Oddzielono impuls siły [N·s] od pracy mechanicznej [J] w statystykach serii.
- Całkowanie impulsu wykorzystuje rzeczywiste odstępy między znacznikami czasu i metodę trapezów, z odrzucaniem nieprawidłowych segmentów oraz obsługą przepełnienia zegara urządzenia.
- Wszystkie podstawowe funkcje czasu (czas powtórzenia, faz, serii i przerw) odrzucają cofnięte/niejednoznaczne przedziały i poprawnie obsługują pojedyncze przepełnienie 32-bitowego zegara urządzenia.
- Pochodne, wskaźnik płynności i czas pod napięciem ignorują nieprawidłowe próbki oraz odstępy czasu cofnięte lub niejednoznaczne.
- Dodano test CTest `biomechanical_parameter_availability_regression`, obejmujący całkowanie przy nierównomiernym próbkowaniu, nieprawidłowe próbki, przepełnienie znacznika czasu, kompletność nazw/jednostek i statusy zależne od kalibracji/czujników/historii.

## 3. Ustalenia wymagające dalszej pracy

### 3.1 Trwałość i kompletność archiwum

- `MeasurementEngine` utrzymuje wyłącznie ograniczoną historię RAM (domyślnie 512 próbek); nie jest magazynem długoterminowym.
- Pełny strumień jest obecnie zachowywany w pamięci zakładki i trafia do pliku dopiero po ręcznym eksporcie JSON. Nagłe zamknięcie aplikacji przed eksportem nadal może utracić bieżącą sesję.
- Eksport CSV pozostaje eksportem tabeli statystyk powtórzeń, a nie pełnego szeregu czasowego. Pełny strumień jest w eksporcie JSON.
- Do pełnej odporności na awarię potrzebny jest trwały dziennik próbek zapisywany przyrostowo (np. plik JSONL/SQLite), z kontrolą integralności i możliwością wznowienia.
- Archiwum aktualnie zapisuje próbki przyjęte do ścieżki rejestracji. Ramki odrzucone przez sterownik/validator powinny otrzymać osobny rejestr jakości, aby zachować informację o przerwach i odrzuceniach.

### 3.2 Katalog 65 parametrów biomechanicznych

W repozytorium istnieją `BiomechanicalParams` i `Formulas::calculateAllParams()` z 65 polami. Nie oznacza to jednak, że 65 parametrów jest obecnie obliczanych, prezentowanych ani zapisywanych w zakładce MeasurementTab.

Aktualna tabela zakładki pokazuje 12 parametrów powtórzenia, a struktura serii przechowuje tylko wybrany zestaw statystyk. Nie ma jeszcze kompletnego połączenia `MeasurementTab -> SeriesData -> Formulas -> 65 parametrów -> UI/JSON/CSV`.

Przegląd przypisań w `Formulas::calculateAllParams()` wykazał również pola, których ta funkcja w ogóle nie wyznacza: `F_min`, `F_range`, `T_rep`, `T_rest_rep`, `T_rest_set`, `v_avg`, `eta`, `FI_inter`, `BPR`, `F_grand_mean`, `I_total`, `V_training`, `INT`, `D_training`, `SEI`, `FMC` i `ISI`. Część z nich wymaga danych z kolejnych powtórzeń/serii, część danych historycznych lub dodatkowych czujników. Wartości zerowe z konstruktora `BiomechanicalParams::reset()` nie są dowodem, że parametr wynosi zero.

Konstruktor `Formulas` ma domyślne założenia `k_elastic=100 N/m` i `patientMass=70 kg`. Zakładka pomiarowa nie przekazuje obecnie jawnie indywidualnej kalibracji taśmy i aktualnej masy pacjenta do tego silnika. Wyniki zależne od tych wartości nie mogą być traktowane jako spersonalizowane pomiary, dopóki konfiguracja nie zostanie jawnie podana i zapisana w sesji.

Dodatkowo część obecnego kodu `calculateAllParams()` podstawia wartości zastępcze zamiast danych pomiarowych:
- TRI używa historycznego maksimum ustawionego sztucznie na 110% bieżącego maksimum;
- FRI zakłada stały limit czujnika 500 N;
- BRC zakłada bazową siłę spoczynkową 10 N;
- LBI przyjmuje 1,0 bez pomiaru trzech serii;
- FG kopiuje ten sam indeks zmęczenia do wszystkich trzech składowych;
- TAI i LPT zwracają zero, gdy brakuje danych wieloseryjnych/historycznych.

Takie wartości nie mogą być prezentowane jako rzeczywiste wyniki pacjenta. Kolejny etap musi wprowadzić jawny status każdego parametru: obliczony, niewystarczające dane, brak kalibracji/konfiguracji albo niewłaściwy typ czujnika. Brak danych nie może być utożsamiany z wartością 0.

### 3.3 Parametry zależne od dodatkowych czujników lub konfiguracji

Nie wszystkie 65 wskaźników można wiarygodnie wyznaczyć z pojedynczego kanału HX711:
- praca, prędkość i moc wymagają poprawnego modelu mechanicznego oraz znanej charakterystyki sprężystości/odkształcenia;
- moc znormalizowana wymaga aktualnej masy pacjenta;
- sprawność mechaniczna wymaga wiarygodnego oszacowania energii metabolicznej;
- BPR wymaga pomiaru oddechu;
- PSE wymaga danych o postawie/równowadze, a nie tylko siły z jednego tensometru;
- TRI, TAI, LBI, FMC, ISI, FI_inter i LPT wymagają historii lub kompletu serii;
- FRI wymaga rzeczywistego, skalibrowanego limitu czujnika;
- BRC wymaga zmierzonej siły bazowej;
- przerwy między powtórzeniami i seriami wymagają wiarygodnych granic wszystkich powtórzeń/serii.

Parametry niewspierane przez dostępny zestaw czujników muszą być jawnie oznaczone jako niedostępne, a nie szacowane z arbitralnych stałych.

## 4. Wymagane testy odbiorcze

1. Strumień 80–100 Hz przez długi czas: brak nieograniczonego wzrostu buforów UI i brak blokowania wątku GUI.
2. Próbki o czasie rosnącym, równym, cofniętym i po przepełnieniu zegara urządzenia.
3. Zmiana zegara systemowego w trakcie pomiaru: oś czasu sesji nadal monotoniczna.
4. Wartości RAW i skalibrowane: poprawne jednostki i brak podwójnej konwersji g → kg → N.
5. Zapis/odczyt JSON: identyczna liczba próbek, kolejność, znaczniki czasu, wartości, jednostki i statystyki.
6. Eksport CSV: jednoznaczny schemat, zgodność wsteczna i poprawne kodowanie pól.
7. Wszystkie 65 parametrów: test wzorcowych danych, status dostępności, jednostka, wzór i źródło wejściowe.
8. Porównanie historyczne: tylko parametry o zgodnych jednostkach, kalibracji, protokole i konfiguracji urządzenia.

## 5. Status

PR #199 zawiera pierwszą serię korekt w ścieżce czasu rzeczywistego, liczeniu kadencji oraz eksporcie próbek JSON. Pełna implementacja i walidacja wszystkich 65 parametrów, automatyczny dziennik odporny na awarie oraz kompletna tabela wyników pozostają dalszym zakresem prac.
