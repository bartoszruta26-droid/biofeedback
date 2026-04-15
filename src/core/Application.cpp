#include "core/Application.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "gui/MainWindow.hpp"
#include "data/DataManager.hpp"
#include "tab/PatientTab.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/OutlineTab.hpp"
#include <QTabWidget>
#include <QMessageBox>

namespace biofeedback {

Application::Application(int& argc, char* argv[])
    : QApplication(argc, argv)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_mainWindow(nullptr)
    , m_dataManager(nullptr)
    , m_patientTab(nullptr)
    , m_measurementTab(nullptr)
    , m_outlineTab(nullptr)
    , m_initialized(false)
    , m_running(false)
{
}

Application::~Application()
{
    shutdown();
}

int Application::run()
{
    if (!initialize()) {
        return EXIT_FAILURE;
    }
    
    m_running = true;
    
    // Show main window
    m_mainWindow->show();
    
    // Run Qt event loop
    int status = exec();
    
    m_running = false;
    return status;
}

void Application::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    std::cout << "Shutting down Biofeedback Application..." << std::endl;
    
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

bool Application::initialize()
{
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
        
        // Create main window and tabs
        createMainWindow();
        
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

void Application::createMainWindow()
{
    // Create main window
    m_mainWindow = std::make_unique<gui::MainWindow>();
    
    // Create tabs
    m_patientTab = std::make_unique<tab::PatientTab>();
    m_measurementTab = std::make_unique<tab::MeasurementTab>();
    m_outlineTab = std::make_unique<tab::OutlineTab>();
    
    // Get central widget from MainWindow to add tabs
    // Note: This assumes MainWindow has a way to access its central widget
    // In a real implementation, you might want to refactor MainWindow to support tabs
}

void Application::setupConnections()
{
    // Connect MainWindow signals to tab slots
    connect(m_mainWindow.get(), &gui::MainWindow::dataCollectionStarted,
            m_measurementTab.get(), &tab::MeasurementTab::startMeasurement);
    
    connect(m_mainWindow.get(), &gui::MainWindow::dataCollectionStopped,
            m_measurementTab.get(), &tab::MeasurementTab::stopMeasurement);
    
    connect(m_mainWindow.get(), &gui::MainWindow::samplingRateChanged,
            m_measurementTab.get(), [this](int rate) {
                // Update measurement tab sampling rate
            });
    
    // Connect MeasurementTab signals to MainWindow slots
    connect(m_measurementTab.get(), &tab::MeasurementTab::newForceSample,
            m_mainWindow.get(), [this](double force, double timestamp) {
                m_mainWindow->updateWeightDisplay(force);
                m_mainWindow->graphWidget()->addDataPoint(force);
            });
    
    connect(m_measurementTab.get(), &tab::MeasurementTab::measurementStarted,
            m_mainWindow.get(), [this]() {
                m_mainWindow->startDataCollection();
            });
    
    connect(m_measurementTab.get(), &tab::MeasurementTab::measurementStopped,
            m_mainWindow.get(), [this]() {
                m_mainWindow->stopDataCollection();
            });
    
    // Connect PatientTab signals
    connect(m_patientTab.get(), &tab::PatientTab::patientAdded,
            this, [this](const QString& pesel, const QString& firstName, const QString& lastName) {
                m_logger->info(QString("Patient added: %1 %2 (%3)").arg(firstName).arg(lastName).arg(pesel));
            });
    
    // Connect OutlineTab signals
    connect(m_outlineTab.get(), &tab::OutlineTab::sessionStarted,
            this, [this](const QString& outlineId) {
                m_logger->info(QString("Session started with outline: %1").arg(outlineId));
            });
    
    connect(m_outlineTab.get(), &tab::OutlineTab::requestGameStart,
            this, [this](const QString& gameId, const ExerciseData& exerciseData) {
                m_logger->info(QString("Starting game: %1 for exercise: %2").arg(gameId).arg(exerciseData.name));
            });
}

bool Application::loadConfiguration()
{
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
