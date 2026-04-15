#ifndef BIOFEEDBACK_APPLICATION_HPP
#define BIOFEEDBACK_APPLICATION_HPP

#include <QApplication>
#include <QMainWindow>
#include <memory>
#include <string>

namespace biofeedback {

class ConfigManager;
class Logger;
class DataManager;
class SerialCommunication;

namespace gui {
    class MainWindow;
}

namespace tab {
    class MeasurementTab;
    class PatientTab;
    class OutlineTab;
}

/**
 * @brief Main application class managing the biofeedback system
 * 
 * Handles initialization, configuration, and lifecycle of the application.
 * Follows medical software standards for data integrity and safety.
 * Uses Qt framework for GUI components.
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
    
    /**
     * @brief Get the main window object
     * @return gui::MainWindow* Pointer to main window
     */
    gui::MainWindow* getMainWindow() { return m_mainWindow.get(); }
    
    /**
     * @brief Get the measurement tab
     * @return tab::MeasurementTab* Pointer to measurement tab
     */
    tab::MeasurementTab* getMeasurementTab() { return m_measurementTab; }
    
    /**
     * @brief Get the patient tab
     * @return tab::PatientTab* Pointer to patient tab
     */
    tab::PatientTab* getPatientTab() { return m_patientTab; }
    
    /**
     * @brief Get the outline tab
     * @return tab::OutlineTab* Pointer to outline tab
     */
    tab::OutlineTab* getOutlineTab() { return m_outlineTab; }

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
     * @brief Create and setup all tabs
     */
    void createTabs();
    
    /**
     * @brief Load application configuration
     * @return true if configuration loaded successfully
     */
    bool loadConfiguration();
    
    /**
     * @brief Setup signal/slot connections between components
     */
    void setupConnections();

private:
    std::unique_ptr<QApplication> m_qtApp;
    std::unique_ptr<ConfigManager> m_configManager;
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<gui::MainWindow> m_mainWindow;
    std::unique_ptr<DataManager> m_dataManager;
    std::unique_ptr<SerialCommunication> m_serialComm;
    
    // Tabs
    tab::MeasurementTab* m_measurementTab;
    tab::PatientTab* m_patientTab;
    tab::OutlineTab* m_outlineTab;
    
    bool m_initialized;
    bool m_running;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_APPLICATION_HPP
