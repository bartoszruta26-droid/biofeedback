#include "api/HttpApiClient.hpp"
#include "api/HttpApiServer.hpp"
#include "core/Authentication.hpp"
#include "data/JsonResourceRepository.hpp"
#include "application/AcquisitionService.hpp"
#include "application/DeviceService.hpp"
#include "application/MeasurementService.hpp"
#include "measurement/MeasurementEngine.hpp"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
class TestDeviceAdapter final : public biofeedback::application::DeviceAdapter {
public:
    bool connect(std::string& error) override { connected = true; error.clear(); return true; }
    void disconnect() noexcept override { connected = false; }
    bool isConnected() const noexcept override { return connected; }
    bool connected{false};
};
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    assert(directory.isValid());

    const QString usersPath = directory.filePath(QStringLiteral("users.json"));
    biofeedback::Authentication bootstrap(usersPath.toStdString());
    assert(bootstrap.createInitialAdministrator("api-client-test", "client-password-123"));

    const QString key = QStringLiteral("0123456789abcdef0123456789abcdef");
    auto repository = std::make_shared<biofeedback::data::JsonResourceRepository>(
        directory.filePath(QStringLiteral("shared-data")), key, true);
    assert(repository->isReady());

    biofeedback::api::HttpApiServer server(usersPath, repository);
    auto measurementEngine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    auto measurementService = std::make_shared<biofeedback::application::MeasurementService>(measurementEngine);
    auto acquisitionService = std::make_shared<biofeedback::application::AcquisitionService>(measurementService);
    auto deviceService = std::make_shared<biofeedback::application::DeviceService>();
    server.setAcquisitionService(acquisitionService);
    server.setDeviceService(deviceService);
    assert(server.listen(QHostAddress::LocalHost, 0));

    biofeedback::api::HttpApiClient client(
        QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    QString error;
    assert(client.login(QStringLiteral("api-client-test"),
                        QStringLiteral("client-password-123"), &error));
    assert(client.isAuthenticated());

    // Device discovery and acquisition control are exposed through the same
    // authenticated client used for shared resources.
    QJsonArray devices;
    assert(client.listDevices(devices, &error));
    assert(devices.isEmpty());

    QJsonObject activeSession;
    assert(client.getAcquisitionSession(activeSession, &error));
    assert(!activeSession.value(QStringLiteral("active")).toBool());

    QJsonArray acquisitionSamples;
    assert(client.listAcquisitionSamples(acquisitionSamples, &error));
    assert(acquisitionSamples.isEmpty());
    assert(client.listAcquisitionSamples(acquisitionSamples, &error, 17));
    assert(acquisitionSamples.isEmpty());

    assert(!client.startAcquisitionSession(QString(), QStringLiteral("patient"),
                                            QStringLiteral("device"), QStringLiteral("protocol"), &error));
    assert(error == QStringLiteral("INVALID_ACQUISITION_SESSION"));

    QJsonObject created;
    assert(client.createResource(QStringLiteral("patients"), QJsonObject{
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-002")},
        {QStringLiteral("firstName"), QStringLiteral("Test")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")}
    }, created, &error));
    const QString id = created.value(QStringLiteral("id")).toString();
    assert(!id.isEmpty());
    assert(created.value(QStringLiteral("version")).toInt() == 1);

    auto testDevice = std::make_shared<TestDeviceAdapter>();
    assert(deviceService->registerDevice(
        {"test-device", "Regression device", "mock",
         biofeedback::application::DeviceState::Disconnected, {}}, testDevice));
    assert(deviceService->connectDevice("test-device"));
    assert(client.listDevices(devices, &error));
    assert(devices.size() == 1);
    assert(devices.first().toObject().value(QStringLiteral("state")).toString() ==
           QStringLiteral("connected"));

    const QString acquisitionId = QStringLiteral("api-client-acquisition-session");
    assert(client.startAcquisitionSession(acquisitionId, id, QStringLiteral("test-device"),
                                          QStringLiteral("regression-protocol"), &error));
    assert(client.getAcquisitionSession(activeSession, &error));
    assert(activeSession.value(QStringLiteral("active")).toBool());
    assert(activeSession.value(QStringLiteral("id")).toString() == acquisitionId);
    assert(activeSession.value(QStringLiteral("device_id")).toString() == QStringLiteral("test-device"));

    // The replay cursor is exercised against actual accepted samples, not only
    // an empty history response.
    biofeedback::measurement::MeasurementInput sampleInput;
    sampleInput.deviceTimestampMs = 1234;
    sampleInput.hasTimestamp = true;
    sampleInput.rawValue = 321;
    sampleInput.value = 321.0;
    sampleInput.sensorValid = true;
    sampleInput.unit = "g";
    sampleInput.source = "api-client-regression";
    biofeedback::measurement::MeasurementSample acceptedSample;
    assert(measurementService->ingest(sampleInput, acceptedSample));

    assert(client.listAcquisitionSamples(acquisitionSamples, &error, 0));
    assert(acquisitionSamples.size() == 1);
    const QJsonObject firstSample = acquisitionSamples.first().toObject();
    assert(firstSample.value(QStringLiteral("sequence")).toVariant().toULongLong() ==
           acceptedSample.sequence);
    assert(firstSample.value(QStringLiteral("raw_value")).toInt() == 321);
    assert(firstSample.value(QStringLiteral("quality")).toString() == QStringLiteral("valid"));

    assert(client.listAcquisitionSamples(acquisitionSamples, &error, acceptedSample.sequence));
    assert(acquisitionSamples.isEmpty());

    assert(client.stopAcquisitionSession(acquisitionId, &error));
    assert(client.getAcquisitionSession(activeSession, &error));
    assert(!activeSession.value(QStringLiteral("active")).toBool());

    QJsonArray items;
    assert(client.listResources(QStringLiteral("patients"), items, &error));
    assert(items.size() == 1);

    QJsonObject updated;
    assert(client.updateResource(QStringLiteral("patients"), id, 1, QJsonObject{
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-002")},
        {QStringLiteral("firstName"), QStringLiteral("Updated")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")}
    }, updated, &error));
    assert(updated.value(QStringLiteral("version")).toInt() == 2);

    assert(!client.updateResource(QStringLiteral("patients"), id, 1, QJsonObject{
        {QStringLiteral("pesel"), QStringLiteral("TEST-PATIENT-002")},
        {QStringLiteral("firstName"), QStringLiteral("Stale")},
        {QStringLiteral("lastName"), QStringLiteral("Update")}
    }, updated, &error));
    assert(error.contains(QStringLiteral("VERSION_CONFLICT")));

    // A second independently authenticated client must observe the same
    // server-side repository, regardless of which client created a record.
    biofeedback::api::HttpApiClient secondClient(
        QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    assert(secondClient.login(QStringLiteral("api-client-test"),
                              QStringLiteral("client-password-123"), &error));

    const auto createShared = [&](const QString& collection, const QJsonObject& payload) {
        QJsonObject record;
        assert(secondClient.createResource(collection, payload, record, &error));
        assert(!record.value(QStringLiteral("id")).toString().isEmpty());
        return record;
    };
    const QJsonObject measurement = createShared(QStringLiteral("measurements"), QJsonObject{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("source_client"), QStringLiteral("api-client-regression")},
        {QStringLiteral("sample_count"), 1}
    });
    const QJsonObject trainingSession = createShared(QStringLiteral("training-sessions"), QJsonObject{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("exercise_name"), QStringLiteral("Cross-client contract")}
    });
    const QJsonObject trainingResult = createShared(QStringLiteral("training-results"), QJsonObject{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("training_session_id"), trainingSession.value(QStringLiteral("id")).toString()},
        {QStringLiteral("completed_repetitions"), 1}
    });
    const QJsonObject gameSession = createShared(QStringLiteral("game-sessions"), QJsonObject{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("game_id"), QStringLiteral("contract-game")}
    });
    const QJsonObject gameResult = createShared(QStringLiteral("game-results"), QJsonObject{
        {QStringLiteral("patient_id"), id},
        {QStringLiteral("game_id"), QStringLiteral("contract-game")},
        {QStringLiteral("score"), 42}
    });

    const auto assertVisibleToFirstClient = [&](const QString& collection, const QJsonObject& created) {
        QJsonObject loaded;
        assert(client.getResource(collection, created.value(QStringLiteral("id")).toString(),
                                  loaded, &error));
        assert(loaded.value(QStringLiteral("id")) == created.value(QStringLiteral("id")));
        assert(loaded.value(QStringLiteral("patient_id")).toString() == id);
    };
    assertVisibleToFirstClient(QStringLiteral("measurements"), measurement);
    assertVisibleToFirstClient(QStringLiteral("training-sessions"), trainingSession);
    assertVisibleToFirstClient(QStringLiteral("training-results"), trainingResult);
    assertVisibleToFirstClient(QStringLiteral("game-sessions"), gameSession);
    assertVisibleToFirstClient(QStringLiteral("game-results"), gameResult);

    // Collection listings must expose the same server-side records too.
    const auto assertListedToFirstClient = [&](const QString& collection, const QJsonObject& created) {
        QJsonArray listed;
        assert(client.listResources(collection, listed, &error));
        const QString createdId = created.value(QStringLiteral("id")).toString();
        bool found = false;
        for (const QJsonValue& value : listed) {
            if (value.toObject().value(QStringLiteral("id")).toString() == createdId) {
                found = true;
                break;
            }
        }
        assert(found);
    };
    assertListedToFirstClient(QStringLiteral("measurements"), measurement);
    assertListedToFirstClient(QStringLiteral("training-sessions"), trainingSession);
    assertListedToFirstClient(QStringLiteral("training-results"), trainingResult);
    assertListedToFirstClient(QStringLiteral("game-sessions"), gameSession);
    assertListedToFirstClient(QStringLiteral("game-results"), gameResult);

    assert(!client.deleteResource(QStringLiteral("patients"), id, 2, &error));
    assert(error.contains(QStringLiteral("PATIENT_HAS_REFERENCES")));

    const auto removeShared = [&](const QString& collection, const QJsonObject& record) {
        assert(client.deleteResource(collection, record.value(QStringLiteral("id")).toString(),
                                     record.value(QStringLiteral("version")).toInt(), &error));
    };
    removeShared(QStringLiteral("game-results"), gameResult);
    removeShared(QStringLiteral("game-sessions"), gameSession);
    assert(!client.deleteResource(QStringLiteral("training-sessions"),
                                  trainingSession.value(QStringLiteral("id")).toString(),
                                  trainingSession.value(QStringLiteral("version")).toInt(), &error));
    assert(error.contains(QStringLiteral("TRAINING_SESSION_HAS_RESULTS")));
    removeShared(QStringLiteral("training-results"), trainingResult);
    removeShared(QStringLiteral("training-sessions"), trainingSession);
    removeShared(QStringLiteral("measurements"), measurement);

    assert(client.deleteResource(QStringLiteral("patients"), id, 2, &error));
    assert(client.listResources(QStringLiteral("patients"), items, &error));
    assert(items.isEmpty());

    secondClient.logout();
    client.logout();
    assert(!client.isAuthenticated());

    bool rejectedRemoteHttp = false;
    try {
        biofeedback::api::HttpApiClient unsafe(QUrl(QStringLiteral("http://example.com")));
    } catch (const std::invalid_argument&) {
        rejectedRemoteHttp = true;
    }
    assert(rejectedRemoteHttp);

    server.close();
    std::cout << "api_client_regression: PASS\n";
    return 0;
}
