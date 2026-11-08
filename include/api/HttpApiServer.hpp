#pragma once
#include <QObject>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QTcpServer>
#include <QString>
#include <memory>

class QTcpSocket;
namespace biofeedback { class Authentication; }

namespace biofeedback::api {
class HttpApiServer final : public QObject {
    Q_OBJECT
public:
    explicit HttpApiServer(const QString& usersFilePath = QString(),
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
    void handleConnection(QTcpSocket* socket);
    void sendJson(QTcpSocket* socket, int status, const QByteArray& body);
    QString bearerToken(const QByteArray& authorization) const;
    QHash<QString, ApiSession> m_sessions;
    QString m_usersFilePath;
    QTcpServer m_server;
};
}
