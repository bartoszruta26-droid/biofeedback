#include "api/HttpApiServer.hpp"
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>
namespace biofeedback::api {
namespace {
constexpr int MaxHeaderBytes = 16 * 1024;
constexpr int MaxBodyBytes = 16 * 1024;
QByteArray reasonPhrase(int status) {
    switch (status) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 413: return "Payload Too Large";
    case 431: return "Request Header Fields Too Large";
    case 503: return "Service Unavailable";
    default: return "Internal Server Error";
    }
}
}
HttpApiServer::HttpApiServer(QObject* parent) : QObject(parent) {
    connect(&m_server, &QTcpServer::newConnection, this, [this]() {
        while (m_server.hasPendingConnections()) {
            QTcpSocket* socket = m_server.nextPendingConnection();
            if (socket) handleConnection(socket);
        }
    });
}
bool HttpApiServer::listen(const QHostAddress& address, quint16 port) {
    return m_server.listen(address, port);
}
void HttpApiServer::close() { m_server.close(); }
bool HttpApiServer::isListening() const { return m_server.isListening(); }
quint16 HttpApiServer::serverPort() const { return m_server.serverPort(); }
void HttpApiServer::handleConnection(QTcpSocket* socket) {
    socket->setParent(this);
    socket->setProperty("requestHandled", false);
    socket->setProperty("requestBuffer", QByteArray());
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        if (!socket || socket->property("requestHandled").toBool()) return;
        QByteArray buffer = socket->property("requestBuffer").toByteArray();
        buffer += socket->readAll();
        socket->setProperty("requestBuffer", buffer);
        const int headerEnd = buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            if (buffer.size() > MaxHeaderBytes) sendJson(socket, 431, R"({"error":"headers_too_large"})");
            return;
        }
        if (headerEnd > MaxHeaderBytes) {
            sendJson(socket, 431, R"({"error":"headers_too_large"})");
            return;
        }
        const QList<QByteArray> lines = buffer.left(headerEnd).split('\n');
        if (lines.isEmpty()) { sendJson(socket, 400, R"({"error":"invalid_request"})"); return; }
        const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
        if (requestLine.size() != 3 || !requestLine.at(2).startsWith("HTTP/1.")) {
            sendJson(socket, 400, R"({"error":"invalid_request_line"})"); return;
        }
        qint64 contentLength = 0;
        for (int i = 1; i < lines.size(); ++i) {
            const QByteArray line = lines.at(i).trimmed();
            const int colon = line.indexOf(':');
            if (colon <= 0) continue;
            if (line.left(colon).trimmed().compare("Content-Length", Qt::CaseInsensitive) == 0) {
                bool ok = false;
                contentLength = line.mid(colon + 1).trimmed().toLongLong(&ok);
                if (!ok || contentLength < 0) { sendJson(socket, 400, R"({"error":"invalid_content_length"})"); return; }
            }
        }
        if (contentLength > MaxBodyBytes) { sendJson(socket, 413, R"({"error":"payload_too_large"})"); return; }
        if (buffer.size() - headerEnd - 4 < contentLength) return;
        const QByteArray method = requestLine.at(0);
        const QByteArray path = requestLine.at(1).split('?').first();
        if (method == "GET" && path == "/healthz") {
            sendJson(socket, 200, R"({"ok":true})"); return;
        }
        if (method == "GET" && path == "/api/v1/status") {
            const QJsonObject status{
                {"status", "ready"}, {"version", "0.1.0"}, {"mode", "desktop-api"},
                {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
                {"capabilities", QJsonArray{"health", "status"}},
                {"limitations", QJsonArray{"device_control", "patient_repository", "measurement_stream"}}
            };
            sendJson(socket, 200, QJsonDocument(status).toJson(QJsonDocument::Compact)); return;
        }
        if (method != "GET" && method != "HEAD" && method != "OPTIONS") {
            sendJson(socket, 503, R"({"code":"CAPABILITY_NOT_AVAILABLE","message":"This API operation is not enabled"})"); return;
        }
        sendJson(socket, 404, R"({"code":"NOT_FOUND","message":"Endpoint not found"})");
    });
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    QTimer::singleShot(10000, socket, [socket]() {
        if (socket && socket->state() != QAbstractSocket::UnconnectedState) socket->disconnectFromHost();
    });
}
void HttpApiServer::sendJson(QTcpSocket* socket, int status, const QByteArray& body) {
    if (!socket || socket->property("requestHandled").toBool()) return;
    socket->setProperty("requestHandled", true);
    QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + " " + reasonPhrase(status) + "\r\n";
    response += "Content-Type: application/json; charset=utf-8\r\nCache-Control: no-store\r\n";
    response += "X-Content-Type-Options: nosniff\r\nContent-Length: " + QByteArray::number(body.size());
    response += "\r\nConnection: close\r\n\r\n" + body;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}
}
