# HX711 — rzeczywisty strumień 80 SPS i prezentacja 120 Hz

## Tor akwizycji

1. HX711 wykonuje konwersje z częstotliwością 80 SPS po ustawieniu wejścia RATE w stan wysoki.
2. Firmware `hx711_advanced.ino` steruje wejściem RATE z pinu Arduino Nano A0.
3. Arduino publikuje każdą gotową konwersję jako binarną ramkę CRC-8 z czasem urządzenia.
4. Aplikacja wysyła komendę `CONT\n` raz przy uruchomieniu odczytu i odbiera strumień bez cyklicznego żądania `DATA\n`.
5. Parser ramek jest utrzymywany pomiędzy odczytami portu, więc ramka podzielona pomiędzy pakiety USB/UART nie jest tracona.
6. Zatrzymanie odczytu wysyła `STOP\n`.

## Okablowanie RATE

Pozostaw dotychczasowe połączenia DT → D2 i SCK → D3. Dodatkowo połącz pad RATE modułu HX711 z pinem A0 Arduino Nano. Firmware ustawia A0 jako wyjście HIGH, co wybiera 80 SPS. Bez tego połączenia sam kod nie może wymusić trybu 80 SPS na przetworniku; moduł może pozostać w trybie 10 SPS.

Po zmianie należy ponownie wgrać `hx711_advanced.ino` do Nano. Po uruchomieniu należy sprawdzić, czy odstępy znaczników czasu kolejnych ramek wynoszą około 12–13 ms.

## Interpolacja prezentacji

Wykres desktopowy odświeża się z docelową częstotliwością 120 Hz i oblicza punkty pośrednie metodą monotonicznej interpolacji sześciennej Hermite'a (ogranicznik Fritscha–Carlsona). Interpolacja jest ograniczona do przedziału pomiędzy sąsiednimi pomiarami, aby nie tworzyć sztucznych szczytów ani minimów.

Punkty interpolowane służą wyłącznie do rysowania. Dziennik, analiza biomechaniczna, detekcja powtórzeń, serie i sterowanie grą otrzymują wyłącznie rzeczywiste próbki z urządzenia. Zapis dziennika przyjmuje próbki nieblokująco do ograniczonej kolejki; synchronizacja z dyskiem jest grupowana co 8 próbek i przy zamykaniu sesji, aby pojedynczy fsync nie zatrzymywał odbioru UART.

## Detekcja powtórzeń i postęp

Domyślna konfiguracja detektora dla strumienia 80 SPS:
- minimalny czas powtórzenia: 120 ms,
- maksymalna dopuszczalna przerwa pomiędzy próbkami: 100 ms,
- próg aktywacji: 20% siły docelowej, nie mniej niż 2 N,
- próg zwolnienia: 10% siły docelowej (czyli 50% progu aktywacji przy domyślnych proporcjach).

Każde powtórzenie jest zaliczane po powrocie siły poniżej progu zwolnienia. To zapewnia rozdzielenie kolejnych naciągnięć i ogranicza wielokrotne zaliczanie jednego utrzymywanego skurczu. Po ukończeniu zadanej liczby powtórzeń aktualizowane są statystyki serii i kursor postępu treningu.

## Weryfikacja

Automatyczne testy obejmują:
- ciągły odbiór wielu ramek binarnych przez pseudo-terminal,
- zachowanie parsera przy kilku ramkach w jednym zapisie,
- trzy krótkie cykle naciągnięcia przy okresie próbkowania około 12 ms,
- rozdzielenie 80 SPS akwizycji od 120 Hz interpolacji wykresu.

Test sprzętowy należy wykonać po wgraniu firmware i podłączeniu RATE do A0. Należy sprawdzić rzeczywisty odstęp timestampów, brak zalegającej kolejki, reakcję gry i liczbę powtórzeń/serii.
