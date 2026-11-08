#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace biofeedback::data {

// One authenticated, encrypted file per accepted sample. QSaveFile makes each
// record visible atomically; a malformed or unauthenticated record fails closed.
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
    bool append(quint64 sequence, const QJsonObject& payload, QString* error = nullptr);
    QVector<Entry> readAfter(quint64 sequence, QString* error = nullptr) const;

private:
    bool recover(QString* error);
    QString recordPath(quint64 sequence) const;
    static void setError(QString* error, const QString& value);

    QString m_directoryPath;
    QString m_encryptionKey;
    QString m_initializationError;
    quint64 m_lastSequence{0};
    mutable QMutex m_mutex;
};

} // namespace biofeedback::data
