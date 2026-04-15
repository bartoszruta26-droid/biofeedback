#include "DataManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

// Pomocnicza funkcja do escape'owania znaków w JSON
static std::string escapeJsonString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

// Pomocnicza funkcja do usuwania białych znaków z początku i końca
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

DataManager::DataManager(const std::string& basePath, const std::string& encryptionKey)
    : m_basePath(basePath), m_encryptionKey(encryptionKey), 
      m_encryptionEnabled(!encryptionKey.empty()) {
    ensureDirectoryExists(m_basePath);
}

bool DataManager::ensureDirectoryExists(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    
    // Tworzenie katalogu rekurencyjnie
    std::string currentPath;
    std::istringstream iss(path);
    std::string segment;
    
    // Obsługa ścieżek absolutnych i względnych
    if (!path.empty() && path[0] == '/') {
        currentPath = "/";
    }
    
    while (std::getline(iss, segment, '/')) {
        if (segment.empty()) continue;
        
        if (!currentPath.empty() && currentPath.back() != '/') {
            currentPath += '/';
        }
        currentPath += segment;
        
        if (stat(currentPath.c_str(), &st) != 0) {
            if (mkdir(currentPath.c_str(), 0755) != 0) {
                std::cerr << "Nie udało się utworzyć katalogu: " << currentPath << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

std::string DataManager::getFullPath(const std::string& filename) const {
    std::string fullPath = m_basePath;
    if (!fullPath.empty() && fullPath.back() != '/') {
        fullPath += '/';
    }
    fullPath += filename;
    return fullPath;
}

bool DataManager::saveData(const std::string& filename, const std::string& data) {
    std::string fullPath = getFullPath(filename);
    
    if (!ensureDirectoryExists(fullPath.substr(0, fullPath.find_last_of('/')))) {
        return false;
    }
    
    std::ofstream file(fullPath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    if (m_encryptionEnabled) {
        std::string encryptedData = Encryption::encrypt(data, m_encryptionKey);
        file.write(encryptedData.c_str(), encryptedData.size());
    } else {
        file.write(data.c_str(), data.size());
    }
    
    file.close();
    return file.good();
}

std::string DataManager::loadData(const std::string& filename) {
    std::string fullPath = getFullPath(filename);
    
    std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Nie udało się otworzyć pliku: " + fullPath);
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Nie udało się odczytać danych z pliku");
    }
    
    std::string content(buffer.begin(), buffer.end());
    file.close();
    
    if (m_encryptionEnabled) {
        return Encryption::decrypt(content, m_encryptionKey);
    }
    
    return content;
}

bool DataManager::fileExists(const std::string& filename) const {
    std::string fullPath = getFullPath(filename);
    struct stat st;
    return (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode));
}

bool DataManager::deleteFile(const std::string& filename) {
    std::string fullPath = getFullPath(filename);
    return (remove(fullPath.c_str()) == 0);
}

std::vector<std::string> DataManager::listFiles() const {
    std::vector<std::string> files;
    
    DIR* dir = opendir(m_basePath.c_str());
    if (dir == nullptr) {
        return files;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            files.push_back(name);
        }
    }
    
    closedir(dir);
    std::sort(files.begin(), files.end());
    return files;
}

void DataManager::setEncryptionKey(const std::string& key) {
    m_encryptionKey = key;
    m_encryptionEnabled = !key.empty();
}

void DataManager::clearEncryptionKey() {
    m_encryptionKey.clear();
    m_encryptionEnabled = false;
}

bool DataManager::isEncryptionEnabled() const {
    return m_encryptionEnabled;
}

// Konwersja PatientData do JSON
std::string DataManager::patientToJson(const PatientData& patient) const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"id\": \"" << escapeJsonString(patient.id) << "\",\n";
    json << "  \"firstName\": \"" << escapeJsonString(patient.firstName) << "\",\n";
    json << "  \"lastName\": \"" << escapeJsonString(patient.lastName) << "\",\n";
    json << "  \"pesel\": \"" << escapeJsonString(patient.pesel) << "\",\n";
    json << "  \"birthDate\": \"" << escapeJsonString(patient.birthDate) << "\",\n";
    json << "  \"gender\": \"" << escapeJsonString(patient.gender) << "\",\n";
    json << "  \"phone\": \"" << escapeJsonString(patient.phone) << "\",\n";
    json << "  \"email\": \"" << escapeJsonString(patient.email) << "\",\n";
    json << "  \"address\": \"" << escapeJsonString(patient.address) << "\",\n";
    json << "  \"medicalHistory\": \"" << escapeJsonString(patient.medicalHistory) << "\",\n";
    json << "  \"notes\": \"" << escapeJsonString(patient.notes) << "\"\n";
    json << "}";
    return json.str();
}

// Konwersja ExerciseData do JSON
static std::string exerciseToJson(const ExerciseData& exercise) {
    std::ostringstream json;
    json << "    {\n";
    json << "      \"exerciseId\": \"" << escapeJsonString(exercise.exerciseId) << "\",\n";
    json << "      \"exerciseName\": \"" << escapeJsonString(exercise.exerciseName) << "\",\n";
    json << "      \"sets\": " << exercise.sets << ",\n";
    json << "      \"reps\": " << exercise.reps << ",\n";
    json << "      \"weight\": " << exercise.weight << ",\n";
    json << "      \"duration\": \"" << escapeJsonString(exercise.duration) << "\",\n";
    json << "      \"difficulty\": \"" << escapeJsonString(exercise.difficulty) << "\",\n";
    json << "      \"notes\": \"" << escapeJsonString(exercise.notes) << "\"\n";
    json << "    }";
    return json.str();
}

// Konwersja TrainingSession do JSON
std::string DataManager::trainingToJson(const TrainingSession& session) const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"sessionId\": \"" << escapeJsonString(session.sessionId) << "\",\n";
    json << "  \"patientId\": \"" << escapeJsonString(session.patientId) << "\",\n";
    json << "  \"date\": \"" << escapeJsonString(session.date) << "\",\n";
    json << "  \"therapist\": \"" << escapeJsonString(session.therapist) << "\",\n";
    json << "  \"sessionType\": \"" << escapeJsonString(session.sessionType) << "\",\n";
    json << "  \"exercises\": [\n";
    
    for (size_t i = 0; i < session.exercises.size(); ++i) {
        json << exerciseToJson(session.exercises[i]);
        if (i < session.exercises.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ],\n";
    json << "  \"overallNotes\": \"" << escapeJsonString(session.overallNotes) << "\",\n";
    json << "  \"rating\": " << session.rating << "\n";
    json << "}";
    return json.str();
}

// Konwersja ExerciseResults do JSON
std::string DataManager::exerciseResultsToJson(const ExerciseResults& results) const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"patientId\": \"" << escapeJsonString(results.patientId) << "\",\n";
    json << "  \"exerciseId\": \"" << escapeJsonString(results.exerciseId) << "\",\n";
    json << "  \"date\": \"" << escapeJsonString(results.date) << "\",\n";
    json << "  \"completedSets\": " << results.completedSets << ",\n";
    json << "  \"completedReps\": " << results.completedReps << ",\n";
    json << "  \"usedWeight\": " << results.usedWeight << ",\n";
    json << "  \"actualDuration\": \"" << escapeJsonString(results.actualDuration) << "\",\n";
    json << "  \"performanceQuality\": \"" << escapeJsonString(results.performanceQuality) << "\",\n";
    json << "  \"painLevel\": \"" << escapeJsonString(results.painLevel) << "\",\n";
    json << "  \"improvements\": \"" << escapeJsonString(results.improvements) << "\"\n";
    json << "}";
    return json.str();
}

