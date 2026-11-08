#include "api/HttpApiServer.hpp"
#include "core/ConfigManager.hpp"
#include "core/RuntimePaths.hpp"
#include "data/JsonResourceRepository.hpp"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QTextStream>

#include <cstdlib>
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

    biofeedback::ConfigManager config;
    const QString configPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("config.json"));
    if (configPath.isEmpty() || !config.load(configPath.toStdString())) {
        config.setDefaults();
    }

    const bool encryptionEnabled = config.getBool("encryption.enabled", false);
    const QString encryptionKey = QString::fromUtf8(qgetenv("BIOFEEDBACK_ENCRYPTION_KEY"));
    const QString patientBackend = QString::fromStdString(
        config.getString("storage.patient_backend", "files"));

    std::shared_ptr<biofeedback::data::JsonResourceRepository> resources;
    if (patientBackend == QStringLiteral("files")) {
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
    if (!server.listen(QHostAddress::LocalHost, static_cast<quint16>(portValue))) {
        QTextStream(stderr) << "Cannot listen on loopback TCP port " << portValue << ".\n";
        return 1;
    }
    QTextStream(stdout) << "Biofeedback API listening on http://127.0.0.1:"
                        << server.serverPort() << "\n";
    return app.exec();
}
