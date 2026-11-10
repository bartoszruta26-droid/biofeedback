#include "data/JsonResourceRepository.hpp"
#include "data/Encryption.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>

#include <limits>
#include <cmath>
#include <QDate>
#include <utility>

namespace biofeedback::data {
namespace {
constexpr qint64 MaxRecordBytes = 16 * 1024 * 1024;
const QStringList Collections = {
    QStringLiteral("patients"),
    QStringLiteral("measurements"),
    QStringLiteral("training-sessions"),
    QStringLiteral("training-results"),
    QStringLiteral("game-sessions"),
    QStringLiteral("game-results"),
    QStringLiteral("rehabilitation-episodes"),
    QStringLiteral("rehabilitation-assessments"),
    QStringLiteral("rehabilitation-goals"),
    QStringLiteral("rehabilitation-sessions"),
    QStringLiteral("rehabilitation-observations"),
    QStringLiteral("rehabilitation-decisions")
};
}

JsonResourceRepository::JsonResourceRepository(QString rootPath, QString encryptionKey,
                                               bool encryptionEnabled)
    : m_rootPath(QDir::cleanPath(std::move(rootPath))),
      m_encryptionKey(std::move(encryptionKey)),
      m_encryptionEnabled(encryptionEnabled) {
    if (m_rootPath.isEmpty() || m_rootPath == QStringLiteral(".")) {
        m_initializationError = QStringLiteral("INVALID_STORAGE_PATH");
        return;
    }
    if (m_encryptionEnabled && m_encryptionKey.toUtf8().size() < 32) {
        m_initializationError = QStringLiteral("ENCRYPTION_KEY_REQUIRED");
        return;
    }
    if (!QDir().mkpath(m_rootPath) ||
        !QFile::setPermissions(m_rootPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                           QFileDevice::ExeOwner)) {
        m_initializationError = QStringLiteral("STORAGE_DIRECTORY_UNAVAILABLE");
        return;
    }
}

bool JsonResourceRepository::isReady() const noexcept {
    return m_initializationError.isEmpty();
}

QString JsonResourceRepository::initializationError() const {
    return m_initializationError;
}

bool JsonResourceRepository::isSupportedCollection(const QString& collection) {
    return Collections.contains(collection);
}

bool JsonResourceRepository::validId(const QString& id) {
    const QUuid uuid = QUuid::fromString(id);
    return !uuid.isNull() && uuid.toString(QUuid::WithoutBraces).compare(id, Qt::CaseInsensitive) == 0;
}

QString JsonResourceRepository::collectionPath(const QString& collection) const {
    return QDir(m_rootPath).filePath(collection);
}

QString JsonResourceRepository::recordPath(const QString& collection, const QString& id) const {
    return QDir(collectionPath(collection)).filePath(id.toLower() + QStringLiteral(".json"));
}

void JsonResourceRepository::setError(QString* error, const QString& value) {
    if (error) *error = value;
}

QJsonArray JsonResourceRepository::list(const QString& collection, QString* error) const {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    QJsonArray records;
    if (!isReady()) { setError(error, m_initializationError); return records; }
    if (!isSupportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return records; }

    QDir directory(collectionPath(collection));
    if (!directory.exists()) return records;
    const QFileInfoList files = directory.entryInfoList(QStringList{QStringLiteral("*.json")},
                                                         QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        const QString id = fileInfo.completeBaseName();
        if (!validId(id)) continue;
        QJsonObject record;
        QString readError;
        if (!readRecord(collection, id, record, &readError)) {
            setError(error, readError);
            return QJsonArray();
        }
        records.append(record);
    }
    return records;
}

bool JsonResourceRepository::get(const QString& collection, const QString& id,
                                 QJsonObject& record, QString* error) const {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (!isSupportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    if (!validId(id)) { setError(error, QStringLiteral("INVALID_RESOURCE_ID")); return false; }
    return readRecord(collection, id, record, error);
}

bool JsonResourceRepository::create(const QString& collection, const QJsonObject& payload,
                                    QJsonObject& created, QString* error) {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (!isSupportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    if (payload.isEmpty()) { setError(error, QStringLiteral("EMPTY_RESOURCE")); return false; }
    if (!validatePayload(collection, QString(), payload, error)) return false;

    QJsonObject record = payload;
    record.remove(QStringLiteral("id"));
    record.remove(QStringLiteral("version"));
    record.remove(QStringLiteral("created_at"));
    record.remove(QStringLiteral("updated_at"));
    record.insert(QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces));
    record.insert(QStringLiteral("version"), 1);
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    record.insert(QStringLiteral("created_at"), now);
    record.insert(QStringLiteral("updated_at"), now);
    if (QJsonDocument(record).toJson(QJsonDocument::Compact).size() > MaxRecordBytes) {
        setError(error, QStringLiteral("RESOURCE_TOO_LARGE"));
        return false;
    }
    const QString id = record.value(QStringLiteral("id")).toString();
    if (!writeRecord(collection, id, record, error)) return false;
    created = record;
    return true;
}

bool JsonResourceRepository::update(const QString& collection, const QString& id,
                                    int expectedVersion, const QJsonObject& payload,
                                    QJsonObject& updated, QString* error) {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (!isSupportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    if (!validId(id) || expectedVersion < 1) { setError(error, QStringLiteral("INVALID_UPDATE")); return false; }

    QJsonObject current;
    if (!readRecord(collection, id, current, error)) return false;
    if (!validatePayload(collection, id, payload, error)) return false;
    const int version = current.value(QStringLiteral("version")).toInt(0);
    if (version != expectedVersion) { setError(error, QStringLiteral("VERSION_CONFLICT")); return false; }

    QJsonObject record = payload;
    record.remove(QStringLiteral("id"));
    record.remove(QStringLiteral("version"));
    record.remove(QStringLiteral("created_at"));
    record.remove(QStringLiteral("updated_at"));
    record.insert(QStringLiteral("id"), current.value(QStringLiteral("id")));
    record.insert(QStringLiteral("version"), version + 1);
    record.insert(QStringLiteral("created_at"), current.value(QStringLiteral("created_at")));
    record.insert(QStringLiteral("updated_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (QJsonDocument(record).toJson(QJsonDocument::Compact).size() > MaxRecordBytes) {
        setError(error, QStringLiteral("RESOURCE_TOO_LARGE"));
        return false;
    }
    if (!writeRecord(collection, id, record, error)) return false;
    updated = record;
    return true;
}

bool JsonResourceRepository::remove(const QString& collection, const QString& id,
                                    int expectedVersion, QString* error) {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (!isSupportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    if (!validId(id) || expectedVersion < 1) { setError(error, QStringLiteral("INVALID_DELETE")); return false; }

    QJsonObject current;
    if (!readRecord(collection, id, current, error)) return false;
    if (current.value(QStringLiteral("version")).toInt(0) != expectedVersion) {
        setError(error, QStringLiteral("VERSION_CONFLICT"));
        return false;
    }
    if (collection == QStringLiteral("rehabilitation-episodes") ||
        collection == QStringLiteral("rehabilitation-sessions")) {
        const QStringList dependents = collection == QStringLiteral("rehabilitation-episodes")
            ? QStringList{QStringLiteral("rehabilitation-assessments"),
                          QStringLiteral("rehabilitation-goals"),
                          QStringLiteral("rehabilitation-sessions"),
                          QStringLiteral("rehabilitation-observations"),
                          QStringLiteral("rehabilitation-decisions")}
            : QStringList{QStringLiteral("rehabilitation-observations"),
                          QStringLiteral("rehabilitation-decisions")};
        const QString referenceField = collection == QStringLiteral("rehabilitation-episodes")
            ? QStringLiteral("episode_id") : QStringLiteral("session_id");
        for (const QString& dependentCollection : dependents) {
            const QDir directory(collectionPath(dependentCollection));
            if (!directory.exists()) continue;
            const QFileInfoList files = directory.entryInfoList(
                QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable, QDir::Name);
            for (const QFileInfo& fileInfo : files) {
                const QString dependentId = fileInfo.completeBaseName();
                if (!validId(dependentId)) continue;
                QJsonObject dependent;
                QString readError;
                if (!readRecord(dependentCollection, dependentId, dependent, &readError)) {
                    setError(error, readError);
                    return false;
                }
                if (dependent.value(referenceField).toString().compare(id, Qt::CaseInsensitive) == 0) {
                    setError(error, collection == QStringLiteral("rehabilitation-episodes")
                        ? QStringLiteral("REHABILITATION_EPISODE_HAS_REFERENCES")
                        : QStringLiteral("REHABILITATION_SESSION_HAS_REFERENCES"));
                    return false;
                }
            }
        }
    }
    if (collection == QStringLiteral("training-sessions")) {
        const QDir directory(collectionPath(QStringLiteral("training-results")));
        if (directory.exists()) {
            const QFileInfoList files = directory.entryInfoList(
                QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable, QDir::Name);
            for (const QFileInfo& fileInfo : files) {
                const QString dependentId = fileInfo.completeBaseName();
                if (!validId(dependentId)) continue;
                QJsonObject dependent;
                QString readError;
                if (!readRecord(QStringLiteral("training-results"), dependentId,
                                dependent, &readError)) {
                    setError(error, readError);
                    return false;
                }
                if (dependent.value(QStringLiteral("training_session_id")).toString().compare(
                        id, Qt::CaseInsensitive) == 0) {
                    setError(error, QStringLiteral("TRAINING_SESSION_HAS_RESULTS"));
                    return false;
                }
            }
        }
    }
    if (collection == QStringLiteral("patients")) {
        for (const QString& dependentCollection : {
                 QStringLiteral("measurements"), QStringLiteral("training-sessions"),
                 QStringLiteral("training-results"), QStringLiteral("game-sessions"),
                 QStringLiteral("game-results"), QStringLiteral("rehabilitation-episodes"),
                 QStringLiteral("rehabilitation-assessments"), QStringLiteral("rehabilitation-goals"),
                 QStringLiteral("rehabilitation-sessions"), QStringLiteral("rehabilitation-observations"),
                 QStringLiteral("rehabilitation-decisions")}) {
            const QDir directory(collectionPath(dependentCollection));
            if (!directory.exists()) continue;
            const QFileInfoList files = directory.entryInfoList(
                QStringList{QStringLiteral("*.json")}, QDir::Files | QDir::Readable, QDir::Name);
            for (const QFileInfo& fileInfo : files) {
                const QString dependentId = fileInfo.completeBaseName();
                if (!validId(dependentId)) continue;
                QJsonObject dependent;
                QString readError;
                if (!readRecord(dependentCollection, dependentId, dependent, &readError)) {
                    setError(error, readError);
                    return false;
                }
                if (dependent.value(QStringLiteral("patient_id")).toString().compare(
                        id, Qt::CaseInsensitive) == 0) {
                    setError(error, QStringLiteral("PATIENT_HAS_REFERENCES"));
                    return false;
                }
            }
        }
    }
    if (!QFile::remove(recordPath(collection, id))) {
        setError(error, QStringLiteral("RESOURCE_DELETE_FAILED"));
        return false;
    }
    return true;
}

bool JsonResourceRepository::validatePayload(const QString& collection,
                                                     const QString& idToExclude,
                                                     const QJsonObject& payload,
                                                     QString* error) const {
    if (collection == QStringLiteral("patients")) {
        const QString reference = payload.value(QStringLiteral("pesel")).toString().trimmed();
        const QString firstName = payload.value(QStringLiteral("firstName")).toString().trimmed();
        const QString lastName = payload.value(QStringLiteral("lastName")).toString().trimmed();
        if (reference.isEmpty() || reference.size() > 32 ||
            firstName.isEmpty() || firstName.size() > 100 ||
            lastName.isEmpty() || lastName.size() > 100) {
            setError(error, QStringLiteral("INVALID_PATIENT_RECORD"));
            return false;
        }

        const QDir directory(collectionPath(collection));
        if (!directory.exists()) return true;
        const QFileInfoList files = directory.entryInfoList(QStringList{QStringLiteral("*.json")},
                                                             QDir::Files | QDir::Readable, QDir::Name);
        for (const QFileInfo& fileInfo : files) {
            const QString existingId = fileInfo.completeBaseName();
            if (!validId(existingId) || existingId.compare(idToExclude, Qt::CaseInsensitive) == 0)
                continue;
            QJsonObject existing;
            QString readError;
            if (!readRecord(collection, existingId, existing, &readError)) {
                setError(error, readError);
                return false;
            }
            if (existing.value(QStringLiteral("pesel")).toString().trimmed() == reference) {
                setError(error, QStringLiteral("DUPLICATE_PATIENT_REFERENCE"));
                return false;
            }
        }
        return true;
    }

    const QString patientId = payload.value(QStringLiteral("patient_id")).toString().trimmed();
    if (!patientId.isEmpty()) {
        if (!validId(patientId)) {
            setError(error, QStringLiteral("INVALID_PATIENT_REFERENCE"));
            return false;
        }
        QJsonObject patient;
        QString patientError;
        if (!readRecord(QStringLiteral("patients"), patientId, patient, &patientError)) {
            setError(error, patientError == QStringLiteral("RESOURCE_NOT_FOUND")
                ? QStringLiteral("PATIENT_NOT_FOUND") : patientError);
            return false;
        }
    }

    const QStringList rehabilitationCollections = {
        QStringLiteral("rehabilitation-episodes"),
        QStringLiteral("rehabilitation-assessments"),
        QStringLiteral("rehabilitation-goals"),
        QStringLiteral("rehabilitation-sessions"),
        QStringLiteral("rehabilitation-observations"),
        QStringLiteral("rehabilitation-decisions")
    };
    if (rehabilitationCollections.contains(collection)) {
        if (patientId.isEmpty()) {
            setError(error, QStringLiteral("REHABILITATION_PATIENT_REQUIRED"));
            return false;
        }
        const bool isEpisode = collection == QStringLiteral("rehabilitation-episodes");
        const QString episodeId = payload.value(QStringLiteral("episode_id")).toString().trimmed();
        if (!isEpisode) {
            if (!validId(episodeId)) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_EPISODE_REFERENCE"));
                return false;
            }
            QJsonObject episode;
            QString episodeError;
            if (!readRecord(QStringLiteral("rehabilitation-episodes"), episodeId, episode, &episodeError)) {
                setError(error, episodeError == QStringLiteral("RESOURCE_NOT_FOUND")
                    ? QStringLiteral("REHABILITATION_EPISODE_NOT_FOUND") : episodeError);
                return false;
            }
            if (episode.value(QStringLiteral("patient_id")).toString().compare(
                    patientId, Qt::CaseInsensitive) != 0) {
                setError(error, QStringLiteral("REHABILITATION_EPISODE_PATIENT_MISMATCH"));
                return false;
            }
        }
        const bool needsSession = collection == QStringLiteral("rehabilitation-observations") ||
                                  collection == QStringLiteral("rehabilitation-decisions");
        if (needsSession) {
            const QString sessionId = payload.value(QStringLiteral("session_id")).toString().trimmed();
            if (!validId(sessionId)) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_SESSION_REFERENCE"));
                return false;
            }
            QJsonObject session;
            QString sessionError;
            if (!readRecord(QStringLiteral("rehabilitation-sessions"), sessionId, session, &sessionError)) {
                setError(error, sessionError == QStringLiteral("RESOURCE_NOT_FOUND")
                    ? QStringLiteral("REHABILITATION_SESSION_NOT_FOUND") : sessionError);
                return false;
            }
            if (session.value(QStringLiteral("patient_id")).toString().compare(
                    patientId, Qt::CaseInsensitive) != 0 ||
                session.value(QStringLiteral("episode_id")).toString().compare(
                    episodeId, Qt::CaseInsensitive) != 0) {
                setError(error, QStringLiteral("REHABILITATION_SESSION_CONTEXT_MISMATCH"));
                return false;
            }
        }
    }

    if (collection == QStringLiteral("rehabilitation-sessions")) {
        const QStringList painFields = {
            QStringLiteral("pre_session_pain_nrs"),
            QStringLiteral("post_session_pain_nrs"),
            QStringLiteral("pain_at_24h_nrs")
        };
        for (const QString& field : painFields) {
            if (!payload.contains(field)) continue;
            const QJsonValue value = payload.value(field);
            if (!value.isDouble() || !std::isfinite(value.toDouble()) ||
                value.toDouble() < 0.0 || value.toDouble() > 10.0 ||
                std::floor(value.toDouble()) != value.toDouble()) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_SESSION_PAIN_NRS"));
                return false;
            }
        }
        const QString status = payload.value(QStringLiteral("session_status")).toString();
        if (!status.isEmpty() &&
            status != QStringLiteral("active") &&
            status != QStringLiteral("awaiting_24h") &&
            status != QStringLiteral("completed") &&
            status != QStringLiteral("paused")) {
            setError(error, QStringLiteral("INVALID_REHABILITATION_SESSION_STATUS"));
            return false;
        }
    }


    if (collection == QStringLiteral("rehabilitation-observations")) {
        const QString exercise = payload.value(QStringLiteral("exercise_name")).toString().trimmed();
        if (exercise.isEmpty() || exercise.size() > 200) {
            setError(error, QStringLiteral("INVALID_REHABILITATION_OBSERVATION_EXERCISE"));
            return false;
        }
        const QStringList requiredIntegerFields = {
            QStringLiteral("completed_sets"), QStringLiteral("completed_repetitions"),
            QStringLiteral("pain_before"), QStringLiteral("pain_after")
        };
        for (const QString& field : requiredIntegerFields) {
            const QJsonValue value = payload.value(field);
            if (!value.isDouble() || !std::isfinite(value.toDouble()) ||
                std::floor(value.toDouble()) != value.toDouble()) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_OBSERVATION_VALUE"));
                return false;
            }
            const double number = value.toDouble();
            const double maximum = field == QStringLiteral("completed_sets") ? 100.0
                : field == QStringLiteral("completed_repetitions") ? 10000.0 : 10.0;
            if (number < 0.0 || number > maximum) {
                setError(error, QStringLiteral("REHABILITATION_OBSERVATION_VALUE_OUT_OF_RANGE"));
                return false;
            }
        }
        const QStringList optionalIntegerFields = {
            QStringLiteral("duration_seconds"), QStringLiteral("rpe"),
            QStringLiteral("pain_at_24h")
        };
        for (const QString& field : optionalIntegerFields) {
            if (!payload.contains(field)) continue;
            const QJsonValue value = payload.value(field);
            if (!value.isDouble() || !std::isfinite(value.toDouble()) ||
                std::floor(value.toDouble()) != value.toDouble()) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_OBSERVATION_VALUE"));
                return false;
            }
            const double number = value.toDouble();
            const double minimum = field == QStringLiteral("duration_seconds") ? 1.0 : 0.0;
            const double maximum = field == QStringLiteral("duration_seconds") ? 86400.0 : 10.0;
            if (number < minimum || number > maximum) {
                setError(error, QStringLiteral("REHABILITATION_OBSERVATION_VALUE_OUT_OF_RANGE"));
                return false;
            }
        }
        const QString quality = payload.value(QStringLiteral("movement_quality")).toString();
        if (!quality.isEmpty() && quality != QStringLiteral("not_assessed") &&
            quality != QStringLiteral("acceptable") && quality != QStringLiteral("unacceptable")) {
            setError(error, QStringLiteral("INVALID_REHABILITATION_MOVEMENT_QUALITY"));
            return false;
        }
    }

