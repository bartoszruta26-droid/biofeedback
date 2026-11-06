#include "core/ConfigManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <QThread>
#include <QFile>
#include <QDir>

using json = nlohmann::json;

namespace biofeedback {

// ============================================================================
// ConfigManagerStatistics Implementation
// ============================================================================

std::string ConfigManagerStatistics::getSummary() const {
    std::ostringstream oss;
    oss << "=== ConfigManager Statistics ===\n"
        << "Loads: " << loadsSuccessful.load() << " successful, " 
        << loadsFailed.load() << " failed\n"
        << "Saves: " << savesSuccessful.load() << " successful, " 
        << savesFailed.load() << " failed\n"
        << "Values Read: " << valuesRead.load() << "\n"
        << "Values Written: " << valuesWritten.load() << "\n"
        << "Validation Errors: " << validationErrors.load() << "\n"
        << "Default Values Used: " << defaultValueUsed.load() << "\n"
        << "Retry Operations: " << retryOperations.load();
    return oss.str();
}

void ConfigManagerStatistics::reset() {
    loadsSuccessful.store(0);
    loadsFailed.store(0);
    savesSuccessful.store(0);
    savesFailed.store(0);
    valuesRead.store(0);
    valuesWritten.store(0);
    validationErrors.store(0);
    defaultValueUsed.store(0);
    retryOperations.store(0);
}

// ============================================================================
// ConfigManager Implementation
// ============================================================================

ConfigManager::ConfigManager() 
    : m_loaded(false)
    , m_minLogLevel(ConfigManagerLogLevel::INFO)
    , m_consoleOutputEnabled(true) {
    logMessage(ConfigManagerLogLevel::DEBUG, "ConfigManager constructed", "ConfigManager");
}

ConfigManager::~ConfigManager() {
    logMessage(ConfigManagerLogLevel::DEBUG, "ConfigManager destructed", "ConfigManager");
}

bool ConfigManager::load(const std::string& path, bool validate) {
    QMutexLocker locker(&m_mutex);
    
    logMessage(ConfigManagerLogLevel::INFO, 
               QString("Loading configuration from: %1").arg(QString::fromStdString(path)).toStdString(), 
               "load");
    
    int retryCount = 0;
    while (retryCount < MAX_RETRY_COUNT) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                logMessage(ConfigManagerLogLevel::WARNING, 
                          QString("Could not open config file: %1").arg(QString::fromStdString(path)).toStdString(),
                          "load");
                m_statistics.loadsFailed.fetch_add(1);
                
                if (retryCount < MAX_RETRY_COUNT - 1) {
                    retryCount++;
                    m_statistics.retryOperations.fetch_add(1);
                    logMessage(ConfigManagerLogLevel::DEBUG,
                              QString("Retry %1/%2").arg(retryCount).arg(MAX_RETRY_COUNT).toStdString(),
                              "load");
                    QThread::msleep(RETRY_DELAY_MS);
                    continue;
                }
                return false;
            }
            
            json j;
            file >> j;
            file.close();
            
            // Flatten JSON to dot-notation keys
            for (auto& [key, value] : j.items()) {
                flattenJson("", key, value);
            }
            
            // Validate configuration if requested
            if (validate && !validateConfiguration()) {
                logMessage(ConfigManagerLogLevel::ERROR, 
                          "Configuration validation failed",
                          "load");
                m_statistics.validationErrors.fetch_add(1);
                m_statistics.loadsFailed.fetch_add(1);
                return false;
            }
            
            m_loaded = true;
            m_statistics.loadsSuccessful.fetch_add(1);
            
            logMessage(ConfigManagerLogLevel::INFO, 
                      QString("Configuration loaded successfully (%1 keys)").arg(m_config.size()),
                      "load");
            return true;
        }
        catch (const json::parse_error& e) {
            logMessage(ConfigManagerLogLevel::ERROR,
                      QString("JSON parse error: %1").arg(e.what()).toStdString(),
                      "load");
            m_statistics.loadsFailed.fetch_add(1);
            return false;
        }
        catch (const std::exception& e) {
            logMessage(ConfigManagerLogLevel::ERROR,
                      QString("Exception during load: %1").arg(e.what()).toStdString(),
                      "load");
            m_statistics.loadsFailed.fetch_add(1);
            
            if (retryCount < MAX_RETRY_COUNT - 1) {
                retryCount++;
                m_statistics.retryOperations.fetch_add(1);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }
            return false;
        }
        catch (...) {
            logMessage(ConfigManagerLogLevel::CRITICAL,
                      "Unknown exception during config load",
                      "load");
            m_statistics.loadsFailed.fetch_add(1);
            return false;
        }
    }
    
    return false;
}

