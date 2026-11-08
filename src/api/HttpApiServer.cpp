#include "api/HttpApiServer.hpp"
#include "core/Authentication.hpp"
#include "core/RuntimePaths.hpp"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTimer>
#include <openssl/rand.h>

namespace biofeedback::api {
namespace {
constexpr int MaxHeaderBytes = 16 * 1024;
constexpr int MaxBodyBytes = 16 * 1024;
constexpr int SessionLifetimeSeconds = 1800;

QByteArray reasonPhrase(int status) {
    switch (status) {
    case 200: return "OK";
    case 201: return "Created";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 413: return "Payload Too Large";
    case 431: return "Request Header Fields Too Large";
    case 503: return "Service Unavailable";
    default: return "Internal Server Error";
    }
}

QByteArray errorBody(const char* code, const char* message) {
    return QJsonDocument(QJsonObject{
        {QStringLiteral("code"), QString::fromLatin1(code)},
        {QStringLiteral("message"), QString::fromUtf8(message)}
    }).toJson(QJsonDocument::Compact);
}

bool isTokenCharacter(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return (u >= '0' && u <= '9') || (u >= 'A' && u <= 'Z') ||
           (u >= 'a' && u <= 'z') ||
           QByteArray("!#$%&'*+-.^_`|~").contains(c);
}
}

HttpApiServer::HttpApiServer(const QString& usersFilePath, QObject* parent)
    : QObject(parent), m_usersFilePath(usersFilePath) {
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

QString HttpApiServer::bearerToken(const QByteArray& authorization) const {
    const QByteArray prefix("Bearer ");
    if (!authorization.startsWith(prefix, Qt::CaseInsensitive)) return {};
    const QByteArray value = authorization.mid(prefix.size()).trimmed();
    if (value.size() != 64) return {};
    for (char c : value) {
        const bool hex = (c >= '0' && c <= '9') ||
                         (c >= 'a' && c <= 'f') ||
                         (c >= 'A' && c <= 'F');
        if (!hex) return {};
    }
    return QString::fromLatin1(value);
}

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
            if (buffer.size() > MaxHeaderBytes)
                sendJson(socket, 431, errorBody("HEADERS_TOO_LARGE", "Nagłówki żądania są zbyt duże."));
            return;
        }
        if (headerEnd > MaxHeaderBytes) {
            sendJson(socket, 431, errorBody("HEADERS_TOO_LARGE", "Nagłówki żądania są zbyt duże."));
            return;
        }

        const QList<QByteArray> lines = buffer.left(headerEnd).split('\n');
        if (lines.isEmpty()) {
            sendJson(socket, 400, errorBody("INVALID_REQUEST", "Niepoprawne żądanie."));
            return;
        }

        QByteArray requestLineBytes = lines.first();
        if (requestLineBytes.endsWith('\r')) requestLineBytes.chop(1);
        const QList<QByteArray> requestLine = requestLineBytes.split(' ');
        if (requestLine.size() != 3 ||
            requestLine.at(0).isEmpty() ||
            requestLine.at(1).isEmpty() ||
            (requestLine.at(2) != "HTTP/1.0" && requestLine.at(2) != "HTTP/1.1") ||
            !requestLine.at(1).startsWith('/') ||
            requestLine.at(1).contains('#') ||
            requestLine.at(1).contains('\\')) {
            sendJson(socket, 400, errorBody("INVALID_REQUEST_LINE", "Niepoprawna linia żądania."));
            return;
        }

        qint64 contentLength = 0;
        bool contentLengthSeen = false;
        bool authorizationSeen = false;
        bool contentTypeSeen = false;
        QByteArray authorization;
        QByteArray contentType;

