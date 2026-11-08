# CMakeLists.txt - Biofeedback Application

cmake_minimum_required(VERSION 3.16)
project(biofeedback VERSION 1.0.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find dependencies
find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Network)

# Core library (no Qt dependency) available in all environments.
add_library(biofeedback_core STATIC
    src/core/Logger.cpp
    src/core/PasswordHasher.cpp
    src/core/Authentication.cpp
    src/data/DataManager.cpp
    src/data/Encryption.cpp
    src/data/LegacyXorMigration.cpp
)

target_include_directories(biofeedback_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/data
        ${Qt5Core_INCLUDE_DIRS}
)

target_link_libraries(biofeedback_core
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
        Qt5::Core
)

# Encrypted acquisition journal: atomically committed batches of up to 256 samples.
add_library(biofeedback_acquisition_journal STATIC src/data/AcquisitionJournal.cpp)
target_include_directories(biofeedback_acquisition_journal PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_acquisition_journal PUBLIC biofeedback_core Qt5::Core)

add_executable(acquisition_journal_regression tests/acquisition_journal_regression.cpp)
target_link_libraries(acquisition_journal_regression PRIVATE biofeedback_acquisition_journal Qt5::Core)
add_test(NAME acquisition_journal_regression COMMAND acquisition_journal_regression)

