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

sudo apt install -y qtbase5-dev qtcharts5-dev libserialport-dev \
    nlohmann-json3-dev libssl-dev git cmake build-essential pkg-config
```

## Installation

### System Setup

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y qtbase5-dev qtcharts5-dev libserialport-dev nlohmann-json3-dev libssl-dev git cmake build-essential geany

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
