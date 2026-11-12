# Audyt i plan rozwoju — płynność gier oraz automatyzacja konspektów

Data audytu: 2026-11-12  
Repozytorium: `bartoszruta26-droid/biofeedback`  
Gałąź bazowa audytu: `fix/smooth-game-motion`  
Zakres: Linux desktop / Qt, 15 gier biofeedback, konspekty i proces rehabilitacji.

## 1. Streszczenie wykonawcze

Repozytorium ma już istotne fundamenty: symulację stałokrokową 120 Hz, interpolację renderowania w grach, centralny strumień danych tensometru, trwały magazyn danych rehabilitacyjnych, oceny, cele, obserwacje ćwiczeń, sesje, reguły progresji i eksport raportu PDF.

Najważniejsze ryzyko w obszarze gier polega na tym, że stała częstotliwość symulacji i obecność funkcji `renderInterpolated` nie są dowodem rzeczywistej płynności prezentacji. Brakuje pełnej telemetrii czasu klatki i porównywalnych testów trajektorii, kolizji oraz jakości prezentacji. Należy mierzyć zachowanie aplikacji, a nie zakładać je na podstawie częstotliwości timera.

Największa luka w konspektach polega na rozdzieleniu planu, wykonania i dokumentacji klinicznej. Istnieją obserwacje wykonanych ćwiczeń, ale potrzebna jest kompletna, wersjonowana recepta ćwiczenia oraz koordynator procesu, który bezpiecznie prowadzi sesję od kontroli stanowiska po raport i kontrolę po 24 godzinach.

## 2. Audyt silnika i jakości obrazu gier

### 2.1. Stan istniejący

- `GameEngine` realizuje symulację ze stałym krokiem `1/120 s`.
- Pętla używa monotonicznego zegara nanosekundowego, timera Qt o interwale 8 ms, akumulatora czasu i ograniczonego nadrabiania zaległości.
- Silnik udostępnia współczynnik interpolacji stanu renderowania.
- Katalog 15 gier ma implementacje `renderInterpolated`.
- Wejście z wątku odczytu czujnika jest scalane do najnowszej próbki przed obsługą w GUI. Strumień próbek dla detektora powtórzeń pozostaje odrębny.
- Istnieją testy cyklu życia silnika, fabryki gier, zmian rozdzielczości, wybranych kolizji i zachowania wejścia.
- Wcześniejsze audyty opisują poprawki skalowania i ruchu, ale wyraźnie zaznaczają, że test zmiany rozmiaru nie dowodzi identycznej trajektorii fizycznej.

### 2.2. Luki i ryzyka

1. Brak rozdzielenia w statystykach liczby kroków symulacji od liczby faktycznie wykonanych zdarzeń prezentacji.
2. Brak pomiaru czasu renderowania i czasu aktualizacji fizyki.
3. Brak liczników maksymalnego czasu klatki, opóźnień pętli oraz odrzuconych kroków symulacji.
4. Brak jednolitego zestawu testów trajektorii i kolizji dla wszystkich gier i rozdzielczości.
5. Interpolacja pozycji nie rozwiązuje automatycznie problemów kątów, faz, animacji postaci, kamery, cząsteczek ani skoków po zmianie układu współrzędnych.
6. Qt Widgets i kompozytor systemu mogą scalać żądania odświeżenia. 120 Hz symulacji nie oznacza 120 FPS na ekranie.
7. Wygładzanie nie może zwiększać opóźnienia wejścia tak bardzo, by pogarszać sterowanie tensometrem.

### 2.3. Zalecenia

**P1 — telemetria wydajności (pierwszy etap implementacji)**

- Osobno liczyć kroki symulacji, wywołania renderowania i przebiegi pętli timera.
- Mierzyć łączny i maksymalny czas aktualizacji fizyki oraz renderowania.
- Rejestrować liczbę kroków utraconych przy ograniczeniu nadrabiania zaległości.
- Udostępnić statystyki w formie czytelnej dla testów oraz późniejszego panelu diagnostycznego.
- Nie nazywać liczby kroków symulacji liczbą FPS.

**P2 — profil jakości obrazu**

- Ujednolicić skalowanie logicznego pola gry i typografii.
- Zapewnić proporcjonalne wymiary obiektów i spójne marginesy.
- Rozdzielić ustawienia antyaliasingu, jakości efektów i skali interfejsu.
- Utrzymać tryb niskiego kosztu renderowania dla słabszych urządzeń.
- Dodać bezpieczne zachowanie po zmianie rozdzielczości i proporcji okna.

