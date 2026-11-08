#include "api/HttpApiServer.hpp"
#include "core/RuntimePaths.hpp"
#include "data/JsonResourceRepository.hpp"
#include "application/AcquisitionService.hpp"
#include "application/MeasurementService.hpp"
#include "application/DeviceService.hpp"
#include "measurement/MeasurementEngine.hpp"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <memory>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Biofeedback");
    QCoreApplication::setApplicationName("Biofeedback");
    QCoreApplication::setApplicationVersion("0.2.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Biofeedback shared data API (loopback by default)");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption({"p", "port"}, "TCP port (default: 8765)", "port", "8765");
    parser.addOption(portOption);
    parser.process(app);

    bool ok = false;
    const uint portValue = parser.value(portOption).toUInt(&ok);
    if (!ok || portValue == 0 || portValue > 65535) {
        QTextStream(stderr) << "Invalid TCP port. Expected 1..65535.\n";
        return 2;
    }

    const QString configPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("config.json"));
    bool configReady = false;
    bool encryptionEnabled = true;
    QString patientBackend = QStringLiteral("unavailable");
    QFile configFile(configPath);
    if (!configPath.isEmpty() && configFile.open(QIODevice::ReadOnly)) {
        QJsonParseError parseError;
        const QJsonDocument configDocument = QJsonDocument::fromJson(configFile.readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError && configDocument.isObject()) {
            const QJsonObject root = configDocument.object();
            const QJsonObject encryption = root.value(QStringLiteral("encryption")).toObject();
            const QJsonObject storage = root.value(QStringLiteral("storage")).toObject();
            if (root.contains(QStringLiteral("encryption")) && root.contains(QStringLiteral("storage")) &&
                encryption.value(QStringLiteral("enabled")).isBool() &&
                storage.value(QStringLiteral("patient_backend")).isString()) {
                configReady = true;
                encryptionEnabled = encryption.value(QStringLiteral("enabled")).toBool();
                patientBackend = storage.value(QStringLiteral("patient_backend")).toString();
            }
        }
    }
    const QString encryptionKey = QString::fromUtf8(qgetenv("BIOFEEDBACK_ENCRYPTION_KEY"));

    std::shared_ptr<biofeedback::data::JsonResourceRepository> resources;
    if (configReady && patientBackend == QStringLiteral("files")) {
        resources = std::make_shared<biofeedback::data::JsonResourceRepository>(
            QDir(biofeedback::runtime::appDataDirectory()).filePath(QStringLiteral("shared-data")),
            encryptionKey, encryptionEnabled);
        if (!resources->isReady()) {
            QTextStream(stderr) << "Shared resource storage is unavailable: "
                                << resources->initializationError()
                                << ". Data endpoints will fail closed.\n";
        }
    } else {
        QTextStream(stderr) << "Configured patient backend is not available in this build; "
                               "resource endpoints will fail closed.\n";
    }

    biofeedback::api::HttpApiServer server(
        biofeedback::runtime::ensureUserConfigFile(QStringLiteral("users.json")),
        resources,
        encryptionEnabled ? encryptionKey : QString());
    auto measurementEngine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    auto measurementService = std::make_shared<biofeedback::application::MeasurementService>(measurementEngine);
    auto acquisitionService = std::make_shared<biofeedback::application::AcquisitionService>(measurementService);
    server.setAcquisitionService(acquisitionService);
    server.setDeviceService(std::make_shared<biofeedback::application::DeviceService>());

    if (!server.listen(QHostAddress::LocalHost, static_cast<quint16>(portValue))) {
        QTextStream(stderr) << "Cannot listen on loopback TCP port " << portValue << ".\n";
        return 1;
    }
    QTextStream(stdout) << "Biofeedback API listening on http://127.0.0.1:"
                        << server.serverPort() << "\n";
    return app.exec();
}
