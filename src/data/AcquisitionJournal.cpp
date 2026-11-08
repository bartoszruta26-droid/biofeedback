#include "data/AcquisitionJournal.hpp"
#include "data/Encryption.hpp"

#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>

#include <limits>
#include <utility>

namespace biofeedback::data {
namespace {
constexpr qsizetype MaxBatchSamples = 256;
constexpr qsizetype MaxBatchPlainBytes = 256 * 1024;
constexpr qint64 MaxEncryptedBatchBytes = 2 * 1024 * 1024;

QString batchName(quint64 first, quint64 last) {
    return QStringLiteral("%1-%2.bfj")
        .arg(first, 20, 10, QLatin1Char('0'))
        .arg(last, 20, 10, QLatin1Char('0'));
}

bool decodeBatch(const QByteArray& encrypted, const QString& key,
                 quint64 expectedFirst, quint64 expectedLast,
                 QVector<AcquisitionJournal::Entry>* entries) {
    try {
        const std::string plain = Encryption::decrypt(encrypted.toStdString(), key.toStdString());
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(plain), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) return false;
        const QJsonObject root = doc.object();
        if (root.value(QStringLiteral("first_sequence")).toVariant().toULongLong() != expectedFirst ||
            root.value(QStringLiteral("last_sequence")).toVariant().toULongLong() != expectedLast ||
            !root.value(QStringLiteral("samples")).isArray()) return false;
        const QJsonArray samples = root.value(QStringLiteral("samples")).toArray();
        if (samples.isEmpty() || samples.size() > MaxBatchSamples ||
            expectedLast < expectedFirst ||
            static_cast<quint64>(samples.size()) != expectedLast - expectedFirst + 1) return false;
        for (qsizetype i = 0; i < samples.size(); ++i) {
            if (!samples.at(i).isObject()) return false;
            const QJsonObject payload = samples.at(i).toObject();
            const quint64 sequence = payload.value(QStringLiteral("sequence")).toVariant().toULongLong();
            if (sequence != expectedFirst + static_cast<quint64>(i)) return false;
            entries->append({sequence, payload});
        }
        return true;
    } catch (...) {
        return false;
    }
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

bool AcquisitionJournal::isReady() const noexcept { return m_initializationError.isEmpty(); }
QString AcquisitionJournal::initializationError() const { return m_initializationError; }

quint64 AcquisitionJournal::lastSequence() const noexcept {
    QMutexLocker lock(&m_mutex);
    return m_lastSequence;
}

qsizetype AcquisitionJournal::pendingCount() const noexcept {
    QMutexLocker lock(&m_mutex);
    return m_pending.size();
}

void AcquisitionJournal::setError(QString* error, const QString& value) {
    if (error) *error = value;
}

QString AcquisitionJournal::batchPath(quint64 first, quint64 last) const {
    return QDir(m_directoryPath).filePath(batchName(first, last));
}

bool AcquisitionJournal::recover(QString* error) {
    QMutexLocker lock(&m_mutex);
    const QFileInfoList files = QDir(m_directoryPath).entryInfoList(
        QStringList{QStringLiteral("*.bfj")}, QDir::Files | QDir::Readable, QDir::Name);
    quint64 last = 0;
    for (const QFileInfo& info : files) {
        if (info.isSymLink() || info.size() <= 0 || info.size() > MaxEncryptedBatchBytes) {
            setError(error, QStringLiteral("JOURNAL_BATCH_INVALID"));
            return false;
        }
        const QString base = info.completeBaseName();
        const QStringList range = base.split(QLatin1Char('-'));
        bool firstOk = false, lastOk = false;
        const quint64 first = range.size() == 2 ? range.at(0).toULongLong(&firstOk) : 0;
        const quint64 end = range.size() == 2 ? range.at(1).toULongLong(&lastOk) : 0;
        if (!firstOk || !lastOk || first == 0 || end < first ||
            end > static_cast<quint64>(std::numeric_limits<qint64>::max()) ||
            batchName(first, end) != info.fileName() || first != last + 1) {
            setError(error, QStringLiteral("JOURNAL_SEQUENCE_INVALID"));
            return false;
        }
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return false;
        }
        const QByteArray encrypted = file.readAll();
        if (file.error() != QFileDevice::NoError) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return false;
        }
        QVector<Entry> decoded;
        if (!decodeBatch(encrypted, m_encryptionKey, first, end, &decoded)) {
            setError(error, QStringLiteral("JOURNAL_AUTHENTICATION_OR_FORMAT_FAILED"));
            return false;
        }
        last = end;
    }
    m_lastSequence = last;
    m_persistedSequence = last;
    if (error) error->clear();
    return true;
}

