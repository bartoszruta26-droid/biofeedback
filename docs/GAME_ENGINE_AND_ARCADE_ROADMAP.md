# Plan rozwoju silnika i gier

## Zakres wymagany

### Gry do utrzymania i poprawy
- Sinus Force Trainer
- Tan Force Trainer
- Pong Force Trainer
- Flappy Bird
- Mario Run
- Arkanoid

### Gry do zaimplementowania
- Galaga
- Archer
- Scorched Earth (w pierwotnym zgłoszeniu zapisano „Scored Earth”)
- Car Race

## Zasady architektury

1. Wszystkie gry korzystają ze wspólnego `GameEngine`: jeden zegar monotoniczny, stały krok symulacji, ograniczona praca nadrabiania czasu, wspólny cykl życia i statystyki.
2. Dane czujnika są wejściem sterującym, a nie bezpośrednim dostępem do stanu gry z wątku portu szeregowego. Zmiany stanu widgetów wykonuje wyłącznie wątek GUI.
3. Strumienie czujnika o dużej częstotliwości należy scalać do najnowszej próbki, aby kolejka zdarzeń Qt nie rosła szybciej niż interfejs potrafi ją obsłużyć.
4. Każda gra musi obsługiwać start, pauzę, wznowienie, zatrzymanie, reset, zmianę rozmiaru okna, brak sensora i nieprawidłowe dane wejściowe.
5. Ruch i kolizje powinny być niezależne od częstotliwości odświeżania GUI. Długie zatrzymanie aplikacji nie może powodować nieograniczonej symulacji nadrabiającej.
6. Wynik, poziom, życia, błędy i czas muszą być spójne z mechaniką konkretnej gry oraz wspólnymi statystykami silnika.

## Wymagania funkcjonalne dla nowych gier

- **Galaga:** statek gracza, fale przeciwników, pociski, kolizje, życia, punkty, narastająca trudność i sterowanie ruchem z czujnika.
- **Archer:** celowanie sterowane czujnikiem, łuk i strzały, ruchome cele, wykrywanie trafień, punkty i poziomy trudności.
- **Scorched Earth:** artyleria, regulacja kąta/siły strzału, trajektoria balistyczna, wpływ wiatru, teren i cele; fizyka musi używać stałego kroku.
- **Car Race:** sterowanie pasem/pozycją pojazdu, ruch ulicy i przeszkód, kolizje, punkty za dystans oraz stopniowanie trudności.

## Plan walidacji

- Testy jednostkowe cyklu życia silnika, zegara, pauzy, resetu, ograniczenia kroku i wejścia czujnika.
- Testy fabryki potwierdzające utworzenie każdej gry po wszystkich obsługiwanych aliasach identyfikatorów.
- Testy scenariuszowe: kolizja, zdobycie punktu, utrata życia, zmiana poziomu, zakończenie i reset.
- Testy bez sensora, z wartościami NaN/Inf, bardzo dużymi wartościami i gwałtownymi zmianami wejścia.
- Pełna kompilacja Qt5 oraz uruchomienie testów w trybie `offscreen`.
- Testy sprzętowe z rzeczywistym Arduino/HX711 jako osobny etap; nie są zastępowane przez CI.

## Stan prac

