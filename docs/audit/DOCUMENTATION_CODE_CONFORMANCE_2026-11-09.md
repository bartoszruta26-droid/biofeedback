# 🔍 Audyt zgodności dokumentacji z kodem — 2026-11-09

> 🧾 Raport audytowy • Gałąź: `main` • Bazowy SHA przed aktualizacją dokumentów: `28272ea153864747d947038a3c0e853f9c1864ec`  
> Zakres: status funkcji, firmware Arduino/Pico, protokoły szeregowe, HTTP API, retencja próbek i macierz Desktop/TUI/WebUI/Android.

## 1. Podsumowanie

Przeprowadzono statyczny przegląd kodu bieżącej gałęzi oraz porównanie z dokumentacją. Wykryto nieaktualne opisy TUI, WebUI, API, przepływu pomiarów i macierzy interfejsów. Najważniejszy wniosek techniczny: samodzielny serwer API udostępnia odpytywanie ograniczonej historii próbek, ale nie utrwala tej historii i nie udostępnia strumienia WebSocket/SSE.

Nie wykonywano testów na fizycznym Arduino Nano, HX711 ani Raspberry Pi Pico. Nie wykonywano w tym audycie pełnej kompilacji wszystkich platform. Wnioski dotyczą przeglądu kodu, a nie walidacji sprzętowej.

## 2. Ustalenia techniczne

### BF-CONF-001 — P1: próbki serwerowej akwizycji nie są trwale archiwizowane

**Dowody w kodzie:**
- `MeasurementEngineConfig::historyCapacity` ma wartość domyślną 512.
- `HttpApiServer::GET /api/v1/acquisition/samples` zwraca `retention: "bounded_in_memory"`.
- `src/api/main.cpp` tworzy `MeasurementEngine` i `MeasurementService`, ale nie podłącza `AcquisitionJournal` ani `CommitHook`.
- Historia silnika jest ograniczana do 512 elementów i przechowywana w pamięci.

**Skutek:** próbki z serwerowego toru akwizycji są tracone po restarcie procesu; endpoint nie jest archiwum klinicznym. Sekwencja rozpoczyna się ponownie od wartości początkowej nowej instancji.

**Działanie:** wdrożyć trwałe zatwierdzanie próbek przed publikacją, odzyskiwanie sekwencji i odczyt z kursorem, który wykrywa luki.

### BF-CONF-002 — P1: WebUI deklaruje trasy, których serwer nie implementuje

**Dowody w kodzie:**
- `platforms/webui/api-client.js` zawiera `connectDevice`, `disconnectDevice`, `subscribeMeasurements` i `subscribeEvents`.
- `src/api/HttpApiServer.cpp` obsługuje wyłącznie `GET /api/v1/devices`; pozostałe nieobsługiwane żądania kończą się odpowiedzią 503/404/405.
- W statusie API jawnie znajdują się ograniczenia `device_control`, `measurement_stream` i `websocket_events`.

**Skutek:** metody klienta dla połączeń urządzeń i WebSocket nie mogą być uznane za działające funkcje.

**Działanie:** albo zaimplementować i przetestować trasy serwera, albo pozostawić je jako jawnie niedostępne i nie wywoływać ich w interfejsie.

### BF-CONF-003 — P1: dokument TUI nie odpowiadał bieżącemu kodowi

**Dowody w kodzie:** `src/tui/main.cpp` pobiera listę `patients`, pozwala wybrać pacjenta strzałkami lewo/prawo i zapisuje `patient_id` w archiwum pomiarowym i treningowym. Wybór jest opcjonalny. TUI otwiera port szeregowy lokalnie i nie korzysta z serwerowej `AcquisitionService`.

**Skutek:** stary dokument błędnie twierdził, że TUI nie wiąże sesji z pacjentem i nie archiwizuje pomiarów.

**Działanie:** dokument `docs/TUI_DESIGN.md` został przepisany zgodnie z aktualnym przepływem.

### BF-CONF-004 — P1: topologia klientów była opisana zbyt szeroko

**Dowody w kodzie:** Desktop i TUI korzystają z lokalnych adapterów szeregowych; samodzielny proces API może otworzyć osobny port szeregowy po ustawieniu `BIOFEEDBACK_SERIAL_PORT`. WebUI i Android korzystają z API i nie prowadzą bezpośredniej akwizycji.

