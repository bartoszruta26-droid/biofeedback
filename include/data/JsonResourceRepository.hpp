#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QString>

namespace biofeedback::data {

// File-backed repository contract used by the shared data API.
// Collection names are allow-listed and record identifiers are server-generated UUIDs.
class JsonResourceRepository final {
public:
    JsonResourceRepository(QString rootPath, QString encryptionKey = QString(),
                           bool encryptionEnabled = false);

    bool isReady() const noexcept;
    QString initializationError() const;

    QJsonArray list(const QString& collection, QString* error = nullptr) const;
    bool get(const QString& collection, const QString& id, QJsonObject& record,
             QString* error = nullptr) const;
    bool create(const QString& collection, const QJsonObject& payload,
                QJsonObject& created, QString* error = nullptr);
    bool update(const QString& collection, const QString& id, int expectedVersion,
                const QJsonObject& payload, QJsonObject& updated,
                QString* error = nullptr);
    bool remove(const QString& collection, const QString& id, int expectedVersion,
                QString* error = nullptr);

    static bool isSupportedCollection(const QString& collection);

private:
    QString collectionPath(const QString& collection) const;
    QString recordPath(const QString& collection, const QString& id) const;
    bool readRecord(const QString& collection, const QString& id,
                    QJsonObject& record, QString* error) const;
    bool validatePayload(const QString& collection, const QString& idToExclude,
                         const QJsonObject& payload, QString* error) const;
    bool writeRecord(const QString& collection, const QString& id,
                     const QJsonObject& record, QString* error) const;
    static bool validId(const QString& id);
    static void setError(QString* error, const QString& value);

    QString m_rootPath;
    QString m_encryptionKey;
    bool m_encryptionEnabled{false};
    QString m_initializationError;
    mutable QMutex m_mutex;
};

} // namespace biofeedback::data