        for (int i = 1; i < lines.size(); ++i) {
            QByteArray line = lines.at(i);
            if (line.endsWith('\r')) line.chop(1);
            if (line.isEmpty() || line.startsWith(' ') || line.startsWith('\t')) {
                sendJson(socket, 400, errorBody("INVALID_HEADER", "Niepoprawny nagłówek."));
                return;
            }
            const int colon = line.indexOf(':');
            if (colon <= 0) {
                sendJson(socket, 400, errorBody("INVALID_HEADER", "Niepoprawny nagłówek."));
                return;
            }

            const QByteArray name = line.left(colon);
            for (char c : name) {
                if (!isTokenCharacter(c)) {
                    sendJson(socket, 400, errorBody("INVALID_HEADER_NAME", "Niepoprawna nazwa nagłówka."));
                    return;
                }
            }

            const QByteArray value = line.mid(colon + 1).trimmed();
            if (name.compare("Transfer-Encoding", Qt::CaseInsensitive) == 0) {
                sendJson(socket, 400, errorBody("TRANSFER_ENCODING_UNSUPPORTED", "Kodowanie transferowe nie jest obsługiwane."));
                return;
            }
            if (name.compare("Content-Length", Qt::CaseInsensitive) == 0) {
                if (contentLengthSeen) {
                    sendJson(socket, 400, errorBody("DUPLICATE_CONTENT_LENGTH", "Powtórzony Content-Length."));
                    return;
                }
                bool ok = false;
                contentLength = value.toLongLong(&ok);
                if (!ok || contentLength < 0) {
                    sendJson(socket, 400, errorBody("INVALID_CONTENT_LENGTH", "Niepoprawny Content-Length."));
                    return;
                }
                contentLengthSeen = true;
            } else if (name.compare("Authorization", Qt::CaseInsensitive) == 0) {
                if (authorizationSeen) {
                    sendJson(socket, 400, errorBody("DUPLICATE_AUTHORIZATION", "Powtórzony nagłówek Authorization."));
                    return;
                }
                authorizationSeen = true;
                authorization = value;
            } else if (name.compare("Content-Type", Qt::CaseInsensitive) == 0) {
                if (contentTypeSeen) {
                    sendJson(socket, 400, errorBody("DUPLICATE_CONTENT_TYPE", "Powtórzony Content-Type."));
                    return;
                }
                contentTypeSeen = true;
                contentType = value;
            }
        }

        if (contentLength > MaxBodyBytes) {
            sendJson(socket, 413, errorBody("PAYLOAD_TOO_LARGE", "Treść żądania jest zbyt duża."));
            return;
        }
        if (buffer.size() - headerEnd - 4 < contentLength) return;

        const QByteArray method = requestLine.at(0);
        const QByteArray path = requestLine.at(1).split('?').first();
        const QByteArray body = buffer.mid(headerEnd + 4, static_cast<int>(contentLength));

        if (method == "GET" && path == "/healthz") {
            sendJson(socket, 200, R"({"ok":true})");
            return;
        }

        if (method == "GET" && path == "/api/v1/status") {
            QJsonArray capabilities;
            capabilities.append(QStringLiteral("health"));
            capabilities.append(QStringLiteral("status"));
            capabilities.append(QStringLiteral("authentication"));
            QJsonArray limitations;
            limitations.append(QStringLiteral("device_control"));
            limitations.append(QStringLiteral("patient_repository"));
            limitations.append(QStringLiteral("measurement_stream"));
            const QJsonObject status{
                {"status", QStringLiteral("ready")},
                {"version", QStringLiteral("0.2.0")},
                {"mode", QStringLiteral("desktop-api")},
                {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
                {"capabilities", capabilities},
                {"limitations", limitations}
            };
            sendJson(socket, 200, QJsonDocument(status).toJson(QJsonDocument::Compact));
            return;
        }

        if (method == "POST" && path == "/api/v1/auth/login") {
            if (!contentType.startsWith("application/json", Qt::CaseInsensitive)) {
                sendJson(socket, 400, errorBody("JSON_REQUIRED", "Wymagany jest Content-Type application/json."));
                return;
            }
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                sendJson(socket, 400, errorBody("INVALID_JSON", "Niepoprawna treść JSON."));
                return;
            }
            const QJsonObject credentials = document.object();
            const QString username = credentials.value(QStringLiteral("username")).toString().trimmed();
            const QString password = credentials.value(QStringLiteral("password")).toString();
            if (username.isEmpty() || password.isEmpty() || username.size() > 128 || password.size() > 1024) {
                sendJson(socket, 400, errorBody("INVALID_CREDENTIALS_FORMAT", "Niepoprawny format danych logowania."));
                return;
            }

            QString usersPath = m_usersFilePath;
            if (usersPath.isEmpty()) usersPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("users.json"));
            if (usersPath.isEmpty()) {
                sendJson(socket, 503, errorBody("AUTH_STORE_UNAVAILABLE", "Magazyn kont jest niedostępny."));
                return;
            }

