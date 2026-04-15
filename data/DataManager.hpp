#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include <string>
#include <vector>
#include <map>
#include "Encryption.hpp"

// Struktura danych pacjenta
struct PatientData {
    std::string id;
    std::string firstName;
    std::string lastName;
    std::string pesel;
    std::string birthDate;
    std::string gender;
    std::string phone;
    std::string email;
    std::string address;
    std::string medicalHistory;
    std::string notes;
};

// Struktura pojedynczego ćwiczenia
struct ExerciseData {
    std::string exerciseId;
    std::string exerciseName;
    int sets;
    int reps;
    double weight;
    std::string duration;      // czas trwania w formacie HH:MM:SS
    std::string difficulty;    // poziom trudności
    std::string notes;
};

// Struktura sesji treningowej
struct TrainingSession {
    std::string sessionId;
    std::string patientId;
    std::string date;
    std::string therapist;
    std::string sessionType;
    std::vector<ExerciseData> exercises;
    std::string overallNotes;
    int rating;                // ocena sesji 1-10
};

// Struktura wyników ćwiczeń
struct ExerciseResults {
    std::string patientId;
    std::string exerciseId;
    std::string date;
    int completedSets;
    int completedReps;
    double usedWeight;
    std::string actualDuration;
    std::string performanceQuality;  // jakość wykonania
    std::string painLevel;           // poziom bólu przed/po
    std::string improvements;        // zauważone postępy
};

class DataManager {
public:
    // Konstruktor z opcjonalnym kluczem szyfrującym
    explicit DataManager(const std::string& basePath = "./data", 
                        const std::string& encryptionKey = "");
    
    // Podstawowe operacje na plikach
    bool saveData(const std::string& filename, const std::string& data);
    std::string loadData(const std::string& filename);
    bool fileExists(const std::string& filename) const;
    bool deleteFile(const std::string& filename);
    std::vector<std::string> listFiles() const;
    
    // Operacje na danych JSON - wersje nieszyfrowane (eksport/import)
    bool savePatientJson(const std::string& filename, const PatientData& patient);
    bool saveTrainingJson(const std::string& filename, const TrainingSession& session);
    bool saveExerciseResultsJson(const std::string& filename, const ExerciseResults& results);
    
    PatientData loadPatientJson(const std::string& filename);
    TrainingSession loadTrainingJson(const std::string& filename);
    ExerciseResults loadExerciseResultsJson(const std::string& filename);
    
    // Operacje na danych JSON - wersje szyfrowane
    bool savePatientEncrypted(const std::string& filename, const PatientData& patient);
    bool saveTrainingEncrypted(const std::string& filename, const TrainingSession& session);
    bool saveExerciseResultsEncrypted(const std::string& filename, const ExerciseResults& results);
    
    PatientData loadPatientEncrypted(const std::string& filename);
    TrainingSession loadTrainingEncrypted(const std::string& filename);
    ExerciseResults loadExerciseResultsEncrypted(const std::string& filename);
    
    // Zarządzanie kluczami szyfrującymi
    void setEncryptionKey(const std::string& key);
    void clearEncryptionKey();
    bool isEncryptionEnabled() const;

private:
    std::string m_basePath;
    std::string m_encryptionKey;
    bool m_encryptionEnabled;
    
    // Pomocnicze metody
    std::string getFullPath(const std::string& filename) const;
    bool ensureDirectoryExists(const std::string& path) const;
    
    // Konwersja struktur do/from JSON
    std::string patientToJson(const PatientData& patient) const;
    std::string trainingToJson(const TrainingSession& session) const;
    std::string exerciseResultsToJson(const ExerciseResults& results) const;
    
    PatientData jsonToPatient(const std::string& json) const;
    TrainingSession jsonToTraining(const std::string& json) const;
    ExerciseResults jsonToExerciseResults(const std::string& json) const;
    
    // Proste parsowanie JSON (bez zewnętrznych bibliotek)
    std::string extractJsonValue(const std::string& json, const std::string& key) const;
    int extractJsonInt(const std::string& json, const std::string& key) const;
    double extractJsonDouble(const std::string& json, const std::string& key) const;
};

#endif // DATAMANAGER_HPP