bool AcquisitionJournal::append(quint64 sequence, const QJsonObject& payload, QString* error) {
    QMutexLocker lock(&m_mutex);
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (sequence == 0 || sequence > static_cast<quint64>(std::numeric_limits<qint64>::max()) ||
        m_lastSequence == static_cast<quint64>(std::numeric_limits<qint64>::max()) ||
        sequence != m_lastSequence + 1 ||
        payload.value(QStringLiteral("sequence")).toVariant().toULongLong() != sequence) {
        setError(error, QStringLiteral("JOURNAL_SEQUENCE_CONFLICT"));
        return false;
    }
    const qsizetype payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact).size();
    if (payloadBytes <= 0 || payloadBytes > MaxBatchPlainBytes) {
        setError(error, QStringLiteral("JOURNAL_SAMPLE_TOO_LARGE"));
        return false;
    }
    m_pending.append({sequence, payload});
    m_pendingBytes += payloadBytes;
    m_lastSequence = sequence;
    if (m_pending.size() >= MaxBatchSamples || m_pendingBytes >= MaxBatchPlainBytes) {
        if (!flushLocked(error)) {
            m_pending.removeLast();
            m_pendingBytes -= payloadBytes;
            m_lastSequence = sequence - 1;
            return false;
        }
    }
    return true;
}

bool AcquisitionJournal::flush(QString* error) {
    QMutexLocker lock(&m_mutex);
    return flushLocked(error);
}

bool AcquisitionJournal::flushLocked(QString* error) {
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return false; }
    if (m_pending.isEmpty()) return true;

    const quint64 first = m_pending.front().sequence;
    const quint64 last = m_pending.back().sequence;
    QJsonArray samples;
    for (const Entry& entry : m_pending) samples.append(entry.payload);
    const QJsonObject root{
        {QStringLiteral("format_version"), 1},
        {QStringLiteral("first_sequence"), static_cast<qint64>(first)},
        {QStringLiteral("last_sequence"), static_cast<qint64>(last)},
        {QStringLiteral("samples"), samples}
    };
    const QByteArray plain = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (plain.isEmpty() || plain.size() > MaxBatchPlainBytes + 4096) {
        setError(error, QStringLiteral("JOURNAL_BATCH_TOO_LARGE"));
        return false;
    }
    std::string encrypted;
    try {
        encrypted = Encryption::encrypt(plain.toStdString(), m_encryptionKey.toStdString());
    } catch (...) {
        setError(error, QStringLiteral("JOURNAL_ENCRYPTION_FAILED"));
        return false;
    }
    if (encrypted.size() > static_cast<std::size_t>(MaxEncryptedBatchBytes)) {
        setError(error, QStringLiteral("JOURNAL_BATCH_TOO_LARGE"));
        return false;
    }
    QSaveFile file(batchPath(first, last));
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(encrypted.data(), static_cast<qint64>(encrypted.size())) !=
            static_cast<qint64>(encrypted.size()) || !file.commit()) {
        file.cancelWriting();
        setError(error, QStringLiteral("JOURNAL_COMMIT_FAILED"));
        return false;
    }
    if (!QFile::setPermissions(batchPath(first, last),
            QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
        QFile::remove(batchPath(first, last));
        setError(error, QStringLiteral("JOURNAL_PERMISSION_FAILED"));
        return false;
    }
    m_persistedSequence = last;
    m_pending.clear();
    m_pendingBytes = 0;
    return true;
}

QVector<AcquisitionJournal::Entry> AcquisitionJournal::readAfter(
    quint64 sequence, QString* error) const {
    QMutexLocker lock(&m_mutex);
    QVector<Entry> result;
    if (error) error->clear();
    if (!isReady()) { setError(error, m_initializationError); return result; }
    const QFileInfoList files = QDir(m_directoryPath).entryInfoList(
        QStringList{QStringLiteral("*.bfj")}, QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo& info : files) {
        const QStringList range = info.completeBaseName().split(QLatin1Char('-'));
        bool firstOk = false, lastOk = false;
        const quint64 first = range.size() == 2 ? range.at(0).toULongLong(&firstOk) : 0;
        const quint64 last = range.size() == 2 ? range.at(1).toULongLong(&lastOk) : 0;
        if (!firstOk || !lastOk || info.isSymLink() || info.size() <= 0 ||
            info.size() > MaxEncryptedBatchBytes || batchName(first, last) != info.fileName()) {
            setError(error, QStringLiteral("JOURNAL_BATCH_INVALID"));
            return {};
        }
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            setError(error, QStringLiteral("JOURNAL_READ_FAILED"));
            return {};
        }
        QVector<Entry> decoded;
        if (!decodeBatch(file.readAll(), m_encryptionKey, first, last, &decoded)) {
            setError(error, QStringLiteral("JOURNAL_AUTHENTICATION_OR_FORMAT_FAILED"));
            return {};
        }
        for (const Entry& entry : decoded)
            if (entry.sequence > sequence) result.append(entry);
    }
    for (const Entry& entry : m_pending)
        if (entry.sequence > sequence) result.append(entry);
    return result;
}

} // namespace biofeedback::data
