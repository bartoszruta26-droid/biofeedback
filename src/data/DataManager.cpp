#include "DataManager.hpp"
#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>

namespace {

std::string escapeJsonStringValue(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    for (const char c : input) {
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

size_t findStringTerminator(const std::string& text, const size_t start) {
    for (size_t i = start; i < text.size(); ++i) {
        if (text[i] != '"') {
            continue;
        }

        size_t backslashes = 0;
        for (size_t j = i; j > 0 && text[j - 1] == '\\'; --j) {
            ++backslashes;
        }

        if (backslashes % 2 == 0) {
            return i;
        }
    }

    return std::string::npos;
}

std::vector<std::string> splitJsonObjectsFromArray(const std::string& arrayJson) {
    std::vector<std::string> objects;
    bool inString = false;
    int depth = 0;
    size_t objectStart = std::string::npos;

    for (size_t i = 0; i < arrayJson.size(); ++i) {
        const char c = arrayJson[i];

        if (c == '"') {
            size_t backslashes = 0;
            for (size_t j = i; j > 0 && arrayJson[j - 1] == '\\'; --j) {
                ++backslashes;
            }
            if (backslashes % 2 == 0) {
                inString = !inString;
            }
        }

        if (inString) {
            continue;
        }

        if (c == '{') {
            if (depth == 0) {
                objectStart = i;
            }
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && objectStart != std::string::npos) {
                objects.push_back(arrayJson.substr(objectStart, i - objectStart + 1));
                objectStart = std::string::npos;
            }
        }
    }

    return objects;
}

} // namespace

// ==================== Implementacja PatientData ====================

std::string PatientData::toJson() const {
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
    return json.str();
}

PatientData PatientData::fromJson(const std::string& json) {
    PatientData patient;
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
