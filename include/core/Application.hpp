#ifndef BIOFEEDBACK_APPLICATION_HPP
#define BIOFEEDBACK_APPLICATION_HPP

#include <QApplication>
#include <string>
#include <memory>

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

namespace biofeedback {

/**
 * @brief Main application class managing the biofeedback system
 * 
 * Handles initialization, configuration, and lifecycle of the application.
 * Follows medical software standards for data integrity and safety.
 */
class Application : public QApplication {
    Q_OBJECT
public:
    /**
     * @brief Construct a new Application object
     * @param argc Command line argument count
     * @param argv Command line argument values
     */
    Application(int& argc, char* argv[]);
    
    /**
     * @brief Destroy the Application object
     */
    ~Application() override;
    
    // Disable copying
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    
    /**
     * @brief Run the application main loop
     * @return int Exit code
     */
    int run();
    
    /**
     * @brief Gracefully shutdown the application
     */
    void shutdown();
    
    /**
     * @brief Get the Config Manager object
     * @return ConfigManager& Reference to config manager
     */
    ConfigManager& getConfigManager() { return *m_configManager; }
    
    /**
     * @brief Get the Logger object
     * @return Logger& Reference to logger
     */
    Logger& getLogger() { return *m_logger; }
    
    /**
     * @brief Get the Data Manager object
     * @return DataManager& Reference to data manager
     */
    DataManager& getDataManager() { return *m_dataManager; }
    
    /**
     * @brief Get the main window
     * @return gui::MainWindow* Pointer to main window
     */
    gui::MainWindow* getMainWindow() { return m_mainWindow.get(); }
    
    /**
     * @brief Get the Authentication object
     * @return Authentication& Reference to authentication manager
     */
    Authentication& getAuthentication() { return *m_authentication; }

private:
    /**
     * @brief Initialize all application components
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Create the main window and tabs
     */
    void createMainWindow();
    
    /**
     * @brief Load application configuration
     * @return true if configuration loaded successfully
     */
    bool loadConfiguration();
    
    /**
     * @brief Setup signal/slot connections between components
     */
    void setupConnections();
    
    /**
     * @brief Show login dialog and authenticate user
     * @return true if login successful
     */
    bool showLoginDialog();

private:
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<gui::MainWindow> m_mainWindow;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr<Authentication> m_authentication;
    
    // Tabs - using unique_ptr for proper memory management
    std::unique_ptr<tab::PatientTab> m_patientTab;
    std::unique_ptr<tab::MeasurementTab> m_measurementTab;
    std::unique_ptr<tab::OutlineTab> m_outlineTab;
    
    bool m_initialized;
    bool m_running;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_APPLICATION_HPP
