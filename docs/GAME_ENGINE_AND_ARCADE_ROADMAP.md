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
- [x] Regresja dla serii 1000 próbek wysłanych z wątku roboczego.
- [ ] Audyt i poprawki mechaniki istniejących gier (w pierwszej kolejności Arkanoid, Pong, Flappy Bird i Mario).
- [ ] Implementacja Galaga.
- [ ] Implementacja Archer.
- [ ] Implementacja Scorched Earth.
- [ ] Implementacja Car Race.
- [ ] Testy scenariuszowe i pełna walidacja CI dla wszystkich gier.

Wszystkie prace prowadzić na gałęziach opartych o `integration/all-work`. Nie wprowadzać zmian bezpośrednio do `main`.
