# Biofeedback Application for Raspberry Pi 4

A C++17/Qt5 biofeedback application with Arduino Nano/HX711 acquisition, live force visualization, repetition analysis, and sensor-driven training. Hardware calibration and clinical validation remain separate acceptance requirements.

## Features

- **Patient Management**: Create, edit, import, export, and delete patient records
- **Live Measurement**: Real-time graph visualization, validated sample stream, and statistical analysis
- **Repetition and Set Analysis**: Hysteresis-based repetition detection, peak/mean force, duration, time-to-peak, target achievement, and set variability (CV%)
- **Desktop Training Analytics**: Peak-force chart, repetition and series tables, explicit local session-history snapshots, and detailed CSV reports including game points
- **Training Games**: Multiple biofeedback-controlled games including:
  - **Sin/Tan Games** - Wave-based training exercises
  - **Flappy Bird** - Classic flapping game controlled by strain gauge
  - **Pong** - Retro tennis game with biofeedback paddle control
  - **Mario Run** - 4-bit endless runner with charge-based jumping mechanics
- **Data Security**: PBKDF2 password hashing, first-run administrator setup, per-user account storage, and optional encryption for sensitive records
- **Medical Standards**: Comprehensive logging, data validation, and audit trails

## Interfejsy użytkownika

- **Desktop GUI (Qt5)** — główna aplikacja okienkowa: pacjenci, pomiary, trening, analityka powtórzeń i serii, wykres siły, historia sesji oraz raporty CSV.
- **TUI (C++/ncurses)** — terminalowy pulpit z modułami Pulpit, Pomiary, Pacjenci, Trening, Urządzenia, Diagnostyka i Ustawienia; konfiguracja ćwiczenia, celu, serii, powtórzeń i odpoczynku, zapis sesji CSV oraz diagnostyka jakości próbek. Zobacz `docs/TUI_DESIGN.md`.
- **WebUI** — responsywny panel przeglądarkowy w `platforms/webui`; uruchomienie opisano w `platforms/webui/README.md`.
- **Android** — natywny szkielet Kotlin/Jetpack Compose w `platforms/android`; instrukcja w `platforms/android/README.md`.

Pierwsze uruchomienie GUI prowadzi przez utworzenie konta administratora z hasłem o długości co najmniej 12 znaków; nie ma zaszytego domyślnego loginu ani hasła. Konfiguracja, konta i dane pacjentów trafiają do katalogów użytkownika XDG.

Wersja TUI korzysta już ze wspólnego adaptera szeregowego i może wyświetlać rzeczywiste próbki z podłączonego, skalibrowanego czujnika (wykres tekstowy). Desktop GUI prezentuje wykres siły, trendy pomiarowe oraz analizę serii i powtórzeń. WebUI i Android pozostają trybami podglądu: ich wykresy są celowo pustymi stanami, ponieważ nie mają jeszcze strumienia pomiarowego ani wspólnego API sesji. Nie należy traktować tych dwóch interfejsów jako gotowych do rejestracji treningu. Żaden interfejs nie tworzy fikcyjnych danych pacjentów.

## Hardware Requirements

Before force-based training, calibrate the HX711 using a known reference mass and confirm the firmware reports `CAL:1`. See [docs/HX711_CALIBRATION.md](docs/HX711_CALIBRATION.md). Uncalibrated RAW frames are diagnostic only.

- Raspberry Pi 4
- Arduino Nano
- HX711 strain gauge sensor module
- USB cable for Arduino-Raspberry connection

## Software Dependencies

- Qt5 (QtWidgets, QtCharts)
- libserialport
- nlohmann/json
- OpenSSL (for encryption)

### Installing Dependencies on Debian/Ubuntu/Raspberry Pi OS

```bash
sudo apt update && sudo apt upgrade -y

sudo apt install -y qtbase5-dev libqt5charts5-dev libserialport-dev \
    nlohmann-json3-dev libssl-dev git cmake build-essential pkg-config
```

> **Package name note:** on Ubuntu/Debian the Qt Charts development package is usually
> `libqt5charts5-dev`. On some Raspberry Pi OS images it may be exposed as
> `qtcharts5-dev`.

## Installation

For the current Debian/Ubuntu build and DEB package instructions, see [docs/INSTALL_LINUX.md](docs/INSTALL_LINUX.md).

### System Setup

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y qtbase5-dev libqt5charts5-dev libserialport-dev nlohmann-json3-dev libssl-dev git cmake build-essential geany

# Set up serial port permissions
sudo usermod -a -G dialout $USER
```

### Git Clone

```bash
# Clone the repository
git clone <repository-url>
cd biofeedback-app
```

### Build Instructions (Command Line)

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./biofeedback
```