void ConfigManager::flattenJson(const std::string& prefix, 
                                 const std::string& key, 
                                 const json& value) {
    std::string fullKey = prefix.empty() ? key : prefix + "." + key;
    
    // Check max config size
    if (m_config.size() >= MAX_CONFIG_SIZE) {
        logMessage(ConfigManagerLogLevel::WARNING,
                  QString("Maximum config size reached (%1), ignoring key: %2")
                      .arg(MAX_CONFIG_SIZE).arg(QString::fromStdString(fullKey)).toStdString(),
                  "flattenJson");
        return;
    }
    
    if (value.is_object()) {
        for (auto& [k, v] : value.items()) {
            flattenJson(fullKey, k, v);
        }
    } else if (value.is_string()) {
        m_config[fullKey] = value.get<std::string>();
    } else if (value.is_number_integer()) {
        m_config[fullKey] = value.get<int>();
    } else if (value.is_number_float()) {
        m_config[fullKey] = value.get<double>();
    } else if (value.is_boolean()) {
        m_config[fullKey] = value.get<bool>();
    } else if (value.is_null()) {
        logMessage(ConfigManagerLogLevel::VERBOSE,
                  QString("Null value for key: %1").arg(QString::fromStdString(fullKey)).toStdString(),
                  "flattenJson");
    }
}

bool ConfigManager::save(const std::string& path, bool backup) const {
    QMutexLocker locker(&m_mutex);
    
    logMessage(ConfigManagerLogLevel::INFO,
              QString("Saving configuration to: %1").arg(QString::fromStdString(path)).toStdString(),
              "save");
    
    int retryCount = 0;
    while (retryCount < MAX_RETRY_COUNT) {
        try {
            // Create backup if requested and file exists
            if (backup && QFile::exists(QString::fromStdString(path))) {
                if (!createBackup(path)) {
                    logMessage(ConfigManagerLogLevel::WARNING,
                              "Failed to create backup, proceeding with save",
                              "save");
                }
            }
            
            json j;
            
            // Reconstruct nested structure from flat keys
            for (const auto& [key, value] : m_config) {
                setNestedValue(j, key, value);
            }
            
            std::ofstream file(path);
            if (!file.is_open()) {
                logMessage(ConfigManagerLogLevel::ERROR,
                          QString("Could not open file for writing: %1").arg(QString::fromStdString(path)).toStdString(),
                          "save");
                m_statistics.savesFailed.fetch_add(1);
                
                if (retryCount < MAX_RETRY_COUNT - 1) {
                    retryCount++;
                    m_statistics.retryOperations.fetch_add(1);
                    QThread::msleep(RETRY_DELAY_MS);
                    continue;
                }
                return false;
            }
            
            file << j.dump(4);
            file.close();
            
            if (!file.good()) {
                logMessage(ConfigManagerLogLevel::ERROR,
                          "Error writing to config file",
                          "save");
                m_statistics.savesFailed.fetch_add(1);
                return false;
            }
            
            m_statistics.savesSuccessful.fetch_add(1);
            logMessage(ConfigManagerLogLevel::INFO,
                      "Configuration saved successfully",
                      "save");
            return true;
        }
        catch (const std::exception& e) {
            logMessage(ConfigManagerLogLevel::ERROR,
                      QString("Exception during save: %1").arg(e.what()).toStdString(),
                      "save");
            m_statistics.savesFailed.fetch_add(1);
            
            if (retryCount < MAX_RETRY_COUNT - 1) {
                retryCount++;
                m_statistics.retryOperations.fetch_add(1);
                QThread::msleep(RETRY_DELAY_MS);
                continue;
            }
            return false;
        }
        catch (...) {
            logMessage(ConfigManagerLogLevel::CRITICAL,
                      "Unknown exception during config save",
                      "save");
            m_statistics.savesFailed.fetch_add(1);
            return false;
        }
    }
    
    return false;
}

bool ConfigManager::createBackup(const std::string& path) const {
    try {
        QString sourcePath = QString::fromStdString(path);
        QFileInfo fileInfo(sourcePath);
        
        if (!fileInfo.exists()) {
            return false;
        }
        
        QString backupPath = sourcePath + ".backup." + 
                            QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        
        if (QFile::copy(sourcePath, backupPath)) {
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Backup created: %1").arg(backupPath.toStdString()).toStdString(),
                      "createBackup");
            return true;
        } else {
            logMessage(ConfigManagerLogLevel::WARNING,
                      QString("Failed to create backup: %1").arg(backupPath.toStdString()).toStdString(),
                      "createBackup");
            return false;
        }
    }
    catch (const std::exception& e) {
        logMessage(ConfigManagerLogLevel::ERROR,
                  QString("Exception creating backup: %1").arg(e.what()).toStdString(),
                  "createBackup");
        return false;
    }
    catch (...) {
        logMessage(ConfigManagerLogLevel::ERROR,
                  "Unknown exception creating backup",
                  "createBackup");
        return false;
    }
}

