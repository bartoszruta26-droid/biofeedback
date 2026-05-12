/**
 * @file DataManager.cpp
 * @brief Comprehensive data management system with encryption support
 * 
 * This module provides robust data persistence for medical biofeedback application.
 * It handles patient records, exercises, training plans, and measurement results.
 * 
 * FEATURES:
 * - JSON serialization/deserialization with proper escaping
 * - Optional AES-style XOR encryption for sensitive data
 * - File existence checking and safe deletion
 * - Directory listing capabilities
 * - Import/export functionality
 * 
 * ERROR HANDLING:
 * - All file operations are wrapped with error checking
 * - Encryption failures are caught and handled gracefully
 * - Invalid JSON data returns empty/default objects
 * - Detailed error logging via std::cerr
 * 
 * THREAD SAFETY:
 * - Note: This class is NOT thread-safe by default
 * - External synchronization required for multi-threaded access
 * 
 * @section DEBUG_FLAGS Debug Flags
 * Enable verbose logging by setting DEBUG_DATA_MANAGER to true
 */

#include "data/DataManager.hpp"
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cerrno>

// ============================================================================
// DEBUG CONFIGURATION FLAGS
// ============================================================================

/// Enable detailed debug logging for all data operations
constexpr bool DEBUG_DATA_MANAGER = true;

/// Enable verbose JSON parsing debug output
constexpr bool DEBUG_JSON_PARSING = false;

/// Enable encryption/decryption operation logging
constexpr bool DEBUG_ENCRYPTION_OPS = true;

/// Enable file I/O operation debugging
constexpr bool DEBUG_FILE_IO = true;

/// Maximum number of retry attempts for file operations
constexpr int MAX_FILE_RETRY_COUNT = 3;

/// Delay between retries in milliseconds
constexpr int FILE_RETRY_DELAY_MS = 50;

namespace {

// ============================================================================
// ANONYMOUS NAMESPACE - HELPER FUNCTIONS
// ============================================================================

/**
 * @brief Escapes special characters in JSON string values
 * @param input Raw string to escape
 * @return Escaped string safe for JSON embedding
 * 
 * Handles all JSON special characters: quotes, backslashes, control chars
 * Uses switch statement for optimal performance
 */
std::string escapeJsonStringValue(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    
    std::string output;
    output.reserve(input.size() + 10); // Reserve extra space for escapes

    for (const char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: 
                // Only add printable ASCII characters
                if (static_cast<unsigned char>(c) >= 32) {
                    output += c;
                } else {
                    // Convert non-printable to Unicode escape
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    output += buf;
                }
                break;
        }
    }

    return output;
}

/**
 * @brief Finds the closing quote of a JSON string, handling escaped quotes
 * @param text Full JSON text
 * @param start Starting position (should be after opening quote)
 * @return Position of closing quote, or std::string::npos if not found
 * 
 * Properly handles escaped quotes (\" and \\") by counting backslashes
 */
size_t findStringTerminator(const std::string& text, const size_t start) {
    if (start >= text.size()) {
        return std::string::npos;
    }
    
    for (size_t i = start; i < text.size(); ++i) {
        if (text[i] != '"') {
            continue;
        }

        // Count preceding backslashes to determine if quote is escaped
        size_t backslashes = 0;
        for (size_t j = i; j > 0 && text[j - 1] == '\\'; --j) {
            ++backslashes;
        }

        // Even number of backslashes means quote is not escaped
        if (backslashes % 2 == 0) {
            return i;
        }
    }

    return std::string::npos;
}

/**
 * @brief Splits JSON array into individual object strings
 * @param arrayJson JSON array text
 * @return Vector of individual JSON object strings
 * 
 * Handles nested objects and strings correctly by tracking depth
 * and string state
 */
