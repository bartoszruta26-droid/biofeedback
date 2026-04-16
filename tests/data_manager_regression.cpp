#include "data/DataManager.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

const char* kExercisesFile = "/tmp/biofeedback_dm_exercises.json";
const char* kResultsFile = "/tmp/biofeedback_dm_results.json";
const char* kPatientFile = "/tmp/biofeedback_dm_patient.json";

void expect(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

} // namespace

int main() {
    try {
        DataManager dm;

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

        std::remove(kExercisesFile);
        std::remove(kResultsFile);
        std::remove(kPatientFile);
        std::cout << "OK: data_manager_regression" << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::remove(kExercisesFile);
        std::remove(kResultsFile);
        std::remove(kPatientFile);
        std::cerr << "FAIL: " << ex.what() << std::endl;
        return 1;
    }
}
