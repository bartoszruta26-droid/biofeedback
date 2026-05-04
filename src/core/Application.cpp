#include "core/Application.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "core/Authentication.hpp"
#include "gui/MainWindow.hpp"
#include "gui/LoginDialog.hpp"
#include "data/DataManager.hpp"
#include "tab/PatientTab.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/OutlineTab.hpp"
#include <QTabWidget>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

namespace biofeedback {

Application::Application(int& argc, char* argv[])
    : QApplication(argc, argv)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_mainWindow(nullptr)
    , m_dataManager(nullptr)
    , m_authentication(nullptr)
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
    
    // Show login dialog before main window
    if (!showLoginDialog()) {
        std::cout << "Login failed or cancelled. Exiting application." << std::endl;
        if (m_logger) {
            m_logger->warning("User login failed or was cancelled");
        }
        return EXIT_SUCCESS;
    }
    
    m_running = true;
    
    // Show main window after successful login
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
        
        // Initialize authentication
        m_authentication = std::make_unique<Authentication>("config/users.json");
        m_authentication->setEncryptionKey("BiofeedbackApp2024SecureKey!");
        
        // Check if passwords need encryption
        int encryptedCount = m_authentication->encryptPasswordsIfNeeded("BiofeedbackApp2024SecureKey!");
        if (encryptedCount > 0) {
            m_logger->info(QString("Encrypted %1 user password(s)").arg(encryptedCount).toStdString());
        }
        
        // Initialize data manager
        m_dataManager = std::make_unique<DataManager>();
        
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

bool Application::showLoginDialog()
{
    gui::LoginDialog loginDialog(*m_authentication);
    
    if (loginDialog.exec() == QDialog::Accepted && loginDialog.loginSuccess()) {
        std::cout << "User logged in: " << loginDialog.getUsername().toStdString() 
                  << " (Role: " << loginDialog.getRole().toStdString() << ")" << std::endl;
        m_logger->info(QString("User logged in: %1 (%2)")
            .arg(loginDialog.getUsername())
            .arg(loginDialog.getRole()).toStdString());
        
        // Log to debug terminal before starting data collection
        if (m_mainWindow && m_mainWindow->debugTerminal()) {
            m_mainWindow->addDebugMessage(
                QString("User logged in: %1 (%2)").arg(loginDialog.getUsername()).arg(loginDialog.getRole()),
                "INFO"
            );
            m_mainWindow->addDebugMessage("Login successful. Data collection will start after clicking Widok->Rozpocznij", "INFO");
        }
        
        // DO NOT start data collection automatically - wait for user to click menu Widok->Rozpocznij
        // QTimer::singleShot(500, m_mainWindow.get(), &gui::MainWindow::startDataCollection);
        
        return true;
    }
    
    return false;
}

void Application::createMainWindow()
{
    // Create main window
    m_mainWindow = std::unique_ptr<gui::MainWindow>(new gui::MainWindow());
    
    // Create tabs
    m_patientTab = std::unique_ptr<tab::PatientTab>(new tab::PatientTab());
    m_measurementTab = std::unique_ptr<tab::MeasurementTab>(new tab::MeasurementTab());
    m_outlineTab = std::unique_ptr<tab::OutlineTab>(new tab::OutlineTab());
    
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
            m_measurementTab.get(), [this](int /*rate*/) {
                // Update measurement tab sampling rate
            });
    