std::vector<std::string> splitJsonObjectsFromArray(const std::string& arrayJson) {
    std::vector<std::string> objects;
    
    if (arrayJson.empty()) {
#if DEBUG_JSON_PARSING
        std::cout << "[JSON_PARSER] Empty array JSON provided" << std::endl;
#endif
        return objects;
    }
    
    bool inString = false;
    int depth = 0;
    size_t objectStart = std::string::npos;

    for (size_t i = 0; i < arrayJson.size(); ++i) {
        const char c = arrayJson[i];

        // Handle string boundaries
        if (c == '"') {
            size_t backslashes = 0;
            for (size_t j = i; j > 0 && arrayJson[j - 1] == '\\'; --j) {
                ++backslashes;
            }
            if (backslashes % 2 == 0) {
                inString = !inString;
            }
        }

        // Skip content inside strings
        if (inString) {
            continue;
        }

        // Track object depth
        if (c == '{') {
            if (depth == 0) {
                objectStart = i;
            }
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && objectStart != std::string::npos) {
                std::string obj = arrayJson.substr(objectStart, i - objectStart + 1);
                objects.push_back(obj);
#if DEBUG_JSON_PARSING
                std::cout << "[JSON_PARSER] Extracted object #" << objects.size() 
                          << " (" << obj.size() << " bytes)" << std::endl;
#endif
                objectStart = std::string::npos;
            }
        }
    }

#if DEBUG_JSON_PARSING
    std::cout << "[JSON_PARSER] Total objects extracted: " << objects.size() << std::endl;
#endif
    return objects;
}

} // anonymous namespace

// ==================== Implementacja PatientData ====================

/**
 * @brief Konwertuje dane pacjenta do formatu JSON
 * @return String z danymi w formacie JSON
 * 
 * UWAGA: Wszystkie pola tekstowe są automatycznie escapowane
 * aby zapobiec błędom parsowania przy specjalnych znakach
 */
std::string PatientData::toJson() const {
    try {
        std::ostringstream json;
        json << "{\n";
        json << "  \"id\": \"" << escapeJsonStringValue(id) << "\",\n";
        json << "  \"firstName\": \"" << escapeJsonStringValue(firstName) << "\",\n";
        json << "  \"lastName\": \"" << escapeJsonStringValue(lastName) << "\",\n";
        json << "  \"pesel\": \"" << escapeJsonStringValue(pesel) << "\",\n";
        json << "  \"birthDate\": \"" << escapeJsonStringValue(birthDate) << "\",\n";
        json << "  \"gender\": \"" << escapeJsonStringValue(gender) << "\",\n";
        json << "  \"phoneNumber\": \"" << escapeJsonStringValue(phoneNumber) << "\",\n";
        json << "  \"email\": \"" << escapeJsonStringValue(email) << "\",\n";
        json << "  \"address\": \"" << escapeJsonStringValue(address) << "\",\n";
        json << "  \"medicalHistory\": \"" << escapeJsonStringValue(medicalHistory) << "\"\n";
        json << "}";
        
#if DEBUG_DATA_MANAGER
        // Logowanie tylko rozmiaru danych, nie treści (dane wrażliwe)
        std::cout << "[DATA_MANAGER] PatientData serialized to JSON (" 
                  << json.str().size() << " bytes)" << std::endl;
#endif
        
        return json.str();
    } catch (const std::exception& e) {
        std::cerr << "[DATA_MANAGER] ERROR: Failed to serialize PatientData: " 
                  << e.what() << std::endl;
        return "{}"; // Return empty JSON object on error
    } catch (...) {
        std::cerr << "[DATA_MANAGER] ERROR: Unknown exception during PatientData serialization" 
                  << std::endl;
        return "{}";
    }
}

/**
 * @brief Parsuje dane pacjenta z formatu JSON
 * @param json String z danymi w formacie JSON
 * @return Obiekt PatientData (pusty jeśli parsing nie powiedzie się)
 * 
 * ERROR HANDLING:
 * - Nieprawidłowy JSON zwraca pusty obiekt PatientData
 * - Brakujące pola są inicjalizowane domyślnymi wartościami
 * - Błędy są logowane ale nie powodują wyjątków
 */
