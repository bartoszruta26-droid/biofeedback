#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrl>

namespace biofeedback::api {

class HttpApiClient final {
public:
    explicit HttpApiClient(QUrl baseUrl = QUrl(QStringLiteral("http://127.0.0.1:8765")),
                           int timeoutMs = 5000);

    bool login(const QString& username, const QString& password, QString* error = nullptr);
    void logout();
    bool isAuthenticated() const noexcept;

    bool listResources(const QString& collection, QJsonArray& items, QString* error = nullptr);
    bool getResource(const QString& collection, const QString& id,
                     QJsonObject& record, QString* error = nullptr);
    bool createResource(const QString& collection, const QJsonObject& payload,
                        QJsonObject& created, QString* error = nullptr);
    bool updateResource(const QString& collection, const QString& id, int expectedVersion,
                        const QJsonObject& payload, QJsonObject& updated, QString* error = nullptr);
    bool deleteResource(const QString& collection, const QString& id, int expectedVersion,
                        QString* error = nullptr);

    bool listDevices(QJsonArray& devices, QString* error = nullptr);
    bool getAcquisitionSession(QJsonObject& session, QString* error = nullptr);
    bool startAcquisitionSession(const QString& sessionId, const QString& patientId,
                                 const QString& deviceId, const QString& protocolId,
                                 QString* error = nullptr);
    bool stopAcquisitionSession(const QString& sessionId, QString* error = nullptr);
    bool listAcquisitionSamples(QJsonArray& samples, QString* error = nullptr);

private:
    bool request(const QByteArray& method, const QString& path, const QJsonObject* body,
                 bool authenticated, QJsonObject& response, QString* error = nullptr);
    static bool supportedCollection(const QString& collection);
    static void setError(QString* error, const QString& value);

    QUrl m_baseUrl;
    int m_timeoutMs{5000};
    QString m_accessToken;
    QNetworkAccessManager m_network;
};

} // namespace biofeedback::api
