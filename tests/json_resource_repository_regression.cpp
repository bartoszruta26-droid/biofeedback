#include "data/JsonResourceRepository.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <cassert>
#include <iostream>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    assert(directory.isValid());

    const QString key = QStringLiteral("0123456789abcdef0123456789abcdef");
    const QString root = directory.filePath(QStringLiteral("shared-store"));
    biofeedback::data::JsonResourceRepository repository(root, key, true);
    assert(repository.isReady());

    QJsonObject patientPayload{
        {QStringLiteral("firstName"), QStringLiteral("Test")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")},
        {QStringLiteral("pesel"), QStringLiteral("90010112345")}
    };
    QJsonObject created;
    QString error;
    assert(repository.create(QStringLiteral("patients"), patientPayload, created, &error));
    const QString id = created.value(QStringLiteral("id")).toString();
    assert(!id.isEmpty());
    assert(created.value(QStringLiteral("version")).toInt() == 1);
    assert(created.value(QStringLiteral("pesel")).toString() == QStringLiteral("90010112345"));

    const QString recordPath = QDir(root).filePath(
        QStringLiteral("patients/") + id + QStringLiteral(".json"));
    QFile encryptedFile(recordPath);
    assert(encryptedFile.open(QIODevice::ReadOnly));
    const QByteArray encryptedBytes = encryptedFile.readAll();
    assert(encryptedBytes.startsWith("BFENC:v1:"));
    assert(!encryptedBytes.contains("90010112345"));

    biofeedback::data::JsonResourceRepository restarted(root, key, true);
    QJsonObject loaded;
    assert(restarted.get(QStringLiteral("patients"), id, loaded, &error));
    assert(loaded.value(QStringLiteral("lastName")).toString() == QStringLiteral("Patient"));

    const QJsonArray patients = restarted.list(QStringLiteral("patients"), &error);
    assert(patients.size() == 1);

    QJsonObject updatePayload{
        {QStringLiteral("firstName"), QStringLiteral("Updated")},
        {QStringLiteral("lastName"), QStringLiteral("Patient")}
    };
    QJsonObject updated;
    assert(restarted.update(QStringLiteral("patients"), id, 1, updatePayload, updated, &error));
    assert(updated.value(QStringLiteral("version")).toInt() == 2);
    assert(!restarted.update(QStringLiteral("patients"), id, 1, updatePayload, updated, &error));
    assert(error == QStringLiteral("VERSION_CONFLICT"));

    assert(!restarted.get(QStringLiteral("../patients"), id, loaded, &error));
    assert(error == QStringLiteral("UNSUPPORTED_COLLECTION"));
    assert(!restarted.get(QStringLiteral("patients"), QStringLiteral("../../users"), loaded, &error));
    assert(error == QStringLiteral("INVALID_RESOURCE_ID"));

    assert(restarted.remove(QStringLiteral("patients"), id, 2, &error));
    assert(!restarted.get(QStringLiteral("patients"), id, loaded, &error));
    assert(error == QStringLiteral("RESOURCE_NOT_FOUND"));

    biofeedback::data::JsonResourceRepository missingKey(root, QString(), true);
    assert(!missingKey.isReady());
    assert(missingKey.initializationError() == QStringLiteral("ENCRYPTION_KEY_REQUIRED"));

    std::cout << "json_resource_repository_regression: PASS\n";
    return 0;
}