### Mandatory Qt5 Code Example

This project includes mandatory Qt5 example code in:

- `examples/qt5_mandatory.cpp`

Build and run it from `build/`:

```bash
./qt5_mandatory_example
```

For Raspberry Pi specific native Qt5 build notes, see:
https://wiki.qt.io/Native_Build_of_Qt5_on_a_Raspberry_Pi

### Working with Geany IDE

#### Opening the Project in Geany

1. Launch Geany:
   ```bash
   geany &
   ```

2. Open the project file:
   - Go to **File** → **Open...**
   - Navigate to the project directory
   - Select `biofeedback.geany` file
   - Click **Open**

   Alternatively, open from command line:
   ```bash
   geany biofeedback.geany
   ```

#### Configuring Geany Settings

The project includes a pre-configured `biofeedback.geany` file with build settings. To verify or modify settings:

1. Go to **Build** → **Set Build Commands**
2. Verify the following commands are configured:
   - **Compile**: `cd $(dir) && mkdir -p build && cd build && cmake .. && make`
   - **Build**: `cd $(dir)/build && make`
   - **Execute**: `$(dir)/build/biofeedback`

3. Optional custom settings:
   - Go to **Edit** → **Preferences**
   - Set indentation width to 4 spaces
   - Enable auto-indentation
   - Set long line column to 80

#### Building and Running in Geany

After opening the project:

1. **Build the project**:
   - Press **F8** (Build) or go to **Build** → **Build**
   - Or use the custom menu: **Build** → **Build Release** for optimized build
   - Or use **Build** → **Clean Build** to start fresh

2. **Run the application**:
   - Press **F5** (Execute) or go to **Build** → **Execute**
   - Or use the custom menu: **Build** → **Run Application**

3. **Available build options** (in Build menu):
   - **Clean Build** - Remove build directory and recreate
   - **Configure Only** - Run CMake without building
   - **Build Debug** - Build with debug symbols
   - **Build Release** - Build with optimizations
   - **Run Application** - Execute the built application

#### Quick Start in Geany

```bash
# Open project in Geany
cd biofeedback-app
geany biofeedback.geany

# Then press F8 to build, F5 to run
```

## Project Structure

```
biofeedback-app/
├── src/                    # Source files
│   ├── core/              # Core functionality
│   ├── gui/               # GUI components
│   ├── tabs/              # Tab implementations
│   ├── sensor/            # Sensor communication
│   ├── games/             # Game implementations
│   └── data/              # Data management
├── include/               # Header files
├── config/                # Configuration files
├── data/                  # Patient data storage
├── resources/             # Images, icons, multimedia
├── build/                 # Build output
└── docs/                  # Documentation
    └── MATHEMATICAL_FORMULAS.md  # Biomechanical formulas
```

## Arduino Nano Communication Protocol

### Format Wysyłania i Odbioru Danych z Arduino Nano

Komunikacja z Arduino Nano (plik `hx711_advanced.ino`) odbywa się przez port szeregowy z następującym formatem:

#### Konfiguracja Portu Szeregowego
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None ('N')
- **Stop Bits**: 1
- **Port**: `/dev/ttyUSB0` lub `/dev/ttyACM0` (automatyczne wykrywanie)

#### Komendy Wysyłane do Arduino (TX - Raspberry Pi → Arduino)

| Komenda | Opis | Przykład |
|---------|------|----------|
| `ID` lub `WHOAMI` | Pobranie identyfikacji urządzenia | `ID\n` |
| `TARE` lub `CALZERO` | Wyzerowanie wagi (tare) | `TARE\n` |
| `DATA` lub `READ` | Żądanie danych binarnych + odczyt | `DATA\n` |
| `FORCE` | Żądanie odczytu siły w formacie tekstowym | `FORCE\n` |
| `CONT` | Włączenie trybu ciągłego (80Hz) | `CONT\n` |
| `STOP` | Wyłączenie trybu ciągłego | `STOP\n` |
| `CAL:<grams>` | Kalibracja ze znaną wagą | `CAL:100.0\n` |
| `RESET` | Reset urządzenia przez watchdog | `RESET\n` |

#### Odpowiedzi z Arduino (RX - Arduino → Raspberry Pi)

**1. Odpowiedź na komendę ID:**
```
DEVICE:Advanced HX711 Weight Scale
ID:HX711-SCALE-V1.0
FW:1.0,BR:115200
OK
```

