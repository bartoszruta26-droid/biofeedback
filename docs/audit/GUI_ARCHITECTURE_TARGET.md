# Docelowa architektura interfejsów Biofeedback

## Cel

Wspólny rdzeń domenowy i Application Services dla Desktop, TUI, Web UI i Android. Interfejsy są adapterami prezentacji; nie implementują własnych reguł pomiarowych ani protokołu HX711.

## Warstwy

1. Presentation: Qt Widgets, ncurses, Web UI, Jetpack Compose.
2. Client adapters: ViewModel/Presenter, klient HTTP, klient zdarzeń.
3. Application Services: PatientService, MeasurementService, SessionService, TrainingService, DeviceService, CalibrationService, DiagnosticsService, UserService.
4. Domain: encje, reguły, walidacja, jednostki, stan sesji, wyniki pomiarów.
5. Infrastructure: repozytoria, logowanie, szyfrowanie, API, transport zdarzeń.
6. Hardware Abstraction Layer: SerialTransport, HX711, Arduino Nano, przyszłe adaptery USB/Bluetooth.

## Zasady

- Jeden właściciel połączenia z urządzeniem i jeden strumień pomiarowy.
- Jawny stan urządzenia: disconnected, connecting, ready, measuring, stopping, fault.
- Próbka pomiarowa zawiera identyfikator sesji, źródło, monotoniczny czas, czas UTC, wartość, jednostkę, status jakości i wersję kalibracji.
- Interfejsy subskrybują zdarzenia; nie odczytują równolegle tego samego portu.
- Komendy sprzętowe są walidowane, autoryzowane i audytowane.
- WebSocket/SSE nie zastępuje trwałego zapisu pomiarów.
- Brak danych nie może być zastępowany syntetycznymi próbkami.
- Wspólne DTO i wersjonowanie API są dokumentowane i testowane kontraktowo.

## Topologia

Desktop/TUI mogą używać tych samych Application Services lokalnie. Web i Android komunikują się z usługą API. Wariant lokalny na Raspberry Pi powinien domyślnie nasłuchiwać na loopback; dostęp LAN wymaga jawnej konfiguracji TLS, uwierzytelnienia i zapory.

## Kolejność wdrożenia

1. Wyodrębnić interfejsy usług bez zmiany zachowania istniejącego Desktop/TUI.
2. Dodać implementację usług i testy jednostkowe.
3. Wprowadzić DTO i kontrakt API wersji v1.
4. Dodać uwierzytelnianie, autoryzację i audyt komend.
5. Dodać strumień pomiarowy z kontrolą backpressure i odtwarzaniem/reconnect.
6. Przenieść Web UI, następnie Android na API.
7. Dodać testy kontraktowe i E2E.

## Bezpieczeństwo i niezawodność

Zasada najmniejszych uprawnień, brak danych osobowych w logach, szyfrowanie danych w spoczynku i w transmisji, limity rozmiaru żądań, limity częstotliwości, walidacja parametrów urządzenia, bezpieczne zatrzymanie pomiaru i testy utraty łączności.
