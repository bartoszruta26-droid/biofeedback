# 📡 Wspólny silnik pomiarowy

> 📘 Weryfikacja kodu: 2026-11-09 • Bazowy SHA: `28272ea153864747d947038a3c0e853f9c1864ec`  
> Zakres: `MeasurementEngine`, adaptery szeregowe, `AcquisitionService`, serwer API i klienci Desktop/TUI.

## 1. Cel i granice odpowiedzialności

`biofeedback_measurement` jest niezależną od Qt warstwą domenową. Przyjmuje próbki z adapterów źródłowych, normalizuje obsługiwane jednostki, przypisuje czas odbioru hosta, waliduje wartości i nadaje status jakości. Utrzymuje ograniczoną historię w pamięci i udostępnia wielokrotne subskrypcje.

W repozytorium istnieją obecnie dwa sposoby podłączenia sprzętu:
- Desktop i TUI otwierają lokalny port szeregowy przez `SerialCommunication` i `MeasurementSerialAdapter`.
- Samodzielny proces `biofeedback_api` może zarejestrować serwerowy adapter Arduino Nano HX711, jeśli ustawiono `BIOFEEDBACK_SERIAL_PORT`; próbki trafiają wtedy przez `AcquisitionSerialDeviceAdapter` i `AcquisitionService`.

Nie wszystkie interfejsy są obecnie wyłącznie klientami zdalnego strumienia API.

## 2. Model próbki

- `sequence`: monotoniczny numer próbki w obrębie instancji silnika.
- `timestampUnixMs`: czas odbioru próbki przez hosta w milisekundach od epoki Unix.
- `deviceTimestampMs`: surowy 32-bitowy znacznik czasu urządzenia w milisekundach.
- `rawValue`: surowy odczyt przetwornika.
- `value`, `unit`: wartość po normalizacji do jednostki kanonicznej.
- `source`: identyfikator adaptera/źródła.
- `quality`: status jakości; konsument musi odrzucać próbki, dla których `isValid()` jest fałszywe.

## 3. Jednostki

Jawne normalizacje:
- masa: `g`, `mg` → `kg`; `kg` pozostaje `kg`;
- długość: `mm`, `cm` → `m`; `m` pozostaje `m`;
- napięcie: `mV`, `uV` → `V`; `V` pozostaje `V`;
- siła: `N`, `newton(s)` → `N`;
- odczyt surowy: `raw`, `count(s)` → `count`.

Silnik nie zgaduje wymiaru fizycznego i nie przelicza kg na N. Adapter musi przekazać jednostkę wynikającą z konfiguracji urządzenia i kalibracji. Wartość surowa bez kalibracji powinna używać `count`.

## 4. Stany jakości

- `valid` — próbka przeszła walidację;
- `sensor_rejected` — źródłowy sterownik oznaczył próbkę jako niepoprawną;
- `non_finite_value` — wartość jest NaN lub nieskończonością;
- `missing_timestamp` — brak znacznika czasu;
- `invalid_unit` — jednostka nie należy do obsługiwanego zestawu;
- `invalid_source` — brak identyfikatora źródła;
- `outside_configured_range` — wartość po normalizacji przekracza skonfigurowany zakres.

Próbki odrzucone są zachowywane w historii i przekazywane subskrybentom ze statusem jakości. Konsument musi sprawdzić `isValid()`/`quality` przed wykreśleniem, agregacją lub eksportem.

## 5. Protokół Arduino Nano / HX711

Wspólny parser oczekuje 11-bajtowej ramki:
- bajty 0–3: `timestamp` — uint32 little-endian, milisekundy od uruchomienia urządzenia;
- bajty 4–7: `rawValue` — int32 little-endian;
- bajt 8: CRC-8 obliczane dla pierwszych 8 bajtów, wielomian `0x07`, wartość początkowa `0xFF`;
- bajty 9–10: znaczniki końca `0xAA 0x55`.

