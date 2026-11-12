# Audyt integracji gier — 2026-11-11

## Cel
Bezpiecznie odzyskać unikalne ulepszenia starszych gałęzi gier na aktualnej bazie `main`, bez cofania bieżącego sterowania, wydajności, skalowania i zabezpieczeń wejścia.

## Stan bazowy
- Gałąź docelowa: `main`.
- Aktualny commit w chwili audytu: `81d3273e9b136558cdb7f18e07d3e04fd2c43a70`.
- Ostatnie scalone PR-y: #272 (Sinus Force / Scorched Earth regresje), #274 (Mario Run), #275 (Reaction Force).
- Brak otwartych PR-ów w chwili audytu.
- Testy statusowe/CI nie zwróciły wyników dla aktualnego HEAD w użytym dostępie; nie można zatem stwierdzić, że bieżący commit jest zweryfikowany pełnym CI.

## Zasady integracji
1. Nie scalać bezpośrednio starych gałęzi, które są silnie rozbieżne z `main`.
2. Nowy kod gier pozostaje sterowany jednym źródłem HX711 przez wspólny potok wejścia. Nie przenosić handlerów klawiatury/myszy/dotyku/gamepada do rozgrywki.
3. Zachować stały krok symulacji 120 Hz, niezależność fizyki od rozdzielczości oraz responsywne skalowanie.
4. Zachować timeout świeżości próbek, walidację NaN/Inf, neutralne uzbrajanie i bezpieczne zachowanie po utracie sygnału.
5. Każda funkcja musi mieć deterministyczny test regresyjny: inicjalizacja, reset, zmiana rozmiaru, brak czujnika, kolizje, punktacja i osiągnięcia.
6. Najpierw portować wyłącznie unikalne zmiany do gałęzi opartej na aktualnym `main`, potem uruchomić pełną kompilację Qt5 i CTest; fizyczny odbiór Nano/HX711 pozostaje odrębnym krokiem.

## Wynik porównania historii
Poniższe gałęzie mają nadal unikalne zmiany, ale nie należy ich scalać bezpośrednio ze względu na rozbieżność historii:
- `feature/scorched-earth-gameplay-overhaul` — 10 commitów naprzód / 303 wstecz; unikalne pliki: ScorchedEarthGame, test fabryki, dokumentacja.
- `feature/archer-gameplay-overhaul` — 7 naprzód / 303 wstecz; unikalne pliki: ArcherGame, test fabryki, dokumentacja.
- `feature/mario-run-gameplay-upgrade` — 7 naprzód / 302 wstecz; starsza implementacja zastąpiona nowszym PR #274, nie przenosić bez ponownego porównania.
- `feature/car-race-gameplay-overhaul` — 12 naprzód / 963 wstecz; ówczesna praca ma alternatywną gałąź zintegrowaną przez PR #267.
- `feature/expanded-sinus-force-trainer` — 10 naprzód / 298 wstecz; funkcjonalność przebiegów przejęta przez PR #272.
- `fix/arcade-additional-games-usability` — 14 naprzód / 305 wstecz; część zmian dotyczy alternatywnego sterowania.
- `fix/car-race-force-trainer-usability` — 7 naprzód / 305 wstecz; część funkcji może być już zawarta w PR #267.
- `fix/flappy-bird-gameplay-usability` — 8 naprzód / 305 wstecz; dodatkowe poprawki zmiany rozmiaru i wejścia, lecz gałąź ma klawiaturę/mysz.
- `fix/arkanoid-force-trainer-usability` — 12 naprzód / 305 wstecz; dodatkowe mechanizmy serwisu/resize, lecz gałąź ma klawiaturę/mysz.

## Bezpiecznie scalone ostatnio
- #272 — pięć przebiegów Sinus Force, testy i korekta responsywności. Scorched Earth odzyskał też brakujące funkcje symulacji/kolizji w bieżącej linii zmian.
- #274 — aktualna wersja Mario Run, oparta na nowszym `main`; poprzednie #265/#273 są zastąpione.
- #275 — Reaction Force; deklarowane testy regresyjne oraz integracja z GameFactory, TrainingTab, OutlineTab i CMake.

## Etap odzyskiwania
Nowa gałąź została utworzona z aktualnego `main`: `integration/gameplay-recovery-main-2026-11-11`.
W tym etapie nie skopiowano jeszcze kodu ze starych gałęzi: do rzetelnego portu potrzebny jest pełny diff bazowy każdego pliku i odczyt wszystkich istotnych źródeł/CI. Sam opis PR nie wystarcza do bezpiecznego przeniesienia logiki.

## Kolejność następnych prac
1. Archer — przenieść tylko wiatr/ruch celu/serie/ładunki skupienia, bez klawiatury; dodać testy przewidywania balistyki i resetu.
2. Scorched Earth — przejrzeć bieżący kod po #272; przenieść AI/odłamkowe obrażenia tylko po wykazaniu braków.
3. Car Race — porównać z #267 i przenieść wyłącznie brakujące elementy.
4. Arkanoid i Flappy Bird — selektywnie przenieść stabilizację wejścia/skalowanie/timeout, bez alternatywnych kontrolerów.
5. Wykonać pełny build Qt5, CTest, kontrolę statyczną własności HX711 i sprawdzić workflow.

