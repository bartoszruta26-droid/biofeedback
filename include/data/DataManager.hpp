#ifndef DATA_MANAGER_HPP
#define DATA_MANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "Encryption.hpp"

// Struktura danych pacjenta
struct PatientData {
    std::string id;
    std::string firstName;
    std::string lastName;
    std::string pesel;
    std::string birthDate;
    std::string gender;
    std::string phoneNumber;
    std::string email;
    std::string address;
    std::string medicalHistory;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static PatientData fromJson(const std::string& json);
};

// Struktura pojedynczego ćwiczenia
struct Exercise {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    int sets;
    int reps;
    double weight;
    std::string notes;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static Exercise fromJson(const std::string& json);
};

// Struktura wyniku ćwiczenia (treningu)
struct ExerciseResult {
    std::string id;
    std::string patientId;
    std::string exerciseId;
    std::string date;
    int completedSets;
    int completedReps;
    double actualWeight;
    std::string rating; // np. "łatwe", "średnie", "trudne"
    std::string notes;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static ExerciseResult fromJson(const std::string& json);
};

// Struktura konspektu treningowego
struct TrainingPlan {
    std::string id;
    std::string patientId;
    std::string name;
    std::string description;
    std::string startDate;
    std::string endDate;
    std::vector<Exercise> exercises;
    
    // Konwersja do JSON
    std::string toJson() const;
    // Parsowanie z JSON
    static TrainingPlan fromJson(const std::string& json);
};

class DataManager {
public:
    DataManager();
    ~DataManager();
    
    // Zarządzanie kluczem szyfrującym
    void setEncryptionKey(const std::string& key);
    void clearEncryptionKey();
    bool isEncryptionEnabled() const;
    
    // Zapis i odczyt danych pacjenta
    bool savePatientData(const PatientData& patient, const std::string& filename, bool encrypt = false);
    PatientData loadPatientData(const std::string& filename, bool decrypt = false);
    
    // Zapis i odczyt listy ćwiczeń
    bool saveExercises(const std::vector<Exercise>& exercises, const std::string& filename, bool encrypt = false);
    std::vector<Exercise> loadExercises(const std::string& filename, bool decrypt = false);
    
    // Zapis i odczyt wyników ćwiczeń
    bool saveExerciseResults(const std::vector<ExerciseResult>& results, const std::string& filename, bool encrypt = false);
    std::vector<ExerciseResult> loadExerciseResults(const std::string& filename, bool decrypt = false);
    
    // Zapis i odczyt konspektu treningowego
    bool saveTrainingPlan(const TrainingPlan& plan, const std::string& filename, bool encrypt = false);
    TrainingPlan loadTrainingPlan(const std::string& filename, bool decrypt = false);
    
    // Operacje na plikach
    bool fileExists(const std::string& filename) const;
    bool deleteFile(const std::string& filename);
    std::vector<std::string> listFiles(const std::string& directory = ".") const;
    
    // Eksport/Import nieszyfrowany JSON
    bool exportToJson(const std::string& data, const std::string& filename);
    std::string importFromJson(const std::string& filename);

private:
    std::string encryptionKey;
    bool encryptionEnabled;
    
    // Pomocnicze metody do operacji na plikach
    std::string readFile(const std::string& filename) const;
    bool writeFile(const std::string& filename, const std::string& content);
    
    // Pomocnicze metody do parsowania JSON (prosta implementacja)
    std::string extractStringValue(const std::string& json, const std::string& key) const;
    int extractIntValue(const std::string& json, const std::string& key) const;
    double extractDoubleValue(const std::string& json, const std::string& key) const;
    std::string escapeJsonString(const std::string& input) const;
    std::string unescapeJsonString(const std::string& input) const;
};

#endif // DATA_MANAGER_HPP
