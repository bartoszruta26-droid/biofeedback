#include "core/ConfigManager.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace biofeedback {

ConfigManager::ConfigManager() : m_loaded(false) {}

ConfigManager::~ConfigManager() {}

bool ConfigManager::load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        
        // Flatten JSON to dot-notation keys
        for (auto& [key, value] : j.items()) {
            flattenJson("", key, value);
        }
        
        m_loaded = true;
        return true;
    }
    catch (...) {
        return false;
    }
}

void ConfigManager::flattenJson(const std::string& prefix, 
                                 const std::string& key, 
                                 const json& value) {
    std::string fullKey = prefix.empty() ? key : prefix + "." + key;
    
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
    }
}

bool ConfigManager::save(const std::string& path) const {
    try {
        json j;
        
        // Reconstruct nested structure from flat keys
        for (const auto& [key, value] : m_config) {
            setNestedValue(j, key, value);
        }
        
        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        
        file << j.dump(4);
        return true;
    }
    catch (...) {
        return false;
    }
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

void ConfigManager::setDefaults() {
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
}

std::string ConfigManager::getString(const std::string& key, 
                                      const std::string& defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<std::string>(&it->second)) {
            return *val;
        }
    }
    return defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<int>(&it->second)) {
            return *val;
        }
    }
    return defaultValue;
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<double>(&it->second)) {
            return *val;
        }
        if (auto* val = std::get_if<int>(&it->second)) {
            return static_cast<double>(*val);
        }
    }
    return defaultValue;
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    auto it = m_config.find(key);
    if (it != m_config.end()) {
        if (auto* val = std::get_if<bool>(&it->second)) {
            return *val;
        }
    }
    return defaultValue;
}

void ConfigManager::setValue(const std::string& key, const ConfigValue& value) {
    m_config[key] = value;
}

bool ConfigManager::hasKey(const std::string& key) const {
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

std::vector<std::string> ConfigManager::parseKey(const std::string& key) const {
    std::vector<std::string> parts;
    std::stringstream ss(key);
    std::string part;
    
    while (std::getline(ss, part, '.')) {
        parts.push_back(part);
    }
    
    return parts;
}

} // namespace biofeedback
