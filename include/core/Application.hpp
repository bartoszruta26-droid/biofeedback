#ifndef BIOFEEDBACK_APPLICATION_HPP
#define BIOFEEDBACK_APPLICATION_HPP

#include <QApplication>
#include <string>
#include <memory>
#include <atomic>
#include <QMutex>
#include <QDateTime>
#include <functional>

// Include full definitions for classes used with std::unique_ptr and in connect()
#include "core/ConfigManager.hpp"
#include "core/Logger.hpp"
#include "core/Authentication.hpp"
#include "data/DataManager.hpp"
#include "gui/MainWindow.hpp"
#include "gui/LoginDialog.hpp"
#include "tab/PatientTab.hpp"
#include "tab/MeasurementTab.hpp"
#include "tab/OutlineTab.hpp"

// Forward declaration for DebugManager (defined in core namespace)
namespace core { class DebugManager; enum class DebugManagerLogLevel; }

namespace biofeedback {

/**
 * @brief Application state enumeration for tracking lifecycle
 */
enum class ApplicationState {
    NotInitialized = 0,   ///< Application not yet initialized
    Initializing = 1,     ///< Initialization in progress
    Initialized = 2,      ///< Successfully initialized
    Running = 3,          ///< Application running
    ShuttingDown = 4,     ///< Shutdown in progress
    Shutdown = 5          ///< Fully shutdown
};

/**
 * @brief Statistics tracking for Application operations
 */
struct ApplicationStatistics {
    std::atomic<uint64_t> startupCount{0};         ///< Number of application startups
    std::atomic<uint64_t> shutdownCount{0};        ///< Number of application shutdowns
    std::atomic<uint64_t> loginAttempts{0};        ///< Total login attempts
    std::atomic<uint64_t> loginSuccesses{0};       ///< Successful logins
    std::atomic<uint64_t> loginFailures{0};        ///< Failed login attempts
    std::atomic<uint64_t> initializationErrors{0}; ///< Initialization errors
    std::atomic<uint64_t> unhandledExceptions{0};  ///< Unhandled exceptions caught
    std::atomic<uint64_t> dataSamplesProcessed{0}; ///< Data samples processed
    std::atomic<uint64_t> uptimeSeconds{0};        ///< Total uptime in seconds
    
    /**
     * @brief Get summary of statistics as formatted string
     * @return std::string Formatted statistics summary
     */
    std::string getSummary() const;
    
    /**
     * @brief Reset all statistics to zero
     */
    void reset();
};

/**
 * @brief Main application class managing the biofeedback system
 * 
 * Handles initialization, configuration, and lifecycle of the application.
 * Follows medical software standards for data integrity and safety.
 * Includes comprehensive error handling, logging, and statistics tracking.
 * 
 * @threadsafe This class is thread-safe for use from multiple threads.
 */
class Application : public QApplication {
    Q_OBJECT
    
public:
    /**
     * @brief Construct a new Application object
     * @param argc Command line argument count
     * @param argv Command line argument values
     * 
     * @note Initializes with default state, actual initialization happens in initialize()
     */
    Application(int& argc, char* argv[]);
    
    /**
     * @brief Destroy the Application object
     * Ensures proper cleanup of all resources
     */
    ~Application() override;
    
    // Disable copying to prevent application state inconsistencies
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    /**
     * @brief Run the application main loop
     * @return int Exit code (EXIT_SUCCESS or EXIT_FAILURE)
     * 
     * @note Calls initialize() internally, handles exceptions gracefully
     */
    int run();
    
    /**
     * @brief Gracefully shutdown the application
     * 
     * @note Thread-safe, can be called from any thread
     */
    void shutdown();
    
    /**
     * @brief Get the Config Manager object
     * @return ConfigManager& Reference to config manager
     * 
     * @throws std::runtime_error if config manager is not initialized
     */
    ConfigManager& getConfigManager();
    
    /**
     * @brief Get the Logger object
     * @return Logger& Reference to logger
     * 
     * @throws std::runtime_error if logger is not initialized
     */
    Logger& getLogger();
    
    /**
     * @brief Get the Data Manager object
     * @return DataManager& Reference to data manager
     * 
     * @throws std::runtime_error if data manager is not initialized
     */
    DataManager& getDataManager();
    