PatientData PatientData::fromJson(const std::string& json) {
    PatientData patient;
    
    if (json.empty()) {
#if DEBUG_DATA_MANAGER
        std::cout << "[DATA_MANAGER] WARNING: Empty JSON provided for PatientData parsing" 
                  << std::endl;
#endif
        return patient;
    }
    
    try {
        DataManager dm;
        
        patient.id = dm.extractStringValue(json, "id");
        patient.firstName = dm.extractStringValue(json, "firstName");
        patient.lastName = dm.extractStringValue(json, "lastName");
        patient.pesel = dm.extractStringValue(json, "pesel");
        patient.birthDate = dm.extractStringValue(json, "birthDate");
        patient.gender = dm.extractStringValue(json, "gender");
        patient.phoneNumber = dm.extractStringValue(json, "phoneNumber");
        patient.email = dm.extractStringValue(json, "email");
        patient.address = dm.extractStringValue(json, "address");
        patient.medicalHistory = dm.extractStringValue(json, "medicalHistory");
        
#if DEBUG_DATA_MANAGER
        std::cout << "[DATA_MANAGER] PatientData parsed successfully. ID: " 
                  << (patient.id.empty() ? "<empty>" : patient.id) << std::endl;
#endif
        
    } catch (const std::exception& e) {
        std::cerr << "[DATA_MANAGER] ERROR: Exception during PatientData parsing: " 
                  << e.what() << std::endl;
        // Return empty patient object on error
        patient = PatientData();
    } catch (...) {
        std::cerr << "[DATA_MANAGER] ERROR: Unknown exception during PatientData parsing" 
                  << std::endl;
        patient = PatientData();
    }
    
    return patient;
}

// ==================== Implementacja Exercise ====================

std::string Exercise::toJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"id\": \"" << escapeJsonStringValue(id) << "\",\n";
    json << "  \"name\": \"" << escapeJsonStringValue(name) << "\",\n";
    json << "  \"description\": \"" << escapeJsonStringValue(description) << "\",\n";
    json << "  \"category\": \"" << escapeJsonStringValue(category) << "\",\n";
    json << "  \"sets\": " << sets << ",\n";
    json << "  \"reps\": " << reps << ",\n";
    json << "  \"weight\": " << weight << ",\n";
    json << "  \"notes\": \"" << escapeJsonStringValue(notes) << "\"\n";
    json << "}";
    return json.str();
}

Exercise Exercise::fromJson(const std::string& json) {
    Exercise exercise;
    DataManager dm;
    
    exercise.id = dm.extractStringValue(json, "id");
    exercise.name = dm.extractStringValue(json, "name");
    exercise.description = dm.extractStringValue(json, "description");
    exercise.category = dm.extractStringValue(json, "category");
    exercise.sets = dm.extractIntValue(json, "sets");
    exercise.reps = dm.extractIntValue(json, "reps");
    exercise.weight = dm.extractDoubleValue(json, "weight");
    exercise.notes = dm.extractStringValue(json, "notes");
    
    return exercise;
}

// ==================== Implementacja ExerciseResult ====================

std::string ExerciseResult::toJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"id\": \"" << escapeJsonStringValue(id) << "\",\n";
    json << "  \"patientId\": \"" << escapeJsonStringValue(patientId) << "\",\n";
    json << "  \"exerciseId\": \"" << escapeJsonStringValue(exerciseId) << "\",\n";
    json << "  \"date\": \"" << escapeJsonStringValue(date) << "\",\n";
    json << "  \"completedSets\": " << completedSets << ",\n";
    json << "  \"completedReps\": " << completedReps << ",\n";
    json << "  \"actualWeight\": " << actualWeight << ",\n";
    json << "  \"rating\": \"" << escapeJsonStringValue(rating) << "\",\n";
    json << "  \"notes\": \"" << escapeJsonStringValue(notes) << "\"\n";
    json << "}";
    return json.str();
}

ExerciseResult ExerciseResult::fromJson(const std::string& json) {
    ExerciseResult result;
    DataManager dm;
    
    result.id = dm.extractStringValue(json, "id");
    result.patientId = dm.extractStringValue(json, "patientId");
    result.exerciseId = dm.extractStringValue(json, "exerciseId");
    result.date = dm.extractStringValue(json, "date");
    result.completedSets = dm.extractIntValue(json, "completedSets");
    result.completedReps = dm.extractIntValue(json, "completedReps");
    result.actualWeight = dm.extractDoubleValue(json, "actualWeight");
    result.rating = dm.extractStringValue(json, "rating");
    result.notes = dm.extractStringValue(json, "notes");
    
    return result;
}

// ==================== Implementacja TrainingPlan ====================

