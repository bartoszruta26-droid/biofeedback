# Strategia testów interfejsów Biofeedback

## Warstwy testów

1. Jednostkowe: modele, walidacja, usługi domenowe, format próbek.
2. Kontraktowe: DTO, API HTTP, kody błędów, autoryzacja i zdarzenia.
3. Integracyjne: DeviceService, MeasurementEngine, repozytoria, sesje.
4. UI: nawigacja, formularze, stany puste/błędy/offline, dostępność.
5. E2E: scenariusz od logowania do zapisu sesji i raportu z użyciem symulatora sprzętu.
6. Sprzętowe: testy na Raspberry Pi/Arduino/HX711 w oddzielnym, oznaczonym środowisku.

## Obowiązkowe scenariusze

- brak urządzenia i ponowne połączenie;
- błędna ramka, CRC, timeout i przerwanie transmisji;
- zatrzymanie pomiaru i zamknięcie aplikacji;
- równoczesne próby rozpoczęcia pomiaru;
- brak uprawnień i wygasła sesja;
- dane pacjenta nie trafiają do logów;
- jednostki, znaczniki czasu i wersja kalibracji są zachowane;
- brak próbek nie generuje fikcyjnego wykresu;
- eksport/odtworzenie danych;
- responsywność Web i różne rozmiary ekranu Android.

## CI

- C++: format/kompilacja, CTest, sanitizery dla wybranych celów.
- TUI: test modelu, smoke test trybów CLI, test terminala pseudoterminalem.
- Web: node --check, testy klienta, testy komponentów i E2E.
- Android: Gradle assembleDebug, unit tests, instrumented tests na emulatorze.
- API: testy kontraktowe i negatywne testy autoryzacji.
- Pull request: wymagane zielone workflow dotkniętych platform.

## Ograniczenia

Symulator nie zastępuje walidacji sprzętowej. Testy kliniczne, użyteczności i zgodności regulacyjnej wymagają odrębnych protokołów i zatwierdzeń.
