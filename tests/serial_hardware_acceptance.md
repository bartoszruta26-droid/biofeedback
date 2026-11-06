# Biofeedback – testy akceptacyjne toru pomiarowego

## Cel i zasada ochrony

Testy służą potwierdzeniu, że refaktoryzacja interfejsu nie zmienia zachowania istniejącego toru pomiarowego. Nie wolno zmieniać formatu ramek Arduino, kolejności bajtów, algorytmu CRC-8, skalowania ani częstotliwości odczytu w ramach prac nad UI.

## A. Testy automatyczne wykonywane bez urządzenia

Uruchomienie po skonfigurowaniu projektu:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Wymagane: wszystkie zarejestrowane testy regresyjne kończą się powodzeniem.

## B. Testy odbioru danych z Arduino

Wymagane wyposażenie: ta sama płytka Arduino Nano, moduł HX711, czujnik i przewód USB używane w działającej konfiguracji.

1. Uruchomić aplikację i potwierdzić wykrycie właściwego portu.
2. Potwierdzić połączenie z prędkością 115200, 8N1.
3. Rozpocząć ciągły odbiór danych zgodnie z dotychczasowym protokołem firmware.
4. Obserwować co najmniej 60 sekund transmisji bez wykonywania poleceń kalibracji.
5. Potwierdzić, że próbki mają poprawny znacznik czasu, wartość surową i status poprawności.
6. Potwierdzić, że błędne ramki/CRC są odrzucane, a prawidłowe kolejne próbki nadal są odbierane.
7. Odłączyć i ponownie podłączyć urządzenie; potwierdzić poprawną obsługę utraty i odzyskania połączenia.

Kryteria zaliczenia:
- brak zawieszenia aplikacji;
- brak nieoczekiwanej zmiany jednostek lub skali;
- brak utraty odbioru po pierwszym błędnym pakiecie;
- brak zmian w protokole i firmware;
- brak pogorszenia częstości odświeżania względem wersji bazowej.

## C. Testy prezentacji danych

1. Otworzyć zakładkę Pomiary i sprawdzić aktualizację wartości na żywo.
2. Sprawdzić wykres przy stałym obciążeniu, zwiększaniu i zmniejszaniu obciążenia.
3. Porównać wartość surową z danymi diagnostycznymi.
4. Sprawdzić tryb wartości skalibrowanych bez zmieniania parametrów kalibracji.
5. Rozpocząć i zatrzymać rejestrację; sprawdzić, czy stan przycisków i status są zgodne.
6. Sprawdzić, czy przejście pomiędzy zakładkami nie zatrzymuje odbioru danych.

## D. Testy dystrybucji danych

Po wdrożeniu centralnego rozdzielania próbek:
- każda zarejestrowana zakładka otrzymuje tę samą próbkę dokładnie raz;
- aktywna gra otrzymuje próbki bez przejmowania callbacku pozostałych odbiorców;
- zmiana aktywnej gry nie powoduje duplikowania odbiorników;
- zamknięcie/zwolnienie odbiorcy nie powoduje wywołania callbacku na zniszczonym obiekcie.

## E. Test DebugTab

1. Otworzyć Diagnostykę.
2. Wygenerować serię komunikatów INFO, DATA, WARNING i ERROR.
3. Nacisnąć Wyczyść wielokrotnie.
4. Potwierdzić, że aplikacja nie zawiesza się i można nadal odbierać dane.
5. Włączyć/wyłączyć automatyczne przewijanie i zmieniać filtr.
6. Wyeksportować log i sprawdzić zawartość.


## F. Testy treningu biofeedback

1. Utworzyć ćwiczenie z dodatnią siłą docelową, liczbą serii i powtórzeń.
2. Potwierdzić, że wybrana gra jest osadzona w panelu treningowym.
3. Wykonać kontrolowany cykl: wzrost siły powyżej 80% celu, a następnie spadek poniżej 40% celu.
4. Potwierdzić, że cykl zostaje zaliczony dokładnie jako jedno powtórzenie.
5. Utrzymać siłę powyżej 40% celu i potwierdzić, że nie są naliczane kolejne powtórzenia bez ponownego rozluźnienia.
6. Dokończyć serię i sprawdzić czas odpoczynku oraz automatyczne przejście do kolejnej serii.
7. Zmienić typ gry pomiędzy ćwiczeniami; potwierdzić, że stara gra zostaje zatrzymana, a nowa jest osadzona w panelu.
8. Zatrzymać i ponownie uruchomić trening; potwierdzić wyzerowanie statystyk nowej sesji.