**P3 — walidacja fizyki i prezentacji**

- Testować 640×480, 1280×720, 1920×1080 i 2560×1440.
- Porównywać pozycję, prędkość i czas przelotu obiektów w tej samej symulacji.
- Testować szybkie obiekty, cienkie przeszkody, krawędzie i narożniki.
- Wykonywać deterministyczne odtwarzanie przebiegów wejścia czujnika.
- Testować pauzę, wznowienie, restart, utratę sygnału i zmianę rozmiaru podczas rozgrywki.
- Raportować oddzielnie wynik symulacji, czas renderowania i obserwowaną płynność prezentacji.

**P4 — optymalizacja renderera**

- Najpierw identyfikować wąskie gardła pomiarami.
- Ograniczać kosztowne alokacje i obliczenia wykonywane w każdej klatce.
- Rozważyć akcelerowany renderer dla gier, w których profilowanie potwierdzi ograniczenia Qt Widgets.
- Nie przenosić całego katalogu do OpenGL bez benchmarku porównawczego.

## 3. Audyt konspektów i procesu rehabilitacji

### 3.1. Stan istniejący

- `OutlineTab` ładuje konspekty JSON, pokazuje ćwiczenia, materiały i przypisane gry.
- Sesja konspektu może przechodzić między ćwiczeniami i uruchamiać gry.
- Repozytorium danych obsługuje epizody, oceny, cele, sesje, obserwacje i decyzje rehabilitacyjne.
- Dane są wiązane z UUID pacjenta i epizodu, a repozytorium kontroluje relacje oraz wersjonowanie.
- Można zapisywać ból przed sesją, po sesji i po 24 godzinach.
- Można rejestrować wykonane serie, powtórzenia, czas, RPE, ból i jakość ruchu.
- Istnieją reguły progresji zwracające rekomendację, a nie samodzielnie zmieniające zalecenia.
- Dostępny jest raport PDF i powiązanie oceny rehabilitacyjnej z wersją oceny ICF.

### 3.2. Luki i ryzyka

1. Plik JSON konspektu jest wczytywany do struktur, ale brakuje pełnego edytora z walidacją, wersjonowaniem, historią i zatwierdzaniem.
2. Plan ćwiczenia nie jest kompletną receptą: należy dodać stronę ciała, dawkę, tempo, obciążenie, przerwy, instrukcję, kryteria jakości i wymagane wyposażenie.
3. Obserwacja wykonania nie jest jednoznacznie powiązana z niezmienną wersją recepty ćwiczenia.
4. Istnieją dwa powiązane, ale odrębne cykle: sekwencja gier/konspektu oraz trwała sesja rehabilitacyjna. Wymagają jednego koordynatora i spójnego identyfikatora sesji.
5. Przejście do kolejnego ćwiczenia powinno być idempotentne. Ręczne przejście i sygnał zakończenia gry nie mogą podwójnie zaliczać ćwiczenia.
6. Brakuje kompletnej kontroli gotowości stanowiska i kalibracji przed rozpoczęciem sesji.
7. Wznowienie procesu po restarcie aplikacji musi odtwarzać zarówno stan trwałej sesji, jak i indeks ćwiczenia, serię, odpoczynek i przypisanie gry.
8. Przypomnienie o kontroli po 24 godzinach powinno być trwałym zadaniem z terminem i statusem, a nie wyłącznie polem oczekującym na ręczne uzupełnienie.
9. Raport powinien wskazywać pochodzenie pomiarów, wersje recept, brakujące dane i zatwierdzenia.
10. Dane kliniczne wymagają kontroli dostępu, minimalizacji danych, audytu zmian i ochrony przed niezamierzonym nadpisaniem.

### 3.3. Zalecany model danych

Rozdzielić trzy warstwy:

1. **Konspekt wzorcowy** — zatwierdzona, wersjonowana definicja programu.
2. **Plan indywidualny** — wersjonowana kopia wybranego konspektu z parametrami pacjenta i ograniczeniami.
3. **Wykonanie** — niezmienny zapis rzeczywiście wykonanych ćwiczeń, serii, pomiarów i reakcji.

