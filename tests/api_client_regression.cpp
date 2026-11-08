#include "api/HttpApiClient.hpp"
#include "api/HttpApiServer.hpp"
#include "core/Authentication.hpp"
#include "data/JsonResourceRepository.hpp"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

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
    assert(server.listen(QHostAddress::LocalHost, 0));

    biofeedback::api::HttpApiClient client(
        QUrl(QStringLiteral("http://127.0.0.1:%1").arg(server.serverPort())));
    QString error;
    assert(client.login(QStringLiteral("api-client-test"),
                        QStringLiteral("client-password-123"), &error));
    assert(client.isAuthenticated());

    QJsonObject created;
    assert(client.createResource(QStringLiteral("patients"), QJsonObject{
        {QStringLiteral("reference"), QStringLiteral("synthetic-patient")},
        {QStringLiteral("display_name"), QStringLiteral("Test Patient")}
    }, created, &error));
    const QString id = created.value(QStringLiteral("id")).toString();
    assert(!id.isEmpty());
    assert(created.value(QStringLiteral("version")).toInt() == 1);

    QJsonArray items;
    assert(client.listResources(QStringLiteral("patients"), items, &error));
    assert(items.size() == 1);

    QJsonObject updated;
    assert(client.updateResource(QStringLiteral("patients"), id, 1, QJsonObject{
        {QStringLiteral("display_name"), QStringLiteral("Updated Patient")}
    }, updated, &error));
    assert(updated.value(QStringLiteral("version")).toInt() == 2);

    assert(!client.updateResource(QStringLiteral("patients"), id, 1, QJsonObject{
        {QStringLiteral("display_name"), QStringLiteral("Stale Update")}
    }, updated, &error));
    assert(error.contains(QStringLiteral("VERSION_CONFLICT")));

    assert(client.deleteResource(QStringLiteral("patients"), id, 2, &error));
    assert(client.listResources(QStringLiteral("patients"), items, &error));
    assert(items.isEmpty());

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
