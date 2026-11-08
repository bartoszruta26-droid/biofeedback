#include "api/HttpApiServer.hpp"
#include "core/Authentication.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTcpSocket>
#include <QTimer>

#include <cassert>
#include <iostream>

namespace {
QByteArray transact(quint16 port, const QByteArray& request) {
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, port);
    assert(socket.waitForConnected(3000));

    QByteArray response;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(&socket, &QTcpSocket::readyRead, &loop, [&]() {
        response += socket.readAll();
        const int headerEnd = response.indexOf("\r\n\r\n");
        if (headerEnd >= 0) {
            const QList<QByteArray> lines = response.left(headerEnd).split('\n');
            int contentLength = 0;
            for (QByteArray line : lines) {
                if (line.startsWith("Content-Length:", Qt::CaseInsensitive)) {
                    contentLength = line.mid(line.indexOf(':') + 1).trimmed().toInt();
                }
            }
            if (response.size() >= headerEnd + 4 + contentLength) loop.quit();
        }
    });
    QObject::connect(&socket, &QTcpSocket::disconnected, &loop, &QEventLoop::quit);
    socket.write(request);
    socket.flush();
    timeout.start(3000);
    loop.exec();
    response += socket.readAll();
    return response;
}

int statusCode(const QByteArray& response) {
    const QList<QByteArray> fields = response.left(response.indexOf("\r\n")).split(' ');
    assert(fields.size() >= 2);
    return fields.at(1).toInt();
}

QByteArray responseBody(const QByteArray& response) {
    const int separator = response.indexOf("\r\n\r\n");
    assert(separator >= 0);
    return response.mid(separator + 4);
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    assert(directory.isValid());
    const QString usersPath = directory.filePath("users.json");

    biofeedback::Authentication bootstrap(usersPath.toStdString());
    assert(bootstrap.createInitialAdministrator("api-test", "test-password-123"));

    biofeedback::api::HttpApiServer server(usersPath);
    assert(server.listen(QHostAddress::LocalHost, 0));
    const quint16 port = server.serverPort();

    const QByteArray unauthenticated = transact(
        port, "GET /api/v1/auth/me HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    assert(statusCode(unauthenticated) == 401);

    const QJsonObject credentials{
        {"username", QStringLiteral("api-test")},
        {"password", QStringLiteral("test-password-123")}
    };
    const QByteArray loginBody = QJsonDocument(credentials).toJson(QJsonDocument::Compact);
    const QByteArray loginRequest =
        "POST /api/v1/auth/login HTTP/1.1\r\nHost: localhost\r\n"
        "Content-Type: application/json\r\nContent-Length: " +
        QByteArray::number(loginBody.size()) +
        "\r\nConnection: close\r\n\r\n" + loginBody;
    const QByteArray loginResponse = transact(port, loginRequest);
    assert(statusCode(loginResponse) == 200);

    const QJsonDocument loginDocument = QJsonDocument::fromJson(responseBody(loginResponse));
    assert(loginDocument.isObject());
    const QString token = loginDocument.object().value("access_token").toString();
    assert(token.size() == 64);

    const QByteArray meRequest =
        "GET /api/v1/auth/me HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer " +
        token.toLatin1() + "\r\nConnection: close\r\n\r\n";
    const QByteArray meResponse = transact(port, meRequest);
    assert(statusCode(meResponse) == 200);
    const QJsonObject me = QJsonDocument::fromJson(responseBody(meResponse)).object();
    assert(me.value("username").toString() == "api-test");
    assert(me.value("role").toString() == "administrator");

    const QByteArray logoutRequest =
        "POST /api/v1/auth/logout HTTP/1.1\r\nHost: localhost\r\nAuthorization: Bearer " +
        token.toLatin1() + "\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    assert(statusCode(transact(port, logoutRequest)) == 200);
    assert(statusCode(transact(port, meRequest)) == 401);

    const QJsonObject badCredentials{
        {"username", QStringLiteral("api-test")},
        {"password", QStringLiteral("incorrect-password")}
    };
    const QByteArray badBody = QJsonDocument(badCredentials).toJson(QJsonDocument::Compact);
    const QByteArray badRequest =
        "POST /api/v1/auth/login HTTP/1.1\r\nHost: localhost\r\n"
        "Content-Type: application/json\r\nContent-Length: " +
        QByteArray::number(badBody.size()) +
        "\r\nConnection: close\r\n\r\n" + badBody;
    assert(statusCode(transact(port, badRequest)) == 401);
    for (int attempt = 0; attempt < 4; ++attempt)
        assert(statusCode(transact(port, badRequest)) == 401);
    assert(statusCode(transact(port, badRequest)) == 429);

    const QByteArray localOriginResponse = transact(
        port, "GET /healthz HTTP/1.1\r\nHost: localhost\r\nOrigin: http://localhost:5173\r\nConnection: close\r\n\r\n");
    assert(statusCode(localOriginResponse) == 200);
    assert(localOriginResponse.contains("Access-Control-Allow-Origin: http://localhost:5173"));

    const QByteArray preflightResponse = transact(
        port, "OPTIONS /api/v1/auth/login HTTP/1.1\r\nHost: localhost\r\n"
              "Origin: http://127.0.0.1:8080\r\n"
              "Access-Control-Request-Method: POST\r\n"
              "Access-Control-Request-Headers: authorization,content-type\r\n"
              "Connection: close\r\n\r\n");
    assert(statusCode(preflightResponse) == 200);
    assert(preflightResponse.contains("Access-Control-Allow-Headers: Authorization, Content-Type"));

    const QByteArray externalOriginResponse = transact(
        port, "GET /healthz HTTP/1.1\r\nHost: localhost\r\nOrigin: https://evil.example\r\nConnection: close\r\n\r\n");
    assert(statusCode(externalOriginResponse) == 403);

    const QByteArray statusResponse = transact(
        port, "GET /api/v1/status HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    assert(statusCode(statusResponse) == 200);

    server.close();
    std::cout << "api_auth_regression: PASS\n";
    return 0;
}