    if (collection == QStringLiteral("rehabilitation-assessments")) {
        const QString icfReference = payload.value(QStringLiteral("icf_assessment_ref")).toString().trimmed();
        if (payload.contains(QStringLiteral("icf_assessment_ref"))) {
            if (icfReference.isEmpty()) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_ICF_REFERENCE"));
                return false;
            }
            QJsonObject patient;
            QString patientError;
            if (!readRecord(QStringLiteral("patients"), patientId, patient, &patientError)) {
                setError(error, patientError);
                return false;
            }
            bool referenceFound = patient.value(QStringLiteral("icf_assessment"))
                .toObject().value(QStringLiteral("assessed_at")).toString() == icfReference;
            const QJsonArray icfHistory = patient.value(QStringLiteral("icf_assessment_history")).toArray();
            for (const QJsonValue& entry : icfHistory) {
                if (entry.toObject().value(QStringLiteral("assessed_at")).toString() == icfReference) {
                    referenceFound = true;
                    break;
                }
            }
            if (!referenceFound) {
                setError(error, QStringLiteral("REHABILITATION_ICF_ASSESSMENT_NOT_FOUND"));
                return false;
            }
        }
        const QStringList numericFields = {
            QStringLiteral("pain_nrs"), QStringLiteral("rom_degrees"),
            QStringLiteral("strength_mrc"), QStringLiteral("swelling_cm")
        };
        bool hasClinicalValue = false;
        for (const QString& field : numericFields) {
            if (!payload.contains(field)) continue;
            hasClinicalValue = true;
            const QJsonValue value = payload.value(field);
            if (!value.isDouble() || !std::isfinite(value.toDouble())) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_ASSESSMENT_VALUE"));
                return false;
            }
            const double number = value.toDouble();
            double minimum = 0.0;
            double maximum = 0.0;
            bool integerOnly = false;
            if (field == QStringLiteral("pain_nrs")) {
                maximum = 10.0;
                integerOnly = true;
            } else if (field == QStringLiteral("rom_degrees")) {
                maximum = 360.0;
            } else if (field == QStringLiteral("strength_mrc")) {
                maximum = 5.0;
                integerOnly = true;
            } else {
                maximum = 100.0;
            }
            if (number < minimum || number > maximum ||
                (integerOnly && std::floor(number) != number)) {
                setError(error, QStringLiteral("REHABILITATION_ASSESSMENT_VALUE_OUT_OF_RANGE"));
                return false;
            }
        }
        const QJsonValue functionalTest = payload.value(QStringLiteral("functional_test"));
        if (!functionalTest.isUndefined()) {
            if (!functionalTest.isString() || functionalTest.toString().trimmed().isEmpty() ||
                functionalTest.toString().size() > 500) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_FUNCTIONAL_TEST"));
                return false;
            }
            hasClinicalValue = true;
        }
        if (!hasClinicalValue) {
            setError(error, QStringLiteral("REHABILITATION_ASSESSMENT_VALUE_REQUIRED"));
            return false;
        }
    }

    if (collection == QStringLiteral("rehabilitation-goals")) {
        const QString title = payload.value(QStringLiteral("title")).toString().trimmed();
        const QString metric = payload.value(QStringLiteral("metric_id")).toString().trimmed();
        const QString unit = payload.value(QStringLiteral("unit")).toString().trimmed();
        const QJsonValue baselineValue = payload.value(QStringLiteral("baseline_value"));
        const QJsonValue targetValue = payload.value(QStringLiteral("target_value"));
        if (title.isEmpty() || title.size() > 200 || metric.isEmpty() || metric.size() > 120 ||
            unit.isEmpty() || unit.size() > 32 ||
            !baselineValue.isDouble() || !targetValue.isDouble() ||
            !std::isfinite(baselineValue.toDouble()) || !std::isfinite(targetValue.toDouble())) {
            setError(error, QStringLiteral("INVALID_REHABILITATION_GOAL"));
            return false;
        }
        const QString targetDate = payload.value(QStringLiteral("target_date")).toString().trimmed();
        if (!targetDate.isEmpty()) {
            const QDate parsedDate = QDate::fromString(targetDate, Qt::ISODate);
            if (!parsedDate.isValid() || parsedDate.toString(Qt::ISODate) != targetDate) {
                setError(error, QStringLiteral("INVALID_REHABILITATION_GOAL_DATE"));
                return false;
            }
        }
    }

    if (collection == QStringLiteral("training-results")) {
        const QString trainingSessionId =
            payload.value(QStringLiteral("training_session_id")).toString().trimmed();
        if (!trainingSessionId.isEmpty()) {
            if (!validId(trainingSessionId)) {
                setError(error, QStringLiteral("INVALID_TRAINING_SESSION_REFERENCE"));
                return false;
            }
            QJsonObject trainingSession;
            QString sessionError;
            if (!readRecord(QStringLiteral("training-sessions"), trainingSessionId,
                            trainingSession, &sessionError)) {
                setError(error, sessionError == QStringLiteral("RESOURCE_NOT_FOUND")
                    ? QStringLiteral("TRAINING_SESSION_NOT_FOUND") : sessionError);
                return false;
            }
            const QString sessionPatientId =
                trainingSession.value(QStringLiteral("patient_id")).toString();
            if (!patientId.isEmpty() && !sessionPatientId.isEmpty() &&
                patientId.compare(sessionPatientId, Qt::CaseInsensitive) != 0) {
                setError(error, QStringLiteral("PATIENT_TRAINING_SESSION_MISMATCH"));
                return false;
            }
        }
    }
    return true;
}

