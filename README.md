# Biofeedback Application for Raspberry Pi 4

A professional medical-grade biofeedback application built with C++, Qt5, and Arduino Nano integration for strain gauge sensor (HX711) data acquisition.

## Features

- **Patient Management**: Create, edit, import, export, and delete patient records
- **Live Measurement**: Real-time graph visualization of sensor data with statistical analysis
- **Training Games**: Multiple biofeedback-controlled games including:
  - **Sin/Tan Games** - Wave-based training exercises
  - **Flappy Bird** - Classic flapping game controlled by strain gauge
  - **Pong** - Retro tennis game with biofeedback paddle control
  - **Mario Run** - 4-bit endless runner with charge-based jumping mechanics
- **Data Security**: JSON storage with optional encryption for sensitive medical data
- **Medical Standards**: Comprehensive logging, data validation, and audit trails

## Hardware Requirements

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

## Configuration

Edit `config/config.json` to customize:
- Serial port settings
- Data storage paths
- Encryption settings
- UI preferences

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
