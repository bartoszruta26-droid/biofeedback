# 🧠 Architektura centralnej akwizycji i dystrybucji danych

> 📘 Dokumentacja projektu • Język: polski • Nazwy techniczne, identyfikatory API i symbole kodu pozostają oryginalne.

Status: docelowy kierunek architektoniczny; nie jest deklaracją, że cały przepływ został już wdrożony.

## Decyzja architektoniczna

Akwizycja ma być własnością serwera/usługi urządzeń, a nie poszczególnych GUI/TUI. Serwer jest jedynym autorytatywnym koordynatorem sesji i źródłem danych dla klientów. Klient może zażądać rozpoczęcia sesji, jej zatrzymania lub zmiany dozwolonej konfiguracji, ale nie otwiera bezpośrednio portu czujnika i nie tworzy konkurencyjnego strumienia pomiarowego.

Należy rozdzielić:
- **serwer danych i orkiestracji** — uwierzytelnia klientów, zarządza sesjami, przypisuje pacjenta/protokół/urządzenie, zapisuje rekordy i publikuje zdarzenia;
- **warstwę akwizycji** — adaptery transportowe (USB/serial, Ethernet, lokalne urządzenia) odbierają ramki, dekodują je i przekazują do wspólnego silnika pomiarowego;
- **firmware urządzeń brzegowych** — wykonuje deterministyczne próbkowanie i lokalne zabezpieczenia czasowe, gdy wymaga tego sprzęt; nie zastępuje serwera jako źródła prawdy dla aplikacji;
- **klienty** — Desktop GUI, WebUI, Android GUI, TUI i aplikacja gier subskrybują ten sam strumień i korzystają z tego samego API.

Serwer może działać na komputerze pomiarowym lub dedykowanym hoście. Nie powinien być uruchamiany wyłącznie wewnątrz procesu Desktop GUI. Zamknięcie okna Desktop nie może przerywać sesji prowadzonej przez niezależny serwer, o ile polityka sesji nie nakazuje zatrzymania.

## Przepływ

```text
HX711 / ADC / czujniki / Arduino Nano / Raspberry Pi Pico
                 |
          USB / UART / Ethernet
                 |
       Adaptery akwizycji serwera
                 |
     Parser protokołu + kalibracja źródła
                 |
       MeasurementEngine / walidacja
                 |
        MeasurementSessionService
           |             |
           |             +--> trwały dziennik próbek i metadane
           |
       Event/Stream Broker
           |
     WebSocket (telemetria) + REST (sterowanie i archiwum)
           |
 Desktop GUI | WebUI | Android | TUI | gry
```

## Własność sesji i spójność

1. Serwer nadaje każdej sesji niepowtarzalny identyfikator i jest jedynym komponentem zmieniającym jej stan.
2. Domyślnie jedno urządzenie może mieć najwyżej jedną aktywną sesję akwizycji. Próba równoległego startu zwraca konflikt wraz z identyfikatorem aktywnej sesji.
3. Sesja wiąże: pacjenta (wewnętrzny UUID), urządzenie, kanały, jednostki, profil kalibracji i jego wersję/hash, protokół, operatora, czas UTC oraz wersję schematu.
4. Każda próbka ma identyfikator sesji, źródło/kanał, sekwencję, czas urządzenia (jeżeli dostępny), czas odbioru serwera UTC, wartość surową, wartość skalibrowaną, jednostkę i status jakości.
5. Sekwencja jest monotoniczna w obrębie strumienia urządzenia/sesji. Utrata, duplikacja i zmiana kolejności muszą być wykrywalne, a nie maskowane.
6. Serwer zapisuje próbki przed lub w kontrolowanym buforze trwałym. UI nie jest magazynem danych. Awaria klienta nie może utracić danych już przyjętych przez serwer.
7. Każdy klient może wznowić subskrypcję od ostatniego potwierdzonego numeru sekwencji. Serwer zwraca informację o luce, jeżeli zakres nie jest już dostępny.
8. Dane ręcznie wpisane są oznaczane osobnym `source_mode=manual_entry`; nie mogą być mieszane z pomiarami sprzętowymi bez zachowania informacji o pochodzeniu.

## API

REST pozostaje interfejsem do uwierzytelniania, katalogu urządzeń, konfiguracji, tworzenia i kończenia sesji oraz odczytu archiwum.

