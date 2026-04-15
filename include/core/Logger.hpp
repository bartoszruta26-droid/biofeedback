#ifndef BIOFEEDBACK_LOGGER_HPP
#define BIOFEEDBACK_LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

namespace biofeedback {

class ConfigManager;

/**
 * @brief Logging levels for the application
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

/**
 * @brief Medical-grade logging system with file rotation
 * 
 * Provides thread-safe logging with multiple output destinations
 * and automatic log rotation for compliance with medical standards.
 */
class Logger {
public:
    /**
     * @brief Construct a new Logger object
     * @param config Configuration manager reference
     */
    explicit Logger(ConfigManager& config);
    
    /**
     * @brief Destroy the Logger object
     */
    ~Logger();
    
    // Disable copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    /**
     * @brief Log a debug message
     * @param message Message to log
     */
    void debug(const std::string& message);
    
    /**
     * @brief Log an info message
     * @param message Message to log
     */
    void info(const std::string& message);
    
    /**
     * @brief Log a warning message
     * @param message Message to log
     */
    void warning(const std::string& message);
    
    /**
     * @brief Log an error message
     * @param message Message to log
     */
    void error(const std::string& message);
    
    /**
     * @brief Log a critical message
     * @param message Message to log
     */
    void critical(const std::string& message);
    
    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief Flush log buffers to disk
     */
    void flush();

private:
    /**
     * @brief Internal logging method
     * @param level Log level
     * @param message Message to log
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return std::string String representation
     */
    std::string levelToString(LogLevel level) const;
    
    /**
     * @brief Get current timestamp string
     * @return std::string Formatted timestamp
     */
    std::string getTimestamp() const;
    
    /**
     * @brief Check if log rotation is needed
     */
    void checkRotation();
    
    /**
     * @brief Perform log rotation
     */
    void rotate();

private:
    ConfigManager& m_config;
    std::ofstream m_logFile;
    std::mutex m_mutex;
    LogLevel m_minLevel;
    std::string m_logPath;
    size_t m_maxSizeBytes;
    int m_rotateCount;
    bool m_consoleOutput;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_LOGGER_HPP