- [x] Scalanie PR #208 do `integration/all-work`.
- [x] Ograniczenie liczby oczekujących zdarzeń wejścia czujnika przez scalanie próbek do najnowszej wartości.
- [x] `GameEngine::reset()` odtwarza również stan gry pochodnej; `start()` korzysta z tej samej ścieżki, bez podwójnego wywołania inicjalizacji. Test regresyjny sprawdza dokładną liczbę inicjalizacji po starcie, pauzie, resecie i ponownym uruchomieniu.
- [x] Mario: inicjalizacja chmurek zabezpieczona dla widoku o zerowym rozmiarze; test resetu przed ustawieniem rozmiaru widgetu oraz podczas sesji.
- [x] Regresja dla serii 1000 próbek wysłanych z wątku roboczego.
- [ ] Audyt i poprawki mechaniki istniejących gier (w pierwszej kolejności Arkanoid, Pong, Flappy Bird i Mario).
- [x] Zabezpieczenia wejściowego deltaTime w Arkanoidzie, Flappy Bird, Mario, Sinus Force Trainer i Tan Force Trainer; normalizacja wektora odbicia piłki w Pongu.
- [x] Sinus/Tan Force Trainer: odrzucanie NaN/Inf w próbkach siły, kontrola dodatniego i skończonego kroku czasu oraz ograniczenie pojedynczej aktualizacji do 100 ms.
- [x] Testy regresyjne wejścia i deltaTime dla Sinus/Tan: bardzo duża delta, NaN, Inf, wartości ujemne oraz uszkodzone próbki czujnika.
- [x] Pierwsza grywalna implementacja Galaga: statek, sterowanie czujnikiem/klawiaturą, pociski obu stron, kolizje, życia, fale i punktacja. Pozostaje rozszerzyć testy scenariuszowe i zbalansować rozgrywkę.
- [x] Pierwsza grywalna implementacja Archer: regulacja kąta z czujnika/klawiatury, strzały balistyczne, ruchomy cel i punktacja. Pozostaje rozszerzyć testy scenariuszowe i dopracować rozgrywkę.
- [x] Pierwsza grywalna implementacja Scorched Earth: artyleria turowa, kąt i siła strzału, grawitacja, wiatr, deformacja terenu, zdrowie czołgów i przeciwnik AI. Pozostaje rozszerzyć testy scenariuszowe, dopracować AI i zbalansować fizykę.
- [x] Pierwsza grywalna implementacja Car Race: trzy pasy, sterowanie czujnikiem/klawiaturą, przeszkody, kolizje, życia, dystans, wynik i rosnąca prędkość. Pozostaje rozszerzyć testy scenariuszowe i dopracować rozgrywkę.
- [x] Rejestracja nowych gier w GameFactory oraz udostępnienie ich w selektorach TrainingTab i OutlineTab.
- [x] Archer, Car Race i Scorched Earth: obsługa zmiany rozmiaru aktywnego widoku; zachowanie/ponowne dopasowanie celu, przeszkód, terenu, czołgów i pocisków.
- [x] Galaga: skalowanie statku, przeciwników i pocisków podczas zmiany rozmiaru; ograniczenie pozycji aktorów do nowego widoku.
- [x] Galaga: czasowa ochrona po trafieniu, czyszczenie wrogich pocisków bez unieważniania iteracji oraz pojedyncza utrata życia przy zejściu formacji; fala jest odtwarzana dopiero po rzeczywistej utracie życia.
- [x] Testy zmiany rozmiaru Galaga i Car Race z aktywnymi aktorami/przeszkodami.
- [x] Car Race: test scenariuszowy potwierdza utratę życia po kolizji z pierwszą przeszkodą na deterministycznie wybranym pasie; klasa udostępnia wyłącznie do odczytu liczbę pozostałych żyć i przeszkód na potrzeby diagnostyki/testów.
- [x] Galaga: testuje początkową liczbę żyć, pierwszą falę (24 przeciwników), zachowanie gry po aktywnej rozgrywce i odtworzenie stanu początkowego przez reset.
- [x] Archer: test potwierdza, że trafienie zwiększa prędkość celu, a więc uruchamia mechanizm narastania trudności.
- [x] Scorched Earth: test potwierdza stan początkowy zdrowia/rundy, istnienie terenu, ciągłość sesji po strzale oraz odtworzenie zdrowia i rundy przez reset. Bezpośrednie testy trafień czołgów, zmiany rund i regeneracji pozostają do dopisania.
- [x] Testy regresyjne zmiany rozmiaru podczas aktywnej rozgrywki Archer i Scorched Earth; Car Race ma już test małego widoku podczas gry.
- [ ] Testy scenariuszowe i pełna walidacja CI dla wszystkich gier.

Wszystkie prace prowadzić na gałęziach opartych o `integration/all-work`. Nie wprowadzać zmian bezpośrednio do `main`.
