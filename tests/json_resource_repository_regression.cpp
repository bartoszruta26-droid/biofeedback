#include "data/JsonResourceRepository.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <cassert>
#include <iostream>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    assert(directory.isValid());

    const QString key = QStringLiteral("0123456789abcdef0123456789abcdef");
    const QString root = directory.filePath(QStringLiteral("shared-store"));
    biofeedback::data::JsonResourceRepository repository(root, key, true);
    assert(repository.isReady());

    QJsonObject patientPayload{
        {QStringLiteral("firstName"), QStringLiteral("Test")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")},
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-001")}
    };
    QJsonObject created;
    QString error;
    assert(repository.create(QStringLiteral("patients"), patientPayload, created, &error));
    const QString id = created.value(QStringLiteral("id")).toString();
    assert(!id.isEmpty());
    assert(created.value(QStringLiteral("version")).toInt() == 1);
    assert(created.value(QStringLiteral("pesel")).toString() == QStringLiteral("TEST-PATIENT-001"));

    QJsonObject validMeasurement{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("sample_count"), 1},
        {QStringLiteral("source_client"), QStringLiteral("contract-test")}
    };
    QJsonObject measurement;
    assert(repository.create(QStringLiteral("measurements"), validMeasurement, measurement, &error));
    assert(measurement.value(QStringLiteral("patient_id")).toString() == id);

    QJsonObject trainingSessionPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("exercise_name"), QStringLiteral("Contract test")}
    };
    QJsonObject trainingSession;
    assert(repository.create(QStringLiteral("training-sessions"), trainingSessionPayload,
                             trainingSession, &error));
    const QString trainingSessionId = trainingSession.value(QStringLiteral("id")).toString();

    QJsonObject validResult{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("training_session_id"), trainingSessionId},
        {QStringLiteral("completed_repetitions"), 1}
    };
    QJsonObject storedResult;
    assert(repository.create(QStringLiteral("training-results"), validResult, storedResult, &error));
    assert(storedResult.value(QStringLiteral("training_session_id")).toString() == trainingSessionId);


    QJsonObject orphanMeasurement{{QStringLiteral("patient_id"),
        QStringLiteral("00000000-0000-4000-8000-000000000001")}};
    QJsonObject rejectedMeasurement;
    assert(!repository.create(QStringLiteral("measurements"), orphanMeasurement,
                              rejectedMeasurement, &error));
    assert(error == QStringLiteral("PATIENT_NOT_FOUND"));

    const QString recordPath = QDir(root).filePath(
        QStringLiteral("patients/") + id + QStringLiteral(".json"));
    QFile encryptedFile(recordPath);
    assert(encryptedFile.open(QIODevice::ReadOnly));
    const QByteArray encryptedBytes = encryptedFile.readAll();
    assert(encryptedBytes.startsWith("BFENC:v1:"));
    assert(!encryptedBytes.contains("90010112345"));

    biofeedback::data::JsonResourceRepository restarted(root, key, true);
    QJsonObject loaded;
    assert(restarted.get(QStringLiteral("patients"), id, loaded, &error));
    assert(loaded.value(QStringLiteral("lastName")).toString() == QStringLiteral("Patient"));

    const QJsonArray patients = restarted.list(QStringLiteral("patients"), &error);
    assert(patients.size() == 1);

    QJsonObject duplicatePatient{
        {QStringLiteral("firstName"), QStringLiteral("Duplicate")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")},
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-001")}
    };
    QJsonObject rejectedDuplicate;
    assert(!restarted.create(QStringLiteral("patients"), duplicatePatient, rejectedDuplicate, &error));
    assert(error == QStringLiteral("DUPLICATE_PATIENT_REFERENCE"));

    QJsonObject updatePayload{
        {QStringLiteral("firstName"), QStringLiteral("Updated")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")},
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-001")}
    };
    QJsonObject updated;
    assert(restarted.update(QStringLiteral("patients"), id, 1, updatePayload, updated, &error));
    assert(updated.value(QStringLiteral("version")).toInt() == 2);
    assert(!restarted.update(QStringLiteral("patients"), id, 1, updatePayload, updated, &error));
    assert(error == QStringLiteral("VERSION_CONFLICT"));

    assert(!restarted.remove(QStringLiteral("patients"), id, 2, &error));
    assert(error == QStringLiteral("PATIENT_HAS_REFERENCES"));
    assert(!restarted.remove(QStringLiteral("training-sessions"), trainingSessionId, 1, &error));
    assert(error == QStringLiteral("TRAINING_SESSION_HAS_RESULTS"));
    assert(restarted.remove(QStringLiteral("training-results"),
                            storedResult.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("training-sessions"), trainingSessionId, 1, &error));
    assert(restarted.remove(QStringLiteral("measurements"),
                            measurement.value(QStringLiteral("id")).toString(), 1, &error));

    assert(!restarted.get(QStringLiteral("../patients"), id, loaded, &error));
    assert(error == QStringLiteral("UNSUPPORTED_COLLECTION"));
    assert(!restarted.get(QStringLiteral("patients"), QStringLiteral("../../users"), loaded, &error));
    assert(error == QStringLiteral("INVALID_RESOURCE_ID"));

    QJsonObject episodePayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("diagnosis_code"), QStringLiteral("M23.2")},
        {QStringLiteral("care_pathway"), QStringLiteral("conservative")}
    };
    QJsonObject episode;
    assert(restarted.create(QStringLiteral("rehabilitation-episodes"), episodePayload, episode, &error));
    const QString episodeId = episode.value(QStringLiteral("id")).toString();

    QJsonObject assessmentPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("assessment_type"), QStringLiteral("baseline")}
    };
    QJsonObject assessment;
    assert(restarted.create(QStringLiteral("rehabilitation-assessments"), assessmentPayload, assessment, &error));

    QJsonObject goalPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("metric"), QStringLiteral("knee_flexion_rom")}
    };
    QJsonObject goal;
    assert(restarted.create(QStringLiteral("rehabilitation-goals"), goalPayload, goal, &error));

    QJsonObject rehabSessionPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("session_type"), QStringLiteral("treatment")}
    };
    QJsonObject rehabSession;
    assert(restarted.create(QStringLiteral("rehabilitation-sessions"), rehabSessionPayload,
                            rehabSession, &error));
    const QString rehabSessionId = rehabSession.value(QStringLiteral("id")).toString();

    QJsonObject observationPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("session_id"), rehabSessionId},
        {QStringLiteral("pain_before"), 4},
        {QStringLiteral("pain_after"), 2}
    };
    QJsonObject observation;
    assert(restarted.create(QStringLiteral("rehabilitation-observations"), observationPayload,
                            observation, &error));

    QJsonObject decisionPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("session_id"), rehabSessionId},
        {QStringLiteral("decision"), QStringLiteral("HOLD")},
        {QStringLiteral("clinician_approval_required"), true}
    };
    QJsonObject decision;
    assert(restarted.create(QStringLiteral("rehabilitation-decisions"), decisionPayload,
                            decision, &error));

    QJsonObject invalidEpisodeContext{
        {QStringLiteral("patient_id"), QStringLiteral("00000000-0000-4000-8000-000000000002")},
        {QStringLiteral("episode_id"), episodeId}
    };
    QJsonObject rejectedContext;
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), invalidEpisodeContext,
                             rejectedContext, &error));
    assert(error == QStringLiteral("PATIENT_NOT_FOUND"));

    assert(!restarted.remove(QStringLiteral("rehabilitation-episodes"), episodeId, 1, &error));
    assert(error == QStringLiteral("REHABILITATION_EPISODE_HAS_REFERENCES"));
    assert(!restarted.remove(QStringLiteral("rehabilitation-sessions"), rehabSessionId, 1, &error));
    assert(error == QStringLiteral("REHABILITATION_SESSION_HAS_REFERENCES"));

    assert(restarted.remove(QStringLiteral("rehabilitation-decisions"),
                            decision.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-observations"),
                            observation.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-sessions"), rehabSessionId, 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-goals"),
                            goal.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-assessments"),
                            assessment.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-episodes"), episodeId, 1, &error));

    assert(restarted.remove(QStringLiteral("patients"), id, 2, &error));
    assert(!restarted.get(QStringLiteral("patients"), id, loaded, &error));
    assert(error == QStringLiteral("RESOURCE_NOT_FOUND"));

    biofeedback::data::JsonResourceRepository missingKey(root, QString(), true);
    assert(!missingKey.isReady());
    assert(missingKey.initializationError() == QStringLiteral("ENCRYPTION_KEY_REQUIRED"));

    std::cout << "json_resource_repository_regression: PASS\n";
    return 0;
}
