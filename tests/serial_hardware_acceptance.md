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

## F. Protokół wyniku

Dla każdego testu zapisać:
- datę i wersję aplikacji;
- commit gałęzi testowanej;
- system operacyjny i wersję Qt;
- model płytki i firmware;
- port oraz baud rate;
- wynik PASS/FAIL;
- logi i opis odchyleń.

Testy sprzętowe oznaczyć jako NIE WYKONANO, jeżeli nie przeprowadzono ich na fizycznym urządzeniu. Sam sukces kompilacji nie jest dowodem poprawności pomiaru.
