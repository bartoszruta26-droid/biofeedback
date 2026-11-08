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
    QStringLiteral("game-results")
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
    if (collection == QStringLiteral("patients")) {
        for (const QString& dependentCollection : {
                 QStringLiteral("measurements"), QStringLiteral("training-sessions"),
                 QStringLiteral("training-results"), QStringLiteral("game-sessions"),
                 QStringLiteral("game-results")}) {
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
