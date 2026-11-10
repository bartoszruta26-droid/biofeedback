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

    const QString currentIcfTimestamp = QStringLiteral("2026-11-10T10:00:00.000Z");
    const QString historicalIcfTimestamp = QStringLiteral("2026-10-10T10:00:00.000Z");
    const QString currentIcfId = QStringLiteral("icf-current-version-001");
    const QString historicalIcfId = QStringLiteral("icf-historical-version-001");
    const QJsonObject currentIcfAssessment{
        {QStringLiteral("id"), currentIcfId},
        {QStringLiteral("schema"), QStringLiteral("icf-functional-screening-v1")},
        {QStringLiteral("assessed_at"), currentIcfTimestamp},
        {QStringLiteral("responses"), QJsonObject{
            {QStringLiteral("b280"), QJsonObject{{QStringLiteral("qualifier"), 2}}}
        }}
    };
    const QJsonObject historicalIcfAssessment{
        {QStringLiteral("id"), historicalIcfId},
        {QStringLiteral("schema"), QStringLiteral("icf-functional-screening-v1")},
        {QStringLiteral("assessed_at"), historicalIcfTimestamp},
        {QStringLiteral("responses"), QJsonObject{
            {QStringLiteral("d450"), QJsonObject{{QStringLiteral("performance"), 1},
                                                  {QStringLiteral("capacity"), 2}}}
        }}
    };
    QJsonObject updatePayload{
        {QStringLiteral("firstName"), QStringLiteral("Updated")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")},
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-001")},
        {QStringLiteral("icf_assessment"), currentIcfAssessment},
        {QStringLiteral("icf_assessment_history"), QJsonArray{historicalIcfAssessment,
                                                               currentIcfAssessment}}
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
        {QStringLiteral("assessment_type"), QStringLiteral("baseline")},
        {QStringLiteral("icf_assessment_ref"), currentIcfId},
        {QStringLiteral("pain_nrs"), 0},
        {QStringLiteral("rom_degrees"), 0}
    };
    QJsonObject assessment;
    assert(restarted.create(QStringLiteral("rehabilitation-assessments"), assessmentPayload, assessment, &error));
    assert(assessment.value(QStringLiteral("pain_nrs")).toInt(-1) == 0);
    assert(assessment.value(QStringLiteral("rom_degrees")).toInt(-1) == 0);
    assert(assessment.value(QStringLiteral("icf_assessment_ref")).toString() == currentIcfId);

    QJsonObject historicalAssessmentPayload = assessmentPayload;
    historicalAssessmentPayload.insert(QStringLiteral("icf_assessment_ref"), historicalIcfId);
    QJsonObject historicalLinkedAssessment;
    assert(restarted.create(QStringLiteral("rehabilitation-assessments"), historicalAssessmentPayload,
                            historicalLinkedAssessment, &error));
    QJsonObject invalidIcfReferencePayload = assessmentPayload;
    invalidIcfReferencePayload.insert(QStringLiteral("icf_assessment_ref"),
                                      QStringLiteral("icf-nonexistent-version-999"));
    QJsonObject rejectedIcfReference;
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), invalidIcfReferencePayload,
                             rejectedIcfReference, &error));
    assert(error == QStringLiteral("REHABILITATION_ICF_ASSESSMENT_NOT_FOUND"));
    // Timestamp collisions must not make a different assessment version resolve.
    QJsonObject collidingTimestamp = historicalIcfAssessment;
    collidingTimestamp.insert(QStringLiteral("assessed_at"), currentIcfTimestamp);
    QJsonObject collisionPatient = updated;
    collisionPatient.insert(QStringLiteral("icf_assessment_history"),
        QJsonArray{collidingTimestamp, historicalIcfAssessment, currentIcfAssessment});
    QJsonObject collisionUpdate;
    assert(restarted.update(QStringLiteral("patients"), id, 2, collisionPatient,
                             collisionUpdate, &error));
    QJsonObject wrongVersionPayload = assessmentPayload;
    wrongVersionPayload.insert(QStringLiteral("icf_assessment_ref"),
                               QStringLiteral("icf-wrong-version-same-timestamp"));
    QJsonObject rejectedWrongVersion;
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), wrongVersionPayload,
                             rejectedWrongVersion, &error));
    assert(error == QStringLiteral("REHABILITATION_ICF_ASSESSMENT_NOT_FOUND"));

    QJsonObject invalidAssessmentType = assessmentPayload;
    invalidAssessmentType.insert(QStringLiteral("assessment_type"), QStringLiteral("unknown"));
    QJsonObject rejectedAssessmentType;
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), invalidAssessmentType,
                             rejectedAssessmentType, &error));
    assert(error == QStringLiteral("INVALID_REHABILITATION_ASSESSMENT_TYPE"));

    QJsonObject invalidAssessment = assessmentPayload;
    invalidAssessment.insert(QStringLiteral("pain_nrs"), 11);
    QJsonObject rejectedAssessment;
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), invalidAssessment,
                             rejectedAssessment, &error));
    assert(error == QStringLiteral("REHABILITATION_ASSESSMENT_VALUE_OUT_OF_RANGE"));
    QJsonObject emptyAssessment{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("assessment_type"), QStringLiteral("baseline")}
    };
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), emptyAssessment,
                             rejectedAssessment, &error));
    assert(error == QStringLiteral("REHABILITATION_ASSESSMENT_VALUE_REQUIRED"));
    QJsonObject fractionalStrength = assessmentPayload;
    fractionalStrength.insert(QStringLiteral("strength_mrc"), 3.5);
    assert(!restarted.create(QStringLiteral("rehabilitation-assessments"), fractionalStrength,
                             rejectedAssessment, &error));
    assert(error == QStringLiteral("REHABILITATION_ASSESSMENT_VALUE_OUT_OF_RANGE"));

    QJsonObject goalPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("title"), QStringLiteral("Poprawa zgięcia kolana")},
        {QStringLiteral("metric_id"), QStringLiteral("knee_flexion_rom")},
        {QStringLiteral("baseline_value"), 75.0},
        {QStringLiteral("target_value"), 120.0},
        {QStringLiteral("unit"), QStringLiteral("degrees")},
        {QStringLiteral("target_date"), QStringLiteral("2026-12-01")}
    };
    QJsonObject goal;
    assert(restarted.create(QStringLiteral("rehabilitation-goals"), goalPayload, goal, &error));
    assert(goal.value(QStringLiteral("baseline_value")).toDouble() == 75.0);
    QJsonObject invalidGoal = goalPayload;
    invalidGoal.insert(QStringLiteral("target_date"), QStringLiteral("2026-02-30"));
    QJsonObject rejectedGoal;
    assert(!restarted.create(QStringLiteral("rehabilitation-goals"), invalidGoal, rejectedGoal, &error));
    assert(error == QStringLiteral("INVALID_REHABILITATION_GOAL_DATE"));
    invalidGoal = goalPayload;
    invalidGoal.remove(QStringLiteral("title"));
    assert(!restarted.create(QStringLiteral("rehabilitation-goals"), invalidGoal, rejectedGoal, &error));
    assert(error == QStringLiteral("INVALID_REHABILITATION_GOAL"));

    QJsonObject rehabSessionPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("session_type"), QStringLiteral("treatment")},
        {QStringLiteral("session_status"), QStringLiteral("active")},
        {QStringLiteral("pre_session_pain_nrs"), 0}
    };
    QJsonObject rehabSession;
    assert(restarted.create(QStringLiteral("rehabilitation-sessions"), rehabSessionPayload,
                            rehabSession, &error));
    const QString rehabSessionId = rehabSession.value(QStringLiteral("id")).toString();
    assert(rehabSession.value(QStringLiteral("pre_session_pain_nrs")).toInt(-1) == 0);
    rehabSessionPayload.insert(QStringLiteral("post_session_pain_nrs"), 3);
    rehabSessionPayload.insert(QStringLiteral("session_status"), QStringLiteral("awaiting_24h"));
    QJsonObject updatedRehabSession;
    assert(restarted.update(QStringLiteral("rehabilitation-sessions"), rehabSessionId,
                            rehabSession.value(QStringLiteral("version")).toInt(),
                            rehabSessionPayload, updatedRehabSession, &error));
    assert(updatedRehabSession.value(QStringLiteral("post_session_pain_nrs")).toInt(-1) == 3);
    updatedRehabSession.insert(QStringLiteral("pain_at_24h_nrs"), 2);
    updatedRehabSession.insert(QStringLiteral("session_status"), QStringLiteral("completed"));
    QJsonObject completedRehabSession;
    assert(restarted.update(QStringLiteral("rehabilitation-sessions"), rehabSessionId,
                            updatedRehabSession.value(QStringLiteral("version")).toInt(),
                            updatedRehabSession, completedRehabSession, &error));
    assert(completedRehabSession.value(QStringLiteral("pain_at_24h_nrs")).toInt(-1) == 2);
    QJsonObject invalidPainSession = completedRehabSession;
    invalidPainSession.insert(QStringLiteral("pain_at_24h_nrs"), 11);
    QJsonObject rejectedSession;
    assert(!restarted.update(QStringLiteral("rehabilitation-sessions"), rehabSessionId,
                             completedRehabSession.value(QStringLiteral("version")).toInt(),
                             invalidPainSession, rejectedSession, &error));
    assert(error == QStringLiteral("INVALID_REHABILITATION_SESSION_PAIN_NRS"));

    QJsonObject observationPayload{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("episode_id"), episodeId},
        {QStringLiteral("session_id"), rehabSessionId},
        {QStringLiteral("exercise_name"), QStringLiteral("Zgięcie kolana w odciążeniu")},
        {QStringLiteral("completed_sets"), 3},
        {QStringLiteral("completed_repetitions"), 10},
        {QStringLiteral("pain_before"), 4},
        {QStringLiteral("pain_after"), 2},
        {QStringLiteral("movement_quality"), QStringLiteral("acceptable")}
    };
    QJsonObject observation;
    assert(restarted.create(QStringLiteral("rehabilitation-observations"), observationPayload,
                            observation, &error));
    QJsonObject invalidObservation = observationPayload;
    invalidObservation.insert(QStringLiteral("pain_after"), 11);
    QJsonObject rejectedObservation;
    assert(!restarted.create(QStringLiteral("rehabilitation-observations"), invalidObservation,
                             rejectedObservation, &error));
    assert(error == QStringLiteral("REHABILITATION_OBSERVATION_VALUE_OUT_OF_RANGE"));
    invalidObservation = observationPayload;
    invalidObservation.insert(QStringLiteral("completed_sets"), 1.5);
    assert(!restarted.create(QStringLiteral("rehabilitation-observations"), invalidObservation,
                             rejectedObservation, &error));
    assert(error == QStringLiteral("INVALID_REHABILITATION_OBSERVATION_VALUE"));

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
    assert(!restarted.remove(QStringLiteral("rehabilitation-sessions"), rehabSessionId,
                            completedRehabSession.value(QStringLiteral("version")).toInt(), &error));
    assert(error == QStringLiteral("REHABILITATION_SESSION_HAS_REFERENCES"));

    assert(restarted.remove(QStringLiteral("rehabilitation-decisions"),
                            decision.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-observations"),
                            observation.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-sessions"), rehabSessionId,
                            completedRehabSession.value(QStringLiteral("version")).toInt(), &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-goals"),
                            goal.value(QStringLiteral("id")).toString(), 1, &error));
    assert(restarted.remove(QStringLiteral("rehabilitation-assessments"),
                            historicalLinkedAssessment.value(QStringLiteral("id")).toString(), 1, &error));
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
