#ifndef BIOFEEDBACK_CONFIG_MANAGER_HPP
#define BIOFEEDBACK_CONFIG_MANAGER_HPP

#include <string>
#include <map>
#include <variant>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

namespace biofeedback {

/**
 * @brief Configuration value type supporting multiple data types
 */
using ConfigValue = std::variant<std::string, int, double, bool>;

/**
 * @brief Manages application configuration from JSON files
 * 
 * Provides type-safe access to configuration parameters with
 * default values and validation for medical software compliance.
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    /**
     * @brief Load configuration from JSON file
     * @param path Path to configuration file
     * @return true if loaded successfully
     */
    bool load(const std::string& path);
    
    /**
     * @brief Save configuration to JSON file
     * @param path Path to save configuration
     * @return true if saved successfully
     */
    bool save(const std::string& path) const;
    
    /**
     * @brief Set default configuration values
     */
    void setDefaults();
    
    /**
     * @brief Get string value from configuration
     * @param key Configuration key (dot notation for nested)
     * @param defaultValue Default value if key not found
     * @return std::string Configuration value or default
     */
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    
    /**
     * @brief Get integer value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return int Configuration value or default
     */
    int getInt(const std::string& key, int defaultValue = 0) const;
    
    /**
     * @brief Get double value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return double Configuration value or default
     */
    double getDouble(const std::string& key, double defaultValue = 0.0) const;
    
    /**
     * @brief Get boolean value from configuration
     * @param key Configuration key
     * @param defaultValue Default value if key not found
     * @return bool Configuration value or default
     */
    bool getBool(const std::string& key, bool defaultValue = false) const;
    
    /**
     * @brief Set a configuration value
     * @param key Configuration key
     * @param value Value to set
     */
    void setValue(const std::string& key, const ConfigValue& value);
    
    /**
     * @brief Check if a key exists in configuration
     * @param key Configuration key
     * @return true if key exists
     */
    bool hasKey(const std::string& key) const;
    
    /**
     * @brief Get the base path for patient data
     * @return std::string Base data path
     */
    std::string getDataPath() const;
    
    /**
     * @brief Get the serial port configuration
     * @return std::string Serial port device path
     */
    std::string getSerialPort() const;
    
    /**
     * @brief Check if encryption is enabled
     * @return true if encryption enabled
     */
    bool isEncryptionEnabled() const;

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

private:
    std::map<std::string, ConfigValue> m_config;
    bool m_loaded;
};

} // namespace biofeedback

#endif // BIOFEEDBACK_CONFIG_MANAGER_HPP
