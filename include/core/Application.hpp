#ifndef BIOFEEDBACK_APPLICATION_HPP
#define BIOFEEDBACK_APPLICATION_HPP

#include <gtk/gtk.h>
#include <string>
#include <memory>

namespace biofeedback {

class ConfigManager;
class Logger;
class MainWindow;
class DataManager;
class HX711Sensor;

/**
 * @brief Main application class managing the biofeedback system
 * 
 * Handles initialization, configuration, and lifecycle of the application.
 * Follows medical software standards for data integrity and safety.
 */
class Application {
public:
    /**
     * @brief Construct a new Application object
     * @param argc Command line argument count
     * @param argv Command line argument values
     */
    Application(int argc, char* argv[]);
    
    /**
     * @brief Destroy the Application object
     */
    ~Application();
    
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

private:
    GtkApplication* m_gtkApp;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr	HX711Sensor> m_sensor;
    
    bool m_initialized;
    bool m_running;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_APPLICATION_HPP