W `hx711_advanced.ino` struktura `DataPacket` jest spakowana do 9 bajtów przed dodaniem dwóch bajtów końcowych. Firmware czyta HX711 na pinach D2/D3 i pracuje z częstotliwością docelową około 80 Hz. Odczyt w obecnym kodzie jest pojedynczy; parametr funkcji `readHX711Averaged` nie oznacza faktycznego uśredniania wielu próbek.

Dodatkowa odpowiedź tekstowa zawiera `FORCE:<grams>`, `RAW:<value>`, `TS:<millis>` i `CAL:0/1`. Wartość `FORCE` jest masą w gramach wyliczoną przez współczynnik kalibracji firmware. Znacznik kalibracji jest zmienną ulotną i po restarcie urządzenia wymaga ponownego ustawienia. Surowa poprawna ramka CRC nie jest równoznaczna ze skalibrowaną wartością fizyczną.

## 6. API akwizycji — stan faktyczny

Samodzielny `biofeedback_api` udostępnia:
- `GET /api/v1/acquisition/session` — stan aktywnej sesji;
- `POST /api/v1/acquisition/session` — rozpoczęcie sesji po wskazaniu `id`, `patient_id`, `device_id`; pacjent musi istnieć, a urządzenie być zarejestrowane i połączone;
- `DELETE /api/v1/acquisition/session` — zatrzymanie sesji po identyfikatorze;
- `GET /api/v1/acquisition/samples?after_sequence=N` — odczyt próbek nowszych niż wskazany numer sekwencji.

Operacje te wymagają zalogowanego administratora. Serwer domyślnie nasłuchuje na loopback. Urządzenie Arduino jest rejestrowane wyłącznie po jawnej konfiguracji `BIOFEEDBACK_SERIAL_PORT`; opcjonalne zmienne to `BIOFEEDBACK_SERIAL_BAUD` i `BIOFEEDBACK_DEVICE_ID`.

Endpoint próbek jest mechanizmem odpytywania HTTP, a nie WebSocket/SSE. Zwraca `retention: "bounded_in_memory"`. Domyślna historia `MeasurementEngine` ma pojemność 512 próbek. W `src/api/main.cpp` nie jest podłączony `AcquisitionJournal` ani trwały `CommitHook`; próbki z tego toru nie są automatycznie zapisywane do magazynu trwałego i znikają po restarcie procesu. Numer sekwencji także rozpoczyna się ponownie od wartości początkowej instancji.

**Konsekwencja:** endpointu `/acquisition/samples` nie wolno traktować jako archiwum klinicznego ani kompletnego rejestru próbek. Wymagane jest trwałe zatwierdzanie próbek przed ich udostępnieniem oraz mechanizm odczytu z kursorem i jawnie określoną retencją.

## 7. Ograniczenia integracji klientów

- Desktop: lokalny odczyt czujnika, wspólny silnik, dziennik próbek oraz archiwizacja zakończonej sesji w repozytorium danych.
- TUI: lokalny odczyt czujnika i lokalny silnik; archiwum pomiarowe wysyłane do API przy rozłączeniu lub wyjściu. Bufor sesji jest ograniczony do 100 000 próbek i nie jest trwałą kolejką offline.
- WebUI: odczyt archiwów i ręczne wpisy sesji treningowych/wyników gier; brak odbioru strumienia czujnika.
- Android: odczyt archiwów; brak akwizycji i tworzenia nowych sesji pomiarowych/treningowych/gier z rzeczywistego czujnika.
- WebSocket/SSE dla próbek i zdarzeń nie jest zaimplementowany po stronie serwera.

## 8. Testy w repozytorium

Weryfikację automatyczną wspierają m.in. `measurement_engine_regression`, `protocol_parser_regression`, `acquisition_serial_adapter_regression`, `acquisition_service_regression`, `measurement_service_regression`, `measurement_integration_regression.py`, `measurement_single_reader_regression.py`, `hx711_calibration_protocol_regression.py` i `pico_firmware_safety_regression.py`. Testy programowe nie zastępują pomiarów z masami wzorcowymi ani odbioru fizycznego stanowiska.
