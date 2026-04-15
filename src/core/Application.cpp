#include "core/Application.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "gui/MainWindow.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/PatientTab.hpp"
#include "tab/OutlineTab.hpp"
#include "data/DataManager.hpp"
#include "sensor/SerialCommunication.hpp"
#include <iostream>

namespace biofeedback {

Application::Application(int argc, char* argv[])
    : m_qtApp(nullptr)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_mainWindow(nullptr)
    , m_dataManager(nullptr)
    , m_serialComm(nullptr)
    , m_measurementTab(nullptr)
    , m_patientTab(nullptr)
    , m_outlineTab(nullptr)
    , m_initialized(false)
    , m_running(false)
{
    // Initialize Qt application
    m_qtApp = std::make_unique<QApplication>(argc, argv);
    
    if (!m_qtApp) {
        throw std::runtime_error("Failed to initialize Qt application");
    }
}

Application::~Application() {
    shutdown();
}

int Application::run() {
    if (!initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return EXIT_FAILURE;
    }
    
    m_running = true;
    
    // Show main window
    if (m_mainWindow) {
        m_mainWindow->show();
    }
    
    // Run Qt main loop
    int status = m_qtApp->exec();
    
    m_running = false;
    return status;
}

void Application::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    std::cout << "Shutting down Biofeedback Application..." << std::endl;
    
    // Stop serial communication
    if (m_serialComm) {
        m_serialComm->disconnect();
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
        
        // Initialize serial communication
        m_serialComm = std::make_unique<SerialCommunication>(*m_configManager, *m_logger);
        
        // Create main window with tabs
        createMainWindow();
        createTabs();
        
        // Setup connections between components
        setupConnections();
        
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
    m_mainWindow = std::make_unique<gui::MainWindow>();
    
    // Set window properties
    m_mainWindow->setWindowTitle("Biofeedback System v1.0");
    m_mainWindow->resize(1200, 800);
}

void Application::createTabs() {
    // Create tabs
    m_measurementTab = new tab::MeasurementTab(m_mainWindow.get());
    m_patientTab = new tab::PatientTab(m_mainWindow.get());
    m_outlineTab = new tab::OutlineTab(m_mainWindow.get());
    
    // Add tabs to main window's central widget
    // The MainWindow should have a QTabWidget or similar container
    // This will be integrated in the MainWindow implementation
    if (m_mainWindow && m_mainWindow->graphWidget()) {
        // Tabs will be added through MainWindow's interface
        m_logger->info("Tabs created successfully");
    }
}

void Application::setupConnections() {
    // Connect serial communication to measurement tab
    if (m_serialComm && m_measurementTab) {
        QObject::connect(m_serialComm.get(), &SerialCommunication::dataReceived,
                        m_measurementTab, &tab::MeasurementTab::readSingleSample);
    }
    
    // Connect measurement tab signals to main window
    if (m_measurementTab && m_mainWindow) {
        QObject::connect(m_measurementTab, &tab::MeasurementTab::newForceSample,
                        [this](double force, double timestamp) {
                            if (m_mainWindow && m_mainWindow->graphWidget()) {
                                m_mainWindow->graphWidget()->addDataPoint(force);
                                m_mainWindow->updateWeightDisplay(force);
                            }
                        });
        
        QObject::connect(m_measurementTab, &tab::MeasurementTab::measurementStarted,
                        [this]() {
                            if (m_serialComm) {
                                m_serialComm->startListening();
                            }
                            if (m_mainWindow) {
                                m_mainWindow->startDataCollection();
                            }
                        });
        
        QObject::connect(m_measurementTab, &tab::MeasurementTab::measurementStopped,
                        [this]() {
                            if (m_serialComm) {
                                m_serialComm->stopListening();
                            }
                            if (m_mainWindow) {
                                m_mainWindow->stopDataCollection();
                            }
                        });
    }
    
    // Connect patient tab signals
    if (m_patientTab && m_measurementTab) {
        QObject::connect(m_patientTab, &tab::PatientTab::patientAdded,
                        [this](const QString& pesel, const QString& firstName, const QString& lastName) {
                            m_logger->info(QString("Patient added: %1 %2 (%3)").arg(firstName).arg(lastName).arg(pesel));
                        });
    }
    
    // Connect outline tab signals to measurement tab
    if (m_outlineTab && m_measurementTab) {
        QObject::connect(m_outlineTab, &tab::OutlineTab::requestGameStart,
                        [this](const QString& gameId, const ExerciseData& exerciseData) {
                            m_logger->info(QString("Starting game: %1 for exercise: %2").arg(gameId).arg(exerciseData.name));
                            // Setup target force based on exercise parameters
                            if (m_measurementTab) {
                                m_measurementTab->setTargetForce(exerciseData.targetRepetitions > 0 ? 
                                    exerciseData.targetRepetitions * 10.0 : 50.0);
                            }
                        });
    }
    
    m_logger->info("Component connections established");
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