Każda recepta ćwiczenia powinna zawierać co najmniej:
- `prescription_id`, `exercise_id`, `prescription_version`;
- cel funkcjonalny, stronę ciała i pozycję;
- liczbę serii i powtórzeń albo czas;
- obciążenie, tempo, czas fazy ekscentrycznej/koncentrycznej, przerwy;
- docelowy zakres i dopuszczalną tolerancję;
- kryteria jakości ruchu i warunki przerwania;
- instrukcję, multimedia, grę i konfigurację wejścia;
- przeciwwskazania, ograniczenia oraz źródło ich zatwierdzenia.

## 4. Docelowy automatyczny przebieg

1. Wybór pacjenta i epizodu.
2. Kontrola kompletności oceny bazowej i ograniczeń.
3. Propozycja konspektu według celu, obszaru, poziomu i ograniczeń.
4. Zatwierdzenie planu indywidualnego przez terapeutę.
5. Kontrola stanowiska: port, czujnik, jakość sygnału, kalibracja, wartość spoczynkowa i dostępność gry.
6. Rozpoczęcie trwałej sesji i zapis punktu kontrolnego.
7. Instrukcja ćwiczenia, realizacja gry/biofeedbacku, zapis serii i obserwacji.
8. Odpoczynek z zegarem niezależnym od timera animacji.
9. Ocena reakcji po ćwiczeniu i po sesji.
10. Zakończenie sesji, zadanie kontrolne po 24 godzinach i przypomnienie.
11. Ocena progresji przez deterministyczne reguły.
12. Jawne zatwierdzenie lub odrzucenie rekomendacji przez uprawnionego terapeutę.
13. Raport porównawczy i przygotowanie kolejnej sesji.

Automatyzacja ma porządkować pracę i przedstawiać rekomendacje. Nie może samodzielnie rozpoznawać chorób, znosić ograniczeń pooperacyjnych, ordynować terapii ani zwiększać obciążenia bez zatwierdzenia.

## 5. Proponowane komponenty

| Komponent | Odpowiedzialność |
|---|---|
| `OutlineEditor` | Edycja, walidacja i podgląd konspektu |
| `OutlineVersionManager` | Wersje, porównywanie i zatwierdzanie |
| `ExercisePrescription` | Kompletna recepta ćwiczenia |
| `SessionOrchestrator` | Sekwencja ćwiczeń, gier, odpoczynku i pomiarów |
| `SensorReadinessMonitor` | Kontrola czujnika i kalibracji |
| `BiofeedbackGameAdapter` | Wspólna konfiguracja gier dla recept |
| `SessionCheckpointManager` | Trwałe punkty kontrolne i odzyskiwanie |
| `ProgressionEvaluator` | Ocena kryteriów i rekomendacje |
| `ClinicalWorkflowEngine` | Koordynacja całego procesu |
| `RehabilitationReportBuilder` | Raporty porównawcze |
| `ReminderScheduler` | Trwałe przypomnienia i kontrola terminów |

## 6. Kolejność realizacji

### Etap A — diagnostyka obrazu i stabilność silnika
1. Telemetria pętli gry, renderowania, symulacji i pominiętych kroków.
2. Testy jednostkowe i integracyjne telemetrii.
3. Testy trajektorii i skalowania w całym katalogu.
4. Profil jakości obrazu i benchmark optymalizacji.

### Etap B — pełna recepta i wersjonowanie konspektów
1. Rozszerzenie modelu danych recepty z zachowaniem zgodności starych JSON.
2. Walidacja schematu i diagnostyka błędów plików.
3. Edytor konspektów oraz wersjonowanie.
4. Powiązanie obserwacji z wersją recepty.

### Etap C — koordynator sesji
1. Jeden automat stanów dla sesji rehabilitacyjnej i sekwencji konspektu.
2. Idempotentne przejścia i ochrona przed podwójnym zaliczeniem.
3. Kontrola gotowości czujnika i gry.
4. Trwałe punkty kontrolne oraz odtwarzanie po restarcie.

### Etap D — przypomnienia, progresja i raport
1. Trwałe zadania kontroli po 24 godzinach.
2. Porównanie celów i wyników.
3. Rekomendacje z uzasadnieniem i zatwierdzeniem.
4. Raport zawierający historię wersji, dawkę, wyniki i braki danych.

## 7. Kryteria odbioru

### Gry
- Symulacja każdej gry pozostaje na stałym kroku 120 Hz.
- Statystyki rozróżniają symulację od prezentacji.
- Czas renderowania i aktualizacji jest mierzony bez istotnego narzutu.
- Testy obejmują 640×480, 1280×720, 1920×1080 i 2560×1440.
- Brak przenikania przez przeszkody przy maksymalnej prędkości w zweryfikowanych scenariuszach.
- Pauza, wznowienie, restart i utrata sygnału nie powodują skoków stanu.
- CI nie jest przedstawiane jako test fizycznego urządzenia.

