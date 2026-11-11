#ifndef BIOFEEDBACK_LOGGER_HPP
#define BIOFEEDBACK_LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <atomic>
#include <functional>
#include <map>

namespace biofeedback {

class ConfigManager;

/**
 * @brief Logging levels for the application
 * 
 * Extended with VERBOSE level for detailed debugging.
 */
enum class LogLevel {
    VERBOSE,  ///< Detailed debugging information
    DEBUG,    ///< Debugging information
    INFO,     ///< Informational messages
    WARNING,  ///< Warning messages
    ERROR,    ///< Error messages
    CRITICAL  ///< Critical errors requiring immediate attention
};

/**
 * @brief Statistics about logging operations
 * 
 * Thread-safe statistics tracking for compliance and monitoring.
 */
struct LoggerStatistics {
    std::atomic<uint64_t> verboseCount{0};   ///< Number of VERBOSE messages logged
    std::atomic<uint64_t> debugCount{0};     ///< Number of DEBUG messages logged
    std::atomic<uint64_t> infoCount{0};      ///< Number of INFO messages logged
    std::atomic<uint64_t> warningCount{0};   ///< Number of WARNING messages logged
    std::atomic<uint64_t> errorCount{0};     ///< Number of ERROR messages logged
    std::atomic<uint64_t> criticalCount{0};  ///< Number of CRITICAL messages logged
    std::atomic<uint64_t> totalMessages{0};  ///< Total number of messages logged
    std::atomic<uint64_t> rotationCount{0};  ///< Number of log rotations performed
    std::atomic<uint64_t> writeErrors{0};    ///< Number of file write errors encountered
    std::atomic<uint64_t> flushOperations{0};///< Number of flush operations performed
    
    /**
     * @brief Default constructor
     */
    LoggerStatistics() = default;
    
    /**
     * @brief Copy constructor - copies atomic values
     */
    LoggerStatistics(const LoggerStatistics& other) 
        : verboseCount(other.verboseCount.load())
        , debugCount(other.debugCount.load())
        , infoCount(other.infoCount.load())
        , warningCount(other.warningCount.load())
        , errorCount(other.errorCount.load())
        , criticalCount(other.criticalCount.load())
        , totalMessages(other.totalMessages.load())
        , rotationCount(other.rotationCount.load())
        , writeErrors(other.writeErrors.load())
        , flushOperations(other.flushOperations.load())
    {}
    
    /**
     * @brief Move constructor - moves atomic values
     */
    LoggerStatistics(LoggerStatistics&& other) noexcept
        : verboseCount(other.verboseCount.load())
        , debugCount(other.debugCount.load())
        , infoCount(other.infoCount.load())
        , warningCount(other.warningCount.load())
        , errorCount(other.errorCount.load())
        , criticalCount(other.criticalCount.load())
        , totalMessages(other.totalMessages.load())
        , rotationCount(other.rotationCount.load())
        , writeErrors(other.writeErrors.load())
        , flushOperations(other.flushOperations.load())
    {}
    
    /**
     * @brief Copy assignment operator - copies atomic values
     */
    LoggerStatistics& operator=(const LoggerStatistics& other) {
        if (this != &other) {
            verboseCount.store(other.verboseCount.load());
            debugCount.store(other.debugCount.load());
            infoCount.store(other.infoCount.load());
            warningCount.store(other.warningCount.load());
            errorCount.store(other.errorCount.load());
            criticalCount.store(other.criticalCount.load());
            totalMessages.store(other.totalMessages.load());
            rotationCount.store(other.rotationCount.load());
            writeErrors.store(other.writeErrors.load());
            flushOperations.store(other.flushOperations.load());
        }
        return *this;
    }
    
    /**
     * @brief Move assignment operator - moves atomic values
     */
    LoggerStatistics& operator=(LoggerStatistics&& other) noexcept {
        if (this != &other) {
            verboseCount.store(other.verboseCount.load());
            debugCount.store(other.debugCount.load());
            infoCount.store(other.infoCount.load());
            warningCount.store(other.warningCount.load());
            errorCount.store(other.errorCount.load());
            criticalCount.store(other.criticalCount.load());
            totalMessages.store(other.totalMessages.load());
            rotationCount.store(other.rotationCount.load());
            writeErrors.store(other.writeErrors.load());
            flushOperations.store(other.flushOperations.load());
        }
        return *this;
    }
    
    /**
     * @brief Get a formatted summary of all statistics
     * @return std::string Formatted statistics summary
     */
    std::string getSummary() const;
    