**2. Dane binarne (pakiet 9 bajtów + markery końca):**
```cpp
struct DataPacket {
    uint32_t timestamp;  // 4 bajty - czas w milisekundach
    int32_t value;       // 4 bajty - wartość surowa z czujnika
    uint8_t crc;         // 1 bajt - suma kontrolna CRC-8
};
// Po pakiecie: 0xAA 0x55 (markery końca ramki)
```

**3. Odpowiedź tekstowa FORCE:**
```
FORCE:123.45g,RAW:6789,TS:1234567ms
```

**4. Potwierdzenia:**
- `Taring...` / `Tare set!` - potwierdzenie zerowania
- `Continuous mode ON/OFF` - status trybu ciągłego
- `Calibrated! Factor:X.XXXXXX` - potwierdzenie kalibracji
- `[DATA_SENT]` - potwierdzenie wysłania danych

#### Suma Kontrolna CRC-8

Arduino używa CRC-8 z wielomianem x^8 + x^2 + x + 1 (0x07):
- Wartość początkowa: 0xFF
- Obliczana dla timestamp + value (8 bajtów)
- CRC jest ostatnim bajtem pakietu

#### Tryb Ciągły (Continuous Mode)

Po wysłaniu komendy `CONT`, Arduino wysyła dane binarne z częstotliwością 80Hz:
- Pakiet binarny co ~12.5ms
- Każdy pakiet zawiera timestamp, wartość i CRC
- Markery końca ramki: 0xAA 0x55

#### Przykład Sekwencji Komunikacji

```cpp
// 1. Połączenie i identyfikacja
send("ID\n");
// Oczekuj: DEVICE:..., ID:..., FW:...

// 2. Zerowanie
send("TARE\n");
// Oczekuj: Taring... Tare set!

// 3. Rozpoczęcie pomiaru ciągłego
send("CONT\n");
// Oczekuj: Continuous mode ON

// 4. Odbieranie danych (80Hz)
while (measuring) {
    readBinaryPacket();  // 9 bajtów + 2 markery
    verifyCRC();
    processForceData();
}

// 5. Zatrzymanie
send("STOP\n");
```

## Mathematical Formulas

For detailed mathematical formulas used to calculate biomechanical and training parameters from strain gauge data, see:

- **[MATHEMATICAL_FORMULAS.md](MATHEMATICAL_FORMULAS.md)** - Complete documentation of 65+ parameters with LaTeX notation including:
  - Force metrics (peak force, mean force, force range)
  - Time-based parameters (rep duration, phase times, rest periods)
  - Power and energy calculations
  - Variability and stability indices
  - Performance and progress tracking
  - Advanced composite scores

- **[MATHEMATICAL_REPORT.docx](MATHEMATICAL_REPORT.docx)** - Graphically formatted report document (.docx) containing:
  - All mathematical formulas with professional formatting
  - Variable legend tables
  - Implementation examples in Python for Raspberry Pi
  - Structured sections for each parameter category
  - Ready for printing or sharing as formal documentation

- **[REPORT_FORMULAS.md](REPORT_FORMULAS.md)** - Markdown version of the graphical report with full LaTeX rendering support

The formulas are designed for analyzing resistance band exercises with 3 sets × 8 repetitions, captured via HX711 strain gauge sensor on Raspberry Pi.

## Architektura aplikacji i przepływ danych

Biofeedback składa się z kilku interfejsów użytkownika, warstwy akwizycji pomiarów,
logiki treningowej, firmware urządzeń oraz rozwijanej usługi wymiany danych.
Należy rozróżniać funkcje już działające od elementów przygotowanych jako klient
lub kontrakt przyszłej integracji. Obecnie nie wszystkie platformy współdzielą
jeszcze dane przez serwer.

### 1. Desktop GUI — Linux i docelowo Windows

Główna aplikacja okienkowa jest napisana w C++17 z użyciem Qt5. Jej główne obszary
funkcjonalne to:

- **Logowanie i konta** — pierwsze uruchomienie prowadzi przez utworzenie konta
  administratora; hasło konta jest przechowywane w postaci skrótu PBKDF2.
- **Pulpit i nawigacja** — główne okno udostępnia moduły aplikacji.
- **Pacjenci** — formularz kart pacjentów, operacje dodawania, odczytu, edycji
  i usuwania oraz import i eksport CSV. Obecna integracja Desktop korzysta
  z lokalnego magazynu plikowego.
- **Pomiary** — połączenie z urządzeniem, prezentacja bieżącego przebiegu siły,
  tabela próbek i statystyki pomiarowe.
- **Trening** — konfiguracja ćwiczenia, celu, liczby serii i powtórzeń,
  odpoczynku oraz analiza wykonania.
