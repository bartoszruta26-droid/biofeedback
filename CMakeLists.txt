# CMakeLists.txt - Biofeedback Application

cmake_minimum_required(VERSION 3.16)
project(biofeedback VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

# Shared measurement pipeline: validation, canonical units, quality and bounded history.
add_library(biofeedback_measurement STATIC src/measurement/MeasurementEngine.cpp)
target_include_directories(biofeedback_measurement PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(measurement_engine_regression tests/measurement_engine_regression.cpp)
target_link_libraries(measurement_engine_regression PRIVATE biofeedback_measurement)
add_test(NAME measurement_engine_regression COMMAND measurement_engine_regression)

add_executable(force_units_regression tests/force_units_regression.cpp)
target_include_directories(force_units_regression PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME force_units_regression COMMAND force_units_regression)

# Local loopback HTTP API. Sensitive and hardware operations remain disabled
# until authentication, authorization and measurement services are integrated.
add_executable(biofeedback_api
    src/api/main.cpp
    src/api/HttpApiServer.cpp
    include/api/HttpApiServer.hpp
)
set_target_properties(biofeedback_api PROPERTIES AUTOMOC ON)
target_include_directories(biofeedback_api PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(biofeedback_api PRIVATE Qt5::Core Qt5::Network)
install(TARGETS biofeedback_api DESTINATION bin)

# Regression targets that can be built without Qt.
enable_testing()
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
    target_link_libraries(serial_pty_regression PRIVATE Qt5::Core pthread)
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
    target_link_libraries(biofeedback_tui PRIVATE biofeedback_tui_model biofeedback_measurement Qt5::Core ${CURSES_LIBRARIES})
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
        src/tab/OutlineTab.cpp
        src/tab/TrainingTab.cpp
        src/tab/DebugTab.cpp
        src/sensor/SerialCommunication.cpp
        src/sensor/MeasurementSerialAdapter.cpp
        src/data/DataManager.cpp
        src/data/Encryption.cpp
        src/games/GameEngine.cpp
        src/games/SinGame.cpp
        src/games/PongGame.cpp
        src/games/FlappyBirdGame.cpp
        src/games/MarioGame.cpp
        src/formulas.cpp
    )

    # Header files
    set(HEADERS
        include/core/Application.hpp
        include/core/Logger.hpp
        include/core/DebugManager.hpp
        include/core/ConfigManager.hpp
        include/core/Authentication.hpp
        include/gui/MainWindow.hpp
        include/gui/LoginDialog.hpp
        include/gui/GraphWidget.hpp
        include/tab/PatientTab.hpp
        include/tab/MeasurementTab.hpp
        include/tab/OutlineTab.hpp
        include/tab/TrainingTab.hpp
        include/tab/DebugTab.hpp
        include/sensor/SerialCommunication.hpp
        include/sensor/MeasurementSerialAdapter.hpp
        include/measurement/MeasurementEngine.hpp
        include/data/DataManager.hpp
        include/data/Encryption.hpp
        include/games/GameEngine.hpp
        include/games/SinGame.hpp
        include/games/PongGame.hpp
        include/games/FlappyBirdGame.hpp
        include/games/MarioGame.hpp
        include/formulas.hpp
    )

    # Create executable
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

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
        Qt5::Core
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
enable_testing()
add_subdirectory(tests OPTIONAL)

# Documentation
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
    doxygen_add_docs(docs ALL ${CMAKE_SOURCE_DIR}/include)
endif()
