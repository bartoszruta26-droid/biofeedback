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

pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(LIBSERIALPORT REQUIRED libserialport)
pkg_check_modules(CAIRO REQUIRED cairo)

find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTK4_INCLUDE_DIRS}
    ${LIBSERIALPORT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
)

# Link directories
link_directories(
    ${GTK4_LIBRARY_DIRS}
    ${LIBSERIALPORT_LIBRARY_DIRS}
    ${CAIRO_LIBRARY_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Logger.cpp
    src/core/ConfigManager.cpp
    src/gui/MainWindow.cpp
    src/gui/GraphWidget.cpp
    src/tab/PatientTab.cpp
    src/tab/MeasurementTab.cpp
    src/tab/OutlineTab.cpp
    src/sensor/HX711Sensor.cpp
    src/sensor/SerialCommunication.cpp
    src/data/DataManager.cpp
    src/data/Encryption.cpp
    src/games/GameEngine.cpp
    src/games/SinGame.cpp
    src/games/PongGame.cpp
)

# Header files
set(HEADERS
    include/core/Application.hpp
    include/core/Logger.hpp
    include/core/ConfigManager.hpp
    include/gui/MainWindow.hpp
    include/gui/GraphWidget.hpp
    include/tab/PatientTab.hpp
    include/tab/MeasurementTab.hpp
    include/tab/OutlineTab.hpp
    include/sensor/HX711Sensor.hpp
    include/sensor/SerialCommunication.hpp
    include/data/DataManager.hpp
    include/data/Encryption.hpp
    include/games/GameEngine.hpp
    include/games/SinGame.hpp
    include/games/PongGame.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GTK4_LIBRARIES}
    ${LIBSERIALPORT_LIBRARIES}
    ${CAIRO_LIBRARIES}
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    pthread
)

# Include GTK4 definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ${GTK4_CFLAGS_OTHER}
)

# Install rules
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
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