bool JsonResourceRepository::readRecord(const QString& collection, const QString& id,
                                        QJsonObject& record, QString* error) const {
    QFile file(recordPath(collection, id));
    if (!file.exists()) { setError(error, QStringLiteral("RESOURCE_NOT_FOUND")); return false; }
    if (!file.open(QIODevice::ReadOnly)) { setError(error, QStringLiteral("RESOURCE_READ_FAILED")); return false; }
    QByteArray bytes = file.read(MaxRecordBytes + 1);
    if (file.error() != QFileDevice::NoError || bytes.size() > MaxRecordBytes) {
        setError(error, QStringLiteral("RESOURCE_READ_FAILED"));
        return false;
    }
    std::string jsonText(bytes.constData(), static_cast<size_t>(bytes.size()));
    if (jsonText.rfind("BFENC:v1:", 0) == 0) {
        if (!m_encryptionEnabled) { setError(error, QStringLiteral("ENCRYPTION_POLICY_MISMATCH")); return false; }
        try { jsonText = Encryption::decrypt(jsonText, m_encryptionKey.toStdString()); }
        catch (...) { setError(error, QStringLiteral("RESOURCE_DECRYPT_FAILED")); return false; }
    } else if (m_encryptionEnabled) {
        setError(error, QStringLiteral("UNENCRYPTED_RESOURCE_REJECTED"));
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(jsonText), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(error, QStringLiteral("RESOURCE_INVALID_JSON"));
        return false;
    }
    record = document.object();
    if (record.value(QStringLiteral("id")).toString().compare(id, Qt::CaseInsensitive) != 0 ||
        record.value(QStringLiteral("version")).toInt(0) < 1) {
        setError(error, QStringLiteral("RESOURCE_INTEGRITY_ERROR"));
        return false;
    }
    return true;
}

