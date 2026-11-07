#pragma once
#include <QObject>
#include <QByteArray>
#include <QHostAddress>
#include <QTcpServer>
class QTcpSocket;
namespace biofeedback::api {
class HttpApiServer final : public QObject {
    Q_OBJECT
public:
    explicit HttpApiServer(QObject* parent = nullptr);
    bool listen(const QHostAddress& address = QHostAddress::LocalHost, quint16 port = 8765);
    void close();
    bool isListening() const;
    quint16 serverPort() const;
private:
    void handleConnection(QTcpSocket* socket);
    void sendJson(QTcpSocket* socket, int status, const QByteArray& body);
    QTcpServer m_server;
};
}