**Skutek:** twierdzenie, że wszystkie interfejsy korzystają z jednego zdalnego strumienia danych, jest nieprawdziwe.

**Działanie:** `docs/ARCHITECTURE_SHARED_DATA_SERVER.md` rozdziela lokalny tor Desktop/TUI od zdalnego API.

### BF-CONF-005 — P1: uprawnienia API są tymczasowo ograniczone do administratora

**Dowody w kodzie:** odczyt i zapis kolekcji, rejestr urządzeń i sterowanie akwizycją sprawdzają rolę `administrator`. Nie ma zakresów per pacjent ani pełnego RBAC.

**Skutek:** API nie zapewnia jeszcze docelowego modelu autoryzacji dla wielu ról klinicznych.

**Działanie:** wdrożyć zakresowe RBAC, audyt operacji i testy negatywne dostępu.

### BF-CONF-006 — P2: rozbieżność wersji API

**Dowody w kodzie:** `/api/v1/status` zwraca wersję `0.3.0`, natomiast `QCoreApplication::setApplicationVersion` w `src/api/main.cpp` ustawia `0.2.0`. Starszy przykład w kontrakcie API pokazywał `1.0.0`.

**Działanie:** ujednolicić wersję w kodzie i dokumentacji; raportowana wersja powinna pochodzić z jednego źródła.

### BF-CONF-007 — P2: WebUI wykonuje ręczne wpisy, a nie rzeczywistą rozgrywkę/trening

**Dowody w kodzie:** `platforms/webui/app.js` tworzy sesje i wyniki treningowe/gier z `source_mode: "manual_entry"`. Nie uruchamia detektora powtórzeń, gry ani akwizycji czujnika.

**Działanie:** macierz odróżnia ręczny wpis archiwalny od rzeczywistej sesji z czujnikiem.

### BF-CONF-008 — P2: firmware Pico jest kanałem diagnostycznym, nie torem pomiarowym

**Dowody w kodzie:** `firmware/pico/main.cpp` obsługuje UART0 GP0/GP1, 115200 8N1, watchdog 2 s oraz komendy PING, STATUS i SAFE. Nie odczytuje HX711, nie wykonuje kalibracji i nie emituje ramki pomiarowej.

**Działanie:** dokument Pico utrzymuje jawne rozróżnienie między diagnostyką a akwizycją. Nie wolno oznaczać toru pomiarowego Pico jako gotowego.

### BF-CONF-009 — P2: odczyt HX711 nie wykonuje uśredniania wielu próbek

**Dowody w kodzie:** `HX711_SAMPLES` wynosi 1, a `readHX711Averaged(int samples)` wykonuje pojedyncze wywołanie `readHX711Raw()` i nie wykorzystuje argumentu `samples`.

**Działanie:** dokumentacja nie może opisywać obecnego toru jako uśredniania wielopróbkowego. Ewentualne uśrednianie wymaga zmiany firmware i testów wpływu na częstotliwość próbkowania.

## 3. Macierz rzeczywistych możliwości

| Funkcja | Desktop Qt | TUI | WebUI | Android |
|---|---|---|---|---|
| Pacjenci — CRUD | Tak | Brak | Tak | Tak |
| Pomiary — lokalny czujnik | Tak | Tak | Brak | Brak |
| Pomiary — archiwum | Tak | Tak | Odczyt | Odczyt |
| Pomiary — strumień API na żywo | Brak | Brak | Brak | Brak |
| Trening z czujnikiem | Tak | Tak | Brak | Brak |
| Ręczny wpis treningu | Brak | Brak | Tak | Brak |
| Silnik gier biofeedback | Tak | Brak | Brak | Brak |
| Ręczny wpis wyniku gry | Brak | Brak | Tak | Brak |
| Urządzenie lokalne | Tak | Tak | Brak | Brak |

Szczegółowa macierz i uwagi: [`GUI_COMPONENT_MATRIX.md`](GUI_COMPONENT_MATRIX.md).

## 4. Firmware i protokoły

### Arduino Nano / HX711