Kryteria zaliczenia:
- powtórzenie rozpoczyna się przy osiągnięciu 80% siły docelowej;
- powtórzenie kończy się po spadku poniżej 40% siły docelowej;
- pojedynczy cykl napięcie–rozluźnienie nalicza dokładnie jedno powtórzenie;
- statystyki średniej i maksymalnej siły szczytowej odpowiadają zarejestrowanym powtórzeniom;
- próbka sensora jest przekazywana do aktywnej gry tylko jednym torem;
- brak zawieszenia przy zmianie gry i zatrzymaniu treningu.

## G. Protokół wyniku

Dla każdego testu zapisać:
- datę i wersję aplikacji;
- commit gałęzi testowanej;
- system operacyjny i wersję Qt;
- model płytki i firmware;
- port oraz baud rate;
- wynik PASS/FAIL;
- logi i opis odchyleń.

Testy sprzętowe oznaczyć jako NIE WYKONANO, jeżeli nie przeprowadzono ich na fizycznym urządzeniu. Sam sukces kompilacji nie jest dowodem poprawności pomiaru.


## Dodatkowe testy po audycie cyklu życia i kalibracji

### Kalibracja i binarna ścieżka danych
Protokół binarny zgodny z dokumentacją firmware ma 11 bajtów:
`0xAA | timestamp[4] | forceValue[4] | CRC-8[1] | 0x55`.
CRC-8 obejmuje bajty 0–8, łącznie z bajtem synchronizacyjnym. Pole `forceValue` jest wynikiem kalibracji i filtracji wykonanej w firmware; aplikacja nie skaluje go ponownie.
1. Wykonaj tare i kalibrację firmware'u dwoma punktami: zero oraz znana masa wzorcowa wyrażona w gramach.
2. Potwierdź, że pole Force Value w ramce binarnej jest już skalibrowane i filtrowane przez firmware; parser zachowuje tę wartość bez ponownego skalowania.
3. Porównaj wartość z ramki binarnej z odpowiedzią tekstową FORCE:g dla tego samego obciążenia.
4. Osobno sprawdź pomocniczą konwersję hosta `setCalibrationPoint()/convertToCalibrated()` dla źródeł, które rzeczywiście dostarczają surowe ADC.
5. Potwierdź, że konfiguracja protokołu jawnie rozróżnia dane surowe i skalibrowane, aby uniknąć podwójnej kalibracji lub interpretacji ADC jako gramów.

### Cykl życia połączenia
1. Uruchom aplikację bez podłączonego Arduino i poczekaj na automatyczne wyszukiwanie.
2. Zamknij aplikację podczas wyszukiwania; proces powinien zakończyć się bez zawieszenia i bez dostępu do zniszczonego widgetu.
3. Podłącz Arduino podczas wyszukiwania; potwierdź identyfikację urządzenia i rozpoczęcie odbioru.
4. Odłącz urządzenie podczas odbioru, a następnie ponownie je podłącz.
5. Zmień zakładki podczas intensywnego odbioru i zamknij aplikację; nie powinno wystąpić zawieszenie ani awaria.

### Pojedynczy kanał wejściowy gier
1. W TrainingTab uruchom grę i wykonaj serię kontrolowanych nacisków.
2. Potwierdź, że pojedyncza próbka wpływa na aktywną grę tylko raz.
3. W zakładce Konspekt uruchom grę sesji i potwierdź przekazywanie skalibrowanej siły w niutonach.
4. Wstrzymaj grę lub sesję; próbki nie powinny zmieniać stanu gry.


### Pojedynczy odbiornik w zakładce Pomiary
1. Uruchom pomiar i obserwuj licznik próbek oraz wykres przez co najmniej 60 sekund.
2. Porównaj liczbę ramek odebranych przez terminal diagnostyczny z liczbą próbek zapisanych w sesji; jedna ramka nie może być zapisana dwukrotnie.
3. Użyj przycisku ręcznego odczytu i potwierdź, że wykorzystuje ostatnią próbkę wspólnego odbiornika, bez dodatkowego żądania `DATA`.
4. Sprawdź, że wybór trybu ADC jest dostępny wyłącznie wtedy, gdy firmware rzeczywiście dostarcza pole `RAW`; dla binarnej ramki skalibrowanej interfejs pozostaje w trybie siły.
