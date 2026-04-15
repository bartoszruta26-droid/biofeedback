project('biofeedback', 'cpp',
  version: '1.0.0',
  default_options: ['cpp_std=c++17']
)

# Dependencies
gtk4 = dependency('gtk4', version: '>=4.0')
libserialport = dependency('libserialport')
cairo = dependency('cairo')
nlohmann_json = dependency('nlohmann_json')
openssl = dependency('openssl')

# Include directories
inc = include_directories('include')

# Source files
sources = files(
  'src/main.cpp',
  'src/core/Application.cpp',
  'src/core/Logger.cpp',
  'src/core/ConfigManager.cpp',
  'src/gui/MainWindow.cpp',
  'src/gui/GraphWidget.cpp',
  'src/tabs/PatientTab.cpp',
  'src/tabs/MeasurementTab.cpp',
  'src/tabs/TrainingTab.cpp',
  'src/tabs/OutlineTab.cpp',
  'src/sensor/HX711Sensor.cpp',
  'src/sensor/SerialCommunication.cpp',
  'src/data/DataManager.cpp',
  'src/data/Encryption.cpp',
  'src/games/GameEngine.cpp',
  'src/games/SinGame.cpp',
  'src/games/TanGame.cpp',
  'src/games/FlappyBirdGame.cpp',
  'src/games/PongGame.cpp',
  'src/games/ArkanoidGame.cpp',
  'src/games/MarioGame.cpp',
  'src/games/GalagaGame.cpp',
  'src/games/ArcherGame.cpp',
  'src/games/ScoredEarthGame.cpp',
  'src/games/CarRaceGame.cpp'
)

# Header files
headers = files(
  'include/core/Application.hpp',
  'include/core/Logger.hpp',
  'include/core/ConfigManager.hpp',
  'include/gui/MainWindow.hpp',
  'include/gui/GraphWidget.hpp',
  'include/tabs/PatientTab.hpp',
  'include/tabs/MeasurementTab.hpp',
  'include/tabs/TrainingTab.hpp',
  'include/tabs/OutlineTab.hpp',
  'include/sensor/HX711Sensor.hpp',
  'include/sensor/SerialCommunication.hpp',
  'include/data/DataManager.hpp',
  'include/data/Encryption.hpp',
  'include/games/GameEngine.hpp',
  'include/games/SinGame.hpp',
  'include/games/TanGame.hpp',
  'include/games/FlappyBirdGame.hpp',
  'include/games/PongGame.hpp',
  'include/games/ArkanoidGame.hpp',
  'include/games/MarioGame.hpp',
  'include/games/GalagaGame.hpp',
  'include/games/ArcherGame.hpp',
  'include/games/ScoredEarthGame.hpp',
  'include/games/CarRaceGame.hpp'
)

# Executable
executable('biofeedback',
  sources,
  headers,
  include_directories: inc,
  dependencies: [gtk4, libserialport, cairo, nlohmann_json, openssl],
  install: true
)

# Install additional files
install_data('config/config.json', install_dir: 'share/biofeedback/config')
install_subdir('resources', install_dir: 'share/biofeedback')
install_subdir('data', install_dir: 'share/biofeedback')
