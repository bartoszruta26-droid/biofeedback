#include "core/Application.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "gui/MainWindow.hpp"
#include "data/DataManager.hpp"
#include "sensor/HX711Sensor.hpp"
#include <iostream>

namespace biofeedback {

Application::Application(int argc, char* argv[])
    : m_gtkApp(nullptr)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_mainWindow(nullptr)
    , m_dataManager(nullptr)
    , m_sensor(nullptr)
    , m_initialized(false)
    , m_running(false)
{
    // Initialize GTK application
    m_gtkApp = gtk_application_new("org.biofeedback.app", G_APPLICATION_DEFAULT_FLAGS);
    
    if (!m_gtkApp) {
        throw std::runtime_error("Failed to initialize GTK application");
    }
}

Application::~Application() {
    shutdown();
    
    if (m_gtkApp) {
        g_object_unref(m_gtkApp);
        m_gtkApp = nullptr;
    }
}

int Application::run() {
    if (!initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return EXIT_FAILURE;
    }
    
    m_running = true;
    
    // Run GTK main loop
    int status = g_application_run(G_APPLICATION(m_gtkApp), 0, nullptr);
    
    m_running = false;
    return status;
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    std::cout << "Shutting down Biofeedback Application..." << std::endl;
    
    // Stop sensor readings
    if (m_sensor) {
        m_sensor->stopReading();
    }
    
    // Save any pending data
    if (m_dataManager) {
        m_dataManager->flush();
    }
    
    // Log shutdown
    if (m_logger) {
        m_logger->info("Application shutdown complete");
    }
    
    m_initialized = false;
}

bool Application::initialize() {
    if (m_initialized) {
        return true;
    }
    
    try {
        // Load configuration first
        if (!loadConfiguration()) {
            return false;
        }
        
        // Initialize logger
        m_logger = std::make_unique<Logger>(*m_configManager);
        m_logger->info("Starting Biofeedback Application");
        
        // Initialize data manager
        m_dataManager = std::make_unique<DataManager>(*m_configManager, *m_logger);
        
        // Initialize sensor
        m_sensor = std::make_unique	HX711Sensor>(*m_configManager, *m_logger);
        
        // Create main window
        createMainWindow();
        
        m_initialized = true;
        m_logger->info("Application initialized successfully");
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Initialization error: " << e.what() << std::endl;
        if (m_logger) {
            m_logger->error(std::string("Initialization failed: ") + e.what());
        }
        return false;
    }
}

void Application::createMainWindow() {
    m_mainWindow = std::make_unique<MainWindow>(
        *this,
        *m_configManager,
        *m_logger,
        *m_dataManager,
        *m_sensor
    );
    
    // Connect window to GTK application
    g_signal_connect(m_mainWindow->getGTKWindow(), "activate", G_CALLBACK([](GtkApplication* app, gpointer user_data) {
        auto* window = static_cast<MainWindow*>(user_data);
        window->show();
    }), m_mainWindow.get());
}

bool Application::loadConfiguration() {
    m_configManager = std::make_unique<ConfigManager>();
    
    // Try to load from default location
    std::string configPath = "config/config.json";
    
    if (!m_configManager->load(configPath)) {
        std::cerr << "Warning: Could not load configuration from " << configPath << std::endl;
        std::cerr << "Using default configuration" << std::endl;
        m_configManager->setDefaults();
    }
    
    return true;
}

} // namespace biofeedback
