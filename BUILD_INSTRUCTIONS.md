# Biofeedback Application - Build Instructions

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
    qtcharts5-dev \
    libserialport-dev \
    nlohmann-json3-dev \
    libssl-dev \
    pkg-config
```

**Note**: Qt Charts (`qtcharts5-dev`) is required for the graph visualization features and all game rendering. Make sure it's properly installed.

### 3. Configure Serial Port Permissions
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Create udev rule for Arduino
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="2341", MODE="0666"' | sudo tee /etc/udev/rules.d/50-arduino.rules

# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Verify serial port
ls -l /dev/ttyUSB*
```

### 4. Verify Qt5 Installation
```bash
pkg-config --modversion Qt5Core
# Should return 5.x.x
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

### Option 2: Using Makefile Directly

```bash
# Navigate to project directory
cd /path/to/biofeedback

# Build using provided Makefile
make -j$(nproc)
```

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
1. Open `hx711_arduino_nano.ino` in Arduino IDE
2. Select Board: Arduino Nano
3. Select correct Port: `/dev/ttyUSB0`
4. Click Upload

### 3. Verify Communication
```bash
# Use screen or minicom to test
screen /dev/ttyUSB0 9600
```

## Configuration

Edit `config/config.json` to customize:

```json
{
    "serial": {
        "port": "/dev/ttyUSB0",
        "baud_rate": 9600
    },
    "data": {
        "base_path": "./data/patients"
    },
    "encryption": {
        "enabled": true,
        "algorithm": "AES-256-CBC"
    },
    "ui": {
        "theme": "default",
        "language": "pl"
    }
}
```

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