bool ConfigManager::validateConfiguration() const {
    // Basic validation - can be extended based on requirements
    bool valid = true;
    
    // Check critical keys exist
    std::vector<std::string> requiredKeys = {
        "serial.port",
        "serial.baud_rate",
        "data.base_path",
        "encryption.enabled"
    };
    
    for (const auto& key : requiredKeys) {
        if (m_config.find(key) == m_config.end()) {
            logMessage(ConfigManagerLogLevel::WARNING,
                      QString("Missing required config key: %1").arg(QString::fromStdString(key)).toStdString(),
                      "validateConfiguration");
            valid = false;
        }
    }
    
    // Validate numeric ranges
    int baudRate = getInt("serial.baud_rate", 0);
    if (baudRate <= 0 || baudRate > 1000000) {
        logMessage(ConfigManagerLogLevel::ERROR,
                  QString("Invalid baud_rate: %1").arg(baudRate).toStdString(),
                  "validateConfiguration");
        valid = false;
    }
    
    double sensitivity = getDouble("games.biofeedback_sensitivity", 0.0);
    if (sensitivity < 0.1 || sensitivity > 10.0) {
        logMessage(ConfigManagerLogLevel::WARNING,
                  QString("Biofeedback sensitivity out of recommended range: %1").arg(sensitivity).toStdString(),
                  "validateConfiguration");
        // Not fatal, just a warning
    }
    
    return valid;
}

bool ConfigManager::validateValue(const std::string& key,
                                  std::optional<double> minValue,
                                  std::optional<double> maxValue) const {
    QMutexLocker locker(&m_mutex);
    
    auto it = m_config.find(key);
    if (it == m_config.end()) {
        logMessage(ConfigManagerLogLevel::VERBOSE,
                  QString("Key not found for validation: %1").arg(QString::fromStdString(key)).toStdString(),
                  "validateValue");
        return false;
    }
    
    double value = 0.0;
    if (auto* val = std::get_if<int>(&it->second)) {
        value = static_cast<double>(*val);
    } else if (auto* val = std::get_if<double>(&it->second)) {
        value = *val;
    } else {
        // Non-numeric value, range validation not applicable
        return true;
    }
    
    if (minValue.has_value() && value < minValue.value()) {
        logMessage(ConfigManagerLogLevel::WARNING,
                  QString("Value %1 for key %2 is below minimum %3")
                      .arg(value).arg(QString::fromStdString(key)).arg(minValue.value()).toStdString(),
                  "validateValue");
        m_statistics.validationErrors.fetch_add(1);
        return false;
    }
    
    if (maxValue.has_value() && value > maxValue.value()) {
        logMessage(ConfigManagerLogLevel::WARNING,
                  QString("Value %1 for key %2 is above maximum %3")
                      .arg(value).arg(QString::fromStdString(key)).arg(maxValue.value()).toStdString(),
                  "validateValue");
        m_statistics.validationErrors.fetch_add(1);
        return false;
    }
    
    return true;
}

