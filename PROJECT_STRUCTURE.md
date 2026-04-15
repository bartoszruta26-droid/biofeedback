# Biofeedback Application - Project Structure

## Directory Layout

```
biofeedback/
├── README.md                 # Project documentation
├── LICENSE                   # License file (Apache 2.0)
├── CONTRIBUTING.md           # Contribution guidelines
├── BUILD_INSTRUCTIONS.md     # Detailed build instructions
├── PROJECT_STRUCTURE.md      # This file
├── CMakeLists.txt            # CMake build configuration
├── Makefile                  # GNU Make build configuration
├── .gitignore                # Git ignore rules
│
├── config/                   # Configuration files
│   ├── config.json           # Main configuration file
│   └── config.json.example   # Example configuration template
│
├── data/                     # Data storage directory
│   └── patients/             # Patient data storage
│       └── .gitkeep          # Keep directory in git
│
├── include/                  # Header files (.hpp)
│   ├── core/                 # Core functionality headers
│   │   ├── Application.hpp   # Main application class
│   │   ├── ConfigManager.hpp # Configuration management
│   │   └── Logger.hpp        # Logging system
│   │
│   ├── gui/                  # GUI component headers
│   │   ├── MainWindow.hpp    # Main application window
│   │   └── GraphWidget.hpp   # Real-time graph widget
│   │
│   ├── tabs/                 # Tab interface headers
│   │   ├── PatientTab.hpp    # Patient management tab
│   │   ├── MeasurementTab.hpp# Measurement and analysis tab
│   │   ├── TrainingTab.hpp   # Training games tab
│   │   └── OutlineTab.hpp    # Exercise plans tab
│   │
│   ├── sensor/               # Sensor communication headers
│   │   ├── HX711Sensor.hpp   # HX711 sensor interface
│   │   └── SerialCommunication.hpp # Serial port communication
│   │
│   ├── games/                # Game engine headers
│   │   ├── GameEngine.hpp    # Base game engine
│   │   ├── SinGame.hpp       # Sine wave control game
│   │   ├── TanGame.hpp       # Tangent control game
│   │   ├── FlappyBirdGame.hpp
│   │   ├── PongGame.hpp
│   │   ├── ArkanoidGame.hpp
│   │   ├── MarioGame.hpp
│   │   ├── GalagaGame.hpp
│   │   ├── ArcherGame.hpp
│   │   ├── ScoredEarthGame.hpp
│   │   └── CarRaceGame.hpp
│   │
│   └── data/                 # Data management headers
│       ├── DataManager.hpp   # Patient data management
│       └── Encryption.hpp    # Data encryption utilities
│
├── src/                      # Source files (.cpp)
│   ├── main.cpp              # Application entry point
│   │
│   ├── core/                 # Core implementation
│   │   ├── Application.cpp
│   │   ├── ConfigManager.cpp
│   │   └── Logger.cpp
│   │
│   ├── gui/                  # GUI implementation
│   │   ├── MainWindow.cpp
│   │   └── GraphWidget.cpp
│   │
│   ├── tabs/                 # Tab implementations
│   │   ├── PatientTab.cpp
│   │   ├── MeasurementTab.cpp
│   │   ├── TrainingTab.cpp
│   │   └── OutlineTab.cpp
│   │
│   ├── sensor/               # Sensor implementations
│   │   ├── HX711Sensor.cpp
│   │   └── SerialCommunication.cpp
│   │
│   ├── games/                # Game implementations
│   │   ├── GameEngine.cpp
│   │   ├── SinGame.cpp
│   │   ├── TanGame.cpp
│   │   ├── FlappyBirdGame.cpp
│   │   ├── PongGame.cpp
│   │   ├── ArkanoidGame.cpp
│   │   ├── MarioGame.cpp
│   │   ├── GalagaGame.cpp
│   │   ├── ArcherGame.cpp
│   │   ├── ScoredEarthGame.cpp
│   │   └── CarRaceGame.cpp
│   │
│   └── data/                 # Data management implementations
│       ├── DataManager.cpp
│       └── Encryption.cpp
│
├── resources/                # Multimedia resources
│   ├── icons/                # Application icons
│   ├── images/               # Images for exercises
│   ├── videos/               # Exercise demonstration videos
│   └── sounds/               # Sound effects for games
│       └── .gitkeep
│
├── logs/                     # Application logs
│   └── .gitkeep
│
├── build/                    # Build output (generated, gitignored)
│   ├── bin/                  # Executables
│   ├── obj/                  # Object files
│   ├── config/               # Copied configuration
│   ├── resources/            # Copied resources
│   └── data/                 # Copied data structure
│
└── tests/                    # Unit tests (optional)
    ├── CMakeLists.txt
    └── *.cpp
```

