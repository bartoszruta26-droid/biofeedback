#include "data/AcquisitionJournal.hpp"
#include "data/Encryption.hpp"

#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>

#include <limits>
#include <stdexcept>

namespace biofeedback::data {
namespace {
constexpr qint64 MaxJournalRecordBytes = 2 * 1024 * 1024;
QString sequenceName(quint64 sequence) {
    return QStringLiteral("%1.bfj").arg(sequence, 20, 10, QLatin1Char('0'));
}
}

AcquisitionJournal::AcquisitionJournal(QString directoryPath, QString encryptionKey)
    : m_directoryPath(QDir::cleanPath(std::move(directoryPath))),
      m_encryptionKey(std::move(encryptionKey)) {
    if (m_directoryPath.isEmpty() || m_directoryPath == QStringLiteral(".")) {
        m_initializationError = QStringLiteral("INVALID_JOURNAL_PATH");
        return;
    }
    if (m_encryptionKey.toUtf8().size() < 32) {
        m_initializationError = QStringLiteral("ENCRYPTION_KEY_REQUIRED");
        return;
    }
    if (!QDir().mkpath(m_directoryPath) ||
        !QFile::setPermissions(m_directoryPath, QFileDevice::ReadOwner |
            QFileDevice::WriteOwner | QFileDevice::ExeOwner)) {
        m_initializationError = QStringLiteral("JOURNAL_DIRECTORY_UNAVAILABLE");
        return;
    }
    QString error;
    if (!recover(&error)) m_initializationError = error;
}

bool AcquisitionJournal::isReady() const noexcept {
    return m_initializationError.isEmpty();
}

QString AcquisitionJournal::initializationError() const {
    return m_initializationError;
}

quint64 AcquisitionJournal::lastSequence() const noexcept {
    QMutexLocker lock(&m_mutex);
    return m_lastSequence;
}

void AcquisitionJournal::setError(QString* error, const QString& value) {
    if (error) *error = value;
}

QString AcquisitionJournal::recordPath(quint64 sequence) const {
    return QDir(m_directoryPath).filePath(sequenceName(sequence));
}

bool AcquisitionJournal::recover(QString* error) {
    QMutexLocker lock(&m_mutex);
    QDir directory(m_directoryPath);
    const QFileInfoList files = directory.entryInfoList(
        QStringList{QStringLiteral("*.bfj")}, QDir::Files | QDir::Readable, QDir::Name);
    quint64 last = 0;
    for (const QFileInfo& info : files) {
        if (info.isSymLink() || info.size() <= 0 || info.size() > MaxJournalRecordBytes) {
            setError(error, QStringLiteral("JOURNAL_RECORD_INVALID"));
            return false;
        }
        const QString base = info.completeBaseName();
        bool sequenceOk = false;
        const quint64 sequence = base.toULongLong(&sequenceOk);
        if (!sequenceOk || sequence == 0 || sequence > static_cast<quint64>(std::numeric_limits<qint64>::max()) || sequenceName(sequence) != info.fileName() ||
            sequence <= last) {
            setError(error, QStringLiteral("JOURNAL_SEQUENCE_INVALID"));
            return false;
        }
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return false;
        }
        const QByteArray encrypted = file.readAll();
        if (encrypted.size() > MaxJournalRecordBytes || file.error() != QFileDevice::NoError) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return false;
        }
        try {
            const std::string plain = Encryption::decrypt(
                encrypted.toStdString(), m_encryptionKey.toStdString());
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(
                QByteArray::fromStdString(plain), &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
                document.object().value(QStringLiteral("sequence")).toVariant().toULongLong() != sequence) {
                setError(error, QStringLiteral("JOURNAL_RECORD_INVALID"));
                return false;
            }
        } catch (...) {
            setError(error, QStringLiteral("JOURNAL_AUTHENTICATION_FAILED"));
            return false;
        }
        last = sequence;
    }
    m_lastSequence = last;
    if (error) error->clear();
    return true;
}

bool AcquisitionJournal::append(quint64 sequence, const QJsonObject& payload, QString* error) {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (sequence == 0 || sequence > static_cast<quint64>(std::numeric_limits<qint64>::max()) ||\n        m_lastSequence == static_cast<quint64>(std::numeric_limits<qint64>::max()) || sequence != m_lastSequence + 1 ||
        payload.value(QStringLiteral("sequence")).toVariant().toULongLong() != sequence) {
        setError(error, QStringLiteral("JOURNAL_SEQUENCE_CONFLICT"));
        return false;
    }
    const QByteArray plain = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    if (plain.isEmpty() || plain.size() > MaxJournalRecordBytes) {
        setError(error, QStringLiteral("JOURNAL_RECORD_TOO_LARGE"));
        return false;
    }
    std::string encrypted;
    try {
        encrypted = Encryption::encrypt(plain.toStdString(), m_encryptionKey.toStdString());
    } catch (...) {
        setError(error, QStringLiteral("JOURNAL_ENCRYPTION_FAILED"));
        return false;
    }
    QSaveFile file(recordPath(sequence));
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(encrypted.data(), static_cast<qint64>(encrypted.size())) !=
            static_cast<qint64>(encrypted.size()) || !file.commit()) {
        file.cancelWriting();
        setError(error, QStringLiteral("JOURNAL_COMMIT_FAILED"));
        return false;
    }
    if (!QFile::setPermissions(recordPath(sequence),
            QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        QFile::remove(recordPath(sequence));
        setError(error, QStringLiteral("JOURNAL_PERMISSION_FAILED"));
        return false;
    }
    m_lastSequence = sequence;
    return true;
}

QVector<AcquisitionJournal::Entry> AcquisitionJournal::readAfter(
    quint64 sequence, QString* error) const {
    QMutexLocker lock(&m_mutex);
    QVector<Entry> entries;
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return entries; }
    QDir directory(m_directoryPath);
    const QFileInfoList files = directory.entryInfoList(
        QStringList{QStringLiteral("*.bfj")}, QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo& info : files) {
        bool sequenceOk = false;
        const quint64 recordSequence = info.completeBaseName().toULongLong(&sequenceOk);
        if (!sequenceOk || recordSequence <= sequence) continue;
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly) || info.isSymLink() ||
            info.size() <= 0 || info.size() > MaxJournalRecordBytes) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return {};
        }
        try {
            const std::string plain = Encryption::decrypt(
                file.readAll().toStdString(), m_encryptionKey.toStdString());
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(
                QByteArray::fromStdString(plain), &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
                document.object().value(QStringLiteral("sequence")).toVariant().toULongLong() != recordSequence) {
                setError(error, QStringLiteral("JOURNAL_RECORD_INVALID"));
                return {};
            }
            entries.append({recordSequence, document.object()});
        } catch (...) {
            setError(error, QStringLiteral("JOURNAL_AUTHENTICATION_FAILED"));
            return {};
        }
    }
    return entries;
}

} // namespace biofeedback::data