- **Gry** — wybór gry biofeedbackowej i sterowanie rozgrywką sygnałem z czujnika.
- **Diagnostyka i ustawienia** — konfiguracja portu, parametrów urządzenia,
  kalibracji i zachowania aplikacji.

Kod głównego okna i warstwa aplikacyjna nadal wymagają dalszego rozdzielenia
odpowiedzialności. Kod współdzielony dla systemów Windows i Linux nie oznacza
jeszcze potwierdzonego wydania Windows: osobna kompilacja MSVC, instalator
i testy Windows muszą być weryfikowane niezależnie.

### 2. TUI — interfejs terminalowy

TUI jest klientem C++17 opartym na ncurses. Korzysta ze wspólnego adaptera
komunikacji szeregowej i silnika pomiarowego, a nie z własnego parsera ramek.
Udostępnia ekrany:

- Pulpit — status programu, połączenia i ostatniej próbki.
- Pomiary — aktualna wartość siły i krótki przebieg ostatnich próbek.
- Pacjenci — informacja o stanie integracji repozytorium kart.
- Trening — konfiguracja i przebieg sesji, serie, powtórzenia, cel siły i odpoczynek.
- Urządzenia — skanowanie portów, wybór urządzenia, połączenie i rozłączenie.
- Diagnostyka — liczba próbek prawidłowych i odrzuconych, jakość ostatniej próbki
  oraz podstawowe statystyki ostatniego powtórzenia.
- Ustawienia — skróty i parametry transmisji.

Sterowanie obejmuje m.in. strzałki i Enter do nawigacji, C do połączenia,
D do rozłączenia, R do ponownego skanowania, S do startu/zatrzymania,
P do pauzy/wznowienia, a także klawisze zmiany celu, serii i powtórzeń.
Dostępne są również tryby `--status`, `--list-ports`, `--version` i `--help`.

TUI odczytuje listę pacjentów i zapisuje ukończone sesje treningowe
przez uwierzytelnione API do wspólnego repozytorium. Sesja zawiera parametry
ćwiczenia oraz statystyki i metadane powtórzeń. Obecnie TUI nie wybiera jeszcze
pacjenta, więc rekord treningu nie ma powiązania `patient_id`; pełne powiązanie
pacjent–sesja oraz osobna kolekcja wyników treningowych pozostają do wdrożenia.

### 3. WebUI — interfejs przeglądarkowy

WebUI jest responsywną aplikacją HTML/CSS/JavaScript w katalogu
`platforms/webui`. Oferuje nawigację modułową opartą o adresy URL, motyw jasny
i ciemny oraz klienta API REST/WebSocket. Klient ma przygotowane metody do
obsługi statusu i urządzeń, pacjentów, sesji pomiarowych i subskrypcji zdarzeń.

Uruchomienie statycznego interfejsu:

```bash
python3 -m http.server 8080 --directory platforms/webui
```

Otwórz `http://localhost:8080`. Interfejs sprawdza dostępność
`GET /api/v1/status` i jawnie pokazuje brak serwera. Nie tworzy fikcyjnych
pacjentów, pomiarów ani wyników.

**Ograniczenie:** klient WebUI jest przygotowany do integracji, ale backend API
nie udostępnia jeszcze zasobów pacjentów, pomiarów, treningów ani gier.
Serwerowy WebSocket również nie jest jeszcze zaimplementowany. Samo uruchomienie
statycznej strony nie uruchamia serwera danych.

### 4. Android GUI

Aplikacja Android jest natywnym klientem Kotlin/Jetpack Compose z Material 3.
Wersja 0.1 zawiera responsywną nawigację dolną oraz widoki:

- Pulpit,
- Pomiary,
- Pacjenci,
- Trening,
- Urządzenia.

Są to obecnie ekrany demonstracyjne/podglądowe. Aplikacja nie ma jeszcze
komunikacji z serwerem API ani obsługi Bluetooth, USB lub portu szeregowego.
Nie należy traktować jej jako gotowego rejestratora pomiarów lub dokumentacji
pacjenta. Docelowo ma korzystać z tego samego wersjonowanego API co WebUI,
Desktop i TUI.

Budowanie opisano w `platforms/android/README.md`; wymagane są JDK 17
i Android SDK Platform 35.

### 5. Serwer wymiany danych i API

Docelowy model zakłada jeden wspólny serwer danych dla wszystkich klientów.
Serwer jest właścicielem trwałych rekordów i udostępnia je przez wersjonowane
API, zamiast pozwalać klientom na bezpośredni dostęp do plików serwera lub bazy.

Docelowe odpowiedzialności serwera:

