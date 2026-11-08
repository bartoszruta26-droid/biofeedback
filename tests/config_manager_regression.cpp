#include "core/ConfigManager.hpp"

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
    assert(reader.getInt("serial.baud_rate", 0) == 9600);
    assert(reader.getString("serial.port").size() > 0);
    assert(reader.isEncryptionEnabled());

    reader.setValue("custom.label", std::string("calibration station"));
    assert(reader.save(path, false));

    biofeedback::ConfigManager reloaded;
    assert(reloaded.load(path, true));
    assert(reloaded.getString("custom.label") == "calibration station");
    assert(reloaded.getStatistics().loadsSuccessful.load() == 1);

    std::cout << "config_manager_regression: PASS\n";
    return 0;
}
