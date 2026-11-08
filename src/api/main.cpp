#include "api/HttpApiServer.hpp"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("biofeedback_api");
    QCoreApplication::setApplicationVersion("0.2.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Loopback HTTP API with authentication for Biofeedback");
    parser.addHelpOption(); parser.addVersionOption();
    QCommandLineOption portOption({"p", "port"}, "TCP port (default: 8765)", "port", "8765");
    parser.addOption(portOption); parser.process(app);
    bool ok = false;
    const uint portValue = parser.value(portOption).toUInt(&ok);
    if (!ok || portValue == 0 || portValue > 65535) {
        QTextStream(stderr) << "Invalid TCP port. Expected 1..65535.\n"; return 2;
    }
    biofeedback::api::HttpApiServer server;
    if (!server.listen(QHostAddress::LocalHost, static_cast<quint16>(portValue))) {
        QTextStream(stderr) << "Cannot listen on loopback TCP port " << portValue << ".\n"; return 1;
    }
    QTextStream(stdout) << "Biofeedback API listening on http://127.0.0.1:" << server.serverPort() << "\n";
    return app.exec();
}