- repozytorium kart pacjentów i mapowanie tożsamości;
- archiwum pomiarów, próbek i metadanych sesji;
- plany treningowe, sesje, checkpointy i wyniki;
- sesje gier, punktacja i historia wyników;
- uwierzytelnianie, autoryzacja do konkretnych zasobów i rejestr audytowy;
- kopie zapasowe, odtwarzanie, eksport, kontrola wersji rekordu i konfliktów;
- powiadomienia o zmianach i zdarzeniach czasu rzeczywistego.

Docelowe rodziny endpointów obejmują `/api/v1/patients`,
`/api/v1/measurements`, `/api/v1/training-sessions`,
`/api/v1/training-results`, `/api/v1/game-sessions`,
`/api/v1/game-results` oraz kanał zdarzeń WebSocket lub SSE.

**Stan obecny:** serwer udostępnia health/status, logowanie i wylogowanie
oraz CRUD sześciu kolekcji: pacjenci, pomiary, sesje treningowe, wyniki treningów,
sesje gier i wyniki gier. Aktualizacja i usunięcie wymagają numeru wersji rekordu;
konflikt zwracany jest jako HTTP 409. Desktop uruchamia API loopback po
inicjalizacji i współdzieli z nim tę samą instancję repozytorium. Endpointy
danych są obecnie ograniczone do roli `administrator` do czasu wdrożenia
pełnych zakresów RBAC. Serwer nie udostępnia jeszcze WebSocket/SSE.

WebUI ma klienta CRUD dla wszystkich kolekcji. Android ma klienta CRUD,
a ekran Pacjenci obsługuje odczyt, dodawanie, edycję i usuwanie kart.
TUI nie jest jeszcze klientem API. Producenci danych pomiarowych, treningowych
i wyników gier w Desktop/TUI nie są jeszcze w pełni przepięci na wspólny zapis
serwerowy. Serwer nasłuchuje domyślnie wyłącznie na loopback; zdalny Android
wymaga bramy HTTPS/WSS. Nie wystawiaj prototypu HTTP bezpośrednio do sieci publicznej.

Szczegółowy opis decyzji i etapów wdrożenia znajduje się w
[docs/ARCHITECTURE_SHARED_DATA_SERVER.md](docs/ARCHITECTURE_SHARED_DATA_SERVER.md).

### 6. Arduino Nano, HX711 i Raspberry Pi Pico

**Arduino Nano + HX711** stanowią aktualną ścieżkę pozyskiwania pomiaru siły.
Arduino odczytuje przetwornik HX711 i przesyła dane do komputera przez port
szeregowy. Typowe parametry łącza to 115200 bit/s, 8N1. Firmware udostępnia
m.in. identyfikację urządzenia, zerowanie (tare), odczyt, tryb ciągły, kalibrację
znaną masą i reset. Strumień binarny zawiera 32-bitowy znacznik czasu,
32-bitową wartość surową, CRC-8 i znaczniki końca ramki `0xAA 0x55`.
Dostępna jest także odpowiedź tekstowa z odczytem siły.

Przed użyciem pomiaru siły wymagane jest zerowanie i kalibracja znaną masą.
Surowe wartości ADC bez potwierdzonej kalibracji są danymi diagnostycznymi,
a nie wiarygodną siłą w niutonach. Szczegóły: [docs/HX711_CALIBRATION.md](docs/HX711_CALIBRATION.md).

**Raspberry Pi Pico** ma obecnie firmware komunikacyjny UART0: GP0 TX, GP1 RX,
115200 bit/s, 8N1, watchdog 2 s oraz ograniczony parser poleceń diagnostycznych
PING, STATUS i SAFE. Akwizycja czujników, kalibracja i generowanie ramek
pomiarowych w tym firmware nie są jeszcze zaimplementowane ani zweryfikowane
na sprzęcie. Kanału diagnostycznego Pico nie wolno utożsamiać ze strumieniem
pomiarowym HX711. Szczegóły: [firmware/pico/README.md](firmware/pico/README.md)
i [docs/PICO_FIRMWARE_MEASUREMENT_STATUS.md](docs/PICO_FIRMWARE_MEASUREMENT_STATUS.md).

### 7. Rejestracja pomiaru, walidacja i wykres

Główna ścieżka pomiarowa Desktop przebiega następująco:

```text
HX711 -> Arduino Nano -> port szeregowy
  -> SerialCommunication
  -> MeasurementSerialAdapter
  -> MeasurementEngine
  -> MeasurementTab
  -> RepetitionDetector
  -> statystyki powtórzenia i serii
  -> ForcePlotWidget + tabela próbek + tabela parametrów
  -> eksport JSON / CSV
```