// Parsowanie wartości string z JSON
std::string DataManager::extractJsonValue(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";
    
    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) return "";
    
    if (json[valueStart] == '"') {
        size_t valueEnd = valueStart + 1;
        while (valueEnd < json.size()) {
            if (json[valueEnd] == '"' && json[valueEnd - 1] != '\\') {
                break;
            }
            valueEnd++;
        }
        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }
    
    return "";
}

// Parsowanie wartości int z JSON
int DataManager::extractJsonInt(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return 0;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return 0;
    
    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) return 0;
    
    size_t valueEnd = json.find_first_of(",\n}", valueStart);
    if (valueEnd == std::string::npos) valueEnd = json.size();
    
    std::string valueStr = trim(json.substr(valueStart, valueEnd - valueStart));
    try {
        return std::stoi(valueStr);
    } catch (...) {
        return 0;
    }
}

// Parsowanie wartości double z JSON
double DataManager::extractJsonDouble(const std::string& json, const std::string& key) const {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return 0.0;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return 0.0;
    
    size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (valueStart == std::string::npos) return 0.0;
    
    size_t valueEnd = json.find_first_of(",\n}", valueStart);
    if (valueEnd == std::string::npos) valueEnd = json.size();
    
    std::string valueStr = trim(json.substr(valueStart, valueEnd - valueStart));
    try {
        return std::stod(valueStr);
    } catch (...) {
        return 0.0;
    }
}