void ConfigManager::setDefaults() {
    QMutexLocker locker(&m_mutex);
    
    logMessage(ConfigManagerLogLevel::INFO, "Setting default configuration values", "setDefaults");
    
    m_config.clear();
    
    // Serial settings
    m_config["serial.port"] = std::string("/dev/ttyUSB0");
    m_config["serial.baud_rate"] = 9600;
    m_config["serial.timeout_ms"] = 1000;
    m_config["serial.retry_count"] = 3;
    
    // Data settings
    m_config["data.base_path"] = std::string("./data/patients");
    m_config["data.backup_enabled"] = true;
    m_config["data.backup_interval_hours"] = 24;
    m_config["data.max_backups"] = 7;
    
    // Encryption settings
    m_config["encryption.enabled"] = true;
    m_config["encryption.algorithm"] = std::string("AES-256-CBC");
    m_config["encryption.key_derivation"] = std::string("PBKDF2");
    m_config["encryption.iterations"] = 100000;
    
    // UI settings
    m_config["ui.theme"] = std::string("default");
    m_config["ui.language"] = std::string("pl");
    m_config["ui.font_size"] = 12;
    m_config["ui.graph_update_interval_ms"] = 100;
    m_config["ui.graph_samples_visible"] = 300;
    
    // Sensor settings
    m_config["sensor.type"] = std::string("HX711");
    m_config["sensor.sample_rate_hz"] = 80;
    m_config["sensor.gain"] = 128;
    m_config["sensor.calibration_enabled"] = true;
    m_config["sensor.zero_threshold"] = 0.5;
    
    // Logging settings
    m_config["logging.level"] = std::string("INFO");
    m_config["logging.file"] = std::string("./logs/biofeedback.log");
    m_config["logging.max_size_mb"] = 10;
    m_config["logging.rotate_count"] = 5;
    m_config["logging.console_output"] = true;
    
    // Games settings
    m_config["games.difficulty_default"] = std::string("medium");
    m_config["games.biofeedback_sensitivity"] = 1.0;
    m_config["games.session_duration_minutes"] = 15;
    
    // Medical settings
    m_config["medical.require_authentication"] = false;
    m_config["medical.audit_log_enabled"] = true;
    m_config["medical.data_retention_days"] = 2555;
    m_config["medical.export_format"] = std::string("JSON");
    
    m_loaded = true;
    
    logMessage(ConfigManagerLogLevel::INFO,
              QString("Default configuration set (%1 keys)").arg(m_config.size()),
              "setDefaults");
}

std::string ConfigManager::getString(const std::string& key, 
                                      const std::string& defaultValue) const {
    QMutexLocker locker(&m_mutex);
    m_statistics.valuesRead.fetch_add(1);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<std::string>(&it->second)) {
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Get string '%1' = '%2'").arg(QString::fromStdString(key)).arg(QString::fromStdString(*val)).toStdString(),
                      "getString");
            return *val;
        }
    }
    
    m_statistics.defaultValueUsed.fetch_add(1);
    logMessage(ConfigManagerLogLevel::VERBOSE,
              QString("Get string '%1' using default: '%2'").arg(QString::fromStdString(key)).arg(QString::fromStdString(defaultValue)).toStdString(),
              "getString");
    return defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    QMutexLocker locker(&m_mutex);
    m_statistics.valuesRead.fetch_add(1);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<int>(&it->second)) {
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Get int '%1' = %2").arg(QString::fromStdString(key)).arg(*val).toStdString(),
                      "getInt");
            return *val;
        }
    }
    
    m_statistics.defaultValueUsed.fetch_add(1);
    logMessage(ConfigManagerLogLevel::VERBOSE,
              QString("Get int '%1' using default: %2").arg(QString::fromStdString(key)).arg(defaultValue).toStdString(),
              "getInt");
    return defaultValue;
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    QMutexLocker locker(&m_mutex);
    m_statistics.valuesRead.fetch_add(1);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<double>(&it->second)) {
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Get double '%1' = %2").arg(QString::fromStdString(key)).arg(*val).toStdString(),
                      "getDouble");
            return *val;
        }
        if (auto* val = std::get_if<int>(&it->second)) {
            double result = static_cast<double>(*val);
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Get double '%1' = %2 (converted from int)").arg(QString::fromStdString(key)).arg(result).toStdString(),
                      "getDouble");
            return result;
        }
    }
    
    m_statistics.defaultValueUsed.fetch_add(1);
    logMessage(ConfigManagerLogLevel::VERBOSE,
              QString("Get double '%1' using default: %2").arg(QString::fromStdString(key)).arg(defaultValue).toStdString(),
              "getDouble");
    return defaultValue;
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    QMutexLocker locker(&m_mutex);
    m_statistics.valuesRead.fetch_add(1);
    
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<bool>(&it->second)) {
            logMessage(ConfigManagerLogLevel::VERBOSE,
                      QString("Get bool '%1' = %2").arg(QString::fromStdString(key)).arg(*val ? "true" : "false").toStdString(),
                      "getBool");
            return *val;
        }
    }
    
    m_statistics.defaultValueUsed.fetch_add(1);
    logMessage(ConfigManagerLogLevel::VERBOSE,
              QString("Get bool '%1' using default: %2").arg(QString::fromStdString(key)).arg(defaultValue ? "true" : "false").toStdString(),
              "getBool");
    return defaultValue;
}

