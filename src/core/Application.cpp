#include "core/Application.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "core/Authentication.hpp"
#include "core/DebugManager.hpp"
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
#include <iostream>
#include <sstream>

namespace biofeedback {

Application::Application(int& argc, char* argv[])
    : QApplication(argc, argv)
    , m_configManager(nullptr)
    , m_logger(nullptr)
    , m_mainWindow(nullptr)
    , m_dataManager(nullptr)
    , m_authentication(nullptr)
    , m_debugManager(nullptr)
    , m_patientTab(nullptr)
    , m_measurementTab(nullptr)
    , m_outlineTab(nullptr)
    , m_state(ApplicationState::NotInitialized)
    , m_startTime(QDateTime::currentDateTime())
{
}

Application::~Application()
{
    shutdown();
}

int Application::run()
{
    try {
        if (!initialize()) {
            std::cerr << "[ERROR] Application initialization failed" << std::endl;
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
        
        setState(ApplicationState::Running);
        
        // Show main window after successful login
        if (m_mainWindow) {
            m_mainWindow->show();
        }
        
        // Run Qt event loop
        int status = exec();
        
        setState(ApplicationState::Shutdown);
        return status;
    } catch (const std::exception& e) {
        handleException(&e, "Application::run");
        return EXIT_FAILURE;
    } catch (...) {
        std::exception_ptr eptr = std::current_exception();
        try {
            if (eptr) std::rethrow_exception(eptr);
        } catch (const std::exception& ex) {
            handleException(&ex, "Application::run(unknown)");
        }
        return EXIT_FAILURE;
    }
}

void Application::shutdown()
{
    QMutexLocker locker(&m_mutex);
    
    ApplicationState currentState = m_state.load();
    if (currentState == ApplicationState::ShuttingDown || 
        currentState == ApplicationState::Shutdown) {
        return;
    }
    
    setState(ApplicationState::ShuttingDown);
    
    std::cout << "Shutting down Biofeedback Application..." << std::endl;
    
    // Update statistics
    m_statistics.shutdownCount++;
    if (m_startTime.isValid()) {
        qint64 seconds = m_startTime.secsTo(QDateTime::currentDateTime());
        if (seconds > 0) {
            m_statistics.uptimeSeconds += static_cast<uint64_t>(seconds);
        }
    }
    
    // Log shutdown
    if (m_logger) {
        m_logger->info("Application shutdown complete");
    }
    
    // Clean up resources in reverse order of creation
    m_outlineTab.reset();
    m_measurementTab.reset();
    m_patientTab.reset();
    m_mainWindow.reset();
    m_dataManager.reset();
    m_authentication.reset();
    m_logger.reset();
    m_configManager.reset();
    // DebugManager is a singleton, don't delete it
    
    setState(ApplicationState::Shutdown);
}

bool Application::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_state.load() != ApplicationState::NotInitialized) {
        return true; // Already initialized
    }
    
    setState(ApplicationState::Initializing);
    
    try {
        m_statistics.startupCount++;
        
        // Initialize DebugManager first (singleton)
        m_debugManager = &core::DebugManager::instance();
        
        // Load configuration first
        if (!loadConfiguration()) {
            m_statistics.initializationErrors++;
            setState(ApplicationState::NotInitialized);
            return false;
        }
        
        // Initialize logger
        m_logger = std::make_unique<Logger>(*m_configManager);
        m_logger->info("Starting Biofeedback Application");
        
        // Initialize authentication
        m_authentication = std::make_unique<Authentication>("config/users.json");
        // Security: In production, use secure key management (e.g., environment variable or secure vault)
        const char* envKey = std::getenv("BIOFEEDBACK_ENCRYPTION_KEY");
        std::string encryptionKey = envKey ? envKey : "BiofeedbackApp2024SecureKey!";
        m_authentication->setEncryptionKey(encryptionKey);
        
        // Check if passwords need encryption
        int encryptedCount = m_authentication->encryptPasswordsIfNeeded(encryptionKey);
        if (encryptedCount > 0) {
            m_logger->info(QString("Encrypted %1 user password(s)").arg(encryptedCount).toStdString());
        }
        
        // Initialize data manager
        m_dataManager = std::make_unique<DataManager>();
        
        // Create main window and tabs
        createMainWindow();
        
        // Setup connections between components
        setupConnections();
        
        setState(ApplicationState::Initialized);
        m_logger->info("Application initialized successfully");
        
        return true;
    }
    catch (const std::exception& e) {
        m_statistics.initializationErrors++;
        std::cerr << "Initialization error: " << e.what() << std::endl;
        if (m_logger) {
            m_logger->error(std::string("Initialization failed: ") + e.what());
        }
        if (m_debugManager) {
            m_debugManager->sendDebugMessage(
                QString("Initialization failed: %1").arg(e.what()),
                core::DebugLevel::CRITICAL,
                "Application"
            );
        }
        setState(ApplicationState::NotInitialized);
        return false;
    }
    catch (...) {
        m_statistics.initializationErrors++;
        std::cerr << "Unknown initialization error" << std::endl;
        setState(ApplicationState::NotInitialized);
        return false;
    }
}

