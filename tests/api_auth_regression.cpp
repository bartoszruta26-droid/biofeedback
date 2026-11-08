#include "api/HttpApiServer.hpp"
#include "application/AcquisitionService.hpp"
#include "application/DeviceService.hpp"
#include "application/MeasurementService.hpp"
#include "measurement/MeasurementEngine.hpp"
#include "core/Authentication.hpp"
#include "data/JsonResourceRepository.hpp"

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
class ConnectedTestDevice final : public biofeedback::application::DeviceAdapter {
public:
    bool connect(std::string&) override { connected = true; return true; }
    void disconnect() noexcept override { connected = false; }
    bool isConnected() const noexcept override { return connected; }
    bool connected{true};
};
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
                const QByteArray contentLengthHeader("Content-Length:");
                if (line.left(contentLengthHeader.size()).compare(contentLengthHeader, Qt::CaseInsensitive) == 0) {
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

    const QString encryptionKey = QStringLiteral("0123456789abcdef0123456789abcdef");
    auto repository = std::make_shared<biofeedback::data::JsonResourceRepository>(
        directory.filePath("shared-data"), encryptionKey, true);
    assert(repository->isReady());
    biofeedback::api::HttpApiServer server(usersPath, repository);
    auto measurementEngine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    auto measurementService = std::make_shared<biofeedback::application::MeasurementService>(measurementEngine);
    auto acquisitionService = std::make_shared<biofeedback::application::AcquisitionService>(measurementService);
    server.setAcquisitionService(acquisitionService);
    auto deviceService = std::make_shared<biofeedback::application::DeviceService>();
    assert(deviceService->registerDevice(biofeedback::application::DeviceDescriptor{"sensor-api-1", "API test sensor", "test"}, std::make_shared<ConnectedTestDevice>()));
    server.setDeviceService(deviceService);
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

    const auto resourceRequest = [&](const QByteArray& method, const QByteArray& path,
                                     const QByteArray& jsonBody = QByteArray()) {
        QByteArray request = method + " " + path + " HTTP/1.1\r\nHost: localhost\r\n";
        request += "Authorization: Bearer " + token.toLatin1() + "\r\n";
        if (!jsonBody.isNull()) {
            request += "Content-Type: application/json\r\nContent-Length: " +
                QByteArray::number(jsonBody.size()) + "\r\n";
        }
        request += "Connection: close\r\n\r\n";
        request += jsonBody;
        return transact(port, request);
    };

    const QByteArray unauthenticatedDevices = transact(
        port, "GET /api/v1/devices HTTP/1.1\\r\\nHost: localhost\\r\\nConnection: close\\r\\n\\r\\n");
    assert(statusCode(unauthenticatedDevices) == 401);

    const QByteArray unauthenticatedAcquisition = transact(
        port, "GET /api/v1/acquisition/session HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    assert(statusCode(unauthenticatedAcquisition) == 401);

    const QByteArray unauthenticatedPatients = transact(
        port, "GET /api/v1/patients HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    assert(statusCode(unauthenticatedPatients) == 401);

    const QByteArray listDevices = resourceRequest("GET", "/api/v1/devices");
    assert(statusCode(listDevices) == 200);
    const QJsonObject deviceList = QJsonDocument::fromJson(responseBody(listDevices)).object();
    assert(deviceList.value("count").toInt() == 1);
    const QJsonObject listedDevice = deviceList.value("items").toArray().first().toObject();
    assert(listedDevice.value("id").toString() == "sensor-api-1");
    assert(listedDevice.value("state").toString() == "connected");

    const QByteArray patientPayload = QJsonDocument(QJsonObject{
        {"firstName", QStringLiteral("API")},
        {"lastName", QStringLiteral("Patient")},
        {"pesel", QStringLiteral("TEST-PATIENT-001")}
    }).toJson(QJsonDocument::Compact);
    const QByteArray createPatient = resourceRequest("POST", "/api/v1/patients", patientPayload);
    assert(statusCode(createPatient) == 201);
    const QJsonObject createdPatient = QJsonDocument::fromJson(responseBody(createPatient)).object();
    const QString patientId = createdPatient.value("id").toString();
    assert(!patientId.isEmpty());
    assert(createdPatient.value("version").toInt() == 1);

    const QByteArray listPatients = resourceRequest("GET", "/api/v1/patients");
    assert(statusCode(listPatients) == 200);
    const QJsonObject patientList = QJsonDocument::fromJson(responseBody(listPatients)).object();
    assert(patientList.value("count").toInt() == 1);

    const QByteArray getPatient = resourceRequest(
        "GET", ("/api/v1/patients/" + patientId).toUtf8());
    assert(statusCode(getPatient) == 200);
    assert(QJsonDocument::fromJson(responseBody(getPatient)).object()
               .value("pesel").toString() == "TEST-PATIENT-001");

    const QByteArray updatePayload = QJsonDocument(QJsonObject{
        {"expected_version", 1},
        {"record", QJsonObject{
            {"firstName", QStringLiteral("Changed")},
            {"lastName", QStringLiteral("Patient")},
            {"pesel", QStringLiteral("TEST-PATIENT-001")}
        }}
    }).toJson(QJsonDocument::Compact);
    const QByteArray updatePatient = resourceRequest(
        "PUT", ("/api/v1/patients/" + patientId).toUtf8(), updatePayload);
    assert(statusCode(updatePatient) == 200);
    assert(QJsonDocument::fromJson(responseBody(updatePatient)).object()
               .value("version").toInt() == 2);
    assert(statusCode(resourceRequest(
        "PUT", ("/api/v1/patients/" + patientId).toUtf8(), updatePayload)) == 409);

    const QByteArray missingPatientAcquisition = resourceRequest("POST", "/api/v1/acquisition/session",
        QJsonDocument(QJsonObject{{"id", "missing-patient-session"}, {"patient_id", "00000000-0000-4000-8000-000000000001"},
                                  {"device_id", "sensor-api-1"}, {"protocol_id", "strength"}}).toJson(QJsonDocument::Compact));
    assert(statusCode(missingPatientAcquisition) == 422);
    const QByteArray missingDeviceAcquisition = resourceRequest("POST", "/api/v1/acquisition/session",
        QJsonDocument(QJsonObject{{"id", "missing-device-session"}, {"patient_id", patientId},
                                  {"device_id", "sensor-does-not-exist"}, {"protocol_id", "strength"}}).toJson(QJsonDocument::Compact));
    assert(statusCode(missingDeviceAcquisition) == 422);
    const QByteArray startAcquisition = resourceRequest("POST", "/api/v1/acquisition/session",
        QJsonDocument(QJsonObject{{"id", "api-session-1"}, {"patient_id", patientId},
                                  {"device_id", "sensor-api-1"}, {"protocol_id", "strength"}}).toJson(QJsonDocument::Compact));
    assert(statusCode(startAcquisition) == 201);
    const QByteArray activeAcquisition = resourceRequest("GET", "/api/v1/acquisition/session");
    assert(statusCode(activeAcquisition) == 200);
    assert(QJsonDocument::fromJson(responseBody(activeAcquisition)).object()
               .value("id").toString() == "api-session-1");
    assert(statusCode(resourceRequest("POST", "/api/v1/acquisition/session",
        QJsonDocument(QJsonObject{
            {"id", "api-session-2"}, {"patient_id", patientId},
            {"device_id", "sensor-api-1"}, {"protocol_id", "strength"}
        }).toJson(QJsonDocument::Compact))) == 409);
    const QByteArray sampleHistory = resourceRequest("GET", "/api/v1/acquisition/samples");
    assert(statusCode(sampleHistory) == 200);
    assert(QJsonDocument::fromJson(responseBody(sampleHistory)).object()
               .value("count").toInt() == 0);
    assert(statusCode(resourceRequest("DELETE", "/api/v1/acquisition/session",
        QJsonDocument(QJsonObject{{"id", "api-session-1"}}).toJson(QJsonDocument::Compact))) == 200);
    const QByteArray inactiveAcquisition = resourceRequest("GET", "/api/v1/acquisition/session");
    assert(statusCode(inactiveAcquisition) == 200);
    assert(!QJsonDocument::fromJson(responseBody(inactiveAcquisition)).object()
                .value("active").toBool());
    const QByteArray deletePayload = QJsonDocument(QJsonObject{{"expected_version", 2}}).toJson(QJsonDocument::Compact);
    assert(statusCode(resourceRequest("DELETE", ("/api/v1/patients/" + patientId).toUtf8(), deletePayload)) == 200);

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
