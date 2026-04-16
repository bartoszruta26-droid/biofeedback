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

# Core library (no Qt dependency) available in all environments.
add_library(biofeedback_core STATIC
    src/core/Logger.cpp
    src/core/Authentication.cpp
    src/data/DataManager.cpp
    src/data/Encryption.cpp
)

target_include_directories(biofeedback_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/core
        ${CMAKE_SOURCE_DIR}/include/data
)

target_link_libraries(biofeedback_core
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
)

# Regression targets that can be built without Qt.
add_executable(auth_regression tests/auth_regression.cpp)
target_link_libraries(auth_regression PRIVATE biofeedback_core)

add_executable(data_manager_regression tests/data_manager_regression.cpp)
target_link_libraries(data_manager_regression PRIVATE biofeedback_core)

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
        src/core/ConfigManager.cpp
        src/core/Authentication.cpp
        src/gui/MainWindow.cpp
        src/gui/LoginDialog.cpp
        src/gui/GraphWidget.cpp
        src/tab/PatientTab.cpp
        src/tab/MeasurementTab.cpp
        src/tab/OutlineTab.cpp
        src/tab/TrainingTab.cpp
        src/sensor/SerialCommunication.cpp
        src/data/DataManager.cpp
        src/data/Encryption.cpp
        src/games/GameEngine.cpp
        src/games/SinGame.cpp
        src/games/PongGame.cpp
        src/games/FlappyBirdGame.cpp
        src/games/MarioGame.cpp
    )

    # Header files
    set(HEADERS
        include/core/Application.hpp
        include/core/Logger.hpp
        include/core/ConfigManager.hpp
        include/core/Authentication.hpp
        include/gui/MainWindow.hpp
        include/gui/LoginDialog.hpp
        include/gui/GraphWidget.hpp
        include/tab/PatientTab.hpp
        include/tab/MeasurementTab.hpp
        include/tab/OutlineTab.hpp
        include/tab/TrainingTab.hpp
        include/sensor/SerialCommunication.hpp
        include/data/DataManager.hpp
        include/data/Encryption.hpp
        include/games/GameEngine.hpp
        include/games/SinGame.hpp
        include/games/PongGame.hpp
        include/games/FlappyBirdGame.hpp
        include/games/MarioGame.hpp
    )

    # Create executable
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

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
        Qt5::Core
        Qt5::Widgets
        Qt5::Charts
        ${LIBSERIALPORT_LIBRARIES}
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
    )
else()
    message(WARNING "Qt5 not found - GUI target '${PROJECT_NAME}' will not be built. Core/regression targets remain available.")
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
