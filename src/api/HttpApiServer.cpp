#include "api/HttpApiServer.hpp"
#include "core/Authentication.hpp"
#include "core/RuntimePaths.hpp"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <utility>

namespace biofeedback::api {
namespace {
constexpr int MaxHeaderBytes = 16 * 1024;
constexpr int MaxBodyBytes = 16 * 1024 * 1024;
constexpr int SessionLifetimeSeconds = 1800;

QByteArray reasonPhrase(int status) {
    switch (status) {
    case 200: return "OK";
    case 201: return "Created";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 409: return "Conflict";
    case 422: return "Unprocessable Entity";
    case 429: return "Too Many Requests";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 413: return "Payload Too Large";
    case 431: return "Request Header Fields Too Large";
    case 503: return "Service Unavailable";
    default: return "Internal Server Error";
    }
}

struct ByteArrayWiper {
    QByteArray& value;
    ~ByteArrayWiper() {
        if (!value.isEmpty()) OPENSSL_cleanse(value.data(), static_cast<std::size_t>(value.size()));
    }
};
struct QStringWiper {
    QString& value;
    ~QStringWiper() { value.fill(QChar(0)); }
};
struct StringWiper {
    std::string& value;
    ~StringWiper() {
        if (!value.empty()) OPENSSL_cleanse(value.data(), value.size());
    }
};

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
    : HttpApiServer(usersFilePath,
                    std::shared_ptr<biofeedback::data::JsonResourceRepository>(),
                    QString(), parent) {}

HttpApiServer::HttpApiServer(
    const QString& usersFilePath,
    std::shared_ptr<biofeedback::data::JsonResourceRepository> resources,
    const QString& authenticationEncryptionKey,
    QObject* parent)
    : QObject(parent), m_usersFilePath(usersFilePath), m_resources(std::move(resources)),
      m_authenticationEncryptionKey(authenticationEncryptionKey) {
    connect(&m_server, &QTcpServer::newConnection, this, [this]() {
        while (m_server.hasPendingConnections()) {
            QTcpSocket* socket = m_server.nextPendingConnection();
            if (socket) handleConnection(socket);
        }
    });
}

void HttpApiServer::setAcquisitionService(
    std::shared_ptr<biofeedback::application::AcquisitionService> acquisition) {
    m_acquisition = std::move(acquisition);
}

void HttpApiServer::setDeviceService(std::shared_ptr<biofeedback::application::DeviceService> devices) {
    m_devices = std::move(devices);
}

bool HttpApiServer::listen(const QHostAddress& address, quint16 port) {
    return m_server.listen(address, port);
}
void HttpApiServer::close() { m_server.close(); }
bool HttpApiServer::isListening() const { return m_server.isListening(); }
quint16 HttpApiServer::serverPort() const { return m_server.serverPort(); }

QString HttpApiServer::bearerToken(const QByteArray& authorization) const {
    const QByteArray prefix("Bearer ");
    if (authorization.size() < prefix.size() ||
        authorization.left(prefix.size()).compare(prefix, Qt::CaseInsensitive) != 0) return {};
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
        bool originSeen = false;
        QByteArray authorization;
        QByteArray contentType;
        QByteArray origin;

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
            } else if (name.compare("Origin", Qt::CaseInsensitive) == 0) {
                if (originSeen) {
                    sendJson(socket, 400, errorBody("DUPLICATE_ORIGIN", "Powtórzony nagłówek Origin."));
                    return;
                }
                originSeen = true;
                origin = value;
            }
        }

        if (contentLength > MaxBodyBytes) {
            sendJson(socket, 413, errorBody("PAYLOAD_TOO_LARGE", "Treść żądania jest zbyt duża."));
            return;
        }
        if (buffer.size() - headerEnd - 4 < contentLength) return;

        const QByteArray method = requestLine.at(0);
        const QUrl requestUrl = QUrl::fromEncoded(requestLine.at(1));
        const QByteArray path = requestUrl.path(QUrl::FullyEncoded).toUtf8();
        const QUrlQuery requestQuery(requestUrl);
        QByteArray body = buffer.mid(headerEnd + 4, static_cast<int>(contentLength));
        ByteArrayWiper bodyWiper{body};

