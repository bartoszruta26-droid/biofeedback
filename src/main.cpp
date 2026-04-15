/**
 * @file main.cpp
 * @brief Main entry point for Biofeedback Application
 * 
 * Medical-grade biofeedback application for Raspberry Pi 4
 * using Qt5, Arduino Nano, and HX711 strain gauge sensor.
 */

#include "core/Application.hpp"
#include <iostream>
#include <csignal>

namespace {
    biofeedback::Application* g_app = nullptr;
    
    void signalHandler(int signum) {
        if (g_app) {
            std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
            g_app->shutdown();
        }
        exit(signum);
    }
}

int main(int argc, char* argv[]) {
    try {
        // Set up signal handlers
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        
        // Create and run application
        biofeedback::Application app(argc, argv);
        g_app = &app;
        
        std::cout << "Biofeedback Application v1.0.0" << std::endl;
        std::cout << "Initializing..." << std::endl;
        
        return app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return EXIT_FAILURE;
    }
    
    g_app = nullptr;
    return EXIT_SUCCESS;
}
