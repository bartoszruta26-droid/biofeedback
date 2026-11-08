#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace biofeedback::data {

// Encrypted batch journal. A file contains up to 256 ordered samples;
// each batch is atomically replaced as one authenticated AES-256-GCM blob.
class AcquisitionJournal final {
public:
    struct Entry {
        quint64 sequence{0};
        QJsonObject payload;
    };

    AcquisitionJournal(QString directoryPath, QString encryptionKey);
    bool isReady() const noexcept;
    QString initializationError() const;
    quint64 lastSequence() const noexcept;
    qsizetype pendingCount() const noexcept;

    // Accepts a sample into the bounded batch buffer. Threshold-triggered
    // flushes are synchronous; callers must also schedule flush() periodically.
    bool append(quint64 sequence, const QJsonObject& payload, QString* error = nullptr);
    bool flush(QString* error = nullptr);
    QVector<Entry> readAfter(quint64 sequence, QString* error = nullptr) const;

private:
    bool recover(QString* error);
    bool flushLocked(QString* error);
    QString batchPath(quint64 firstSequence, quint64 lastSequence) const;
    static void setError(QString* error, const QString& value);

    QString m_directoryPath;
    QString m_encryptionKey;
    QString m_initializationError;
    quint64 m_lastSequence{0};
    quint64 m_persistedSequence{0};
    qsizetype m_pendingBytes{0};
    QVector<Entry> m_pending;
    mutable QMutex m_mutex;
};

} // namespace biofeedback::data