Docelowe operacje:
- `POST /api/v1/acquisition/sessions` — żądanie rozpoczęcia sesji z pacjentem, urządzeniem, kanałami i protokołem;
- `GET /api/v1/acquisition/sessions/{id}` — stan i metadane sesji;
- `POST /api/v1/acquisition/sessions/{id}/stop` — kontrolowane zatrzymanie;
- `GET /api/v1/acquisition/sessions/{id}/samples?after_sequence=N` — odczyt/wznowienie archiwum;
- `GET /api/v1/devices` — wykryte urządzenia, stan adaptera i dostępne kanały;
- `GET /api/v1/status` — gotowość, aktywne ograniczenia i wersje usług.

WebSocket jest kanałem dystrybucji telemetrii i zdarzeń:
- `/ws/acquisition/sessions/{id}` — próbki jednej sesji;
- `/ws/events` — zmiany stanu urządzeń i sesji dostępne dla uprawnionego klienta.

Wiadomość telemetrii zawiera `session_id`, `sequence`, `device_timestamp_ms`, `received_at_utc`, `source`, `channel`, `raw_value`, `value`, `unit`, `quality` i `calibration_version`. Format musi być wersjonowany. Nie wolno wysyłać PESEL ani danych osobowych w strumieniu próbek.

REST nie może być używany do wysyłania pojedynczych próbek z dużą częstotliwością. WebSocket/SSE jest dla dystrybucji; trwały dziennik jest źródłem odtwarzania po rozłączeniu. Klient powinien tolerować rozłączenie, reconnect, duplikaty i luki.

## Kolejki, obciążenie i trwałość

- Akwizycja nie może czekać na renderowanie UI ani na wolnego subskrybenta.
- Każdy odbiorca ma ograniczony bufor; przepełnienie powoduje jawny licznik utraconych wiadomości i możliwość odtworzenia z dziennika.
- Zapis archiwalny działa niezależnie od subskrybentów. Polityka buforowania i zachowania przy braku miejsca na dysku jest jawna.
- Dla wysokich częstotliwości transport może grupować próbki w ramki, zachowując sekwencje i znaczniki czasu.
- Dane surowe i przetworzone są przechowywane rozdzielnie. Ponowne przeliczenie nie nadpisuje danych surowych.
- Eksport/backup obejmuje rekordy, dzienniki próbek, konfigurację urządzenia i kalibrację, z kontrolą integralności.

## Bezpieczeństwo i granice sterowania

- Operacje akwizycji wymagają uwierzytelnienia i uprawnienia do danego pacjenta, urządzenia i sesji.
- Serwer sprawdza konflikt sesji, stan adaptera, zakresy kanałów i dopuszczalne częstotliwości.
- Klient nie może dowolnie podawać portu, komendy szeregowej ani surowej ramki urządzenia.
- Utrata połączenia klienta nie powinna zatrzymywać akwizycji automatycznie; reguła ta jest konfigurowana polityką sesji.
- Sterowanie efektorami (PWM, elektrody, anteny, grzałki, silniki) jest osobną płaszczyzną poleceń, z niezależnymi zabezpieczeniami sprzętowymi, limitami, watchdogiem i stanem bezpiecznym. Sam serwer ani GUI nie zastępują interlocków sprzętowych.
- WebSocket wymaga WSS poza loopback. Surowy HTTP nie może być wystawiany do sieci.
- Logi nie zawierają PESEL, tokenów, surowych danych klinicznych ani sekretów.

## Obecny stan kodu i luki

