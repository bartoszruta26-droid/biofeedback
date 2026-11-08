#include "core/Application.hpp"
#include "api/HttpApiServer.hpp"
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "core/Authentication.hpp"
#include "core/DebugManager.hpp"
#include "core/RuntimePaths.hpp"
#include <QDir>
#include <QFile>
#include <QFileDevice>
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
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QSignalBlocker>
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
    , m_patientTab(nullptr)
    , m_measurementTab(nullptr)
    , m_outlineTab(nullptr)
    , m_state(ApplicationState::NotInitialized)
    , m_startTime(QDateTime::currentDateTime())
{
    setOrganizationName("Biofeedback");
    setApplicationName("Biofeedback");
    setOrganizationDomain("biofeedback.local");
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
    
    // MainWindow is the sole owner of its child tabs. Destroy it once, then
    // invalidate the non-owning observers held by Application.
    if (m_localApiServer) {
        m_localApiServer->close();
        m_localApiServer.reset();
    }
    m_mainWindow.reset();
    m_outlineTab = nullptr;
    m_measurementTab = nullptr;
    m_patientTab = nullptr;
    m_dataManager.reset();
    m_sharedResources.reset();
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
        
        // Initialize DebugManager first (singleton) - store pointer to singleton instance
        // Note: We don't own the singleton, so we use a raw pointer instead of unique_ptr
        core::DebugManager* debugManagerInstance = &core::DebugManager::instance();
        
        // Load configuration first
        if (!loadConfiguration()) {
            m_statistics.initializationErrors++;
            setState(ApplicationState::NotInitialized);
            return false;
        }
        
        // Initialize logger
        m_logger = std::make_unique<Logger>(*m_configManager);
        m_logger->info("Starting Biofeedback Application");
        
        // File encryption is configurable. Development defaults to plaintext files;
        // account passwords remain one-way PBKDF2 hashes regardless of this setting.
        const bool encryptionEnabled = m_configManager->getBool("encryption.enabled", false);
        const char* envKey = std::getenv("BIOFEEDBACK_ENCRYPTION_KEY");
        const std::string encryptionKey = envKey ? std::string(envKey) : std::string();
        if (encryptionEnabled && encryptionKey.size() < 32) {
            throw std::runtime_error(
                "Encryption is enabled in config/config.json but BIOFEEDBACK_ENCRYPTION_KEY "
                "is missing or shorter than 32 bytes."
            );
        }

        // The administrator and regular-user database follows the same file policy.
        const QString usersPath = runtime::ensureUserConfigFile("users.json");
        if (usersPath.isEmpty())
            throw std::runtime_error("Cannot prepare the per-user account database.");
        m_authentication = std::make_unique<Authentication>(usersPath.toStdString());
        m_authentication->setFileEncryption(encryptionEnabled, encryptionKey);

        // Patient storage backend is explicit. Never silently fall back from MariaDB
        // to local files: that could split a shared patient registry across clients.
        const std::string patientBackend =
            m_configManager->getString("storage.patient_backend", "files");
        if (patientBackend == "mariadb") {
            throw std::runtime_error(
                "MariaDB patient storage is selected in config, but the MariaDB repository "
                "adapter is not implemented yet. No patient data was written to local files."
            );
        }
        if (patientBackend != "files") {
            throw std::runtime_error("Unsupported storage.patient_backend value.");
        }

        // Patient-linked files follow the same policy.
        m_dataManager = std::make_unique<DataManager>();
        if (!encryptionKey.empty()) m_dataManager->setEncryptionKey(encryptionKey);
        m_dataManager->setEncryptionEnabled(encryptionEnabled);

        // One repository instance is shared by Desktop and the in-process API.
        m_sharedResources = std::make_shared<data::JsonResourceRepository>(
            QDir(runtime::appDataDirectory()).filePath(QStringLiteral("shared-data")),
            QString::fromStdString(encryptionKey), encryptionEnabled);
        if (!m_sharedResources->isReady()) {
            throw std::runtime_error(
                "Shared data repository initialization failed: " +
                m_sharedResources->initializationError().toStdString());
        }
        
        // Store debug manager pointer for later use (we don't own it)
        // m_debugManager is not used as unique_ptr anymore since DebugManager is a singleton
        (void)debugManagerInstance; // Suppress unused variable warning
        
        // Create main window and tabs
        createMainWindow();
        
        // Setup connections between components
        setupConnections();

        // The API and Desktop share this exact repository instance.
        m_localApiServer = std::make_unique<api::HttpApiServer>(
            usersPath, m_sharedResources,
            encryptionEnabled ? QString::fromStdString(encryptionKey) : QString());
        if (!m_localApiServer->listen(QHostAddress::LocalHost, 8765)) {
            m_logger->warning("Local shared API could not bind to 127.0.0.1:8765");
            m_localApiServer.reset();
        }

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
        // Use singleton DebugManager directly
        core::DebugManager::instance().sendDebugMessage(
            QString("Initialization failed: %1").arg(e.what()),
            core::DebugLevel::CRITICAL,
            "Application"
        );
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
    try {
        // MainWindow constructs and owns all tab widgets. Application only observes them.
        m_mainWindow = std::make_unique<gui::MainWindow>();
        if (!m_mainWindow) throw std::runtime_error("Failed to create MainWindow");

        m_patientTab = m_mainWindow->patientTab();
        m_measurementTab = m_mainWindow->measurementTab();
        m_outlineTab = m_mainWindow->outlineTab();
        if (!m_patientTab || !m_measurementTab || !m_outlineTab) {
            m_mainWindow.reset();
            m_patientTab = nullptr;
            m_measurementTab = nullptr;
            m_outlineTab = nullptr;
            throw std::runtime_error("MainWindow did not create all required tab widgets");
        }

        if (m_logger) m_logger->info("Main window and owned tabs created successfully");
    } catch (const std::exception& e) {
        if (m_logger) m_logger->error(std::string("Failed to create main window: ") + e.what());
        throw;
    }
}
void Application::setupConnections()
{
    // Connect MainWindow signals to tab slots
    connect(m_mainWindow.get(), &gui::MainWindow::dataCollectionStarted,
            m_measurementTab, &tab::MeasurementTab::startMeasurement);
    
    connect(m_mainWindow.get(), &gui::MainWindow::dataCollectionStopped,
            m_measurementTab, &tab::MeasurementTab::stopMeasurement);
    
    connect(m_mainWindow.get(), &gui::MainWindow::samplingRateChanged,
            m_measurementTab, [this](int /*rate*/) {
                // Update measurement tab sampling rate
            });
    
    // Connect MeasurementTab signals to MainWindow slots
    connect(m_measurementTab, &tab::MeasurementTab::newForceSample,
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
    
    connect(m_measurementTab, &tab::MeasurementTab::measurementStarted,
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
    
    connect(m_measurementTab, &tab::MeasurementTab::measurementStopped,
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
    
    const auto findPatientByPesel = [this](const QString& pesel, QJsonObject& found) {
        if (!m_sharedResources || pesel.isEmpty()) return false;
        QString error;
        const QJsonArray records = m_sharedResources->list(QStringLiteral("patients"), &error);
        if (!error.isEmpty()) return false;
        for (const QJsonValue& value : records) {
            if (!value.isObject()) continue;
            const QJsonObject record = value.toObject();
            if (record.value(QStringLiteral("pesel")).toString() == pesel) {
                found = record;
                return true;
            }
        }
        return false;
    };

    // Persist and load patient charts through the same repository used by the API.
    connect(m_patientTab, &tab::PatientTab::patientAdded,
            this, [this](const QString&, const QString&, const QString&) {
                if (m_logger) m_logger->info("Patient chart added [personal fields redacted]");
            });


    connect(m_patientTab, &tab::PatientTab::patientRecordChanged,
            this, [this, findPatientByPesel](const QString& pesel, const QString& firstName,
                         const QString& lastName, const QDate& birthDate,
                         const QString& gender, const QString& notes) {
                if (!m_sharedResources || pesel.isEmpty()) return;
                const QJsonObject payload{
                    {QStringLiteral("pesel"), pesel},
                    {QStringLiteral("firstName"), firstName},
                    {QStringLiteral("lastName"), lastName},
                    {QStringLiteral("birthDate"), birthDate.toString(Qt::ISODate)},
                    {QStringLiteral("gender"), gender},
                    {QStringLiteral("medicalHistory"), notes}
                };
                QJsonObject existing;
                QString error;
                bool saved = false;
                if (findPatientByPesel(pesel, existing)) {
                    QJsonObject updated;
                    saved = m_sharedResources->update(
                        QStringLiteral("patients"),
                        existing.value(QStringLiteral("id")).toString(),
                        existing.value(QStringLiteral("version")).toInt(),
                        payload, updated, &error);
                } else {
                    QJsonObject created;
                    saved = m_sharedResources->create(
                        QStringLiteral("patients"), payload, created, &error);
                }
                if (!saved) {
                    if (m_logger) m_logger->error("Shared patient repository write failed: " + error.toStdString());
                    if (m_mainWindow && m_mainWindow->debugTerminal())
                        m_mainWindow->addDebugMessage("Nie udało się zapisać kartoteki pacjenta w serwerze danych.", "ERROR");
                }
            });

    connect(m_patientTab, &tab::PatientTab::patientRecordDeleted,
            this, [this, findPatientByPesel](const QString& pesel) {
                if (!m_sharedResources || pesel.isEmpty()) return;
                QJsonObject existing;
                QString error;
                bool repositoryDeleteSucceeded = false;
                if (findPatientByPesel(pesel, existing)) {
                    repositoryDeleteSucceeded = m_sharedResources->remove(
                        QStringLiteral("patients"),
                        existing.value(QStringLiteral("id")).toString(),
                        existing.value(QStringLiteral("version")).toInt(), &error);
                    if (!repositoryDeleteSucceeded && m_logger)
                        m_logger->warning("Shared patient repository delete failed: " + error.toStdString());
                } else {
                    repositoryDeleteSucceeded = true;
                }
                // Retire the legacy local chart only after the canonical record is deleted.
                if (repositoryDeleteSucceeded) {
                    const QString legacyFile = QDir(runtime::patientDirectory(pesel))
                        .filePath(QStringLiteral("patient.json"));
                    if (QFile::exists(legacyFile)) QFile::remove(legacyFile);
                }
            });

    QSignalBlocker suppressPatientReloadSignals(m_patientTab);
    QSet<QString> loadedPatientIds;
    QString repositoryError;
    const QJsonArray sharedPatients = m_sharedResources
        ? m_sharedResources->list(QStringLiteral("patients"), &repositoryError) : QJsonArray();
    if (!repositoryError.isEmpty()) {
        throw std::runtime_error("Cannot load shared patient records: " + repositoryError.toStdString());
    }
    for (const QJsonValue& value : sharedPatients) {
        if (!value.isObject()) continue;
        const QJsonObject patient = value.toObject();
        const QString pesel = patient.value(QStringLiteral("pesel")).toString();
        if (pesel.isEmpty()) continue;
        loadedPatientIds.insert(pesel);
        m_patientTab->addPatient(
            pesel,
            patient.value(QStringLiteral("firstName")).toString(),
            patient.value(QStringLiteral("lastName")).toString(),
            QDate::fromString(patient.value(QStringLiteral("birthDate")).toString(), Qt::ISODate),
            patient.value(QStringLiteral("gender")).toString(),
            patient.value(QStringLiteral("medicalHistory")).toString());
    }

    // One-time, non-destructive migration from the former per-patient files.
    // Source files are retained until the operator explicitly removes the chart.
    const QString legacyRootPath = QDir(runtime::appDataDirectory()).filePath(QStringLiteral("patients"));
    QDir legacyRoot(legacyRootPath);
    const QStringList legacyDirectories = legacyRoot.entryList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString& directoryName : legacyDirectories) {
        const QString legacyFile = QDir(legacyRoot.filePath(directoryName))
            .filePath(QStringLiteral("patient.json"));
        if (!QFile::exists(legacyFile) || !m_dataManager) continue;
        const PatientData legacy = m_dataManager->loadPatientData(legacyFile.toStdString());
        if (legacy.id.empty() || legacy.pesel.empty()) continue;
        const QString pesel = QString::fromStdString(legacy.pesel);
        if (loadedPatientIds.contains(pesel)) continue;

        const QJsonObject payload{
            {QStringLiteral("pesel"), pesel},
            {QStringLiteral("firstName"), QString::fromStdString(legacy.firstName)},
            {QStringLiteral("lastName"), QString::fromStdString(legacy.lastName)},
            {QStringLiteral("birthDate"), QString::fromStdString(legacy.birthDate)},
            {QStringLiteral("gender"), QString::fromStdString(legacy.gender)},
            {QStringLiteral("medicalHistory"), QString::fromStdString(legacy.medicalHistory)}
        };
        QJsonObject migrated;
        QString migrationError;
        if (m_sharedResources->create(QStringLiteral("patients"), payload, migrated, &migrationError)) {
            loadedPatientIds.insert(pesel);
            m_patientTab->addPatient(
                pesel, QString::fromStdString(legacy.firstName),
                QString::fromStdString(legacy.lastName),
                QDate::fromString(QString::fromStdString(legacy.birthDate), Qt::ISODate),
                QString::fromStdString(legacy.gender),
                QString::fromStdString(legacy.medicalHistory));
        } else if (m_logger) {
            m_logger->error("Legacy patient migration failed: " + migrationError.toStdString());
        }
    }

    connect(m_patientTab, &tab::PatientTab::currentPatientChanged,
            m_measurementTab, &tab::MeasurementTab::onPatientChanged);

    // Connect OutlineTab signals
    connect(m_outlineTab, &tab::OutlineTab::sessionStarted,
            this, [this](const QString& outlineId) {
                m_logger->info(QString("Session started with outline: %1").arg(outlineId).toStdString());
                if (m_mainWindow && m_mainWindow->debugTerminal()) {
                    m_mainWindow->addDebugMessage(
                        QString("Session started with outline: %1").arg(outlineId),
                        "INFO"
                    );
                }
            });
    
    connect(m_outlineTab, &tab::OutlineTab::requestGameStart,
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
        
        // Resolve packaged defaults and keep writable configuration in the
        // current user's XDG config directory, never beside the installed binary.
        const QString configPath = runtime::ensureUserConfigFile("config.json");
        if (configPath.isEmpty() || !m_configManager->load(configPath.toStdString())) {
            std::cerr << "Warning: Could not load user configuration; using safe defaults" << std::endl;
            m_configManager->setDefaults();
        }

        const QString patientRoot = QDir(runtime::appDataDirectory()).filePath("patients");
        QDir().mkpath(patientRoot);
        m_configManager->setValue("data.base_path", patientRoot.toStdString());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading configuration: " << e.what() << std::endl;
        // Use singleton DebugManager directly
        core::DebugManager::instance().sendDebugMessage(
            QString("Configuration load error: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "Application"
        );
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

std::shared_ptr<data::JsonResourceRepository> Application::getSharedResources() const
{
    return m_sharedResources;
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
    return core::DebugManager::instance();
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
    ApplicationStatistics stats;
    stats.startupCount = m_statistics.startupCount.load();
    stats.shutdownCount = m_statistics.shutdownCount.load();
    stats.loginAttempts = m_statistics.loginAttempts.load();
    stats.loginSuccesses = m_statistics.loginSuccesses.load();
    stats.loginFailures = m_statistics.loginFailures.load();
    stats.initializationErrors = m_statistics.initializationErrors.load();
    stats.unhandledExceptions = m_statistics.unhandledExceptions.load();
    stats.dataSamplesProcessed = m_statistics.dataSamplesProcessed.load();
    stats.uptimeSeconds = m_statistics.uptimeSeconds.load();
    return stats;
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

void Application::logMessage(core::DebugLevel level, const std::string& message, const std::string& source) const
{
    core::DebugManager::instance().sendDebugMessage(QString::fromStdString(message), level, QString::fromStdString(source));
}

void Application::handleException(const std::exception* e, const std::string& context)
{
    m_statistics.unhandledExceptions++;
    
    std::string errorMsg = e ? e->what() : "Unknown exception";
    std::cerr << "[EXCEPTION] in " << context << ": " << errorMsg << std::endl;
    
    if (m_logger) {
        m_logger->error(context + ": " + errorMsg);
    }
    
    // Use singleton DebugManager directly
    core::DebugManager::instance().sendDebugMessage(
        QString("Exception in %1: %2").arg(QString::fromStdString(context)).arg(QString::fromStdString(errorMsg)),
        core::DebugLevel::CRITICAL,
        "Application"
    );
    
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
