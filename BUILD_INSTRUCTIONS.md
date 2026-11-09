# Biofeedback — instrukcja budowania i konfiguracji

> **Uwaga o aktualności:** ten dokument został skorygowany względem bieżącego firmware i konfiguracji repozytorium. Główną instrukcją instalacji Linux jest również [`docs/INSTALL_LINUX.md`](docs/INSTALL_LINUX.md). Nie stosuj starych wartości portu 9600 ani historycznego szkicu Arduino.

## Prerequisites

### Hardware
- Raspberry Pi 4 (4GB RAM recommended)
- Arduino Nano
- HX711 strain gauge sensor module
- MicroSD card (16GB minimum, Class 10)
- USB cable for Arduino connection

### Software
- Raspberry Pi OS (64-bit, Bullseye or later)
- C++ compiler with C++17 support (g++ 10+)
- Qt5 development libraries
- Additional dependencies

## System Setup

### 1. Update System
```bash
sudo apt update
sudo apt upgrade -y
sudo reboot
```

### 2. Install Dependencies
```bash
sudo apt install -y \
    build-essential \
    cmake \
    git \
    qtbase5-dev \
    libqt5charts5-dev \
    libserialport-dev \
    nlohmann-json3-dev \
    libssl-dev \
    pkg-config
```

**Note**: Qt Charts is required for graph visualization and game rendering.
Depending on distro naming, install either:

- `libqt5charts5-dev` (Ubuntu/Debian)
- `qtcharts5-dev` (some Raspberry Pi OS images)

### 3. Configure Serial Port Permissions
```bash
# Dodaj bieżącego użytkownika do grupy dostępu do portów szeregowych
sudo usermod -aG dialout "$USER"

# Sprawdź port urządzenia
ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Wyloguj się i zaloguj ponownie, aby zastosować zmianę grup. Nie twórz reguły udev nadającej wszystkim użytkownikom uprawnienia `0666` do portu urządzenia.

### 4. Verify Qt5 Installation
```bash
pkg-config --modversion Qt5Core
# Should return 5.x.x

pkg-config --modversion Qt5Charts
# Should return 5.x.x

dpkg -s libqt5charts5-dev | head -n 5
# Should show: Status: install ok installed
```

### 5. Raspberry Pi Native Qt5 Reference

For Raspberry Pi native Qt5 builds, use the official Qt Wiki guide:

- https://wiki.qt.io/Native_Build_of_Qt5_on_a_Raspberry_Pi

This repository now also ships a mandatory Qt5 code sample in:

- `examples/qt5_mandatory.cpp`

## Building the Application

### Option 1: Using CMake (Recommended)

```bash
# Navigate to project directory
cd /path/to/biofeedback

# Create build directory
mkdir build && cd build

# Configure build
cmake ..

# Build
make -j$(nproc)
```

After building with Qt5 available, you can run the mandatory Qt5 sample:

```bash
./qt5_mandatory_example
```

### Dostępna metoda budowania

Repozytorium nie zawiera obecnie pliku `Makefile` do samodzielnego budowania. Używaj CMake (opcja 1); nie uruchamiaj `make` poza katalogiem kompilacji wygenerowanym przez CMake.

## Running the Application

### First Run
```bash
# From build directory
./biofeedback

# Or if installed
biofeedback
```

### Troubleshooting

#### Serial Port Issues
```bash
# Check if Arduino is detected
dmesg | grep tty

# Test serial communication
sudo chmod 666 /dev/ttyUSB0
```

#### Qt5 Display Issues
```bash
# Ensure you're running in a graphical environment
# Check DISPLAY variable
echo $DISPLAY

# If empty, set it
export DISPLAY=:0
```

#### Permission Denied
```bash
# Re-add user to dialout group and reboot
sudo usermod -a -G dialout $USER
sudo reboot
```

## Arduino Setup

### 1. Install Arduino IDE (Optional)
```bash
sudo apt install -y arduino
```

### 2. Upload Firmware
1. Otwórz plik [`hx711_advanced.ino`](hx711_advanced.ino) w Arduino IDE. To aktualny firmware zgodny z parserem ramek binarnych aplikacji.
2. Wybierz płytkę Arduino Nano oraz właściwy procesor/bootloader dla używanej wersji płytki.
3. Wybierz port urządzenia, np. `/dev/ttyUSB0` lub `/dev/ttyACM0`.
4. Skompiluj szkic i wgraj go do Arduino.

Plik `hx711_arduino_nano.ino` jest historycznym przykładem tekstowym i nie jest zgodny z bieżącym parserem ramek binarnych. Nie używaj go do aktualnej aplikacji.

### 3. Verify Communication
```bash
# Use screen or minicom to test
screen /dev/ttyUSB0 115200
```

## Configuration

Konfiguracja bazowa znajduje się w [`config/config.json`](config/config.json). Zmieniaj wyłącznie potrzebne pola, zachowując pozostałe ustawienia i strukturę. Dla aktualnego firmware Nano wymagane jest 115200 bit/s, protokół `hx711-frame-v1`, a domyślny backend danych to `files`. Przykładowe kluczowe wartości:

```json
{
    "serial": {
        "port": "/dev/ttyUSB0",
        "baud_rate": 115200
    },
    "data": {
        "base_path": "./data/patients"
    },
    "encryption": {
        "enabled": false,
        "algorithm": "AES-256-GCM"
    },
    "ui": {
        "theme": "default",
        "language": "pl"
    }
}
```

**Bezpieczeństwo konfiguracji:** przykładowy plik repozytorium ma `encryption.enabled=false` dla profilu deweloperskiego. Profil produkcyjny wymaga włączonego AES-256-GCM i zewnętrznego sekretu klucza; nie wpisuj sekretu bezpośrednio do pliku JSON ani repozytorium. Adapter MariaDB nie jest obecnie zaimplementowany — używaj `storage.patient_backend: "files"`.

Kalibracja firmware Arduino jest przechowywana w RAM i po restarcie urządzenia wymaga ponownego wykonania. `HX711_SAMPLES=1`, więc aktualny szkic nie uśrednia wielu próbek. Procedura: [`docs/HX711_CALIBRATION.md`](docs/HX711_CALIBRATION.md).

## Testing

### Unit Tests
```bash
cd build
ctest --verbose
```

### Hardware Integration Test
1. Connect HX711 sensor to Arduino
2. Upload test firmware
3. Run application
4. Navigate to Measurement tab
5. Verify real-time graph updates

## Performance Optimization

### For Raspberry Pi 4
```bash
# Enable GPU acceleration
sudo raspi-config
# Advanced Options -> GL Drivers -> GL (Full KMS)

# Increase GPU memory
sudo raspi-config
# Advanced Options -> Memory Split -> 128MB
```

## Updates

```bash
# Pull latest changes
git pull origin main

# Rebuild
cd build
make -j$(nproc)

# Restart application
```

## Uninstallation

```bash
# Remove build artifacts
rm -rf build

# Remove configuration
rm -rf ~/.config/biofeedback

# Remove data (careful!)
rm -rf /path/to/biofeedback/data
```

## Support

For issues and questions:
- Check the README.md
- Review CONTRIBUTING.md
- Open an issue on GitHub
- Consult the documentation wiki
