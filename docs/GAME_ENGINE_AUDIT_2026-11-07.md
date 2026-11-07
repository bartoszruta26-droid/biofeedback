# Audyt GameEngine i gier — 2026-11-07

Zakres: gałąź `integration/all-work`. Zmiany nie są kierowane do `main`.

## 1. GameEngine — poprawki wprowadzone

- Zegar symulacji oparto na `QElapsedTimer`, a nie zegarze ściennym. Eliminuje to ujemne `deltaTime` przy północy i zależność od korekt czasu systemowego.
- Aktualizacja gry odbywa się ze stałym krokiem 1/120 s. Delta klatki jest ograniczona, a nadmiar zaległości po długim zatrzymaniu jest odrzucany, aby uniknąć nieograniczonego nadrabiania.
- Dane z czujnika otrzymane poza wątkiem GUI są przekazywane do wątku widgetu przez queued invocation. Wartości NaN i Inf są odrzucane.
- Dodano implementacje `getGameStats()` i `resetGameStats()`; statystyki bazowe zawierają wynik i czas aktywnej sesji w milisekundach.
- `start()` wywołane podczas pauzy wznawia grę zamiast resetować jej stan.

## 2. Poprawki w grach

### SinGame
- Zmiana trudności zeruje rzeczywisty wynik klasy bazowej przez `setScore(0)`, a nie emituje wyłącznie sygnał z wartością zero.

### PongGame
- Zmiana trudności resetuje wynik klasy bazowej oraz oba liczniki meczu: gracza i AI.
- Symulacja stałokrokowa ogranicza zależność kolizji od opóźnień timera GUI.

### FlappyBirdGame
- Naciśnięcie tensometru wyzwala pojedyncze machnięcie przy narastającym zboczu sygnału. Utrzymywanie nacisku nie ustawia prędkości pionowej w każdej klatce, więc ptak nie może zawisnąć dzięki ciągłemu naciskowi.
- Po zakończeniu gry aktualizacja przeszkód i punktacji jest przerywana w tej samej klatce.

### MarioGame
- Generowanie przeszkód używa przekazanego `deltaTime`, a nie założenia 60 FPS.
- Czas ładowania skoku korzysta z `QElapsedTimer`; usunięto zależność od czasu ściennego, w tym w animacji dekoracyjnych chmur.
- Po zakończeniu gry dalsze aktualizacje przeszkód w tej klatce są przerywane.

## 3. Test regresyjny

Dodano `tests/game_engine_regression.cpp` i target Qt `game_engine_regression`:
- start i zatrzymanie;
- stały krok symulacji;
- pauza bez aktualizacji;
- wznowienie przez `start()` bez resetowania;
- przekazanie wejścia z wątku roboczego do GUI;
- podstawowy zapis i reset statystyk.

Test jest skonfigurowany z `QT_QPA_PLATFORM=offscreen`.

## 4. Inwentaryzacja gier i brakujących implementacji