            auto authentication = std::make_shared<biofeedback::Authentication>(usersPath.toStdString());
            if (!authentication->loadUsers()) {
                sendJson(socket, 503, errorBody("AUTH_STORE_UNAVAILABLE", "Nie można odczytać magazynu kont."));
                return;
            }
            if (!authentication->login(username.toStdString(), password.toStdString())) {
                sendJson(socket, 401, errorBody("INVALID_LOGIN", "Nieprawidłowy login lub hasło."));
                return;
            }
            if (authentication->requiresPasswordChange()) {
                authentication->logout();
                sendJson(socket, 403, errorBody("PASSWORD_CHANGE_REQUIRED", "Wymagana jest zmiana hasła przed uzyskaniem dostępu API."));
                return;
            }

            unsigned char randomBytes[32]{};
            if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1) {
                authentication->logout();
                sendJson(socket, 503, errorBody("SESSION_GENERATION_FAILED", "Nie można utworzyć bezpiecznej sesji."));
                return;
            }
            const QString token = QString::fromLatin1(
                QByteArray(reinterpret_cast<const char*>(randomBytes), sizeof(randomBytes)).toHex());
            const QDateTime expiry = QDateTime::currentDateTimeUtc().addSecs(SessionLifetimeSeconds);
            m_sessions.insert(token, ApiSession{authentication, expiry});

            const QJsonObject response{
                {"access_token", token},
                {"token_type", QStringLiteral("Bearer")},
                {"expires_at", expiry.toString(Qt::ISODateWithMs)},
                {"user", QJsonObject{
                    {"username", QString::fromStdString(authentication->getCurrentUsername())},
                    {"role", QString::fromStdString(authentication->getCurrentUserRole())}
                }}
            };
            sendJson(socket, 200, QJsonDocument(response).toJson(QJsonDocument::Compact));
            return;
        }

        const QString token = bearerToken(authorization);
        auto sessionIt = token.isEmpty() ? m_sessions.end() : m_sessions.find(token);
        const bool sessionValid = sessionIt != m_sessions.end() &&
            sessionIt->expiresAtUtc > QDateTime::currentDateTimeUtc() &&
            sessionIt->authentication && sessionIt->authentication->isLoggedIn();

        if (method == "GET" && path == "/api/v1/auth/me") {
            if (!sessionValid) {
                if (sessionIt != m_sessions.end()) m_sessions.erase(sessionIt);
                sendJson(socket, 401, errorBody("AUTHENTICATION_REQUIRED", "Wymagane jest zalogowanie."));
                return;
            }
            const QJsonObject response{
                {"username", QString::fromStdString(sessionIt->authentication->getCurrentUsername())},
                {"role", QString::fromStdString(sessionIt->authentication->getCurrentUserRole())},
                {"expires_at", sessionIt->expiresAtUtc.toString(Qt::ISODateWithMs)}
            };
            sendJson(socket, 200, QJsonDocument(response).toJson(QJsonDocument::Compact));
            return;
        }

        if (method == "POST" && path == "/api/v1/auth/logout") {
            if (!sessionValid) {
                if (sessionIt != m_sessions.end()) m_sessions.erase(sessionIt);
                sendJson(socket, 401, errorBody("AUTHENTICATION_REQUIRED", "Wymagane jest zalogowanie."));
                return;
            }
            sessionIt->authentication->logout();
            m_sessions.erase(sessionIt);
            sendJson(socket, 200, R"({"ok":true})");
            return;
        }

        if (method != "GET" && method != "HEAD" && method != "OPTIONS" && method != "POST") {
            sendJson(socket, 405, errorBody("METHOD_NOT_ALLOWED", "Metoda HTTP nie jest obsługiwana."));
            return;
        }
        sendJson(socket, 404, errorBody("NOT_FOUND", "Endpoint nie istnieje lub nie jest jeszcze wdrożony."));
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    QTimer::singleShot(10000, socket, [socket]() {
        if (socket && socket->state() != QAbstractSocket::UnconnectedState)
            socket->disconnectFromHost();
    });
}

void HttpApiServer::sendJson(QTcpSocket* socket, int status, const QByteArray& body) {
    if (!socket || socket->property("requestHandled").toBool()) return;
    socket->setProperty("requestHandled", true);

    QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + " " + reasonPhrase(status) + "\r\n";
    response += "Content-Type: application/json; charset=utf-8\r\nCache-Control: no-store\r\n";
    response += "X-Content-Type-Options: nosniff\r\n";
    response += "Pragma: no-cache\r\n";
    response += "Content-Length: " + QByteArray::number(body.size());
    response += "\r\nConnection: close\r\n\r\n" + body;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}
}
