# Biofeedback Application for Raspberry Pi 4

A professional medical-grade biofeedback application built with C++, Qt, and Arduino Nano integration for strain gauge sensor (HX711) data acquisition.

## Features

- **Patient Management**: Create, edit, import, export, and delete patient records
- **Live Measurement**: Real-time graph visualization of sensor data with statistical analysis
- **Training Games**: Multiple biofeedback-controlled games (Sin, Tan, Flappy Bird, Pong, Arkanoid, Mario, Galaga, Archer, Scored Earth, Car Race)
- **Exercise Plans**: Structured exercise protocols with multimedia content and game assignments
- **Data Security**: JSON storage with optional encryption for sensitive medical data
- **Medical Standards**: Comprehensive logging, data validation, and audit trails

## Hardware Requirements

- Raspberry Pi 4
- Arduino Nano
- HX711 strain gauge sensor module
- USB cable for Arduino-Raspberry connection

## Software Dependencies

- Q4
- libserialport
- nlohmann/json
- OpenSSL (for encryption)
- Cairo (for graphics)

## Installation

### System Setup

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y libgtk-4-dev libserialport-dev nlohmann-json3-dev libssl-dev libcairo2-dev git cmake build-essential

# Set up serial port permissions
sudo usermod -a -G dialout $USER
```

### Build Instructions

```bash
# Clone repository
git clone <repository-url>
cd biofeedback-app

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run
./biofeedback
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
└── build/                 # Build output
```

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
