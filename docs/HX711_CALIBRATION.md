# ⚖️ HX711 — kalibracja i kontrola wiarygodności pomiaru

> 📘 Dokumentacja projektu • Język: polski • Nazwy techniczne, identyfikatory API i symbole kodu pozostają oryginalne.

## Domyślny tryb desktop GUI: RAW

Desktop GUI domyślnie odbiera, rejestruje i wyświetla surowe próbki ADC. Do poprawnego odbioru ramki wymagane są poprawny format i CRC, ale flaga firmware `CAL:1` ani pole `FORCE` nie są warunkiem zapisu danych RAW.

Kalibracja nie jest wykonywana w firmware. Aplikacja zachowuje surowy odczyt w trwałym dzienniku, a przeliczenie uruchamia dopiero po wybraniu w GUI jednostki `N` albo `kg`. Wtedy GUI odczytuje profil z pliku konfiguracyjnego i przelicza próbki lokalnie. Przełączenie z powrotem na `RAW` pokazuje pierwotne wartości ADC.

Domyślna ścieżka profilu:
`QStandardPaths::AppConfigLocation/calibration/hx711.json` (na Linuksie zwykle `~/.config/Biofeedback/calibration/hx711.json`)

Można ją zmienić kluczem `sensor.calibration_profile` w pliku `QStandardPaths::AppConfigLocation/config.json` (na Linuksie zwykle `~/.config/Biofeedback/config.json`). Ścieżka względna jest liczona od katalogu konfiguracji aplikacji. Profil musi być poprawnym dokumentem `CalibrationProfile`, zawierać własne `deviceId`, jednostkę `g`, `zeroOffsetRaw` oraz dodatni `scaleFactor`. Współczynnik oznacza gramy na jeden zliczony krok RAW. Bez poprawnego profilu GUI pozostaje w trybie RAW i nie pozwala prezentować wartości jako N/kg.

Przeliczenie w GUI:
`masa_g = (raw - zeroOffsetRaw) * scaleFactor`
`masa_kg = masa_g / 1000`
`siła_N = masa_kg * 9.80665`

Wewnętrzne buforowanie siły i analiza biomechaniczna pozostają w jednostkach SI. Wartość RAW jest zawsze zachowana niezależnie od wybranej jednostki prezentacji.


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
