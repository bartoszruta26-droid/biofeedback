#ifndef BIOFEEDBACK_CONFIG_MANAGER_HPP
#define BIOFEEDBACK_CONFIG_MANAGER_HPP

#include <string>
#include <map>
#include <variant>
#include <optional>
#include <vector>
#include <mutex>
#include <atomic>
#include <nlohmann/json.hpp>
#include <QMutex>
#include <QDateTime>

namespace biofeedback {

/**
 * @brief Configuration value type supporting multiple data types
 */
using ConfigValue = std::variant<std::string, int, double, bool>;

/**
 * @brief Log levels for ConfigManager debugging and auditing
 */
enum class ConfigManagerLogLevel {
    VERBOSE = 0,   ///< Detailed internal state information
    DEBUG = 1,     ///< Debugging information for development
    INFO = 2,      ///< General operational information
    WARNING = 3,   ///< Potential issues that don't stop operation
    ERROR = 4,     ///< Errors that affect functionality
    CRITICAL = 5   ///< Critical failures requiring immediate attention
};

/**
 * @brief Statistics tracking for ConfigManager operations
 */
struct ConfigManagerStatistics {
    std::atomic<uint64_t> loadsSuccessful{0};      ///< Number of successful config loads
    std::atomic<uint64_t> loadsFailed{0};          ///< Number of failed config loads
    std::atomic<uint64_t> savesSuccessful{0};      ///< Number of successful config saves
    std::atomic<uint64_t> savesFailed{0};          ///< Number of failed config saves
    std::atomic<uint64_t> valuesRead{0};           ///< Total number of config values read
    std::atomic<uint64_t> valuesWritten{0};        ///< Total number of config values written
    std::atomic<uint64_t> validationErrors{0};     ///< Number of validation errors detected
    std::atomic<uint64_t> defaultValueUsed{0};     ///< Times default value was used
    std::atomic<uint64_t> retryOperations{0};      ///< Number of retry operations performed
    
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
 * @brief Manages application configuration from JSON files
 * 
 * Provides type-safe access to configuration parameters with
 * default values and validation for medical software compliance.
 * Includes comprehensive logging, error handling, and statistics tracking.
 * 
 * @threadsafe This class is thread-safe and can be used from multiple threads.
 */
class ConfigManager {
public:
    /**
     * @brief Construct a new ConfigManager object
     * Initializes with empty configuration and default log level
     */
    ConfigManager();
    
    /**
     * @brief Destroy the ConfigManager object
     * Ensures proper cleanup of resources
     */
    ~ConfigManager();
    
    // Disable copying to prevent configuration inconsistencies
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    /**
     * @brief Load configuration from JSON file
     * @param path Path to configuration file
     * @param validate If true, validate configuration values after loading
     * @return true if loaded successfully, false otherwise
     * 
     * @throws None - Uses gentle error handling, returns false on failure
     * @note Thread-safe, increments statistics counters
     */
    bool load(const std::string& path, bool validate = true);
    
    /**
     * @brief Save configuration to JSON file
     * @param path Path to save configuration
     * @param backup If true, create backup of existing file before saving
     * @return true if saved successfully, false otherwise
     * 
     * @throws None - Uses gentle error handling, returns false on failure
     * @note Thread-safe, increments statistics counters
     */
    bool save(const std::string& path, bool backup = true) const;
    
    /**
     * @brief Set default configuration values
     * Clears existing configuration and populates with safe defaults
     * 
     * @note Thread-safe, logs the operation
     */
    void setDefaults();
    
    /**
     * @brief Get string value from configuration
     * @param key Configuration key (dot notation for nested)
     * @param defaultValue Default value if key not found
     * @return std::string Configuration value or default
     * 
     * @note Thread-safe, tracks read operations in statistics
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    
    /**
     * @brief Get integer value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return int Configuration value or default
     * 
     * @note Thread-safe, tracks read operations in statistics
     */
    int getInt(const std::string& key, int defaultValue = 0) const;
    
    /**
     * @brief Get double value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return double Configuration value or default
     * 
     * @note Thread-safe, tracks read operations in statistics
     */
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    
    /**
     * @brief Get boolean value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return bool Configuration value or default
     * 
     * @note Thread-safe, tracks read operations in statistics
     */
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    /**
     * @brief Set a configuration value
     * @param key Configuration key
     * @param value Value to set
     * 
     * @note Thread-safe, tracks write operations in statistics
     */
    void setValue(const std::string& key, const ConfigValue& value);
    
    /**
     * @brief Check if a key exists in configuration
     * @param key Configuration key
     * @return true if key exists, false otherwise
     * 
     * @note Thread-safe
     */
    bool hasKey(const std::string& key) const;
    