`SerialCommunication` odbiera dane z portu, a `MeasurementSerialAdapter`
mapuje dane sterownika na wspólny model wejściowy. `MeasurementEngine`
nadaje próbkom numer sekwencyjny i czas odbioru hosta UTC, zachowuje czas
urządzenia, wartość surową, wartość przeliczoną, jednostkę, źródło i status
jakości. Silnik normalizuje wyłącznie jawnie obsługiwane jednostki, sprawdza
wartości niefinitywne, znaczniki czasu, źródło i skonfigurowany zakres.
Próbki odrzucone są oznaczane statusem jakości; konsument musi sprawdzić
poprawność próbki przed wykreśleniem, agregacją lub eksportem.

Desktop używa `ForcePlotWidget` do prezentacji przebiegu siły w czasie.
Widok pomiarowy obejmuje tabelę ostatnich próbek oraz tabele statystyk
powtórzeń/serii. Wykres korzysta z próbek sesji i osi czasu, a nie z danych
wymyślonych. Bufory interfejsu są ograniczane, aby długotrwały pomiar nie
powodował nieograniczonego wzrostu pamięci.

`MeasurementEngine` utrzymuje ograniczoną historię w pamięci RAM (domyślnie
512 próbek); nie jest samodzielnym archiwum długoterminowym. W Desktop pełny
strumień bieżącej sesji może zostać ręcznie wyeksportowany do JSON, natomiast
CSV służy przede wszystkim do eksportu tabeli statystyk. Nagłe zamknięcie
aplikacji przed eksportem może utracić bieżącą sesję. Trwały, przyrostowy
dziennik próbek i automatyczna synchronizacja przez serwer pozostają do
wdrożenia.

TUI subskrybuje wspólny silnik i prezentuje skrócony przebieg próbek w terminalu.
WebUI i Android nie otrzymują jeszcze rzeczywistego strumienia pomiarowego
z backendu. Szczegółowy opis: [docs/MEASUREMENT_PIPELINE.md](docs/MEASUREMENT_PIPELINE.md)
oraz [docs/REALTIME_MEASUREMENT_AUDIT.md](docs/REALTIME_MEASUREMENT_AUDIT.md).

### 8. Obliczanie parametrów biomechanicznych

`RepetitionDetector` wykorzystuje próg aktywacji i niższy próg zwolnienia
(histereza), aby wyznaczać początek i koniec powtórzenia. Rekord powtórzenia
zawiera m.in. czas początku i końca, czas trwania, czas do osiągnięcia szczytu,
siłę szczytową, średnią siłę, liczbę próbek i próbki źródłowe.

Podsumowanie serii obejmuje m.in. liczbę powtórzeń, średnią i maksymalną siłę
szczytową, średni czas powtórzenia, średni czas do szczytu, współczynnik
zmienności siły szczytowej oraz procent realizacji celu. Dodatkowe wzory
biomechaniczne są zebrane w module `Formulas` i dokumentacji matematycznej.
Parametry wymagające kalibracji, dodatkowego czujnika, historii lub
wystarczającej liczby próbek powinny być oznaczane statusem dostępności;
brak danych nie może być prezentowany jako wynik równy zero.

Katalog modelu obejmuje 65 nazw parametrów, ale nie oznacza to, że wszystkie
są już w pełni obliczane, wyświetlane i archiwizowane w każdym widoku.
Dostępność zależy od danych wejściowych i stopnia integracji danego parametru.
Nie należy interpretować wyniku matematycznego jako diagnozy klinicznej.
Opis wzorów: [MATHEMATICAL_FORMULAS.md](MATHEMATICAL_FORMULAS.md).

### 9. Trening i analiza sesji

Logika treningowa obejmuje konfigurację ćwiczenia, celu siły, serii,
powtórzeń i przerw. `SessionStateMachine` modeluje stany Idle, Active,
Paused, Resting, Completed i Interrupted. `TrainingSessionClock` odpowiada
za pomiar czasu sesji, a `RepetitionDetector` wykrywa powtórzenia na podstawie
próbkowanej siły i progów histerezy.

Po zakończeniu powtórzeń można analizować siłę szczytową i średnią, czas
powtórzenia, czas do szczytu, realizację celu oraz zmienność pomiędzy
powtórzeniami. Podsumowania serii są wykorzystywane w analizie treningowej
i mogą być eksportowane z Desktop. TUI zapisuje własną lokalną historię CSV.
Wspólne repozytorium sesji i wyników dla wszystkich platform jest elementem
docelowego serwera danych, a nie ukończoną funkcją bieżącej wersji.

### 10. Gry biofeedbackowe

