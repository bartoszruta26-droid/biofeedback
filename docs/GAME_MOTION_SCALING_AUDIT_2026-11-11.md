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

## Etap 2 — wspólna prezentacja i migracja wybranych gier

Wspólny kontrakt renderowania został dodany do GameEngine:

- fizyka pozostaje na stałym kroku 120 Hz;
- silnik wylicza współczynnik interpolacji z reszty akumulatora czasu;
- renderInterpolated(painter, alpha) jest domyślnie zgodny wstecz z render(painter), więc gry można migrować etapami;
- interpolacja dotyczy wyłącznie prezentacji i nie zmienia czasu symulacji, kolizji ani danych wejściowych HX711;
- po wznowieniu gry i resecie współczynnik interpolacji jest zerowany.

Gry, które w tej iteracji przejęły interpolację stanu ruchu:

- Pong — piłka i obie paletki;
- Arkanoid — piłka i paletka;
- Archer — strzały i ruchomy cel;
- Mario — postać, przesuw świata, przeszkody, przeciwnicy, przedmioty i chmurki;
- Scorched Earth — pociski.

Pozostałe gry nadal korzystają z domyślnego renderowania. Nie należy uznawać migracji całego katalogu za zakończoną.

## Ograniczenia weryfikacji etapu 2

Zmiany zostały zapisane w gałęzi PR. Repozytorium nie zgłosiło workflow runs ani status checks dla aktualnego commita. Kompilacja i testy uruchomieniowe nie zostały zatem potwierdzone. Przed scaleniem trzeba zbudować wszystkie cele Qt5 i wykonać game_engine_regression, game_factory_regression oraz pozostałe testy CTest.

Dalsza część etapu 2:

- Flappy Bird — interpolowana pozycja ptaka i ruch przeszkód; własny paintEvent w trakcie gry przekazuje renderowanie do wspólnego silnika.
- Car Race — interpolowana pozycja pojazdu, przeszkody, przewijanie drogi i pozycja wizualna pasa.
- Reaction Force — interpolowany zegar wizualny; własny paintEvent korzysta z tego samego współczynnika prezentacji.
- Mario — własny paintEvent został podłączony do interpolowanego renderera.
- Force Maze — interpolowana pozycja gracza, postęp tunelu, czas wizualny i drganie kamery.
- Balance Tower — interpolowana pozycja celu, przechylenie, stabilność i zegar animacji.
- Precision Crane — interpolowane położenie wózka, haka, kołysanie ładunku i faza przeszkód.

Wszystkie wymienione migrowane gry zachowują oryginalne wejście czujnika i logikę symulacji. Przy zmianie rozmiaru lub restarcie migrowane stany poprzednie są synchronizowane z bieżącymi, aby uniknąć interpolacji przez różne układy współrzędnych.


## Etap 3 — kolejne gry

W tej iteracji migrację rozszerzono o Force Maze, Balance Tower i Precision Crane. Do migracji pozostają Galaga, Sin, Tan i Rhythm Force. Konieczna jest też kontrola wszystkich skalowań fizyki i kolizji, ponieważ interpolacja renderowania sama w sobie nie naprawia błędnej prędkości ani przenikania przez przeszkody.


## Etap 4 — pełna migracja prezentacji

Wspólny interfejs interpolacji został wdrożony dla wszystkich 15 gier z katalogu:

1. Pong
2. Arkanoid
3. Archer
4. Mario
5. Scorched Earth
6. Flappy Bird
7. Car Race
8. Reaction Force
9. Force Maze
10. Balance Tower
11. Precision Crane
12. Sin
13. Tan
14. Rhythm Force
15. Galaga

Każda gra interpoluje odpowiednie dla swojej mechaniki elementy ruchome lub zegar wizualny. Dodano też test regresyjny, który przeprowadza wszystkie gry przez rozdzielczości 640×480, 1280×720, 1920×1080 i 2560×1440 oraz powrót do 640×480, sprawdzając, czy sesja pozostaje aktywna.

To jest test odporności na zmianę rozmiaru, a nie jeszcze dowód identycznej prędkości fizycznej w każdym formacie. Niezależny test trajektorii, czasu przelotu, kolizji i proporcji obiektów wymaga dalszej walidacji per gra. W szczególności trzeba zweryfikować skalowanie prędkości, grawitacji, wymiarów obiektów i granic pola gry — interpolacja usuwa nierówności prezentacji, ale nie koryguje automatycznie błędów modelu fizycznego.