## File Descriptions

### Documentation Files
- **README.md**: Project overview, features, installation guide
- **LICENSE**: Apache License 2.0
- **CONTRIBUTING.md**: Guidelines for contributors
- **BUILD_INSTRUCTIONS.md**: Detailed build and setup instructions
- **PROJECT_STRUCTURE.md**: This file, project organization

### Build Configuration
- **CMakeLists.txt**: CMake build system configuration
- **Makefile**: GNU Make build system

### Configuration
- **config/config.json**: Runtime configuration (serial port, paths, etc.)
- **config/config.json.example**: Template for configuration

### Core Components
- **Application**: Main application lifecycle management
- **ConfigManager**: JSON configuration loading and access
- **Logger**: Thread-safe logging with rotation

### GUI Components
- **MainWindow**: GTK4 main window with tabbed interface
- **GraphWidget**: Real-time sensor data visualization

### Tabs
- **PatientTab**: Patient CRUD operations (new, edit, import, export, delete)
- **MeasurementTab**: Live graph, measurements, statistics, data management
- **TrainingTab**: Game selection and biofeedback training
- **OutlineTab**: Exercise plans with multimedia content

### Sensor Interface
- **HX711Sensor**: Strain gauge sensor data acquisition
- **SerialCommunication**: Arduino Nano serial communication

### Games
Each game implements biofeedback control using sensor data:
- **Sin/Tan**: Mathematical function-based control
- **Flappy Bird**: Jump control via muscle tension
- **Pong**: Paddle control
- **Arkanoid**: Brick breaker
- **Mario**: Platformer elements
- **Galaga**: Space shooter
- **Archer**: Archery simulation
- **Scored Earth**: Target practice
- **Car Race**: Racing game

### Data Management
- **DataManager**: Patient data organization and persistence
- **Encryption**: AES-256 encryption for sensitive data

## Data Storage Structure

```
data/patients/
└── [patient_id]/           # Unique patient identifier
    ├── profile.json        # Patient demographic data
    ├── measurements/       # Measurement sessions
    │   ├── [timestamp]_session.json
    │   └── [timestamp]_session.enc (encrypted)
    ├── exercises/          # Completed exercises
    │   └── [date]_exercises.json
    └── backups/            # Automated backups
        └── [timestamp]_backup.zip
```

## Build System Options

### Option 1: CMake (Recommended for IDEs)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Option 2: Make (Direct)
```bash
make -j$(nproc)
```

## Dependencies

### System Libraries
- GTK4 (`libgtk-4-dev`)
- libserialport (`libserialport-dev`)
- Cairo (`libcairo2-dev`)
- OpenSSL (`libssl-dev`)

### C++ Libraries
- nlohmann/json (header-only)
- pthread (standard)

## Medical Software Compliance

The project structure supports:
- **Data Integrity**: Separate patient folders, checksums
- **Audit Trail**: Comprehensive logging
- **Data Security**: Encryption support
- **Traceability**: Version control, clear structure
- **Validation**: Input validation in all components