Gry korzystają ze wspólnego `GameEngine`: monotonicznego zegara, stałego kroku
symulacji, ograniczonego nadrabiania czasu, wspólnego cyklu życia oraz
obsługi wejścia z czujnika. Próbki czujnika są wejściem sterującym; nie powinny
bezpośrednio modyfikować widgetów z wątku portu szeregowego. Gry obsługują
lub rozwijają obsługę startu, pauzy, wznowienia, zatrzymania, resetu,
zmiany rozmiaru i nieprawidłowych danych wejściowych.

Dostępne identyfikatory gier w fabryce:

| Gra | Charakter rozgrywki |
|---|---|
| **Sin Force Trainer** | Ćwiczenie sterowania siłą według przebiegu sinusoidalnego. |
| **Tan Force Trainer** | Ćwiczenie sterowania siłą według przebiegu tangensoidalnego. |
| **Pong** | Sterowanie paletką siłą z czujnika; odbijanie piłki i punktacja. |
| **Arkanoid** | Sterowanie paletką, odbijanie piłki i niszczenie bloków. |
| **Flappy Bird** | Sterowanie ruchem ptaka sygnałem siły, omijanie przeszkód. |
| **Mario Run** | Bieg z przeszkodami i skokami sterowanymi wejściem biofeedback. |
| **Galaga** | Strzelanka z falami przeciwników, pociskami, życiami i punktacją. |
| **Archer** | Strzelanie z łuku do ruchomych celów; regulacja kąta i punktacja. |
| **Car Race** | Wyścig z przeszkodami i sterowaniem pozycją pojazdu na pasach. |
| **Scorched Earth** | Turowa artyleria z kątem, siłą strzału, grawitacją, wiatrem i terenem. |

Gry są dostępne przez `GameFactory` i selektory interfejsu Desktop. Obecność
gry w fabryce nie oznacza automatycznie, że każda platforma ma już jej klienta
lub że każda mechanika przeszła pełne testy sprzętowe. Testy scenariuszowe,
balans rozgrywki i walidacja z fizycznym czujnikiem pozostają osobnymi etapami.
Wyniki gier nie są jeszcze zapisywane we wspólnym repozytorium serwera.

### 11. Dane pacjentów i prywatność

Desktop i serwer API korzystają ze wspólnego `JsonResourceRepository`
w katalogu danych aplikacji `shared-data`. Karty pacjentów mają identyfikatory
UUID, metadane wersji i znaczniki czasu; nazwy plików nie ujawniają PESEL.
Zapis jest atomowy przez Qt `QSaveFile`, a katalogi i pliki otrzymują
uprawnienia tylko dla właściciela. Pierwsze uruchomienie migruje stare pliki
pacjentów do repozytorium; źródła migracji pozostają zachowane do czasu
usunięcia karty przez operatora.

Gdy `encryption.enabled=true`, rekordy repozytorium są szyfrowane AES-256-GCM
kluczem `BIOFEEDBACK_ENCRYPTION_KEY`. W profilu deweloperskim szyfrowanie może
pozostawać wyłączone; przed użyciem rzeczywistych danych pacjentów wymagane
jest włączenie szyfrowania, bezpieczne przechowanie klucza i sprawdzenie kopii
zapasowych. Brak klucza przy włączonym szyfrowaniu powoduje fail-closed.
Nie należy przechowywać klucza w repozytorium.

Wybór `storage.patient_backend` nadal dotyczy backendu kart pacjentów.
`files` jest obsługiwany; adapter MariaDB nie jest jeszcze dostępny.
Kolekcje pomiarów, treningów i gier mają wspólny serwerowy CRUD, lecz integracja
producentów tych danych i klienta TUI pozostaje w toku. Nie należy zakładać
pełnej synchronizacji wszystkich pięciu interfejsów, dopóki te etapy nie
zostaną ukończone i potwierdzone testami integracyjnymi.

## Configuration

Edit `config/config.json` to customize:
- Serial port settings
- Data storage paths
- Encryption settings
- UI preferences

## Wybór miejsca przechowywania danych pacjentów

Sposób przechowywania danych pacjentów wybiera się w pliku `config/config.json`,
w sekcji `storage`, za pomocą pola `patient_backend`.

### Zapis w plikach — opcja działająca

Aby przechowywać dane pacjentów w plikach na dysku, ustaw:

```json
"storage": {
  "patient_backend": "files",
  "mariadb": {
    "host": "127.0.0.1",
    "port": 3306,
    "database": "biofeedback",
    "username": "biofeedback_app",
    "password_env": "BIOFEEDBACK_MARIADB_PASSWORD",
    "tls": true,
    "connect_timeout_seconds": 5
  }
}
```

