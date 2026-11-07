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
- `PongGame`
- `FlappyBirdGame`
- `MarioGame`

W menu `OutlineTab` są ponadto pozycje Tan Game, Arkanoid, Galaga, Archer, Scored Earth i Car Race. W dokumentacji struktury są deklarowane pliki tych gier, ale w aktualnej konfiguracji kompilacji nie ma odpowiadających im źródeł. Wyszukiwanie repozytorium nie potwierdziło implementacji klas. Nie należy zatem oznaczać ich jako działających ani tworzyć pozornych pustych klas. Pozostają jawnie w planie kolejnych etapów:

1. TanGame — implementacja i testy.
2. Arkanoid — implementacja i testy.
3. Galaga — implementacja i testy.
4. Archer — implementacja i testy.
5. Scored Earth — implementacja i testy.
6. Car Race — implementacja i testy.

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
5. Następnie TanGame, Arkanoid, Galaga, Archer, Scored Earth i Car Race — każda gra jako osobny zakres z testami.
