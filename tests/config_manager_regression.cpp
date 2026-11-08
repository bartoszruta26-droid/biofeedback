#include "core/ConfigManager.hpp"
#include "sensor/SerialProtocolConfig.hpp"

#include <QCoreApplication>
#include <QTemporaryDir>

#include <cassert>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    assert(directory.isValid());

    const std::string path = directory.filePath("biofeedback-config.json").toStdString();

    biofeedback::ConfigManager writer;
    writer.setDefaults();
    assert(writer.save(path, false));

    biofeedback::ConfigManager reader;
    assert(reader.load(path, true));
    assert(reader.getInt("serial.baud_rate", 0) == sensor::protocol::DefaultBaudRate);
    assert(reader.getString("serial.port").size() > 0);
    assert(!reader.isEncryptionEnabled());
    assert(reader.getString("encryption.algorithm") == "AES-256-GCM");
    assert(reader.getString("encryption.key_derivation") == "SHA-256 key normalization (32+ byte external secret)");
    assert(reader.getString("deployment.profile") == "development");
    assert(reader.getString("storage.patient_backend") == "files");
    assert(reader.getString("storage.mariadb.host") == "127.0.0.1");
    assert(reader.getInt("storage.mariadb.port", 0) == 3306);
    reader.setValue("storage.patient_backend", std::string("mariadb"));
    assert(reader.save(path, false));
    biofeedback::ConfigManager mariadbConfig;
    assert(mariadbConfig.load(path, true));
    assert(mariadbConfig.getString("storage.patient_backend") == "mariadb");
    reader.setValue("storage.patient_backend", std::string("invalid"));
    assert(reader.save(path, false));
    biofeedback::ConfigManager invalidStorage;
    assert(!invalidStorage.load(path, true));
    reader.setValue("storage.patient_backend", std::string("files"));
    assert(reader.save(path, false));
    assert(reader.load(path, true));
    assert(reader.getBool("medical.require_authentication", false));
    assert(reader.getInt("devices.pico.baud_rate", 0) == sensor::protocol::DefaultControlBaudRate);
    assert(reader.getInt("devices.pico.uart_tx_pin", -1) == sensor::protocol::PicoUartTxPin);
    assert(reader.getInt("devices.pico.uart_rx_pin", -1) == sensor::protocol::PicoUartRxPin);
    assert(reader.getInt("devices.pico.protocol_version", 0) == sensor::protocol::ControlProtocolVersion);
    assert(!reader.getBool("logging.console_output", true));
    assert(!reader.getBool("logging.diagnostics_enabled", true));
    assert(reader.getInt("encryption.iterations", -1) == -1);
    assert(reader.getInt("sensor.sample_rate_hz", 0) == static_cast<int>(sensor::protocol::DefaultSampleRateHz));
    reader.setValue("deployment.profile", std::string("production"));
    assert(reader.save(path, false));
    biofeedback::ConfigManager production;
    assert(!production.load(path, true)); // production must fail closed without encryption

    reader.setValue("deployment.profile", std::string("produciton"));
    assert(reader.save(path, false));
    biofeedback::ConfigManager invalidProfile;
    assert(!invalidProfile.load(path, true));

    reader.setValue("deployment.profile", std::string("development"));
    reader.setValue("custom.label", std::string("calibration station"));
    assert(reader.save(path, false));

    biofeedback::ConfigManager reloaded;
    assert(reloaded.load(path, true));
    assert(reloaded.getString("custom.label") == "calibration station");
    assert(reloaded.getStatistics().loadsSuccessful.load() == 1);

    std::cout << "config_manager_regression: PASS\n";
    return 0;
}