W repozytorium istnieją `MeasurementEngine`, `MeasurementService`, `MeasurementSerialAdapter` i `DeviceService`. Desktop ma działającą ścieżkę próbek do silnika, a TUI posiada integrację callbackową. Jednak obecna implementacja nie jest jeszcze serwerem akwizycji:
- `biofeedback_api` tworzy `MeasurementEngine`, `MeasurementService`, `AcquisitionService` i `DeviceService`, a następnie wstrzykuje je do `HttpApiServer`;
- dodano `AcquisitionSerialDeviceAdapter`: odbiera `SensorData` z istniejącego `SerialCommunication`, buduje `MeasurementInput` i przekazuje próbkę wyłącznie przez `AcquisitionService::ingest(deviceId, ...)`; próbka spoza aktywnej sesji lub z innego urządzenia jest odrzucana;
- serwer może jawnie zarejestrować jeden adapter Nano przez `BIOFEEDBACK_SERIAL_PORT`, opcjonalnie `BIOFEEDBACK_SERIAL_BAUD` i `BIOFEEDBACK_DEVICE_ID`; bez wskazanego portu sprzęt pozostaje wyłączony, a serwer nie skanuje i nie przejmuje przypadkowych portów;
- `GET /api/v1/devices` prezentuje stan zarejestrowanego urządzenia; start sesji nadal wymaga urządzenia połączonego i istniejącego pacjenta;
- starszy `MeasurementSerialAdapter` jest nadal używany przez część Desktop/TUI i prowadzi lokalny tor do `MeasurementEngine`; migracja tych klientów na API serwera pozostaje otwarta i do czasu jej zakończenia nie wolno deklarować, że wszystkie klienty używają wyłącznie centralnej akwizycji;
- `MeasurementService` jest prostą fasadą nad silnikiem i nie zarządza adapterami, sesją ani trwałym dziennikiem;
- `HttpApiServer` udostępnia uwierzytelnione endpointy `GET/POST/DELETE /api/v1/acquisition/session` i `GET /api/v1/acquisition/samples`; przed startem sprawdza istnienie pacjenta i zarejestrowane, połączone urządzenie. Historia próbek jest ograniczona pamięcią RAM; serwer nie implementuje jeszcze WebSocket/SSE;
- część desktopowej akwizycji nadal jest inicjowana przez zakładkę UI;
- Android i WebUI nie odbierają jeszcze wspólnego strumienia próbek;
- historia silnika jest ograniczona pamięcią RAM i nie stanowi trwałego archiwum.

Zatem obecne CRUD współdzieli rekordy, ale nie jest równoznaczny z centralną akwizycją ani synchronizacją czasu rzeczywistego.

## Etapy wdrożenia i kryteria odbioru

### Etap A — serwis sesji akwizycji (częściowo wdrożony)
- Istnieje niezależny od GUI `AcquisitionService`, który koordynuje stan sesji i przekazuje próbki do `MeasurementService`.
- HTTP API uruchamia i zatrzymuje sesję, a walidacja pacjenta i rejestru urządzeń działa przed startem.
- Testy obejmują start/stop, podwójny start, nieznanego pacjenta, nieznane urządzenie i wyścig stop/ingest.
- Do ukończenia pozostaje rejestracja rzeczywistych adapterów, wiązanie ich cyklu życia z sesją oraz przeniesienie wszystkich ścieżek akwizycji klientów do usługi serwerowej.

### Etap B — trwały dziennik
- Dodać przyrostowy zapis próbek z sekwencją i kontrolą integralności.
- Zapewnić odtworzenie po restarcie i wykrywanie luk.
- Testy: awaria procesu, niepełna końcówka pliku, uszkodzony rekord, limit dysku, odtworzenie kolejności.

### Etap C — API i transport strumieniowy
- Wydzielić serwerowy interfejs publikacji zdarzeń.
- Dodać WebSocket/WSS lub SSE z uwierzytelnianiem, kontrolą zakresu sesji, reconnect i backpressure.
- Testy dwóch lub więcej klientów jednocześnie, reconnect, replay od sekwencji, uprawnienia i brak wycieku danych pomiędzy pacjentami.

### Etap D — wszystkie klienty
- Desktop, WebUI, Android i TUI stają się klientami tej samej usługi sesji.
- Każdy klient wyświetla ten sam identyfikator sesji, numer ostatniej próbki i stan połączenia.
- Polecenia z jednego klienta są widoczne w pozostałych.
- Test integracyjny uruchamia jeden serwer i co najmniej dwóch różnych klientów, potwierdzając identyczny identyfikator sesji i sekwencję próbek.

### Etap E — odporność i wdrożenie
- testy długotrwałe 80–100 Hz i docelowych częstotliwości urządzeń;
- pomiar opóźnienia, jittera, utraty próbek i wzrostu pamięci;
- kontrola TLS, RBAC, kopii zapasowej, odtwarzania i audytu.

**Warunek ukończenia:** żaden klient nie otwiera bezpośrednio portu urządzenia w trybie produkcyjnej współdzielonej akwizycji; jedna sesja serwerowa publikuje jeden uporządkowany strumień, który może być równocześnie odbierany i odtwarzany przez wszystkie uprawnione interfejsy.