bool JsonResourceRepository::writeRecord(const QString& collection, const QString& id,
                                         const QJsonObject& record, QString* error) const {
    if (!QDir().mkpath(collectionPath(collection)) ||
        !QFile::setPermissions(collectionPath(collection),
            QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner)) {
        setError(error, QStringLiteral("STORAGE_DIRECTORY_UNAVAILABLE"));
        return false;
    }
    const QByteArray plain = QJsonDocument(record).toJson(QJsonDocument::Compact);
    if (plain.size() > MaxRecordBytes) { setError(error, QStringLiteral("RESOURCE_TOO_LARGE")); return false; }

    QByteArray stored = plain;
    if (m_encryptionEnabled) {
        try {
            const std::string encrypted = Encryption::encrypt(
                std::string(plain.constData(), static_cast<size_t>(plain.size())),
                m_encryptionKey.toStdString());
            stored = QByteArray::fromStdString(encrypted);
        } catch (...) {
            setError(error, QStringLiteral("RESOURCE_ENCRYPT_FAILED"));
            return false;
        }
    }

    QSaveFile file(recordPath(collection, id));
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(stored) != stored.size() || !file.commit()) {
        file.cancelWriting();
        setError(error, QStringLiteral("RESOURCE_WRITE_FAILED"));
        return false;
    }
    if (!QFile::setPermissions(recordPath(collection, id),
            QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        QFile::remove(recordPath(collection, id));
        setError(error, QStringLiteral("RESOURCE_PERMISSION_FAILED"));
        return false;
    }
    return true;
}

} // namespace biofeedback::data