        if (originSeen) {
            const QUrl originUrl(QString::fromUtf8(origin));
            const QString host = originUrl.host().toLower();
            const bool localHost = host == QStringLiteral("localhost") ||
                host == QStringLiteral("127.0.0.1") || host == QStringLiteral("::1");
            const bool allowedScheme = originUrl.scheme() == QStringLiteral("http") ||
                originUrl.scheme() == QStringLiteral("https");
            if (!originUrl.isValid() || !localHost || !allowedScheme ||
                (!originUrl.path().isEmpty() && originUrl.path() != QStringLiteral("/")) ||
                !originUrl.userInfo().isEmpty() || !originUrl.query().isEmpty() ||
                !originUrl.fragment().isEmpty()) {
                sendJson(socket, 403, errorBody("ORIGIN_NOT_ALLOWED", "Dozwolone są wyłącznie lokalne źródła."));
                return;
            }
            socket->setProperty("corsOrigin", origin);
        }

        if (method == "OPTIONS" && originSeen) {
            sendJson(socket, 200, R"({"ok":true})");
            return;
        }

        if (method == "GET" && path == "/healthz") {
            sendJson(socket, 200, R"({"ok":true})");
            return;
        }

        if (method == "GET" && path == "/api/v1/status") {
            QJsonArray capabilities;
            capabilities.append(QStringLiteral("health"));
            capabilities.append(QStringLiteral("status"));
            capabilities.append(QStringLiteral("authentication"));
            if (m_acquisition) capabilities.append(QStringLiteral("acquisition_session_control"));
            QJsonArray limitations;
            limitations.append(QStringLiteral("device_control"));
            limitations.append(QStringLiteral("measurement_stream"));
            if (!m_acquisition) limitations.append(QStringLiteral("acquisition_service_unavailable"));
            limitations.append(QStringLiteral("role_scoped_authorization"));
            limitations.append(QStringLiteral("websocket_events"));
            limitations.append(QStringLiteral("mariadb_adapter"));
            QJsonArray collections;
            const QStringList resourceCollections{
                QStringLiteral("patients"), QStringLiteral("measurements"),
                QStringLiteral("training-sessions"), QStringLiteral("training-results"),
                QStringLiteral("game-sessions"), QStringLiteral("game-results")
            };
            for (const QString& collection : resourceCollections) collections.append(collection);
            const bool sharedDataReady = m_resources && m_resources->isReady();
            if (sharedDataReady) capabilities.append(QStringLiteral("shared_resource_crud"));
            else limitations.append(QStringLiteral("shared_resource_store_unavailable"));
            const QJsonObject status{
                {"status", QStringLiteral("ready")},
                {"version", QStringLiteral("0.3.0")},
                {"mode", QStringLiteral("shared-data-api")},
                {"timestamp", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
                {"shared_data_ready", sharedDataReady},
                {"storage_backend", sharedDataReady ? QStringLiteral("files") : QStringLiteral("unavailable")},
                {"resource_collections", collections},
                {"capabilities", capabilities},
                {"limitations", limitations}
            };
            sendJson(socket, 200, QJsonDocument(status).toJson(QJsonDocument::Compact));
            return;
        }

        if (method == "POST" && path == "/api/v1/auth/login") {
            const QByteArray jsonContentType("application/json");
            if (contentType.size() < jsonContentType.size() ||
                contentType.left(jsonContentType.size()).compare(jsonContentType, Qt::CaseInsensitive) != 0) {
                sendJson(socket, 400, errorBody("JSON_REQUIRED", "Wymagany jest Content-Type application/json."));
                return;
            }
            QJsonParseError parseError;
            QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                sendJson(socket, 400, errorBody("INVALID_JSON", "Niepoprawna treść JSON."));
                return;
            }
            QJsonObject credentials = document.object();
            const QString username = credentials.value(QStringLiteral("username")).toString().trimmed();
            QString password = credentials.value(QStringLiteral("password")).toString();
            QStringWiper passwordWiper{password};
            if (username.isEmpty() || password.isEmpty() || username.size() > 128 || password.size() > 1024) {
                sendJson(socket, 400, errorBody("INVALID_CREDENTIALS_FORMAT", "Niepoprawny format danych logowania."));
                return;
            }

            const QString clientAddress = socket->peerAddress().toString();
            const QDateTime nowUtc = QDateTime::currentDateTimeUtc();
            const auto throttle = m_loginAttempts.value(clientAddress);
            if (throttle.blockedUntilUtc.isValid() && throttle.blockedUntilUtc > nowUtc) {
                sendJson(socket, 429, errorBody("LOGIN_RATE_LIMITED", "Zbyt wiele prób logowania. Spróbuj ponownie za minutę."));
                return;
            }

            QString usersPath = m_usersFilePath;
            if (usersPath.isEmpty()) usersPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("users.json"));
            if (usersPath.isEmpty()) {
                sendJson(socket, 503, errorBody("AUTH_STORE_UNAVAILABLE", "Magazyn kont jest niedostępny."));
                return;
            }