std::string TrainingPlan::toJson() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"id\": \"" << escapeJsonStringValue(id) << "\",\n";
    json << "  \"patientId\": \"" << escapeJsonStringValue(patientId) << "\",\n";
    json << "  \"name\": \"" << escapeJsonStringValue(name) << "\",\n";
    json << "  \"description\": \"" << escapeJsonStringValue(description) << "\",\n";
    json << "  \"startDate\": \"" << escapeJsonStringValue(startDate) << "\",\n";
    json << "  \"endDate\": \"" << escapeJsonStringValue(endDate) << "\",\n";
    json << "  \"exercises\": [\n";
    
    for (size_t i = 0; i < exercises.size(); ++i) {
        json << "    " << exercises[i].toJson();
        if (i < exercises.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}";
    return json.str();
}

TrainingPlan TrainingPlan::fromJson(const std::string& json) {
    TrainingPlan plan;
    DataManager dm;
    
    plan.id = dm.extractStringValue(json, "id");
    plan.patientId = dm.extractStringValue(json, "patientId");
    plan.name = dm.extractStringValue(json, "name");
    plan.description = dm.extractStringValue(json, "description");
    plan.startDate = dm.extractStringValue(json, "startDate");
    plan.endDate = dm.extractStringValue(json, "endDate");
    
    // Uwaga: parsowanie tablicy ćwiczeń wymaga bardziej zaawansowanego parsera JSON
    // W tej prostej implementacji pomijamy automatyczne parsowanie tablic
    
    return plan;
}

// ==================== Implementacja DataManager ====================

DataManager::DataManager() : encryptionEnabled(false) {}

DataManager::~DataManager() {}

void DataManager::setEncryptionKey(const std::string& key) {
    encryptionKey = key;
    encryptionEnabled = !key.empty();
}

void DataManager::clearEncryptionKey() {
    encryptionKey.clear();
    encryptionEnabled = false;
}

bool DataManager::isEncryptionEnabled() const {
    return encryptionEnabled;
}

bool DataManager::savePatientData(const PatientData& patient, const std::string& filename, bool encrypt) {
    std::string json = patient.toJson();
    
    if (encrypt && encryptionEnabled) {
        try {
            json = Encryption::encrypt(json, encryptionKey);
        } catch (...) {
            return false;
        }
    }
    
    return writeFile(filename, json);
}

PatientData DataManager::loadPatientData(const std::string& filename, bool decrypt) {
    std::string content = readFile(filename);
    
    if (content.empty()) {
        return PatientData();
    }
    
    if (decrypt && encryptionEnabled) {
        try {
            content = Encryption::decrypt(content, encryptionKey);
        } catch (...) {
            return PatientData();
        }
    }
    
    return PatientData::fromJson(content);
}