bool Application::showLoginDialog()
{
    gui::LoginDialog loginDialog(*m_authentication);
    
    if (loginDialog.exec() == QDialog::Accepted && loginDialog.loginSuccess()) {
        // Security: Do not log raw username - use redacted form
        std::string redactedUsername = "[REDACTED]";
        std::cout << "User logged in: " << redactedUsername
                  << " (Role: " << loginDialog.getRole().toStdString() << ")" << std::endl;
        
        m_logger->info(QString("User logged in: [REDACTED] (%1)")
            .arg(loginDialog.getRole()).toStdString());
        
        // Log to debug terminal before starting data collection
        if (m_mainWindow && m_mainWindow->debugTerminal()) {
            m_mainWindow->addDebugMessage(
                QString("User logged in: [REDACTED] (%1)").arg(loginDialog.getRole()),
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
    try {
        m_configManager = std::make_unique<ConfigManager>();
        
        // Try to load from default location
        std::string configPath = "config/config.json";
        
        if (!m_configManager->load(configPath)) {
            std::cerr << "Warning: Could not load configuration from " << configPath << std::endl;
            std::cerr << "Using default configuration" << std::endl;
            m_configManager->setDefaults();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        if (m_debugManager) {
            m_debugManager->sendDebugMessage(
                QString("Configuration load error: %1").arg(e.what()),
                core::DebugLevel::ERROR,
                "Application"
            );
        }
        return false;
    }
}

// ============================================================================
// GETTER METHODS
// ============================================================================

ConfigManager& Application::getConfigManager()
{
    if (!m_configManager) {
        throw std::runtime_error("ConfigManager not initialized");
    }
    return *m_configManager;
}

Logger& Application::getLogger()
{
    if (!m_logger) {
        throw std::runtime_error("Logger not initialized");
    }
    return *m_logger;
}

DataManager& Application::getDataManager()
{
    if (!m_dataManager) {
        throw std::runtime_error("DataManager not initialized");
    }
    return *m_dataManager;
}

gui::MainWindow* Application::getMainWindow()
{
    return m_mainWindow.get();
}

Authentication& Application::getAuthentication()
{
    if (!m_authentication) {
        throw std::runtime_error("Authentication not initialized");
    }
    return *m_authentication;
}

core::DebugManager& Application::getDebugManager()
{
    if (!m_debugManager) {
        throw std::runtime_error("DebugManager not initialized");
    }
    return *m_debugManager;
}

ApplicationState Application::getState() const
{
    return m_state.load();
}

bool Application::isRunning() const
{
    return m_state.load() == ApplicationState::Running;
}

ApplicationStatistics Application::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return m_statistics;
}

void Application::resetStatistics()
{
    QMutexLocker locker(&m_mutex);
    m_statistics.reset();
}

std::string Application::getStatisticsSummary() const
{
    return m_statistics.getSummary();
}

void Application::registerCriticalErrorCallback(std::function<void(const std::string&)> callback)
{
    QMutexLocker locker(&m_mutex);
    m_criticalErrorCallbacks.push_back(callback);
}

void Application::setState(ApplicationState newState)
{
    ApplicationState oldState = m_state.exchange(newState);
    if (oldState != newState) {
        emit stateChanged(oldState, newState);
    }
}

void Application::logMessage(DebugManagerLogLevel level, const std::string& message, const std::string& source) const
{
    if (m_debugManager) {
        core::DebugLevel debugLevel = static_cast<core::DebugLevel>(static_cast<int>(level));
        m_debugManager->sendDebugMessage(QString::fromStdString(message), debugLevel, QString::fromStdString(source));
    }
}

void Application::handleException(const std::exception* e, const std::string& context)
{
    m_statistics.unhandledExceptions++;
    
    std::string errorMsg = e ? e->what() : "Unknown exception";
    std::cerr << "[EXCEPTION] in " << context << ": " << errorMsg << std::endl;
    
    if (m_logger) {
        m_logger->error(context + ": " + errorMsg);
    }
    
    if (m_debugManager) {
        m_debugManager->sendDebugMessage(
            QString("Exception in %1: %2").arg(QString::fromStdString(context)).arg(QString::fromStdString(errorMsg)),
            core::DebugLevel::CRITICAL,
            "Application"
        );
    }
    
    // Notify critical error callbacks
    QMutexLocker locker(&m_mutex);
    for (auto& callback : m_criticalErrorCallbacks) {
        try {
            callback(context + ": " + errorMsg);
        } catch (...) {
            // Ignore callback errors
        }
    }
    
    emit criticalError(QString::fromStdString(context + ": " + errorMsg));
}

// ============================================================================
// APPLICATIONSTATISTICS IMPLEMENTATION
// ============================================================================

std::string ApplicationStatistics::getSummary() const
{
    std::ostringstream oss;
    oss << "=== Application Statistics ===" << std::endl;
    oss << "Startups: " << startupCount.load() << std::endl;
    oss << "Shutdowns: " << shutdownCount.load() << std::endl;
    oss << "Login attempts: " << loginAttempts.load() << std::endl;
    oss << "Login successes: " << loginSuccesses.load() << std::endl;
    oss << "Login failures: " << loginFailures.load() << std::endl;
    oss << "Initialization errors: " << initializationErrors.load() << std::endl;
    oss << "Unhandled exceptions: " << unhandledExceptions.load() << std::endl;
    oss << "Data samples processed: " << dataSamplesProcessed.load() << std::endl;
    oss << "Uptime (seconds): " << uptimeSeconds.load() << std::endl;
    return oss.str();
}

void ApplicationStatistics::reset()
{
    startupCount = 0;
    shutdownCount = 0;
    loginAttempts = 0;
    loginSuccesses = 0;
    loginFailures = 0;
    initializationErrors = 0;
    unhandledExceptions = 0;
    dataSamplesProcessed = 0;
    uptimeSeconds = 0;
}

} // namespace biofeedback
