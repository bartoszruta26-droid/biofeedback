# HX711 — kalibracja i kontrola wiarygodności pomiaru

## Warunek uruchomienia analizy siły

Aplikacja przyjmuje wartość siły do analizy ruchu dopiero wtedy, gdy firmware zgłasza `CAL:1`. Poprawna ramka binarna z surowym odczytem ADC nie oznacza jeszcze, że wynik jest skalibrowany. Wartość `FORCE` bez potwierdzenia kalibracji pozostaje dostępna wyłącznie jako informacja diagnostyczna.

Aktualny firmware `hx711_advanced.ino` rozpoczyna pracę z `is_calibrated=false`. Po restarcie lub odłączeniu zasilania wymaga ponownego potwierdzenia kalibracji. Współczynnik kalibracji nie jest obecnie zapisywany w pamięci nieulotnej.

## Procedura

1. Zamontuj czujnik tensometryczny w docelowej mechanice i sprawdź przewody HX711.
2. Uruchom firmware `hx711_advanced.ino` i połącz aplikację z właściwym portem szeregowym.
3. Usuń obciążenie z czujnika i wykonaj tarowanie poleceniem `TARE`.
4. Umieść na czujniku znany, stabilny ciężar referencyjny.
5. Wyślij polecenie `CAL:<masa_w_gramach>`, np. `CAL:500` dla wzorca 500 g.
6. Sprawdź odpowiedź firmware i kolejne ramki `FORCE:...g,...,CAL:1`.
7. Zdejmij wzorzec, ponownie sprawdź zero, a następnie wykonaj pomiary kontrolne z co najmniej dwoma innymi znanymi obciążeniami.
8. Powtórz obciążanie i odciążanie, aby ocenić powtarzalność, histerezę i dryft.

## Interpretacja

- `CAL:1`: firmware potwierdza aktywny współczynnik kalibracji; aplikacja może przeliczyć gramy na niutony.
- `CAL:0` lub brak flagi: surowa ramka może być poprawna, ale nie wolno używać jej do obliczania siły, powtórzeń ani sterowania grą zależnego od siły.
- Tryb RAW: służy do diagnostyki sygnału ADC, nie do oceny parametrów biomechanicznych.

Przeliczenie programowe używa (F = m g), gdzie masa jest w kilogramach, a (g = 9{,}80665\,m/s^2). Jest to wyłącznie konwersja jednostek. Nie potwierdza dokładności czujnika ani poprawności konstrukcji mechanicznej.

## Granice walidacji

Test pseudo-terminala sprawdza protokół binarny + tekstowy, tekstowy tryb zgodności oraz odrzucanie `CAL:0`. Nie zastępuje pomiarów na rzeczywistym HX711. Przed zastosowaniem do rzeczywistych osób wymagane są udokumentowane testy z wzorcami masy, kontrola zakresu, liniowości, histerezy, dryftu, powtarzalności i bezpieczeństwa mechanicznego.
