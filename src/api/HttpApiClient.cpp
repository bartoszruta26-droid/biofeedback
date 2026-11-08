#include "api/HttpApiClient.hpp"

#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include <stdexcept>
#include <utility>

namespace biofeedback::api {
namespace {
const QStringList Collections = {
    QStringLiteral("patients"), QStringLiteral("measurements"),
    QStringLiteral("training-sessions"), QStringLiteral("training-results"),
    QStringLiteral("game-sessions"), QStringLiteral("game-results")
};
}

HttpApiClient::HttpApiClient(QUrl baseUrl, int timeoutMs)
    : m_baseUrl(std::move(baseUrl)), m_timeoutMs(timeoutMs) {
    m_baseUrl.setPath(QString());
    m_baseUrl.setQuery(QString());
    m_baseUrl.setFragment(QString());
    const QString scheme = m_baseUrl.scheme().toLower();
    const QString host = m_baseUrl.host().toLower();
    const bool localHttp = scheme == QStringLiteral("http") &&
        (host == QStringLiteral("localhost") || host == QStringLiteral("127.0.0.1") ||
         host == QStringLiteral("::1"));
    if ((scheme != QStringLiteral("https") && !localHttp) ||
        !m_baseUrl.isValid() || host.isEmpty() || !m_baseUrl.userInfo().isEmpty() ||
        !m_baseUrl.query().isEmpty() || !m_baseUrl.fragment().isEmpty() ||
        timeoutMs < 100 || timeoutMs > 60000) {
        throw std::invalid_argument("API URL must use HTTPS, or HTTP on loopback, with a valid timeout.");
    }
}

bool HttpApiClient::isAuthenticated() const noexcept {
    return !m_accessToken.isEmpty();
}

void HttpApiClient::setError(QString* error, const QString& value) {
    if (error) *error = value;
}

bool HttpApiClient::supportedCollection(const QString& collection) {
    return Collections.contains(collection);
}

bool HttpApiClient::request(const QByteArray& method, const QString& path,
                            const QJsonObject* body, bool authenticated,
                            QJsonObject& response, QString* error) {
    if (authenticated && m_accessToken.isEmpty()) {
        setError(error, QStringLiteral("AUTHENTICATION_REQUIRED"));
        return false;
    }

    QNetworkRequest request(m_baseUrl.resolved(QUrl(path)));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Cache-Control", "no-store");
    if (authenticated)
        request.setRawHeader("Authorization", QByteArray("Bearer ") + m_accessToken.toLatin1());
    QByteArray payload;
    if (body) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        payload = QJsonDocument(*body).toJson(QJsonDocument::Compact);
    }

    QNetworkReply* reply = nullptr;
    if (method == "GET") reply = m_network.get(request);
    else if (method == "POST") reply = m_network.post(request, payload);
    else if (method == "PUT") reply = m_network.put(request, payload);
    else if (method == "DELETE") reply = m_network.sendCustomRequest(request, method, payload);
    else {
        setError(error, QStringLiteral("METHOD_NOT_SUPPORTED"));
        return false;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    bool timedOut = false;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        timedOut = true;
        reply->abort();
        loop.quit();
    });
    timer.start(m_timeoutMs);
    loop.exec();
    timer.stop();

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray responseBytes = reply->readAll();
    const QString networkError = reply->errorString();
    reply->deleteLater();

    if (timedOut) {
        setError(error, QStringLiteral("API_TIMEOUT"));
        return false;
    }
    if (status == 401 && authenticated) m_accessToken.clear();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        (!document.isObject() && !document.isArray())) {
        setError(error, status == 0 ? networkError : QStringLiteral("INVALID_API_RESPONSE"));
        return false;
    }
    if (status < 200 || status >= 300) {
        const QJsonObject object = document.isObject() ? document.object() : QJsonObject();
        setError(error, object.value(QStringLiteral("code")).toString(
            QStringLiteral("HTTP_%1").arg(status)) + QStringLiteral(": ") +
            object.value(QStringLiteral("message")).toString(networkError));
        return false;
    }
    if (!document.isObject()) {
        setError(error, QStringLiteral("INVALID_API_OBJECT_RESPONSE"));
        return false;
    }
    response = document.object();
    return true;
}