# Bounded asynchronous JSONL writer used by the desktop measurement tab.
add_executable(sample_journal_writer_regression
    tests/sample_journal_writer_regression.cpp
    src/tab/SampleJournalWriter.cpp
)
target_include_directories(sample_journal_writer_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(sample_journal_writer_regression PRIVATE Qt5::Core Threads::Threads)
add_test(NAME sample_journal_writer_regression COMMAND sample_journal_writer_regression)

# Shared encrypted JSON resource repository for patients, measurements, training and games.
add_library(biofeedback_data_repository STATIC src/data/JsonResourceRepository.cpp)
target_include_directories(biofeedback_data_repository PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_data_repository PUBLIC biofeedback_core Qt5::Core)

add_executable(json_resource_repository_regression tests/json_resource_repository_regression.cpp)
target_link_libraries(json_resource_repository_regression PRIVATE biofeedback_data_repository Qt5::Core)
add_test(NAME json_resource_repository_regression COMMAND json_resource_repository_regression)

# Shared authenticated HTTP client for terminal and other native clients.
add_library(biofeedback_api_client STATIC src/api/HttpApiClient.cpp)
target_include_directories(biofeedback_api_client PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_api_client PUBLIC Qt5::Core Qt5::Network)

add_executable(api_client_regression
    tests/api_client_regression.cpp
    src/api/HttpApiServer.cpp
    include/api/HttpApiServer.hpp
)
set_target_properties(api_client_regression PROPERTIES AUTOMOC ON)
target_include_directories(api_client_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(api_client_regression PRIVATE
    biofeedback_api_client biofeedback_data_repository biofeedback_core biofeedback_application_services Qt5::Core Qt5::Network
)
add_test(NAME api_client_regression COMMAND api_client_regression)
set_tests_properties(api_client_regression PROPERTIES TIMEOUT 20)

# Shared measurement pipeline: validation, canonical units, quality and bounded history.
add_library(biofeedback_measurement STATIC src/measurement/MeasurementEngine.cpp)
target_include_directories(biofeedback_measurement PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Repetition segmentation and set-level biomechanics are independent of Qt.
add_library(biofeedback_training STATIC src/training/RepetitionDetector.cpp)
target_include_directories(biofeedback_training PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_executable(repetition_detector_regression tests/repetition_detector_regression.cpp)
target_link_libraries(repetition_detector_regression PRIVATE biofeedback_training)
add_test(NAME repetition_detector_regression COMMAND repetition_detector_regression)

add_executable(session_state_machine_regression tests/session_state_machine_regression.cpp)
target_include_directories(session_state_machine_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME session_state_machine_regression COMMAND session_state_machine_regression)

add_executable(training_session_clock_regression tests/training_session_clock_regression.cpp)
target_include_directories(training_session_clock_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME training_session_clock_regression COMMAND training_session_clock_regression)

add_executable(measurement_engine_regression tests/measurement_engine_regression.cpp)
target_link_libraries(measurement_engine_regression PRIVATE biofeedback_measurement Threads::Threads)
add_test(NAME measurement_engine_regression COMMAND measurement_engine_regression)

# Device calibration profiles and incremental binary protocol parsing.
add_library(biofeedback_sensor_protocol STATIC
    src/sensor/CalibrationProfile.cpp
    src/sensor/ProtocolParser.cpp
)
target_include_directories(biofeedback_sensor_protocol PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_sensor_protocol PUBLIC nlohmann_json::nlohmann_json)

add_executable(calibration_profile_regression tests/calibration_profile_regression.cpp)
target_link_libraries(calibration_profile_regression PRIVATE biofeedback_sensor_protocol)
add_test(NAME calibration_profile_regression COMMAND calibration_profile_regression)

add_executable(protocol_parser_regression tests/protocol_parser_regression.cpp)
target_link_libraries(protocol_parser_regression PRIVATE biofeedback_sensor_protocol)
add_test(NAME protocol_parser_regression COMMAND protocol_parser_regression)

add_executable(measurement_training_pipeline_regression tests/measurement_training_pipeline_regression.cpp)
target_link_libraries(measurement_training_pipeline_regression PRIVATE biofeedback_measurement biofeedback_training)
add_test(NAME measurement_training_pipeline_regression COMMAND measurement_training_pipeline_regression)

add_executable(force_units_regression tests/force_units_regression.cpp)
target_include_directories(force_units_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME force_units_regression COMMAND force_units_regression)

# Biomechanical formulas and availability metadata are tested independently of Qt Widgets.
add_executable(biomechanical_parameter_availability_regression
    tests/biomechanical_parameter_availability_regression.cpp
    src/formulas.cpp
)
target_include_directories(biomechanical_parameter_availability_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME biomechanical_parameter_availability_regression
    COMMAND biomechanical_parameter_availability_regression)


# Application-facing lifecycle boundary for the shared measurement pipeline.
add_library(biofeedback_application_services STATIC
    src/application/MeasurementService.cpp
    src/application/AcquisitionService.cpp
    src/application/DeviceService.cpp
    src/application/ApplicationServices.cpp
)
target_include_directories(biofeedback_application_services PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_application_services PUBLIC biofeedback_measurement Threads::Threads)

add_executable(measurement_service_regression tests/measurement_service_regression.cpp)
target_link_libraries(measurement_service_regression PRIVATE biofeedback_application_services)
add_test(NAME measurement_service_regression COMMAND measurement_service_regression)

add_executable(acquisition_service_regression tests/acquisition_service_regression.cpp)
target_link_libraries(acquisition_service_regression PRIVATE biofeedback_application_services)
add_test(NAME acquisition_service_regression COMMAND acquisition_service_regression)

add_executable(device_service_regression tests/device_service_regression.cpp)
target_link_libraries(device_service_regression PRIVATE biofeedback_application_services)
add_test(NAME device_service_regression COMMAND device_service_regression)

# Server-owned serial adapter: physical samples must pass the active acquisition session.
add_executable(acquisition_serial_adapter_regression
    tests/acquisition_serial_adapter_regression.cpp
    src/sensor/AcquisitionSerialDeviceAdapter.cpp
    src/sensor/SerialCommunication.cpp
    src/core/DebugManager.cpp
    include/core/DebugManager.hpp
)
set_target_properties(acquisition_serial_adapter_regression PROPERTIES AUTOMOC ON)
target_include_directories(acquisition_serial_adapter_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(acquisition_serial_adapter_regression PRIVATE
    biofeedback_application_services biofeedback_sensor_protocol Qt5::Core Threads::Threads)
add_test(NAME acquisition_serial_adapter_regression COMMAND acquisition_serial_adapter_regression)
set_tests_properties(acquisition_serial_adapter_regression PROPERTIES TIMEOUT 20)

# Local loopback HTTP API. Hardware acquisition is opt-in and requires an explicitly
# configured, connected serial adapter plus an authorized active patient session.
add_executable(biofeedback_api
    src/api/main.cpp
    src/api/HttpApiServer.cpp
    src/sensor/AcquisitionSerialDeviceAdapter.cpp
    src/sensor/SerialCommunication.cpp
    src/core/DebugManager.cpp
    include/core/DebugManager.hpp
    include/api/HttpApiServer.hpp
)
set_target_properties(biofeedback_api PROPERTIES AUTOMOC ON)
target_include_directories(biofeedback_api PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_api PRIVATE Qt5::Core Qt5::Network biofeedback_core biofeedback_data_repository biofeedback_application_services biofeedback_sensor_protocol Threads::Threads)

# HTTP API authentication and bearer-session lifecycle regression.
add_executable(api_auth_regression
    tests/api_auth_regression.cpp
    src/api/HttpApiServer.cpp
    include/api/HttpApiServer.hpp
)
set_target_properties(api_auth_regression PROPERTIES AUTOMOC ON)
target_include_directories(api_auth_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(api_auth_regression PRIVATE biofeedback_core biofeedback_data_repository biofeedback_application_services Qt5::Core Qt5::Network)
add_test(NAME api_auth_regression COMMAND api_auth_regression)
set_tests_properties(api_auth_regression PROPERTIES TIMEOUT 20)

install(TARGETS biofeedback_api DESTINATION bin)

# Regression targets that can be built without Qt.
add_executable(encryption_regression tests/encryption_regression.cpp)
target_link_libraries(encryption_regression PRIVATE biofeedback_core)
add_test(NAME encryption_regression COMMAND encryption_regression)

add_executable(password_hasher_regression tests/password_hasher_regression.cpp)
target_link_libraries(password_hasher_regression PRIVATE biofeedback_core)
add_test(NAME password_hasher_regression COMMAND password_hasher_regression)

add_executable(auth_regression tests/auth_regression.cpp)
target_link_libraries(auth_regression PRIVATE biofeedback_core)
add_test(NAME auth_regression COMMAND auth_regression)

add_executable(data_manager_regression tests/data_manager_regression.cpp)
target_link_libraries(data_manager_regression PRIVATE biofeedback_core)
add_test(NAME data_manager_regression COMMAND data_manager_regression)

# ConfigManager load/validate/save regression (Qt Core only).
add_executable(config_manager_regression
    tests/config_manager_regression.cpp
    src/core/ConfigManager.cpp
)
target_include_directories(config_manager_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(config_manager_regression PRIVATE Qt5::Core nlohmann_json::nlohmann_json)
add_test(NAME config_manager_regression COMMAND config_manager_regression)
set_tests_properties(config_manager_regression PROPERTIES TIMEOUT 10)

# Shared terminal-interface model is independent of ncurses and is always testable.
add_library(biofeedback_tui_model STATIC src/tui/TuiModel.cpp)
target_include_directories(biofeedback_tui_model PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_executable(tui_model_regression tests/tui_model_regression.cpp)
target_link_libraries(tui_model_regression PRIVATE biofeedback_tui_model)
add_test(NAME tui_model_regression COMMAND tui_model_regression)

# POSIX pseudo-terminal integration test for the production serial implementation.
# This compiles the real SerialCommunication and DebugManager sources and exercises
# command writes, response reads, bounded read timeouts, and raw binary writes.
if(UNIX)
    add_executable(serial_pty_regression
        tests/serial_pty_regression.cpp
        src/sensor/SerialCommunication.cpp
        src/core/DebugManager.cpp
        include/core/DebugManager.hpp
    )
    set_target_properties(serial_pty_regression PROPERTIES AUTOMOC ON)
    target_include_directories(serial_pty_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(serial_pty_regression PRIVATE biofeedback_sensor_protocol Qt5::Core pthread)
    add_test(NAME serial_pty_regression COMMAND serial_pty_regression)
endif()

# Build the interactive terminal UI when ncurses development files are available.
find_package(Curses QUIET)
if(CURSES_FOUND)
    add_executable(biofeedback_tui
        src/tui/main.cpp
        src/sensor/SerialCommunication.cpp
        src/sensor/MeasurementSerialAdapter.cpp
        src/core/DebugManager.cpp
        include/core/DebugManager.hpp
    )
    set_target_properties(biofeedback_tui PROPERTIES AUTOMOC ON)
    target_include_directories(biofeedback_tui PRIVATE ${CMAKE_SOURCE_DIR}/include ${CURSES_INCLUDE_DIRS})
    target_link_libraries(biofeedback_tui PRIVATE biofeedback_tui_model biofeedback_measurement biofeedback_training biofeedback_sensor_protocol biofeedback_api_client biofeedback_core Qt5::Core Qt5::Network ${CURSES_LIBRARIES})
    install(TARGETS biofeedback_tui DESTINATION bin)
else()
    message(STATUS "ncurses not found - skipping optional biofeedback_tui target")
endif()

# Explicit, offline-only legacy migration utility and synthetic regression.
add_executable(biofeedback_legacy_migrate src/tools/legacy_xor_migrate.cpp)
target_link_libraries(biofeedback_legacy_migrate PRIVATE biofeedback_core)
add_executable(legacy_xor_migration_regression tests/legacy_xor_migration_regression.cpp)
target_link_libraries(legacy_xor_migration_regression PRIVATE biofeedback_core)
add_test(NAME legacy_xor_migration_regression COMMAND legacy_xor_migration_regression)

# Build GUI app only when Qt5 development packages are available.
find_package(Qt5 QUIET COMPONENTS Core Widgets Charts)
if(Qt5_FOUND)
    find_package(nlohmann_json REQUIRED)
    pkg_check_modules(LIBSERIALPORT REQUIRED libserialport)

    # Qt5 MOC headers
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    # Source files
    set(SOURCES
        src/main.cpp
        src/api/HttpApiServer.cpp
        src/data/JsonResourceRepository.cpp
        src/core/Application.cpp
        src/core/Logger.cpp
        src/core/DebugManager.cpp
        src/core/PasswordHasher.cpp
        src/core/ConfigManager.cpp
        src/core/Authentication.cpp
        src/gui/MainWindow.cpp
        src/gui/LoginDialog.cpp
        src/gui/GraphWidget.cpp
        src/tab/PatientTab.cpp
        src/tab/MeasurementTab.cpp
        src/tab/SampleJournalWriter.cpp
        src/tab/OutlineTab.cpp
        src/tab/TrainingTab.cpp
        src/tab/TrainingAnalyticsWidget.cpp
        src/tab/DebugTab.cpp
        src/sensor/SerialCommunication.cpp
        src/sensor/MeasurementSerialAdapter.cpp
        src/data/DataManager.cpp
        src/data/Encryption.cpp
        src/games/GameEngine.cpp
        src/games/SinGame.cpp
        src/games/TanGame.cpp
        src/games/ArkanoidGame.cpp
        src/games/PongGame.cpp
        src/games/FlappyBirdGame.cpp
        src/games/MarioGame.cpp
        src/games/GalagaGame.cpp
        src/games/ArcherGame.cpp
        src/games/CarRaceGame.cpp
        src/games/ScorchedEarthGame.cpp
        src/formulas.cpp
    )

    # Header files
    set(HEADERS
        include/core/Application.hpp
        include/api/HttpApiServer.hpp
        include/data/JsonResourceRepository.hpp
        include/core/Logger.hpp
        include/core/DebugManager.hpp
        include/core/ConfigManager.hpp
        include/core/Authentication.hpp
        include/gui/MainWindow.hpp
        include/gui/LoginDialog.hpp
        include/gui/GraphWidget.hpp
        include/tab/PatientTab.hpp
        include/tab/MeasurementTab.hpp
        include/tab/SampleJournalWriter.hpp
        include/tab/OutlineTab.hpp
        include/tab/TrainingTab.hpp
        include/tab/TrainingAnalyticsWidget.hpp
        include/tab/DebugTab.hpp
        include/sensor/SerialCommunication.hpp
        include/sensor/MeasurementSerialAdapter.hpp
        include/measurement/MeasurementEngine.hpp
        include/data/DataManager.hpp
        include/data/Encryption.hpp
        include/games/GameEngine.hpp
        include/games/SinGame.hpp
        include/games/TanGame.hpp
        include/games/ArkanoidGame.hpp
        include/games/GalagaGame.hpp
        include/games/ArcherGame.hpp
        include/games/CarRaceGame.hpp
        include/games/ScorchedEarthGame.hpp
        include/games/GameFactory.hpp
        include/games/PongGame.hpp
        include/games/FlappyBirdGame.hpp
        include/games/MarioGame.hpp
        include/formulas.hpp
    )

    # Create executable
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

    # GameEngine lifecycle, monotonic timing, fixed-step simulation and
    # cross-thread sensor input regression.
    add_executable(game_engine_regression
        tests/game_engine_regression.cpp
        src/games/GameEngine.cpp
        src/core/DebugManager.cpp
        include/games/GameEngine.hpp
        include/core/DebugManager.hpp
    )
    set_target_properties(game_engine_regression PROPERTIES AUTOMOC ON)
    target_include_directories(game_engine_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(game_engine_regression PRIVATE Qt5::Core Qt5::Widgets Threads::Threads)


    # Smoke-test factory creation and lifecycle for every currently supported game.
    add_executable(game_factory_regression
        tests/game_factory_regression.cpp
        src/games/GameEngine.cpp
        src/games/SinGame.cpp
        src/games/TanGame.cpp
        src/games/ArkanoidGame.cpp
        src/games/PongGame.cpp
        src/games/FlappyBirdGame.cpp
        src/games/MarioGame.cpp
        src/games/GalagaGame.cpp
        src/games/ArcherGame.cpp
        src/games/CarRaceGame.cpp
        src/games/ScorchedEarthGame.cpp
        src/sensor/SerialCommunication.cpp
        src/core/DebugManager.cpp
        include/games/GameEngine.hpp
        include/games/SinGame.hpp
        include/games/TanGame.hpp
        include/games/ArkanoidGame.hpp
        include/games/PongGame.hpp
        include/games/FlappyBirdGame.hpp
        include/games/MarioGame.hpp
        include/games/GalagaGame.hpp
        include/games/ArcherGame.hpp
        include/games/CarRaceGame.hpp
        include/games/ScorchedEarthGame.hpp
        include/sensor/SerialCommunication.hpp
        include/core/DebugManager.hpp
    )
    set_target_properties(game_factory_regression PROPERTIES AUTOMOC ON)
    target_include_directories(game_factory_regression PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${LIBSERIALPORT_INCLUDE_DIRS}
    )
    target_link_directories(game_factory_regression PRIVATE ${LIBSERIALPORT_LIBRARY_DIRS})
    target_link_libraries(game_factory_regression PRIVATE
        Qt5::Core Qt5::Widgets Threads::Threads ${LIBSERIALPORT_LIBRARIES} biofeedback_sensor_protocol
    )

    # Process-restart regression for durable analytics and checkpoint history.
    add_executable(training_history_restart_regression
        tests/training_history_restart_regression.cpp
        src/tab/TrainingAnalyticsWidget.cpp
        include/tab/TrainingAnalyticsWidget.hpp
    )
    set_target_properties(training_history_restart_regression PROPERTIES AUTOMOC ON)
    target_include_directories(training_history_restart_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(training_history_restart_regression PRIVATE
        biofeedback_training Qt5::Core Qt5::Widgets Qt5::Charts
    )
    add_test(NAME training_history_restart_regression
        COMMAND training_history_restart_regression
    )
    set_tests_properties(training_history_restart_regression PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    # Full TrainingTab integration test. The test launches child processes to
    # simulate abrupt termination and a fresh application process on recovery.
    add_executable(training_tab_process_restart_regression
        tests/training_tab_process_restart_regression.cpp
        src/tab/TrainingTab.cpp
        src/tab/TrainingAnalyticsWidget.cpp
        src/core/DebugManager.cpp
        src/sensor/SerialCommunication.cpp
        src/games/GameEngine.cpp
        src/games/PongGame.cpp
        src/games/FlappyBirdGame.cpp
        src/games/MarioGame.cpp
        src/games/SinGame.cpp
        src/games/TanGame.cpp
        src/games/ArkanoidGame.cpp
        src/games/GalagaGame.cpp
        src/games/ArcherGame.cpp
        src/games/CarRaceGame.cpp
        src/games/ScorchedEarthGame.cpp
        include/tab/TrainingTab.hpp
        include/tab/TrainingAnalyticsWidget.hpp
        include/core/DebugManager.hpp
        include/sensor/SerialCommunication.hpp
        include/measurement/MeasurementEngine.hpp
        include/games/GameEngine.hpp
        include/games/PongGame.hpp
        include/games/FlappyBirdGame.hpp
        include/games/MarioGame.hpp
        include/games/SinGame.hpp
        include/games/TanGame.hpp
        include/games/ArkanoidGame.hpp
        include/games/GalagaGame.hpp
        include/games/ArcherGame.hpp
        include/games/CarRaceGame.hpp
        include/games/ScorchedEarthGame.hpp
        include/games/GameFactory.hpp
    )
    set_target_properties(training_tab_process_restart_regression PROPERTIES AUTOMOC ON)
    target_include_directories(training_tab_process_restart_regression PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${LIBSERIALPORT_INCLUDE_DIRS}
    )
    target_link_directories(training_tab_process_restart_regression PRIVATE
        ${LIBSERIALPORT_LIBRARY_DIRS}
    )
    target_link_libraries(training_tab_process_restart_regression PRIVATE
        biofeedback_training
        biofeedback_measurement
        biofeedback_sensor_protocol
        biofeedback_data_repository
        Qt5::Core Qt5::Widgets Qt5::Charts
        ${LIBSERIALPORT_LIBRARIES}
        OpenSSL::SSL OpenSSL::Crypto pthread
    )
    add_test(NAME training_tab_process_restart_regression
        COMMAND training_tab_process_restart_regression
    )
    set_tests_properties(training_tab_process_restart_regression PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 120
    )

    # Minimal mandatory Qt5 example source.
    add_executable(qt5_mandatory_example examples/qt5_mandatory.cpp)

    target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${LIBSERIALPORT_INCLUDE_DIRS}
    )

    target_link_directories(${PROJECT_NAME}
        PRIVATE
            ${LIBSERIALPORT_LIBRARY_DIRS}
    )

    # Link libraries
    target_link_libraries(${PROJECT_NAME}
        biofeedback_measurement
        biofeedback_training
        biofeedback_sensor_protocol
        biofeedback_application_services
        Qt5::Core
        Qt5::Network
        Qt5::Widgets
        Qt5::Charts
        ${LIBSERIALPORT_LIBRARIES}
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    )

    target_link_libraries(qt5_mandatory_example
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
    )
else()
    message(WARNING "Qt5 not found - GUI target '${PROJECT_NAME}' will not be built. Core/regression targets remain available.")
endif()

# Compile the changed login dialog independently from unrelated legacy GUI tabs.
find_package(Qt5 QUIET COMPONENTS Core Widgets)
if(Qt5Core_FOUND AND Qt5Widgets_FOUND)
    add_library(login_dialog_compile OBJECT src/gui/LoginDialog.cpp)
    target_include_directories(login_dialog_compile PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(login_dialog_compile PRIVATE Qt5::Core Qt5::Widgets)
endif()

# Install rules
if(TARGET ${PROJECT_NAME})
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
install(DIRECTORY config/ DESTINATION share/${PROJECT_NAME}/config)
install(DIRECTORY resources/ DESTINATION share/${PROJECT_NAME}/resources)

# Copy config and resources to build directory
file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Testing
# Static regression for bounded, low-latency Qt sensor dispatch paths.
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME measurement_tab_lifecycle_regression
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/measurement_tab_lifecycle_regression.py)
    set_tests_properties(measurement_tab_lifecycle_regression PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

enable_testing()
if(TARGET game_engine_regression)
    add_test(NAME game_engine_regression COMMAND game_engine_regression)
    set_tests_properties(game_engine_regression PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
if(TARGET game_factory_regression)
    add_test(NAME game_factory_regression COMMAND game_factory_regression)
    set_tests_properties(game_factory_regression PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
add_subdirectory(tests OPTIONAL)

# Documentation
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
    doxygen_add_docs(docs ALL ${CMAKE_SOURCE_DIR}/include)
endif()

# Linux desktop integration and Debian package.
install(FILES packaging/linux/biofeedback.desktop
    DESTINATION share/applications)
install(FILES resources/biofeedback.svg
    DESTINATION share/icons/hicolor/scalable/apps)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "biofeedback")
set(CPACK_PACKAGE_VENDOR "Biofeedback Project")
set(CPACK_PACKAGE_CONTACT "Biofeedback Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Biofeedback measurement and training application")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/bartoszruta26-droid/biofeedback")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Biofeedback Project")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
include(CPack)