    // Connect MeasurementTab signals to MainWindow slots
    connect(m_measurementTab.get(), &tab::MeasurementTab::newForceSample,
            m_mainWindow.get(), [this](double force, double /*timestamp*/, bool /*isRaw*/) {
                try {
                    if (m_mainWindow && m_mainWindow->measurementTab()) {
                        m_mainWindow->updateWeightDisplay(force);
                        // GraphWidget is now managed by MeasurementTab - data is added there directly
                        // via ForcePlotWidget
                        
                        // Debug logging for data processing
                        if (m_mainWindow->debugTerminal()) {
                            m_mainWindow->addDebugMessage(
                                QString("Received force data: %1 N").arg(force, 0, 'f', 2), 
                                "DATA"
                            );
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[ERROR] Exception processing force data: " << e.what() << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage(
                            QString("ERROR processing data: %1").arg(e.what()), 
                            "ERROR"
                        );
                    }
                } catch (...) {
                    std::cerr << "[ERROR] Unknown exception processing force data" << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage("ERROR: Unknown exception processing data", "ERROR");
                    }
                }
            });
    
    connect(m_measurementTab.get(), &tab::MeasurementTab::measurementStarted,
            m_mainWindow.get(), [this]() {
                try {
                    m_mainWindow->startDataCollection();
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage("Measurement started by MeasurementTab", "INFO");
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[ERROR] Exception starting measurement: " << e.what() << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage(
                            QString("ERROR starting measurement: %1").arg(e.what()), 
                            "ERROR"
                        );
                    }
                } catch (...) {
                    std::cerr << "[ERROR] Unknown exception starting measurement" << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage("ERROR: Unknown exception starting measurement", "ERROR");
                    }
                }
            });
    
    connect(m_measurementTab.get(), &tab::MeasurementTab::measurementStopped,
            m_mainWindow.get(), [this]() {
                try {
                    m_mainWindow->stopDataCollection();
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage("Measurement stopped by MeasurementTab", "INFO");
                    }
                } catch (const std::exception& e) {
                    std::cerr << "[ERROR] Exception stopping measurement: " << e.what() << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage(
                            QString("ERROR stopping measurement: %1").arg(e.what()), 
                            "ERROR"
                        );
                    }
                } catch (...) {
                    std::cerr << "[ERROR] Unknown exception stopping measurement" << std::endl;
                    if (m_mainWindow && m_mainWindow->debugTerminal()) {
                        m_mainWindow->addDebugMessage("ERROR: Unknown exception stopping measurement", "ERROR");
                    }
                }
            });
    
    // Connect PatientTab signals
    connect(m_patientTab.get(), &tab::PatientTab::patientAdded,
            this, [this](const QString& pesel, const QString& firstName, const QString& lastName) {
                m_logger->info(QString("Patient added: %1 %2 (%3)").arg(firstName).arg(lastName).arg(pesel).toStdString());
                if (m_mainWindow && m_mainWindow->debugTerminal()) {
                    m_mainWindow->addDebugMessage(
                        QString("Patient added: %1 %2 (%3)").arg(firstName).arg(lastName).arg(pesel),
                        "INFO"
                    );
                }
            });
    
    // Connect PatientTab to MeasurementTab for automatic data loading when patient changes
    connect(m_patientTab.get(), &tab::PatientTab::currentPatientChanged,
            m_measurementTab.get(), &tab::MeasurementTab::onPatientChanged);
    
    // Connect OutlineTab signals
    connect(m_outlineTab.get(), &tab::OutlineTab::sessionStarted,
            this, [this](const QString& outlineId) {
                m_logger->info(QString("Session started with outline: %1").arg(outlineId).toStdString());
                if (m_mainWindow && m_mainWindow->debugTerminal()) {
                    m_mainWindow->addDebugMessage(
                        QString("Session started with outline: %1").arg(outlineId),
                        "INFO"
                    );
                }
            });
    
    connect(m_outlineTab.get(), &tab::OutlineTab::requestGameStart,
            this, [this](const QString& gameId, const tab::ExerciseData& exerciseData) {
                m_logger->info(QString("Starting game: %1 for exercise: %2").arg(gameId).arg(exerciseData.name).toStdString());
                if (m_mainWindow && m_mainWindow->debugTerminal()) {
                    m_mainWindow->addDebugMessage(
                        QString("Request to start game: %1 for exercise: %2").arg(gameId).arg(exerciseData.name),
                        "INFO"
                    );
                }
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
