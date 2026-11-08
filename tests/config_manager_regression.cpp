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
    assert(reader.getBool("medical.require_authentication", false));
    assert(reader.getInt("encryption.iterations", -1) == -1);
    assert(reader.getInt("sensor.sample_rate_hz", 0) == static_cast<int>(sensor::protocol::DefaultSampleRateHz));
    reader.setValue("deployment.profile", std::string("production"));
    assert(reader.save(path, false));
    biofeedback::ConfigManager production;
    assert(!production.load(path, true)); // production must fail closed without encryption


    reader.setValue("custom.label", std::string("calibration station"));
    assert(reader.save(path, false));

    biofeedback::ConfigManager reloaded;
    assert(reloaded.load(path, true));
    assert(reloaded.getString("custom.label") == "calibration station");
    assert(reloaded.getStatistics().loadsSuccessful.load() == 1);

    std::cout << "config_manager_regression: PASS
";
    return 0;
}