### Konspekty i sesje
- Stary konspekt JSON nadal można wczytać.
- Nieprawidłowy konspekt otrzymuje jednoznaczny błąd walidacji.
- Zmiana wzorca nie zmienia istniejących planów i historii.
- Obserwacja wskazuje konkretną wersję recepty.
- Jedno ćwiczenie nie może zostać zaliczone dwukrotnie.
- Restart aplikacji pozwala odzyskać spójny stan sesji.
- Każda zmiana planu i decyzja progresji ma autora, czas i uzasadnienie.
- Raport jest odtwarzalny z danych źródłowych.
- Brak danych pozostaje brakiem danych, a nie wartością zero.

## 8. Ograniczenia i decyzje zachowane

- Pozostaje jeden centralny odczyt tensometru; gry nie implementują własnych czytników HX711.
- Gry wykorzystują skalibrowane wejście, a nie surowy protokół sprzętowy.
- Symulacja 120 Hz nie oznacza próbkowania czujnika 120 SPS. Przy HX711 80 SPS rzeczywista próbka pojawia się co około 12,5 ms.
- Testy Python pozostają w repozytorium jako narzędzia testowe. Nie są częścią skompilowanego programu Linux i nie są migrowane wyłącznie ze względów estetycznych.
- Nie zmieniać firmware ani protokołu bez wykazanego wymagania.
- Nie scalać gałęzi do `main` bez zakończonej kompilacji i właściwych testów.
- Nie wykonywać automatycznych decyzji klinicznych.

## 9. Plan wykonawczy

Prace rozpoczynają się od Etapu A, ponieważ telemetria umożliwi podejmowanie decyzji na podstawie pomiarów. Równolegle można przygotowywać Etap B, ale migracja danych musi zachować zgodność istniejących konspektów. Następnie należy zintegrować sesję przez jeden automat stanów. Każdy etap powinien kończyć się testami regresyjnymi i aktualizacją tego dokumentu o rzeczywisty stan, commit oraz wynik CI.



## 10. Dziennik realizacji — 2026-11-12

### Wdrożono w gałęzi `fix/smooth-game-motion`

**Etap A — pierwszy podetap: telemetria silnika**
- Rozdzielono liczbę kroków symulacji (`simulation_steps`) od liczby wywołań renderowania (`render_frames`) i liczby przebiegów timera (`timer_pumps`).
- Dodano średni i maksymalny czas renderowania oraz aktualizacji symulacji w mikrosekundach.
- Dodano licznik kroków symulacji odrzuconych podczas ograniczonego nadrabiania zaległości.
- Dodano szacunkową częstotliwość wywołań malowania (`paint_fps_estimate`). Jest to pomiar paintEvent, a nie gwarantowana liczba klatek faktycznie wyświetlonych przez kompozytor/monitor.
- Zachowano pole zgodności `frames`, które nadal oznacza liczbę kroków symulacji.
- Liczniki są zerowane zarówno przez reset gry, jak i reset statystyk.

**Etap C — pierwszy podetap: odporność przejść ćwiczeń**
- Dodano rejestr zakończonych ćwiczeń w bieżącej sesji, aby wielokrotne wywołanie zakończenia nie zwiększało licznika postępu i nie emitowało ponownie `exerciseCompleted`.
- Dodano uzbrajanie/rozbrajanie obsługi zakończenia gry przy przejściu między ćwiczeniami. Poprzednia gra jest rozbrajana przed wymianą, a nowa po obsłużeniu żądania uruchomienia.
- Weryfikacja kompilacji i regresji dla tych zmian pozostaje w toku; nie należy traktować ich jako odebranych przed zielonym CI.

### Następne zadania

1. Uruchomić CI dla aktualnego HEAD i poprawić ewentualne błędy kompilacji/testów.
2. Dodać deterministyczne testy porównania trajektorii i skalowania dla wszystkich gier.
3. Rozszerzyć model recepty ćwiczenia wstecznie kompatybilnie z istniejącymi plikami JSON.
4. Dodać walidator konspektów i raport błędów importu.
5. Zaprojektować i wdrożyć trwały, idempotentny automat sesji łączący konspekt z epizodem rehabilitacyjnym.
6. Powiązać każdą obserwację z wersją recepty, a następnie dodać trwałe przypomnienia po 24 godzinach.
