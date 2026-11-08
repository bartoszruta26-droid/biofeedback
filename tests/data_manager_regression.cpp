#include "data/DataManager.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

namespace {

const char* kExercisesFile = "/tmp/biofeedback_dm_exercises.json";
const char* kResultsFile = "/tmp/biofeedback_dm_results.json";
const char* kPatientFile = "/tmp/biofeedback_dm_patient.json";
const char* kAtomicFailureDirectory = "/tmp/biofeedback_dm_atomic_failure_dir";
const char* kAtomicFailureFile = "/tmp/biofeedback_dm_atomic_failure_dir/target.dat";

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        DataManager noKey; // Default development mode: plaintext files, no key required.
        PatientData developmentPatient; developmentPatient.id = "development";
        expect(noKey.savePatientData(developmentPatient, kPatientFile, false), "plaintext development-mode patient write");
        expect(noKey.loadPatientData(kPatientFile, false).id == "development", "plaintext development-mode patient read");

        DataManager dm;
        dm.setEncryptionKey("regression-key-material-32-bytes");

        std::vector<Exercise> exercises;
        Exercise ex1;
        ex1.id = "ex-1";
        ex1.name = "Bracing \"A\"";
        ex1.description = "Stabilizacja\\Core";
        ex1.category = "core";
        ex1.sets = 3;
        ex1.reps = 10;
        ex1.weight = 0.0;
        ex1.notes = "uwaga: token-with-}-inside";

        Exercise ex2;
        ex2.id = "ex-2";
        ex2.name = "Glute bridge";
        ex2.description = "Mostek biodrowy";
        ex2.category = "rehab";
        ex2.sets = 4;
        ex2.reps = 12;
        ex2.weight = 5.5;
        ex2.notes = "progresja";

        exercises.push_back(ex1);
        exercises.push_back(ex2);

        expect(dm.saveExercises(exercises, kExercisesFile, false), "saveExercises powinno się udać");

        const auto loadedExercises = dm.loadExercises(kExercisesFile, false);
        expect(loadedExercises.size() == 2, "loadExercises powinno zwrócić 2 elementy");
        expect(loadedExercises[0].id == "ex-1", "exercise[0].id");
        expect(loadedExercises[0].name == "Bracing \"A\"", "exercise[0].name z escapowaniem");
        expect(loadedExercises[1].name == "Glute bridge", "exercise[1].name");

        std::vector<ExerciseResult> results;
        ExerciseResult r1;
        r1.id = "r-1";
        r1.patientId = "p-1";
        r1.exerciseId = "ex-1";
        r1.date = "2026-04-16";
        r1.completedSets = 3;
        r1.completedReps = 10;
        r1.actualWeight = 0.0;
        r1.rating = "łatwe";
        r1.notes = "ok";

        ExerciseResult r2;
        r2.id = "r-2";
        r2.patientId = "p-2";
        r2.exerciseId = "ex-2";
        r2.date = "2026-04-16";
        r2.completedSets = 4;
        r2.completedReps = 12;
        r2.actualWeight = 5.5;
        r2.rating = "średnie";
        r2.notes = "postęp";

        results.push_back(r1);
        results.push_back(r2);

        expect(dm.saveExerciseResults(results, kResultsFile, false), "saveExerciseResults powinno się udać");

        const auto loadedResults = dm.loadExerciseResults(kResultsFile, false);
        expect(loadedResults.size() == 2, "loadExerciseResults powinno zwrócić 2 elementy");
        expect(loadedResults[0].patientId == "p-1", "result[0].patientId");
        expect(loadedResults[1].exerciseId == "ex-2", "result[1].exerciseId");

        PatientData patient;
        patient.id = "p-1";
        patient.firstName = "Jan \"J\"";
        patient.lastName = "Kowalski";
        patient.pesel = "90010112345";
        patient.birthDate = "1990-01-01";
        patient.gender = "M";
        patient.phoneNumber = "+48 123 456 789";
        patient.email = "jan@example.com";
        patient.address = "ul. Testowa 1\\2";
        patient.medicalHistory = "Ból kolana\npo urazie";

        expect(dm.savePatientData(patient, kPatientFile, false), "savePatientData powinno się udać");
        const auto loadedPatient = dm.loadPatientData(kPatientFile, false);
        expect(loadedPatient.firstName == patient.firstName, "patient.firstName z escapowaniem");
        expect(loadedPatient.address == patient.address, "patient.address z backslash");
        expect(loadedPatient.medicalHistory == patient.medicalHistory, "patient.medicalHistory z newline");