Aktualny plik budowania aplikacji wymienia źródła:
- `GameEngine`
- `SinGame`
- `TanGame` (dodany w PR #205)
- `PongGame`
- `FlappyBirdGame`
- `MarioGame`

W menu `OutlineTab` pozostają pozycje Arkanoid, Galaga, Archer, Scored Earth i Car Race, ale nie mają jeszcze aktywnych implementacji. Są oznaczone jako niedostępne. Nie należy traktować ich jako działających gier ani tworzyć pozornych pustych klas. Pozostają jawnie w planie kolejnych etapów:

1. Arkanoid — implementacja i testy.
2. Galaga — implementacja i testy.
3. Archer — implementacja i testy.
4. Scored Earth — implementacja i testy.
5. Car Race — implementacja i testy.

## 5. Ograniczenia walidacji

Zmiany zapisano bezpośrednio na `integration/all-work`. Na moment sporządzenia dokumentu nie ma potwierdzonego zielonego przebiegu CI dla najnowszego commita, więc nie deklaruje się kompilacji ani testów jako zaliczonych. W szczególności należy zweryfikować dostępność przeciążenia funktorowego `QMetaObject::invokeMethod` w najstarszej wspieranej wersji Qt5.

Przed uznaniem audytu za zamknięty wymagane są:
- zielona kompilacja Qt5 i test `game_engine_regression`;
- testy zachowania wszystkich czterech istniejących gier;
- testy kolizji Pong i Flappy przy granicznych deltaTime;
- testy ładowania skoku i generatora przeszkód Mario;
- walidacja kalibracji wejścia sensora i progów dla każdej gry.

## 6. Kolejność dalszych prac

Po zielonym CI:
1. SinGame — punktacja, tolerancja, częstotliwość, testy wejścia.
2. Pong — geometria i ciągłość kolizji, wynik, reset, trudność.
3. Flappy Bird — fizyka impulsu, luki, kolizje i punktacja.
4. Mario — ładowanie skoku, platformy, dziury, kolizje i progresja.
5. TanGame — implementacja i testy (PR #205).
6. Następnie Arkanoid, Galaga, Archer, Scored Earth i Car Race — każda gra jako osobny zakres z testami.


## 7. Uzupełniające poprawki — PR #204

W kolejnym etapie zachowano istniejącą symulację stałokrokową 1/120 s i uzupełniono jej obsługę:
- licznik kroków symulacji i licznik odrzuconych wejść;
- statystyki obejmujące nazwę gry, stan, wynik, czas, liczbę kroków i błędne próbki;
- bezpieczne kolejkowanie wejścia przez QPointer;
- odrzucanie NaN/Inf i nasycanie wyniku do zakresu int;
- start() podczas pauzy wznawia sesję, bez ponownej inicjalizacji.

Poprawki gier:
- SinGame: faza jest przesuwana z uwzględnieniem częstotliwości przestrzennej; dodano histerezę i ograniczenie punktacji za wejście w strefę.
- PongGame: pozycja paletek jest ograniczona również przy małym oknie. Stały krok silnika ogranicza przenikanie piłki przez paletki.
- FlappyBirdGame: sterowanie tensometrem działa jako impuls na narastającym zboczu siły; geometria luk uwzględnia ich faktyczną wysokość i promień ptaka; odrzucono nieprawidłowy mnożnik prędkości.
- MarioGame: ładowanie skoku jest akumulowane przez deltaTime symulacji, zatem pauza go nie nalicza; otwory są rzeczywistymi przerwami w podłożu, widocznymi również po renderowaniu gruntu; aktualizacja przerywa się po zakończeniu gry.

Rozszerzono game_engine_regression o testy statystyk, NaN, przepełnienia wyniku, startu w aktywnej sesji i wznowienia przez start() oraz resume(). Test działa również w konfiguracji Release, ponieważ używa jawnego mechanizmu REQUIRE zamiast assert.

Target testowy linkuje DebugManager.cpp i Threads::Threads oraz jest jawnie budowany w workflow security-regression. Status kompilacji i testów należy ocenić na podstawie przebiegu CI PR #204; testy sprzętowe Arduino/HX711 i walidacja kliniczna pozostają odrębnym zadaniem.


## 8. TanGame i wspólny mechanizm uruchamiania — PR #205

- Dodano TanGame jako grę sterowaną znormalizowaną siłą. Wykres tangensa jest ograniczony do skończonego zakresu, a ścieżka renderowania jest przerywana przy skokach w pobliżu asymptot.
- Dodano poziomy trudności, histerezę trafienia i punktację ograniczoną do 1–10 punktów za wejście w strefę.
- Dodano GameFactory, który rozpoznaje identyfikatory sin, tan, pong, flappy i mario wraz z aliasami zapisanymi w konspektach.
- TrainingTab i MainWindow korzystają z tego samego factory. MainWindow nie ogranicza już uruchamiania gier OutlineTab do samego SinGame.
- W OutlineTab gry bez implementacji są widoczne jako przygotowywane, ale ich pozycje są wyłączone.
- Dodano game_factory_regression, sprawdzający utworzenie pięciu dostępnych gier, ich cykl życia i odrzucanie nieznanych identyfikatorów.

Walidacja PR #205 obejmuje kompilację Qt5, CTest i security-regression. Dopiero zielone przebiegi dla aktualnego HEAD uprawniają do scalenia. Symulowane próbki nie zastępują testu fizycznego czujnika.