    /**
     * @brief Reset all counters to zero
     */
    void reset();
};

/**
 * @brief Callback function type for log events
 * @param level Log level of the message
 * @param message The logged message
 * @param timestamp Timestamp when message was logged
 */
using LogCallback = std::function<void(LogLevel level, const std::string& message, const std::string& timestamp)>;

/**
 * @brief Medical-grade logging system with file rotation
 * 
 * Provides thread-safe logging with multiple output destinations
 * and automatic log rotation for compliance with medical standards.
 * 
 * @section FEATURES Features
 * - Thread-safe logging with mutex protection
 * - Multiple log levels (VERBOSE, DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Automatic file rotation based on size
 * - Configurable via ConfigManager
 * - Console output option
 * - Timestamp with millisecond precision
 * - Statistics tracking for compliance
 * - Callback support for real-time log monitoring
 * - Graceful error handling without exceptions
 * 
 * @section THREAD_SAFETY Thread Safety
 * All public methods are thread-safe and can be called from multiple threads.
 * Internal mutex ensures serialized access to log file.
 * Atomic counters ensure thread-safe statistics updates.
 * 
 * @section ERROR_HANDLING Error Handling
 * - File operation errors are logged but don't throw exceptions
 * - Rotation failures are retried with exponential backoff
 * - Invalid configurations fall back to safe defaults
 * - Write errors are tracked in statistics
 * 
 * @section COMPLIANCE Medical Compliance
 * - Audit trail through comprehensive logging
 * - Tamper-resistant design (append-only mode)
 * - Configurable retention through rotation settings
 * - Statistics for monitoring system health
 */
class Logger {
public:
    /**
     * @brief Construct a new Logger object
     * @param config Configuration manager reference
     * 
     * @throws std::exception If critical initialization fails
     */
    explicit Logger(ConfigManager& config);
    
    /**
     * @brief Destroy the Logger object
     * 
     * Ensures all buffers are flushed and files are closed properly.
     */
    ~Logger();
    
    // Disable copying to prevent resource sharing issues
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Enable moving for efficiency
    Logger(Logger&&) noexcept;
    Logger& operator=(Logger&&) noexcept;
    
    /**
     * @brief Log a verbose message
     * @param message Message to log
     * 
     * VERBOSE level is used for very detailed debugging information,
     * typically only enabled during development or troubleshooting.
     */
    void verbose(const std::string& message);
    
    /**
     * @brief Log a debug message
     * @param message Message to log
     * 
     * DEBUG level is used for debugging information that may be
     * useful during development or debugging sessions.
     */
    void debug(const std::string& message);
    
    /**
     * @brief Log an info message
     * @param message Message to log
     * 
     * INFO level is used for general informational messages
     * about normal application operation.
     */
    void info(const std::string& message);
    
    /**
     * @brief Log a warning message
     * @param message Message to log
     * 
     * WARNING level indicates potential issues that don't prevent
     * operation but should be reviewed.
     */
    void warning(const std::string& message);
    
    /**
     * @brief Log an error message
     * @param message Message to log
     * 
     * ERROR level indicates errors that affect specific operations
     * but don't crash the application.
     */
    void error(const std::string& message);
    
    /**
     * @brief Log a critical message
     * @param message Message to log
     * 
     * CRITICAL level indicates severe errors that may require
     * immediate attention or could lead to application failure.
     */
    void critical(const std::string& message);
    
    /**
     * @brief Set the minimum log level
     * @param level Minimum level to log
     * 
     * Messages below this level will be filtered out.
     * Thread-safe operation.
     */
    void setLogLevel(LogLevel level);
    
    /**
     * @brief Get the current minimum log level
     * @return LogLevel Current minimum log level
     * 
     * Thread-safe operation.
     */
    LogLevel getLogLevel() const;
    
    /**
     * @brief Enable or disable console output
     * @param enabled true to enable console output, false to disable
     * 
     * When enabled, log messages are also written to stdout/stderr
     * in addition to the log file.
     */
    void setConsoleOutput(bool enabled);
    
    /**
     * @brief Check if console output is enabled
     * @return true if console output is enabled, false otherwise
     */
    bool isConsoleOutputEnabled() const;
    
    /**
     * @brief Flush log buffers to disk
     * 
     * Forces any buffered data to be written to the log file.
     * Automatically called in destructor.
     * Thread-safe operation.
     */
    void flush();
    
    /**
     * @brief Get current logging statistics
     * @return LoggerStatistics Copy of current statistics
     * 
     * Thread-safe operation. Returns a snapshot of current statistics.
     */
    LoggerStatistics getStatistics() const;
    