            auto authentication = std::make_shared<biofeedback::Authentication>(usersPath.toStdString());
            if (!m_authenticationEncryptionKey.isEmpty()) {
                authentication->setFileEncryption(true, m_authenticationEncryptionKey.toStdString());
            }
            if (!authentication->loadUsers()) {
                sendJson(socket, 503, errorBody("AUTH_STORE_UNAVAILABLE", "Nie można odczytać magazynu kont."));
                return;
            }
            const std::string usernameUtf8 = username.toStdString();
            std::string passwordUtf8 = password.toUtf8().toStdString();
            StringWiper passwordUtf8Wiper{passwordUtf8};
            const bool credentialsValid = authentication->login(usernameUtf8, passwordUtf8);
            credentials.remove(QStringLiteral("password"));
            document = QJsonDocument();
            if (!credentialsValid) {
                auto& attempt = m_loginAttempts[clientAddress];
                ++attempt.failures;
                if (attempt.failures >= 5) {
                    attempt.failures = 0;
                    attempt.blockedUntilUtc = nowUtc.addSecs(60);
                }
                sendJson(socket, 401, errorBody("INVALID_LOGIN", "Nieprawidłowy login lub hasło."));
                return;
            }
            m_loginAttempts.remove(clientAddress);
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
            for (auto it = m_sessions.begin(); it != m_sessions.end();) {
                if (it->expiresAtUtc <= QDateTime::currentDateTimeUtc() ||
                    !it->authentication || !it->authentication->isLoggedIn()) {
                    it = m_sessions.erase(it);
                } else {
                    ++it;
                }
            }
            if (m_sessions.size() >= 64) {
                authentication->logout();
                sendJson(socket, 503, errorBody("SESSION_CAPACITY_REACHED", "Limit aktywnych sesji API został osiągnięty."));
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

        // Device discovery is read-only; physical control remains behind DeviceService.
        if (path == "/api/v1/devices") {
            if (!sessionValid) {
                if (sessionIt != m_sessions.end()) m_sessions.erase(sessionIt);
                sendJson(socket, 401, errorBody("AUTHENTICATION_REQUIRED", "Wymagane jest zalogowanie."));
                return;
            }
            if (sessionIt->authentication->getCurrentUserRole() != "administrator") {
                sendJson(socket, 403, errorBody("DEVICE_REGISTRY_FORBIDDEN", "Brak uprawnień do rejestru urządzeń."));
                return;
            }
            if (method != "GET") {
                sendJson(socket, 405, errorBody("METHOD_NOT_ALLOWED", "Dozwolony jest wyłącznie GET."));
                return;
            }
            if (!m_devices) {
                sendJson(socket, 503, errorBody("DEVICE_REGISTRY_UNAVAILABLE", "Rejestr urządzeń nie jest skonfigurowany."));
                return;
            }
            QJsonArray items;
            for (const auto& device : m_devices->devices()) {
                const char* state = "disconnected";
                switch (device.state) {
                case biofeedback::application::DeviceState::Disconnected: state = "disconnected"; break;
                case biofeedback::application::DeviceState::Connecting: state = "connecting"; break;
                case biofeedback::application::DeviceState::Connected: state = "connected"; break;
                case biofeedback::application::DeviceState::Error: state = "error"; break;
                }
                items.append(QJsonObject{
                    {"id", QString::fromStdString(device.id)},
                    {"name", QString::fromStdString(device.name)},
                    {"transport", QString::fromStdString(device.transport)},
                    {"state", QString::fromLatin1(state)},
                    {"last_error", QString::fromStdString(device.lastError)}
                });
            }
            sendJson(socket, 200, QJsonDocument(QJsonObject{
                {"items", items}, {"count", items.size()}
            }).toJson(QJsonDocument::Compact));
            return;
        }

        // Central acquisition session control. Only administrators may start/stop
        // acquisition until patient-scoped authorization is implemented.
        if (path == "/api/v1/acquisition/session" ||
            path == "/api/v1/acquisition/samples") {
            if (!sessionValid) {
                if (sessionIt != m_sessions.end()) m_sessions.erase(sessionIt);
                sendJson(socket, 401, errorBody("AUTHENTICATION_REQUIRED", "Wymagane jest zalogowanie."));
                return;
            }
            if (sessionIt->authentication->getCurrentUserRole() != "administrator") {
                sendJson(socket, 403, errorBody("ACQUISITION_FORBIDDEN", "Brak uprawnień do sterowania akwizycją."));
                return;
            }
            if (!m_acquisition) {
                sendJson(socket, 503, errorBody("ACQUISITION_UNAVAILABLE", "Usługa akwizycji nie jest skonfigurowana."));
                return;
            }
            if (path == "/api/v1/acquisition/samples") {
                if (method != "GET") {
                    sendJson(socket, 405, errorBody("METHOD_NOT_ALLOWED", "Dozwolony jest wyłącznie GET."));
                    return;
                }
                quint64 afterSequence = 0;
                const QString afterSequenceText = requestQuery.queryItemValue(QStringLiteral("after_sequence"));
                if (!afterSequenceText.isEmpty()) {
                    bool sequenceOk = false;
                    afterSequence = afterSequenceText.toULongLong(&sequenceOk);
                    if (!sequenceOk || afterSequenceText.startsWith(QLatin1Char('-'))) {
                        sendJson(socket, 400, errorBody("INVALID_SEQUENCE",
                            "Parametr after_sequence musi być nieujemną liczbą całkowitą."));
                        return;
                    }
                }

                QJsonArray samples;
                quint64 nextSequence = afterSequence;
                const auto history = m_acquisition->measurementService()->history();
                for (const auto& sample : history) {
                    const quint64 sequence = static_cast<quint64>(sample.sequence);
                    if (sequence <= afterSequence) continue;
                    samples.append(QJsonObject{
                        {"sequence", static_cast<qint64>(sample.sequence)},
                        {"timestamp_unix_ms", QJsonValue(static_cast<qint64>(sample.timestampUnixMs))},
                        {"device_timestamp_ms", static_cast<qint64>(sample.deviceTimestampMs)},
                        {"raw_value", sample.rawValue},
                        {"value", sample.value},
                        {"unit", QString::fromStdString(sample.unit)},
                        {"source", QString::fromStdString(sample.source)},
                        {"quality", QString::fromLatin1(biofeedback::measurement::qualityName(sample.quality))}
                    });
                    if (sequence > nextSequence) nextSequence = sequence;
                }
                biofeedback::application::AcquisitionSession active;
                const bool sessionActive = m_acquisition->activeSession(active);
                QJsonObject response{
                    {"items", samples},
                    {"count", samples.size()},
                    {"after_sequence", static_cast<qint64>(afterSequence)},
                    {"next_sequence", static_cast<qint64>(nextSequence)},
                    {"retention", QStringLiteral("bounded_in_memory")}
                };
                if (sessionActive) {
                    response.insert(QStringLiteral("session_id"), QString::fromStdString(active.id));
                    response.insert(QStringLiteral("device_id"), QString::fromStdString(active.deviceId));
                }
                sendJson(socket, 200, QJsonDocument(response).toJson(QJsonDocument::Compact));
                return;
            }
            if (method == "GET") {
                biofeedback::application::AcquisitionSession active;
                if (!m_acquisition->activeSession(active)) {
                    sendJson(socket, 200, R"({"active":false})");
                    return;
                }
                sendJson(socket, 200, QJsonDocument(QJsonObject{
                    {"active", true},
                    {"id", QString::fromStdString(active.id)},
                    {"patient_id", QString::fromStdString(active.patientId)},
                    {"device_id", QString::fromStdString(active.deviceId)},
                    {"protocol_id", QString::fromStdString(active.protocolId)}
                }).toJson(QJsonDocument::Compact));
                return;
            }
            if (method == "POST") {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
                if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                    sendJson(socket, 400, errorBody("INVALID_JSON", "Treść musi być obiektem JSON."));
                    return;
                }
                const QJsonObject request = document.object();
                biofeedback::application::AcquisitionSession requested;
                requested.id = request.value(QStringLiteral("id")).toString().toStdString();
                requested.patientId = request.value(QStringLiteral("patient_id")).toString().toStdString();
                requested.deviceId = request.value(QStringLiteral("device_id")).toString().toStdString();
                requested.protocolId = request.value(QStringLiteral("protocol_id")).toString().toStdString();
                if (!m_resources || !m_resources->isReady()) {
                    sendJson(socket, 503, errorBody("RESOURCE_STORE_UNAVAILABLE",
                        "Repozytorium pacjentów nie jest dostępne; akwizycja została zablokowana."));
                    return;
                }
                QJsonObject patientRecord;
                QString patientError;
                if (!m_resources->get(QStringLiteral("patients"),
                                      QString::fromStdString(requested.patientId),
                                      patientRecord, &patientError)) {
                    const bool missing = patientError == "RESOURCE_NOT_FOUND";
                    sendJson(socket, missing ? 422 : 503,
                        errorBody(missing ? "PATIENT_NOT_FOUND" : "PATIENT_VALIDATION_UNAVAILABLE",
                                  missing ? "Pacjent wskazany dla sesji akwizycji nie istnieje." :
                                            "Nie można zweryfikować pacjenta; akwizycja została zablokowana."));
                    return;
                }
                if (!m_devices) {
                    sendJson(socket, 503, errorBody("DEVICE_REGISTRY_UNAVAILABLE",
                        "Rejestr urządzeń nie jest skonfigurowany; akwizycja została zablokowana."));
                    return;
                }
                biofeedback::application::DeviceDescriptor device;
                if (!m_devices->device(requested.deviceId, device)) {
                    sendJson(socket, 422, errorBody("DEVICE_NOT_FOUND",
                        "Urządzenie wskazane dla sesji akwizycji nie jest zarejestrowane."));
                    return;
                }
                if (device.state != biofeedback::application::DeviceState::Connected) {
                    sendJson(socket, 409, errorBody("DEVICE_NOT_CONNECTED",
                        "Urządzenie wskazane dla sesji akwizycji nie jest połączone."));
                    return;
                }
                std::string acquisitionError;
                if (!m_acquisition->startSession(std::move(requested), acquisitionError)) {
                    const bool conflict = acquisitionError == "ACQUISITION_ALREADY_ACTIVE";
                    sendJson(socket, conflict ? 409 : 400,
                        errorBody(conflict ? "ACQUISITION_ALREADY_ACTIVE" : "INVALID_SESSION",
                                  conflict ? "Inna sesja akwizycji jest już aktywna." :
                                             "Wymagane są id, patient_id i device_id."));
                    return;
                }
                sendJson(socket, 201, R"({"active":true})");
                return;
            }
            if (method == "DELETE") {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
                if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                    sendJson(socket, 400, errorBody("INVALID_JSON", "Treść musi być obiektem JSON."));
                    return;
                }
                const std::string id = document.object().value(QStringLiteral("id")).toString().toStdString();
                std::string acquisitionError;
                if (!m_acquisition->stopSession(id, acquisitionError)) {
                    sendJson(socket, 409, errorBody("ACTIVE_SESSION_NOT_FOUND",
                        "Aktywna sesja nie istnieje lub identyfikator jest niezgodny."));
                    return;
                }
                sendJson(socket, 200, R"({"active":false})");
                return;
            }
            sendJson(socket, 405, errorBody("METHOD_NOT_ALLOWED", "Nieobsługiwana metoda sterowania sesją."));
            return;
        }

        // Shared, versioned resource API. Until full role scopes are implemented,
        // mutations and reads are restricted to administrator accounts.
        const QString decodedPath = QUrl::fromEncoded(path).path();
        const QStringList pathParts = decodedPath.split('/', Qt::SkipEmptyParts);
        if (pathParts.size() >= 3 && pathParts.at(0) == QStringLiteral("api") &&
            pathParts.at(1) == QStringLiteral("v1") &&
            biofeedback::data::JsonResourceRepository::isSupportedCollection(pathParts.at(2))) {
            if (!sessionValid) {
                if (sessionIt != m_sessions.end()) m_sessions.erase(sessionIt);
                sendJson(socket, 401, errorBody("AUTHENTICATION_REQUIRED", "Wymagane jest zalogowanie."));
                return;
            }
            if (sessionIt->authentication->getCurrentUserRole() != "administrator") {
                sendJson(socket, 403, errorBody("RESOURCE_FORBIDDEN", "Brak uprawnień do repozytorium danych."));
                return;
            }
            if (!m_resources || !m_resources->isReady()) {
                sendJson(socket, 503, errorBody("RESOURCE_STORE_UNAVAILABLE",
                    m_resources ? "Magazyn danych nie jest gotowy." : "Repozytorium danych nie zostało skonfigurowane."));
                return;
            }

            const QString collection = pathParts.at(2);
            const bool collectionRoute = pathParts.size() == 3;
            const bool itemRoute = pathParts.size() == 4;
            if (!collectionRoute && !itemRoute) {
                sendJson(socket, 404, errorBody("NOT_FOUND", "Nieznana ścieżka zasobu."));
                return;
            }

            QString repositoryError;
            if (collectionRoute && method == "GET") {
                const QJsonArray items = m_resources->list(collection, &repositoryError);
                if (!repositoryError.isEmpty()) {
                    sendJson(socket, 503, errorBody("RESOURCE_LIST_FAILED", "Nie można odczytać kolekcji."));
                    return;
                }
                sendJson(socket, 200, QJsonDocument(QJsonObject{
                    {"items", items}, {"count", items.size()}
                }).toJson(QJsonDocument::Compact));
                return;
            }

            if (collectionRoute && method == "POST") {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
                if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                    sendJson(socket, 400, errorBody("INVALID_JSON", "Treść musi być obiektem JSON."));
                    return;
                }
                QJsonObject created;
                if (!m_resources->create(collection, document.object(), created, &repositoryError)) {
                    const bool invalid = repositoryError == "EMPTY_RESOURCE" ||
                        repositoryError == "RESOURCE_TOO_LARGE" ||
                        repositoryError == "INVALID_PATIENT_RECORD" ||
                        repositoryError == "DUPLICATE_PATIENT_REFERENCE" ||
                        repositoryError == "INVALID_PATIENT_REFERENCE" ||
                        repositoryError == "PATIENT_NOT_FOUND" ||
                        repositoryError == "INVALID_TRAINING_SESSION_REFERENCE" ||
                        repositoryError == "TRAINING_SESSION_NOT_FOUND" ||
                        repositoryError == "PATIENT_TRAINING_SESSION_MISMATCH" ||
                        repositoryError == "UNSUPPORTED_COLLECTION";
                    sendJson(socket, invalid ? 400 : 503,
                        errorBody(invalid ? "INVALID_RESOURCE" : "RESOURCE_CREATE_FAILED",
                                  "Nie można utworzyć rekordu."));
                    return;
                }
                sendJson(socket, 201, QJsonDocument(created).toJson(QJsonDocument::Compact));
                return;
            }

            const QString resourceId = itemRoute ? pathParts.at(3) : QString();
            if (itemRoute && method == "GET") {
                QJsonObject record;
                if (!m_resources->get(collection, resourceId, record, &repositoryError)) {
                    const bool missing = repositoryError == "RESOURCE_NOT_FOUND";
                    sendJson(socket, missing ? 404 : 400,
                        errorBody(missing ? "RESOURCE_NOT_FOUND" : "INVALID_RESOURCE_ID",
                                  missing ? "Rekord nie istnieje." : "Niepoprawny identyfikator rekordu."));
                    return;
                }
                sendJson(socket, 200, QJsonDocument(record).toJson(QJsonDocument::Compact));
                return;
            }

            if (itemRoute && (method == "PUT" || method == "DELETE")) {
                QJsonParseError parseError;
                const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
                if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                    sendJson(socket, 400, errorBody("INVALID_JSON", "Treść musi być obiektem JSON."));
                    return;
                }
                const QJsonObject requestObject = document.object();
                const int expectedVersion = requestObject.value(QStringLiteral("expected_version")).toInt(0);
                if (expectedVersion < 1) {
                    sendJson(socket, 400, errorBody("EXPECTED_VERSION_REQUIRED", "Wymagane jest expected_version."));
                    return;
                }
                if (method == "PUT") {
                    if (!requestObject.value(QStringLiteral("record")).isObject()) {
                        sendJson(socket, 400, errorBody("RECORD_REQUIRED", "Wymagany jest obiekt record."));
                        return;
                    }
                    QJsonObject updated;
                    if (!m_resources->update(collection, resourceId, expectedVersion,
                                             requestObject.value(QStringLiteral("record")).toObject(),
                                             updated, &repositoryError)) {
                        const bool missing = repositoryError == "RESOURCE_NOT_FOUND";
                        const bool conflict = repositoryError == "VERSION_CONFLICT";
                        sendJson(socket, missing ? 404 : (conflict ? 409 : 400),
                            errorBody(missing ? "RESOURCE_NOT_FOUND" :
                                      (conflict ? "VERSION_CONFLICT" : "RESOURCE_UPDATE_FAILED"),
                                      missing ? "Rekord nie istnieje." :
                                      (conflict ? "Rekord został zmieniony przez innego klienta." :
                                                  "Nie można zaktualizować rekordu.")));
                        return;
                    }
                    sendJson(socket, 200, QJsonDocument(updated).toJson(QJsonDocument::Compact));
                    return;
                }
                if (!m_resources->remove(collection, resourceId, expectedVersion, &repositoryError)) {
                    const bool missing = repositoryError == "RESOURCE_NOT_FOUND";
                    const bool patientReferenced = repositoryError == "PATIENT_HAS_REFERENCES";
                    const bool trainingSessionReferenced =
                        repositoryError == "TRAINING_SESSION_HAS_RESULTS";
                    const bool referenceConflict = patientReferenced || trainingSessionReferenced;
                    const bool conflict = repositoryError == "VERSION_CONFLICT" || referenceConflict;
                    sendJson(socket, missing ? 404 : (conflict ? 409 : 400),
                        errorBody(missing ? "RESOURCE_NOT_FOUND" :
                                  (patientReferenced ? "PATIENT_HAS_REFERENCES" :
                                   (trainingSessionReferenced ? "TRAINING_SESSION_HAS_RESULTS" :
                                    (conflict ? "VERSION_CONFLICT" : "RESOURCE_DELETE_FAILED"))),
                                  missing ? "Rekord nie istnieje." :
                                  (patientReferenced ? "Pacjent ma powiązane rekordy kliniczne." :
                                   (trainingSessionReferenced ? "Sesja treningowa ma powiązane wyniki." :
                                    (conflict ? "Rekord został zmieniony przez innego klienta." :
                                                "Nie można usunąć rekordu.")))));
                    return;
                }
                sendJson(socket, 200, R"({"ok":true})");
                return;
            }

            sendJson(socket, 405, errorBody("METHOD_NOT_ALLOWED", "Metoda nie jest obsługiwana dla tego zasobu."));
            return;
        }

        if (method == "POST") {
            sendJson(socket, 503, errorBody("CAPABILITY_NOT_AVAILABLE", "Ta operacja API nie jest jeszcze wdrożona."));
            return;
        }
        if (method != "GET" && method != "HEAD" && method != "OPTIONS") {
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
    const QByteArray corsOrigin = socket->property("corsOrigin").toByteArray();
    if (!corsOrigin.isEmpty()) {
        response += "Access-Control-Allow-Origin: " + corsOrigin + "\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Authorization, Content-Type\r\n";
        response += "Access-Control-Max-Age: 600\r\n";
        response += "Vary: Origin\r\n";
    }
    response += "Content-Length: " + QByteArray::number(body.size());
    response += "\r\nConnection: close\r\n\r\n" + body;
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}
}