Wartość `"files"` jest ustawieniem domyślnym. Lokalizacja danych wynika z
ustawienia `data.base_path` (w przykładowej konfiguracji: `./data/patients`).
Wybór backendu plikowego nie wymaga konfiguracji MariaDB ani hasła do bazy.

### MariaDB — konfiguracja docelowa, adapter jeszcze niedostępny

Aby wskazać MariaDB jako wybrany backend, zmień:

```json
"patient_backend": "mariadb"
```

Parametry połączenia znajdują się w `storage.mariadb`:

| Pole | Znaczenie |
|---|---|
| `host` | Adres serwera MariaDB, np. `127.0.0.1` |
| `port` | Port serwera, domyślnie `3306` |
| `database` | Nazwa bazy danych, domyślnie `biofeedback` |
| `username` | Użytkownik bazy, domyślnie `biofeedback_app` |
| `password_env` | Nazwa zmiennej środowiskowej zawierającej hasło |
| `tls` | Żądanie używania szyfrowanego połączenia TLS |
| `connect_timeout_seconds` | Limit czasu na połączenie w sekundach |

**Ważny stan implementacji:** bieżąca wersja waliduje wybór `mariadb` i jego
parametry, ale nie zawiera jeszcze sterownika ani adaptera repozytorium MariaDB.
Po wybraniu `"mariadb"` aplikacja celowo kończy inicjalizację z czytelnym błędem.
Nie zapisuje wtedy danych zastępczo do plików. Samo ustawienie hasła i parametrów
połączenia nie uruchamia jeszcze obsługi bazy. Do czasu wdrożenia adaptera używaj
`"files"`.

### Ustawianie hasła MariaDB

Hasła **nie wpisuj** do `config.json`, `config.json.example`, kodu źródłowego
ani repozytorium. Konfiguracja przechowuje wyłącznie nazwę zmiennej środowiskowej:
`BIOFEEDBACK_MARIADB_PASSWORD`. Nazwa ta musi odpowiadać wartości
`storage.mariadb.password_env`.

Linux — ustawienie tymczasowe dla bieżącej sesji terminala:

```bash
read -rsp "Hasło MariaDB: " BIOFEEDBACK_MARIADB_PASSWORD
echo
export BIOFEEDBACK_MARIADB_PASSWORD
```

Zmienna będzie dostępna dla procesów uruchomionych z tego terminala. Po zamknięciu
sesji ustaw ją ponownie. Nie umieszczaj hasła bezpośrednio w poleceniu, ponieważ
może ono zostać zapisane w historii powłoki.

Windows PowerShell — ustawienie tymczasowe dla bieżącego okna:

```powershell
$securePassword = Read-Host "Hasło MariaDB" -AsSecureString
$ptr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($securePassword)
try {
    $env:BIOFEEDBACK_MARIADB_PASSWORD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr)
}
finally {
    [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr)
}
```

To ustawienie obowiązuje tylko w bieżącym procesie PowerShell i procesach
uruchomionych z niego. Zakończenie sesji usuwa zmienną z tego środowiska.

Na serwerze produkcyjnym ustaw zmienną w bezpiecznym menedżerze sekretów lub
mechanizmie zarządzania usługą, z dostępem ograniczonym do konta uruchamiającego
Biofeedback. Nie zapisuj jawnego hasła w plikach konfiguracyjnych projektu,
dziennikach ani systemie kontroli wersji. Używaj odrębnego konta MariaDB
o minimalnych uprawnieniach wymaganych przez aplikację.

## Data Storage

Patient data is organized as:
```
data/
└── patients/
    └── [patient_id]/
        ├── profile.json
        ├── measurements/
        └── exercises/
```

## Safety & Compliance

This application follows medical software standards:
- Data validation and integrity checks
- Comprehensive error handling
- Audit logging
- User authentication ready
- Data backup capabilities

## License

[Specify your license]

## Contributing

Please read CONTRIBUTING.md for details on our code of conduct and the process for submitting pull requests.

## Disclaimer

This software is for research and development purposes. For medical use, proper certification and regulatory approval are required.


## Lokalny serwer API

Minimalny serwer statusu HTTP (Qt Network) można zbudować i uruchomić:

    cmake -S . -B build
    cmake --build build --target biofeedback_api
    ./build/biofeedback_api --port 8765

Usługa nasłuchuje wyłącznie na `127.0.0.1` i udostępnia `/healthz` oraz
`/api/v1/status`. Sterowanie sprzętem, dane pacjentów i strumień pomiarowy
pozostają wyłączone do czasu wdrożenia uwierzytelniania, autoryzacji oraz
wspólnych usług domenowych. Nie wystawiaj tego prototypu na sieć publiczną.