        TrainingPlan plan;
        plan.id = "plan-1";
        plan.patientId = "p-1";
        plan.name = "Rehabilitation plan";
        plan.description = "Encrypted plan";
        expect(dm.saveTrainingPlan(plan, kResultsFile, false), "szyfrowany plan treningowy");
        const auto loadedPlan = dm.loadTrainingPlan(kResultsFile, false);
        expect(loadedPlan.id == plan.id && loadedPlan.patientId == plan.patientId, "odszyfrowany plan treningowy");

        const std::string exportPayload = "protected-json-payload";
        expect(dm.exportToJson(exportPayload, kResultsFile), "szyfrowany eksport JSON");
        expect(dm.importFromJson(kResultsFile) == exportPayload, "odszyfrowany import JSON");

        // Turning encryption off must not make existing BFENC files unreadable.
        // The existing encrypted patient record remains readable with the same key.
        dm.setEncryptionEnabled(false);
        expect(dm.loadPatientData(kPatientFile, false).firstName == patient.firstName,
               "encrypted patient data remains readable after disabling encryption");

        // Explicitly switch the same manager to plaintext development mode.
        patient.id = "p-plaintext";
        expect(dm.savePatientData(patient, kPatientFile, false), "plaintext mode patient save");
        std::ifstream plainPatientFile(kPatientFile, std::ios::binary);
        std::string plainPatient((std::istreambuf_iterator<char>(plainPatientFile)), std::istreambuf_iterator<char>());
        expect(plainPatient.find("BFENC:v1:") != 0, "patient file must be plaintext when encryption is disabled");
        expect(dm.loadPatientData(kPatientFile, false).id == "p-plaintext", "plaintext mode patient read");

        expect(dm.saveExerciseResults(results, kResultsFile, false), "plaintext mode exercise results save");
        expect(dm.loadExerciseResults(kResultsFile, false).size() == 2, "plaintext mode exercise results read");
        expect(dm.saveTrainingPlan(plan, kResultsFile, false), "plaintext mode training plan save");
        expect(dm.loadTrainingPlan(kResultsFile, false).id == plan.id, "plaintext mode training plan read");
        expect(dm.exportToJson(exportPayload, kResultsFile), "plaintext mode JSON export");
        expect(dm.importFromJson(kResultsFile) == exportPayload, "plaintext mode JSON import");


        // Make the destination directory unwritable so QSaveFile cannot create
        // its sibling temporary file. The existing destination must remain intact.
        ::chmod(kAtomicFailureDirectory, 0700);
        ::unlink(kAtomicFailureFile);
        ::rmdir(kAtomicFailureDirectory);
        expect(::mkdir(kAtomicFailureDirectory, 0700) == 0, "create atomic failure directory");
        {
            std::ofstream original(kAtomicFailureFile, std::ios::binary);
            original << "original-content";
        }
        expect(::chmod(kAtomicFailureDirectory, 0500) == 0, "make atomic destination directory read-only");
        const bool interruptedWrite = dm.exportToJson("payload-that-cannot-be-written", kAtomicFailureFile);
        const int restoreResult = ::chmod(kAtomicFailureDirectory, 0700);
        expect(restoreResult == 0, "restore atomic destination directory permissions");
        expect(!interruptedWrite, "unwritable atomic destination must fail");
        std::ifstream preserved(kAtomicFailureFile, std::ios::binary);
        std::string preservedContent((std::istreambuf_iterator<char>(preserved)), std::istreambuf_iterator<char>());
        expect(preservedContent == "original-content", "previous destination must survive failed atomic write");

        std::remove(kAtomicFailureFile);
        ::rmdir(kAtomicFailureDirectory);
        std::remove(kExercisesFile);
        std::remove(kResultsFile);
        std::remove(kPatientFile);
        std::cout << "OK: data_manager_regression" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::remove(kAtomicFailureFile);
        ::chmod(kAtomicFailureDirectory, 0700);
        ::rmdir(kAtomicFailureDirectory);
        std::remove(kExercisesFile);
        std::remove(kResultsFile);
        std::remove(kPatientFile);
        std::cerr << "FAIL: " << ex.what() << std::endl;
        return 1;
    }
}
