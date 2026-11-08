#pragma once
#include <QObject>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QTcpServer>
#include <QString>
#include <memory>
#include "data/JsonResourceRepository.hpp"

class QTcpSocket;
namespace biofeedback { class Authentication; }

namespace biofeedback::api {
class HttpApiServer final : public QObject {
    Q_OBJECT
public:
    explicit HttpApiServer(const QString& usersFilePath = QString(),
                           QObject* parent = nullptr);
    HttpApiServer(const QString& usersFilePath,
                  std::shared_ptr<biofeedback::data::JsonResourceRepository> resources,
                  const QString& authenticationEncryptionKey = QString(),
                  QObject* parent = nullptr);
    bool listen(const QHostAddress& address = QHostAddress::LocalHost, quint16 port = 8765);
    void close();
    bool isListening() const;
    quint16 serverPort() const;
private:
    struct ApiSession {
        std::shared_ptr<biofeedback::Authentication> authentication;
        QDateTime expiresAtUtc;
    };
    struct LoginThrottle {
        int failures{0};
        QDateTime blockedUntilUtc;
    };
    void handleConnection(QTcpSocket* socket);
    void sendJson(QTcpSocket* socket, int status, const QByteArray& body);
    QString bearerToken(const QByteArray& authorization) const;
    QHash<QString, ApiSession> m_sessions;
    QHash<QString, LoginThrottle> m_loginAttempts;
    QString m_usersFilePath;
    std::shared_ptr<biofeedback::data::JsonResourceRepository> m_resources;
    QString m_authenticationEncryptionKey;
    QTcpServer m_server;
};
}