bool DataManager::saveExercises(const std::vector<Exercise>& exercises, const std::string& filename, bool encrypt) {
    std::ostringstream json;
    json << "[\n";
    
    for (size_t i = 0; i < exercises.size(); ++i) {
        json << "  " << exercises[i].toJson();
        if (i < exercises.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "]";
    
    std::string content = json.str();
    
    if (encrypt && encryptionEnabled) {
        try {
            content = Encryption::encrypt(content, encryptionKey);
        } catch (...) {
            return false;
        }
    }
    
    return writeFile(filename, content);
}

std::vector<Exercise> DataManager::loadExercises(const std::string& filename, bool decrypt) {
    std::vector<Exercise> exercises;
    std::string content = readFile(filename);
    
    if (content.empty()) {
        return exercises;
    }
    
    if (decrypt && encryptionEnabled) {
        try {
            content = Encryption::decrypt(content, encryptionKey);
        } catch (...) {
            return exercises;
        }
    }
    
    for (const auto& objectJson : splitJsonObjectsFromArray(content)) {
        Exercise exercise = Exercise::fromJson(objectJson);
        if (!exercise.id.empty() || !exercise.name.empty()) {
            exercises.push_back(exercise);
        }
    }
    
    return exercises;
}

bool DataManager::saveExerciseResults(const std::vector<ExerciseResult>& results, const std::string& filename, bool encrypt) {
    std::ostringstream json;
    json << "[\n";
    
    for (size_t i = 0; i < results.size(); ++i) {
        json << "  " << results[i].toJson();
        if (i < results.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "]";
    
    std::string content = json.str();
    
    if (encrypt && encryptionEnabled) {
        try {
            content = Encryption::encrypt(content, encryptionKey);
        } catch (...) {
            return false;
        }
    }
    
    return writeFile(filename, content);
}

std::vector<ExerciseResult> DataManager::loadExerciseResults(const std::string& filename, bool decrypt) {
    std::vector<ExerciseResult> results;
    std::string content = readFile(filename);
    
    if (content.empty()) {
        return results;
    }
    
    if (decrypt && encryptionEnabled) {
        try {
            content = Encryption::decrypt(content, encryptionKey);
        } catch (...) {
            return results;
        }
    }
    
    for (const auto& objectJson : splitJsonObjectsFromArray(content)) {
        ExerciseResult result = ExerciseResult::fromJson(objectJson);
        if (!result.id.empty() || !result.exerciseId.empty() || !result.patientId.empty()) {
            results.push_back(result);
        }
    }
    
    return results;
}

bool DataManager::saveTrainingPlan(const TrainingPlan& plan, const std::string& filename, bool encrypt) {
    std::string json = plan.toJson();
    
    if (encrypt && encryptionEnabled) {
        try {
            json = Encryption::encrypt(json, encryptionKey);
        } catch (...) {
            return false;
        }
    }
    
    return writeFile(filename, json);
}

TrainingPlan DataManager::loadTrainingPlan(const std::string& filename, bool decrypt) {
    std::string content = readFile(filename);
    
    if (content.empty()) {
        return TrainingPlan();
    }
    
    if (decrypt && encryptionEnabled) {
        try {
            content = Encryption::decrypt(content, encryptionKey);
        } catch (...) {
            return TrainingPlan();
        }
    }
    
    return TrainingPlan::fromJson(content);
}

bool DataManager::fileExists(const std::string& filename) const {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool DataManager::deleteFile(const std::string& filename) {
    return (std::remove(filename.c_str()) == 0);
}

std::vector<std::string> DataManager::listFiles(const std::string& directory) const {
    std::vector<std::string> files;
    DIR* dir = opendir(directory.c_str());
    
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string name = entry->d_name;
            if (name != "." && name != "..") {
                files.push_back(name);
            }
        }
        closedir(dir);
    }
    
    return files;
}

bool DataManager::exportToJson(const std::string& data, const std::string& filename) {
    return writeFile(filename, data);
}

std::string DataManager::importFromJson(const std::string& filename) {
    return readFile(filename);
}

std::string DataManager::readFile(const std::string& filename) const {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

bool DataManager::writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

std::string DataManager::extractStringValue(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    
    if (pos == std::string::npos) {
        return "";
    }
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return "";
    }
    
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t endPos = findStringTerminator(json, pos + 1);
    if (endPos == std::string::npos) {
        return "";
    }
    
    std::string value = json.substr(pos + 1, endPos - pos - 1);
    return unescapeJsonString(value);
}

int DataManager::extractIntValue(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    
    if (pos == std::string::npos) {
        return 0;
    }
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return 0;
    }
    
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        pos++;
    }
    
    std::string numStr;
    while (pos < json.size() && (isdigit(json[pos]) || json[pos] == '-')) {
        numStr += json[pos];
        pos++;
    }
    
    if (numStr.empty()) {
        return 0;
    }
    
    return std::stoi(numStr);
}

double DataManager::extractDoubleValue(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    
    if (pos == std::string::npos) {
        return 0.0;
    }
    
    pos = json.find(':', pos);
    if (pos == std::string::npos) {
        return 0.0;
    }
    
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) {
        pos++;
    }
    
    std::string numStr;
    while (pos < json.size() && (isdigit(json[pos]) || json[pos] == '-' || json[pos] == '.')) {
        numStr += json[pos];
        pos++;
    }
    
    if (numStr.empty()) {
        return 0.0;
    }
    
    return std::stod(numStr);
}

std::string DataManager::escapeJsonString(const std::string& input) const {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default: output += c; break;
        }
    }
    return output;
}

std::string DataManager::unescapeJsonString(const std::string& input) const {
    std::string output;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[i + 1]) {
                case '"': output += '"'; i++; break;
                case '\\': output += '\\'; i++; break;
                case 'b': output += '\b'; i++; break;
                case 'f': output += '\f'; i++; break;
                case 'n': output += '\n'; i++; break;
                case 'r': output += '\r'; i++; break;
                case 't': output += '\t'; i++; break;
                default: output += input[i]; break;
            }
        } else {
            output += input[i];
        }
    }
    return output;
}