// Parsowanie PatientData z JSON
PatientData DataManager::jsonToPatient(const std::string& json) const {
    PatientData patient;
    patient.id = extractJsonValue(json, "id");
    patient.firstName = extractJsonValue(json, "firstName");
    patient.lastName = extractJsonValue(json, "lastName");
    patient.pesel = extractJsonValue(json, "pesel");
    patient.birthDate = extractJsonValue(json, "birthDate");
    patient.gender = extractJsonValue(json, "gender");
    patient.phone = extractJsonValue(json, "phone");
    patient.email = extractJsonValue(json, "email");
    patient.address = extractJsonValue(json, "address");
    patient.medicalHistory = extractJsonValue(json, "medicalHistory");
    patient.notes = extractJsonValue(json, "notes");
    return patient;
}

// Parsowanie ExerciseData z JSON (pomocnicza funkcja)
static ExerciseData jsonToExercise(const std::string& json) {
    ExerciseData exercise;
    size_t pos = 0;
    
    auto extractValue = [&json, &pos](const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey, pos);
        if (keyPos == std::string::npos) return "";
        
        size_t colonPos = json.find(':', keyPos);
        if (colonPos == std::string::npos) return "";
        
        size_t valueStart = json.find_first_not_of(" \t\n\r", colonPos + 1);
        if (valueStart == std::string::npos) return "";
        
        if (json[valueStart] == '"') {
            size_t valueEnd = valueStart + 1;
            while (valueEnd < json.size()) {
                if (json[valueEnd] == '"' && json[valueEnd - 1] != '\\') {
                    break;
                }
                valueEnd++;
            }
            pos = valueEnd;
            return json.substr(valueStart + 1, valueEnd - valueStart - 1);
        }
        
        size_t valueEnd = json.find_first_of(",\n}", valueStart);
        if (valueEnd == std::string::npos) valueEnd = json.size();
        pos = valueEnd;
        return trim(json.substr(valueStart, valueEnd - valueStart));
    };
    
    auto extractInt = [&extractValue](const std::string& key) -> int {
        std::string val = extractValue(key);
        try { return std::stoi(val); } catch (...) { return 0; }
    };
    
    auto extractDouble = [&extractValue](const std::string& key) -> double {
        std::string val = extractValue(key);
        try { return std::stod(val); } catch (...) { return 0.0; }
    };
    
    exercise.exerciseId = extractValue("exerciseId");
    exercise.exerciseName = extractValue("exerciseName");
    exercise.sets = extractInt("sets");
    exercise.reps = extractInt("reps");
    exercise.weight = extractDouble("weight");
    exercise.duration = extractValue("duration");
    exercise.difficulty = extractValue("difficulty");
    exercise.notes = extractValue("notes");
    
    return exercise;
}

// Parsowanie TrainingSession z JSON
TrainingSession DataManager::jsonToTraining(const std::string& json) const {
    TrainingSession session;
    session.sessionId = extractJsonValue(json, "sessionId");
    session.patientId = extractJsonValue(json, "patientId");
    session.date = extractJsonValue(json, "date");
    session.therapist = extractJsonValue(json, "therapist");
    session.sessionType = extractJsonValue(json, "sessionType");
    session.overallNotes = extractJsonValue(json, "overallNotes");
    session.rating = extractJsonInt(json, "rating");
    
    // Parsowanie tablicy exercises
    size_t exercisesStart = json.find("\"exercises\"");
    if (exercisesStart != std::string::npos) {
        size_t arrayStart = json.find('[', exercisesStart);
        size_t arrayEnd = json.rfind(']');
        
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos && arrayEnd > arrayStart) {
            std::string exercisesArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            
            size_t objStart = 0;
            while ((objStart = exercisesArray.find('{', objStart)) != std::string::npos) {
                size_t objEnd = exercisesArray.find('}', objStart);
                if (objEnd == std::string::npos) break;
                
                std::string exerciseJson = exercisesArray.substr(objStart, objEnd - objStart + 1);
                session.exercises.push_back(jsonToExercise(exerciseJson));
                objStart = objEnd + 1;
            }
        }
    }
    
    return session;
}