    /**
     * @brief Get the base path for patient data
     * @return std::string Base data path
     * 
     * @note Convenience method, equivalent to getString("data.base_path")
     */
    std::string getDataPath() const;
    
    /**
     * @brief Get the serial port configuration
     * @return std::string Serial port device path
     * 
     * @note Convenience method, equivalent to getString("serial.port")
     */
    std::string getSerialPort() const;
    
    /**
     * @brief Check if encryption is enabled
     * @return true if encryption enabled, false otherwise
     * 
     * @note Convenience method, equivalent to getBool("encryption.enabled")
     */
    bool isEncryptionEnabled() const;
    
    /**
     * @brief Set minimum log level for ConfigManager logging
     * @param level Minimum log level to output
     * 
     * @note Controls verbosity of internal logging
     */
    void setMinimumLogLevel(ConfigManagerLogLevel level);
    
    /**
     * @brief Get current minimum log level
     * @return ConfigManagerLogLevel Current minimum log level
     */
    ConfigManagerLogLevel getMinimumLogLevel() const;
    
    /**
     * @brief Enable or disable console output for logs
     * @param enabled true to enable console output, false to disable
     */
    void setConsoleOutputEnabled(bool enabled);
    
    /**
     * @brief Check if console output is enabled
     * @return true if console output is enabled
     */
    bool isConsoleOutputEnabled() const;
    
    /**
     * @brief Get current statistics
     * @return ConfigManagerStatistics Copy of current statistics
     */
    ConfigManagerStatistics getStatistics() const;
    
    /**
     * @brief Reset all statistics to zero
     */
    void resetStatistics();
    
    /**
     * @brief Get formatted summary of statistics
     * @return std::string Formatted statistics summary
     */
    std::string getStatisticsSummary() const;
    
    /**
     * @brief Validate configuration value against expected range/type
     * @param key Configuration key to validate
     * @param minValue Minimum allowed value (for numeric types)
     * @param maxValue Maximum allowed value (for numeric types)
     * @return true if value is valid, false otherwise
     * 
     * @note Used for medical compliance validation
     */
    bool validateValue(const std::string& key, 
                      std::optional<double> minValue = std::nullopt,
                      std::optional<double> maxValue = std::nullopt) const;

private:
    /**
     * @brief Parse nested key into path components
     * @param key Dot-notation key
     * @return std::vector<std::string> Key components
     */
    std::vector<std::string> parseKey(const std::string& key) const;
    
    /**
     * @brief Flatten nested JSON into dot-notation keys
     * @param prefix Key prefix for nested structure
     * @param key Current key name
     * @param value JSON value to process
     */
    void flattenJson(const std::string& prefix, const std::string& key, const nlohmann::json& value);
    
    /**
     * @brief Set nested value in JSON from dot-notation key
     * @param j JSON object to modify
     * @param key Dot-notation key
     * @param value Value to set
     */
    void setNestedValue(nlohmann::json& j, const std::string& key, const ConfigValue& value) const;
    
    /**
     * @brief Internal logging method with level filtering
     * @param level Log level of the message
     * @param message Message to log
     * @param source Source function/component name
     */
    void logMessage(ConfigManagerLogLevel level, const std::string& message, const std::string& source = "") const;
    
    /**
     * @brief Convert log level enum to string
     * @param level Log level enum
     * @return std::string String representation
     */
    std::string logLevelToString(ConfigManagerLogLevel level) const;
    
    /**
     * @brief Create backup of file before overwriting
     * @param path Path to file to backup
     * @return true if backup created successfully
     */
    bool createBackup(const std::string& path) const;
    
    /**
     * @brief Validate loaded configuration for consistency
     * @return true if configuration is valid
     */
    bool validateConfiguration() const;

private:
    mutable QMutex m_mutex;                          ///< Mutex for thread-safe access
    std::map<std::string, ConfigValue> m_config;     ///< Configuration storage
    bool m_loaded;                                   ///< Flag indicating if config is loaded
    mutable ConfigManagerStatistics m_statistics;    ///< Operation statistics
    mutable ConfigManagerLogLevel m_minLogLevel;     ///< Minimum log level
    mutable bool m_consoleOutputEnabled;             ///< Console output flag
    
    static constexpr int MAX_RETRY_COUNT = 3;        ///< Maximum retry attempts for file operations
    static constexpr int RETRY_DELAY_MS = 50;        ///< Delay between retries in milliseconds
    static constexpr size_t MAX_CONFIG_SIZE = 1000;  ///< Maximum number of config entries
};

} // namespace biofeedback

#endif // BIOFEEDBACK_CONFIG_MANAGER_HPP
