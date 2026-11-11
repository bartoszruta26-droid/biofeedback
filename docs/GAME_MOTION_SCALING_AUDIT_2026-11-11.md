# Audyt ruchu i skalowania gier — 2026-11-11

## Zmiany w tej gałęzi

- Pong: proporcje paletek zmienione na wyraźnie pionowe (wąskie, wysokie), z wymiarami opartymi o wspólną skalę pola gry.
- Pong: prędkość piłki i AI używa jednolitej skali wynikającej z mniejszego z dwóch współczynników rozmiaru pola. Zmiana rozdzielczości nie mnoży już składowych prędkości niezależnie w osi X i Y.
- Pong: kolizje z paletkami sprawdzają przecięcie toru piłki z przednią płaszczyzną paletki (swept collision), a odbicie jest możliwe tylko przy ruchu w stronę tej paletki. Krótkie boki nie uruchamiają już obrotu zależnego od położenia piłki.
- Test regresyjny: proporcje paletek są sprawdzane dla 640×480 i 1920×1080.

## Wspólna pętla gry

GameEngine wykonuje symulację ze stałym krokiem 1/120 s, ogranicza nadrabianie zaległości i odrzuca skrajne opóźnienia. To ogranicza zależność fizyki od częstotliwości timera. Sam stały krok nie gwarantuje jednak płynnego obrazu na monitorze 60 Hz: do pełnej płynności przy różnych częstotliwościach odświeżania potrzebna jest interpolacja stanu renderowania.

## Dalszy audyt wymagany przed uznaniem wszystkich gier za naprawione

Wstępny przegląd źródeł wskazuje, że gry używają różnych polityk skalowania:

- Archer, Galaga, Mario, Car Race, Flappy Bird i Scorched Earth zawierają osobne współczynniki X/Y albo przeliczanie po zmianie rozmiaru.
- Arkanoid, Sin, Tan, Force Maze, Balance Tower, Rhythm Force, Reaction Force i Precision Crane wymagają weryfikacji prędkości obiektów, kolizji i proporcji w małych, HD i panoramicznych polach.
- Należy sprawdzić osobno każdy obiekt poruszający się szybko: pociski, piłki, przeszkody i elementy sterowane czujnikiem. Dla obiektów o dużej prędkości należy stosować kolizje ciągłe lub odpowiednio małe podkroki.
- Dalsza wspólna poprawka powinna wprowadzić jeden kontrakt współrzędnych logicznych, stałą symulację oraz interpolację renderowania, a następnie dostosować do niego każdą grę bez zmiany zasad sterowania HX711.

## Kryteria odbioru

1. Ta sama trasa i czas ruchu w polu logicznym dla 640×480, 1280×720, 1920×1080 i 2560×1440.
2. Brak przenikania przez cienkie przeszkody przy maksymalnej prędkości.
3. Brak zmiany kierunku bez rzeczywistego kontaktu z powierzchnią kolizyjną.
4. Brak wzrostu trudności wynikającego wyłącznie z rozdzielczości lub proporcji ekranu.
5. Testy regresyjne dla zmiany rozmiaru podczas gry, utraty sygnału i wznowienia.