    /**
     * @brief Get the main window
     * @return gui::MainWindow* Pointer to main window (nullptr if not created)
     */
    gui::MainWindow* getMainWindow();
    
    /**
     * @brief Get the Authentication object
     * @return Authentication& Reference to authentication manager
     * 
     * @throws std::runtime_error if authentication is not initialized
     */
    Authentication& getAuthentication();
    
    /**
     * @brief Get the Debug Manager object
     * @return core::DebugManager& Reference to debug manager
     */
    core::DebugManager& getDebugManager();
    
    /**
     * @brief Get current application state
     * @return ApplicationState Current state
     */
    ApplicationState getState() const;
    
    /**
     * @brief Check if application is running
     * @return true if application is in Running state
     */
    bool isRunning() const;
    
    /**
     * @brief Get application statistics
     * @return ApplicationStatistics Copy of current statistics
     */
    ApplicationStatistics getStatistics() const;
    
    /**
     * @brief Reset application statistics
     */
    void resetStatistics();
    
    /**
     * @brief Get formatted statistics summary
     * @return std::string Formatted statistics summary
     */
    std::string getStatisticsSummary() const;
    
    /**
     * @brief Register a callback for critical errors
     * @param callback Function to call on critical error
     * 
     * @note Callback should be thread-safe
     */
    void registerCriticalErrorCallback(std::function<void(const std::string&)> callback);

signals:
    /**
     * @brief Emitted when application state changes
     * @param oldState Previous state
     * @param newState New state
     */
    void stateChanged(ApplicationState oldState, ApplicationState newState);
    
    /**
     * @brief Emitted on critical error
     * @param error Error message
     */
    void criticalError(const QString& error);

private:
    /**
     * @brief Initialize all application components
     * @return true if initialization successful, false otherwise
     * 
     * @note Thread-safe, uses gentle error handling
     */
    bool initialize();
    
    /**
     * @brief Create the main window and tabs
     * 
     * @note Logs creation process, handles exceptions
     */
    void createMainWindow();
    
    /**
     * @brief Load application configuration
     * @return true if configuration loaded successfully
     * 
     * @note Falls back to defaults if file not found
     */
    bool loadConfiguration();
    
    /**
     * @brief Setup signal/slot connections between components
     * 
     * @note Comprehensive connection setup with error handling
     */
    void setupConnections();
    
    /**
     * @brief Show login dialog and authenticate user
     * @return true if login successful, false otherwise
     * 
     * @note Tracks login attempts in statistics
     */
    bool showLoginDialog();
    
    /**
     * @brief Update application state
     * @param newState New state to transition to
     * 
     * @note Thread-safe, emits stateChanged signal
     */
    void setState(ApplicationState newState);
    
    /**
     * @brief Log application message with level
     * @param level Log level
     * @param message Message to log
     * @param source Source component name
     */
    void logMessage(DebugManagerLogLevel level, const std::string& message, const std::string& source = "") const;
    
    /**
     * @brief Handle unhandled exception
     * @param e Exception pointer
     * @param context Context where exception occurred
     */
    void handleException(const std::exception* e, const std::string& context);

private:
    mutable QMutex m_mutex;                      ///< Mutex for thread-safe access
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<gui::MainWindow> m_mainWindow;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr<Authentication> m_authentication;
    std::unique_ptr<DebugManager> m_debugManager;
    
    // Tabs - using unique_ptr for proper memory management
    std::unique_ptr<tab::PatientTab> m_patientTab;
    std::unique_ptr<tab::MeasurementTab> m_measurementTab;
    std::unique_ptr<tab::OutlineTab> m_outlineTab;
    
    std::atomic<ApplicationState> m_state;       ///< Current application state
    ApplicationStatistics m_statistics;          ///< Operation statistics
    QDateTime m_startTime;                       ///< Application start time
    std::vector<std::function<void(const std::string&)>> m_criticalErrorCallbacks;
    
    static constexpr int STARTUP_TIMEOUT_MS = 30000;  ///< Maximum startup time in ms
    static constexpr int SHUTDOWN_TIMEOUT_MS = 10000; ///< Maximum shutdown time in ms
};

} // namespace biofeedback

#endif // BIOFEEDBACK_APPLICATION_HPP