// Parsowanie ExerciseResults z JSON
ExerciseResults DataManager::jsonToExerciseResults(const std::string& json) const {
    ExerciseResults results;
    results.patientId = extractJsonValue(json, "patientId");
    results.exerciseId = extractJsonValue(json, "exerciseId");
    results.date = extractJsonValue(json, "date");
    results.completedSets = extractJsonInt(json, "completedSets");
    results.completedReps = extractJsonInt(json, "completedReps");
    results.usedWeight = extractJsonDouble(json, "usedWeight");
    results.actualDuration = extractJsonValue(json, "actualDuration");
    results.performanceQuality = extractJsonValue(json, "performanceQuality");
    results.painLevel = extractJsonValue(json, "painLevel");
    results.improvements = extractJsonValue(json, "improvements");
    return results;
}

// Zapis nieszyfrowany JSON - Patient
bool DataManager::savePatientJson(const std::string& filename, const PatientData& patient) {
    std::string json = patientToJson(patient);
    // Upewnij się, że plik ma rozszerzenie .json
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    return saveData(finalFilename, json);
}

// Zapis nieszyfrowany JSON - Training
bool DataManager::saveTrainingJson(const std::string& filename, const TrainingSession& session) {
    std::string json = trainingToJson(session);
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    return saveData(finalFilename, json);
}

// Zapis nieszyfrowany JSON - ExerciseResults
bool DataManager::saveExerciseResultsJson(const std::string& filename, const ExerciseResults& results) {
    std::string json = exerciseResultsToJson(results);
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    return saveData(finalFilename, json);
}

// Odczyt nieszyfrowany JSON - Patient
PatientData DataManager::loadPatientJson(const std::string& filename) {
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    std::string json = loadData(finalFilename);
    return jsonToPatient(json);
}

// Odczyt nieszyfrowany JSON - Training
TrainingSession DataManager::loadTrainingJson(const std::string& filename) {
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    std::string json = loadData(finalFilename);
    return jsonToTraining(json);
}

// Odczyt nieszyfrowany JSON - ExerciseResults
ExerciseResults DataManager::loadExerciseResultsJson(const std::string& filename) {
    std::string finalFilename = filename;
    if (finalFilename.size() < 5 || finalFilename.substr(finalFilename.size() - 5) != ".json") {
        finalFilename += ".json";
    }
    std::string json = loadData(finalFilename);
    return jsonToExerciseResults(json);
}

// Zapis szyfrowany JSON - Patient
bool DataManager::savePatientEncrypted(const std::string& filename, const PatientData& patient) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    bool result = savePatientJson(filename, patient);
    m_encryptionEnabled = wasEnabled;
    return result;
}

// Zapis szyfrowany JSON - Training
bool DataManager::saveTrainingEncrypted(const std::string& filename, const TrainingSession& session) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    bool result = saveTrainingJson(filename, session);
    m_encryptionEnabled = wasEnabled;
    return result;
}

// Zapis szyfrowany JSON - ExerciseResults
bool DataManager::saveExerciseResultsEncrypted(const std::string& filename, const ExerciseResults& results) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    bool result = saveExerciseResultsJson(filename, results);
    m_encryptionEnabled = wasEnabled;
    return result;
}

// Odczyt szyfrowany JSON - Patient
PatientData DataManager::loadPatientEncrypted(const std::string& filename) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    PatientData result = loadPatientJson(filename);
    m_encryptionEnabled = wasEnabled;
    return result;
}

// Odczyt szyfrowany JSON - Training
TrainingSession DataManager::loadTrainingEncrypted(const std::string& filename) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    TrainingSession result = loadTrainingJson(filename);
    m_encryptionEnabled = wasEnabled;
    return result;
}

// Odczyt szyfrowany JSON - ExerciseResults
ExerciseResults DataManager::loadExerciseResultsEncrypted(const std::string& filename) {
    bool wasEnabled = m_encryptionEnabled;
    if (!m_encryptionEnabled && !m_encryptionKey.empty()) {
        m_encryptionEnabled = true;
    }
    ExerciseResults result = loadExerciseResultsJson(filename);
    m_encryptionEnabled = wasEnabled;
    return result;
}