    /**
     * @brief Get a formatted summary of logging statistics
     * @return std::string Formatted statistics summary
     * 
     * Convenience method that returns getStatistics().getSummary()
     */
    std::string getStatisticsSummary() const;
    
    /**
     * @brief Reset logging statistics to zero
     * 
     * Thread-safe operation. Useful for periodic reporting.
     */
    void resetStatistics();
    
    /**
     * @brief Register a callback for log events
     * @param callback Function to call when a message is logged
     * @param id Unique identifier for this callback
     * 
     * Callbacks are invoked for every logged message (after filtering).
     * Multiple callbacks can be registered with different IDs.
     * Thread-safe operation.
     * 
     * @note Callbacks are executed synchronously within the log() method.
     *       Keep callback execution fast to avoid blocking logging.
     */
    void registerCallback(const LogCallback& callback, const std::string& id);
    
    /**
     * @brief Unregister a previously registered callback
     * @param id ID of the callback to remove
     * @return true if callback was found and removed, false otherwise
     * 
     * Thread-safe operation.
     */
    bool unregisterCallback(const std::string& id);
    
    /**
     * @brief Check if logger is properly initialized
     * @return true if log file is open and ready, false otherwise
     * 
     * Useful for checking if file logging is available.
     * Console logging may still work even if file logging is disabled.
     */
    bool isInitialized() const;
    
    /**
     * @brief Get the current log file path
     * @return std::string Path to the current log file
     */
    std::string getLogPath() const;

private:
    /**
     * @brief Internal logging method
     * @param level Log level
     * @param message Message to log
     * 
     * Core logging implementation used by all public log methods.
     * Handles filtering, formatting, file writing, console output,
     * rotation checking, and callback invocation.
     * 
     * Thread-safe operation.
     */
    void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return std::string String representation
     * 
     * Thread-safe operation.
     */
    std::string levelToString(LogLevel level) const;
    
    /**
     * @brief Convert string to log level
     * @param levelStr String representation (case-insensitive)
     * @param defaultLevel Default level if conversion fails
     * @return LogLevel Converted log level
     * 
     * Supports: VERBOSE, DEBUG, INFO, WARNING/WARN, ERROR/ERR, CRITICAL/CRIT/FATAL
     * Thread-safe operation.
     */
    LogLevel stringToLevel(const std::string& levelStr, LogLevel defaultLevel = LogLevel::INFO) const;
    
    /**
     * @brief Get current timestamp string
     * @return std::string Formatted timestamp with milliseconds
     * 
     * Format: YYYY-MM-DD HH:MM:SS.mmm
     * Thread-safe operation.
     */
    std::string getTimestamp() const;
    
    /**
     * @brief Check if log rotation is needed
     * 
     * Compares current file size against maximum allowed size.
     * Called automatically after each log write.
     * Thread-safe operation.
     */
    void checkRotation();
    
    /**
     * @brief Perform log rotation
     * 
     * Renames current log file and creates a new empty one.
     * Implements retry logic with exponential backoff on failure.
     * Updates rotation count in statistics.
     * Thread-safe operation.
     * 
     * @note Uses MAX_ROTATION_RETRIES and ROTATION_RETRY_DELAY_MS constants
     */
    void rotate();
    
    /**
     * @brief Invoke all registered callbacks
     * @param level Log level
     * @param message Logged message
     * @param timestamp Message timestamp
     * 
     * Wraps callback invocation in try-catch to prevent callback
     * errors from affecting logging.
     * Thread-safe operation.
     */
    void invokeCallbacks(LogLevel level, const std::string& message, const std::string& timestamp);

private:
    ConfigManager& m_config;              ///< Reference to configuration manager
    std::ofstream m_logFile;              ///< Log file stream
    mutable std::mutex m_mutex;           ///< Mutex for thread-safe access
    mutable std::mutex m_callbackMutex;   ///< Separate mutex for callback map
    LogLevel m_minLevel;                  ///< Minimum log level to record
    std::string m_logPath;                ///< Path to log file
    size_t m_maxSizeBytes;                ///< Maximum file size before rotation
    int m_rotateCount;                    ///< Number of rotated files to keep
    bool m_consoleOutput;                 ///< Whether to output to console
    std::map<std::string, LogCallback> m_callbacks;  ///< Registered callbacks
    LoggerStatistics m_statistics;        ///< Logging statistics
    bool m_initialized;                   ///< Whether logger is properly initialized
};

} // namespace biofeedback

#endif // BIOFEEDBACK_LOGGER_HPP