void ConfigManager::setValue(const std::string& key, const ConfigValue& value) {
    QMutexLocker locker(&m_mutex);
    
    // Check max config size
    if (m_config.size() >= MAX_CONFIG_SIZE && m_config.find(key) == m_config.end()) {
        logMessage(ConfigManagerLogLevel::WARNING,
                  QString("Maximum config size reached, cannot add key: %1")
                      .arg(QString::fromStdString(key)).toStdString(),
                  "setValue");
        return;
    }
    
    m_config[key] = value;
    m_statistics.valuesWritten.fetch_add(1);
    
    std::visit([this, &key](auto&& arg) {
        logMessage(ConfigManagerLogLevel::VERBOSE,
                  QString("Set '%1' = %2").arg(QString::fromStdString(key)).arg(QString::fromStdString(std::to_string(static_cast<double>(arg)))).toStdString(),
                  "setValue");
    }, value);
}

bool ConfigManager::hasKey(const std::string& key) const {
    QMutexLocker locker(&m_mutex);
    return m_config.find(key) != m_config.end();
}

std::string ConfigManager::getDataPath() const {
    return getString("data.base_path", "./data/patients");
}

std::string ConfigManager::getSerialPort() const {
    return getString("serial.port", "/dev/ttyUSB0");
}

bool ConfigManager::isEncryptionEnabled() const {
    return getBool("encryption.enabled", true);
}

void ConfigManager::setMinimumLogLevel(ConfigManagerLogLevel level) {
    QMutexLocker locker(&m_mutex);
    m_minLogLevel = level;
    logMessage(ConfigManagerLogLevel::INFO,
              QString("Minimum log level set to: %1").arg(logLevelToString(level)).toStdString(),
              "setMinimumLogLevel");
}

ConfigManagerLogLevel ConfigManager::getMinimumLogLevel() const {
    QMutexLocker locker(&m_mutex);
    return m_minLogLevel;
}

void ConfigManager::setConsoleOutputEnabled(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_consoleOutputEnabled = enabled;
    logMessage(ConfigManagerLogLevel::INFO,
              QString("Console output %1").arg(enabled ? "enabled" : "disabled").toStdString(),
              "setConsoleOutputEnabled");
}

bool ConfigManager::isConsoleOutputEnabled() const {
    QMutexLocker locker(&m_mutex);
    return m_consoleOutputEnabled;
}

ConfigManagerStatistics ConfigManager::getStatistics() const {
    QMutexLocker locker(&m_mutex);
    return m_statistics;
}

void ConfigManager::resetStatistics() {
    QMutexLocker locker(&m_mutex);
    m_statistics.reset();
    logMessage(ConfigManagerLogLevel::INFO, "Statistics reset", "resetStatistics");
}

std::string ConfigManager::getStatisticsSummary() const {
    QMutexLocker locker(&m_mutex);
    return m_statistics.getSummary();
}

void ConfigManager::logMessage(ConfigManagerLogLevel level, const std::string& message, const std::string& source) const {
    if (level < m_minLogLevel) {
        return;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    
    QString logLine = QString("[%1] [%2] [%3:%4] %5")
        .arg(timestamp)
        .arg(QString::fromStdString(logLevelToString(level)))
        .arg(QString::fromStdString(source))
        .arg(threadId)
        .arg(QString::fromStdString(message));
    
    if (m_consoleOutputEnabled) {
        if (level >= ConfigManagerLogLevel::ERROR) {
            std::cerr << logLine.toStdString() << std::endl;
        } else {
            std::cout << logLine.toStdString() << std::endl;
        }
    }
    
    // Could also write to a log file here if needed
}

std::string ConfigManager::logLevelToString(ConfigManagerLogLevel level) const {
    switch (level) {
        case ConfigManagerLogLevel::VERBOSE: return "VERBOSE";
        case ConfigManagerLogLevel::DEBUG:   return "DEBUG";
        case ConfigManagerLogLevel::INFO:    return "INFO";
        case ConfigManagerLogLevel::WARNING: return "WARNING";
        case ConfigManagerLogLevel::ERROR:   return "ERROR";
        case ConfigManagerLogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::vector<std::string> ConfigManager::parseKey(const std::string& key) const {
    std::vector<std::string> parts;
    std::stringstream ss(key);
    std::string part;
    
    while (std::getline(ss, part, '.')) {
        parts.push_back(part);
    }
    
    return parts;
}

void ConfigManager::setNestedValue(json& j, const std::string& key, 
                                    const ConfigValue& value) const {
    auto parts = parseKey(key);
    json* current = &j;
    
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (!current->contains(parts[i])) {
            (*current)[parts[i]] = json::object();
        }
        current = &(*current)[parts[i]];
    }
    
    const std::string& lastKey = parts.back();
    std::visit([current, &lastKey](auto&& arg) {
        (*current)[lastKey] = arg;
    }, value);
}

} // namespace biofeedback