bool HttpApiClient::login(const QString& username, const QString& password, QString* error) {
    m_accessToken.clear();
    QJsonObject response;
    const QJsonObject credentials{
        {QStringLiteral("username"), username},
        {QStringLiteral("password"), password}
    };
    if (!request("POST", QStringLiteral("/api/v1/auth/login"), &credentials, false, response, error))
        return false;
    const QString token = response.value(QStringLiteral("access_token")).toString();
    if (token.size() != 64) {
        setError(error, QStringLiteral("INVALID_ACCESS_TOKEN"));
        return false;
    }
    for (const QChar ch : token) {
        if (!ch.isDigit() && !(ch >= QLatin1Char('a') && ch <= QLatin1Char('f')) &&
            !(ch >= QLatin1Char('A') && ch <= QLatin1Char('F'))) {
            setError(error, QStringLiteral("INVALID_ACCESS_TOKEN"));
            return false;
        }
    }
    m_accessToken = token;
    return true;
}

void HttpApiClient::logout() {
    if (!m_accessToken.isEmpty()) {
        QJsonObject ignored;
        request("POST", QStringLiteral("/api/v1/auth/logout"), nullptr, true, ignored, nullptr);
        m_accessToken.clear();
    }
}

bool HttpApiClient::listResources(const QString& collection, QJsonArray& items, QString* error) {
    if (!supportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    QJsonObject response;
    if (!request("GET", QStringLiteral("/api/v1/") + collection, nullptr, true, response, error)) return false;
    if (!response.value(QStringLiteral("items")).isArray()) {
        setError(error, QStringLiteral("INVALID_COLLECTION_RESPONSE"));
        return false;
    }
    items = response.value(QStringLiteral("items")).toArray();
    return true;
}

bool HttpApiClient::getResource(const QString& collection, const QString& id,
                                QJsonObject& record, QString* error) {
    if (!supportedCollection(collection) || id.isEmpty()) {
        setError(error, QStringLiteral("INVALID_RESOURCE_PATH")); return false;
    }
    return request("GET", QStringLiteral("/api/v1/") + collection + QLatin1Char('/') +
                   QString::fromLatin1(QUrl::toPercentEncoding(id)), nullptr, true, record, error);
}

bool HttpApiClient::createResource(const QString& collection, const QJsonObject& payload,
                                   QJsonObject& created, QString* error) {
    if (!supportedCollection(collection)) { setError(error, QStringLiteral("UNSUPPORTED_COLLECTION")); return false; }
    return request("POST", QStringLiteral("/api/v1/") + collection, &payload, true, created, error);
}

bool HttpApiClient::updateResource(const QString& collection, const QString& id,
                                   int expectedVersion, const QJsonObject& payload,
                                   QJsonObject& updated, QString* error) {
    if (!supportedCollection(collection) || id.isEmpty() || expectedVersion < 1) {
        setError(error, QStringLiteral("INVALID_RESOURCE_UPDATE")); return false;
    }
    const QJsonObject body{
        {QStringLiteral("expected_version"), expectedVersion},
        {QStringLiteral("record"), payload}
    };
    return request("PUT", QStringLiteral("/api/v1/") + collection + QLatin1Char('/') +
                   QString::fromLatin1(QUrl::toPercentEncoding(id)), &body, true, updated, error);
}

bool HttpApiClient::deleteResource(const QString& collection, const QString& id,
                                   int expectedVersion, QString* error) {
    if (!supportedCollection(collection) || id.isEmpty() || expectedVersion < 1) {
        setError(error, QStringLiteral("INVALID_RESOURCE_DELETE")); return false;
    }
    const QJsonObject body{{QStringLiteral("expected_version"), expectedVersion}};
    QJsonObject response;
    return request("DELETE", QStringLiteral("/api/v1/") + collection + QLatin1Char('/') +
                   QString::fromLatin1(QUrl::toPercentEncoding(id)), &body, true, response, error);
}

} // namespace biofeedback::api