- Piny HX711: DT=D2, SCK=D3.
- UART: 115200 bodów.
- Częstotliwość odczytu: około 80 Hz.
- Ramka binarna: 9 bajtów danych (uint32 timestamp LE, int32 raw LE, CRC-8) + zakończenie `0xAA 0x55`.
- CRC-8: wielomian `0x07`, inicjalizacja `0xFF`.
- Odpowiedź tekstowa zawiera masę w gramach i znacznik `CAL:0/1`.
- Kalibracja jest ulotna; nie jest zapisywana w EEPROM przez obecny kod.
- Pojedyncza próbka, bez uśredniania wielopróbkowego.

### Raspberry Pi Pico

- UART0: GP0 TX / GP1 RX, 115200, 8N1.
- Watchdog: 2 sekundy.
- Maksymalna długość komendy: 63 znaki.
- Komendy: `PING`, `STATUS`, `SAFE`.
- Brak akwizycji czujnika i binarnych ramek pomiarowych.

## 5. Zmiany dokumentacji wykonane w ramach audytu

- Przepisano `docs/TUI_DESIGN.md`.
- Uaktualniono `docs/audit/GUI_COMPONENT_MATRIX.md`.
- Przepisano `docs/MEASUREMENT_PIPELINE.md` z opisem endpointu pollingu i retencji.
- Skorygowano topologię i kontrakt API w `docs/ARCHITECTURE_SHARED_DATA_SERVER.md`.
- Przepisano `platforms/webui/README.md` i `platforms/webui/API_CONTRACT.md`.
- Zaktualizowano `platforms/android/README.md`.
- Skorygowano główny `README.md`: rozróżniono bieżący firmware `hx711_advanced.ino` od historycznego przykładu `hx711_arduino_nano.ino`, opisano binarną transmisję ciągłą i ulotność kalibracji; doprecyzowano brak uśredniania wielopróbkowego.
- Uzupełniono `docs/HX711_CALIBRATION.md` o bezpośrednie wskazanie właściwego firmware i niezgodności przykładu historycznego z parserem binarnym.
- Naprawiono strukturę tabeli w `docs/README.md`, aby audyt zgodności był prawidłowym wierszem tabeli.
- Skorygowano `BUILD_INSTRUCTIONS.md`: właściwy szkic Arduino, prędkość 115200, bezpieczne uprawnienia `dialout`, aktualny algorytm AES-256-GCM i ostrzeżenia dotyczące kalibracji oraz backendu MariaDB.
- Dodano ostrzeżenia statusowe do rozdziałów `doc/rozdzial_2_3_architektura_hardware.md`, `doc/rozdzial_8_zalaczniki.md` i `doc/rozdzial_7_3_skalowanie_komercjalizacja_integracja.md`, aby założenia koncepcyjne, materiały robocze i planowane integracje nie były mylone z pomiarami, certyfikacją lub wdrożonymi funkcjami.

## 6. Testy i ograniczenia audytu

W repozytorium znajdują się testy: `measurement_engine_regression`, `protocol_parser_regression`, `acquisition_serial_adapter_regression`, `acquisition_service_regression`, `measurement_service_regression`, `hx711_calibration_protocol_regression.py`, `pico_firmware_safety_regression.py`, `tui_model_regression` oraz `platforms/webui/api-client.test.js`.

Ten audyt nie oznacza, że wszystkie te testy zostały uruchomione w ramach bieżącej czynności. Nie potwierdza kalibracji fizycznej, dokładności pomiarowej, działania na Raspberry Pi ani zgodności klinicznej. Wyniki GitHub Actions należy przypisać do konkretnego SHA.

## 7. Zalecana kolejność napraw kodu

1. **P1:** trwały dziennik serwerowej akwizycji i odzyskiwanie sekwencji.
2. **P1:** zgodność metod WebUI z rzeczywistymi trasami API; usunąć wywołania tras niezaimplementowanych.
3. **P1:** zakresowe RBAC i audyt dostępu do danych pacjenta.
4. **P1:** strumień próbek z wykrywaniem luk i kontrolą backpressure.
5. **P2:** ujednolicenie wersji API w jednym źródle.
6. **P2:** testy integracyjne wszystkich czterech klientów i testy sprzętowe Arduino/Pico.
