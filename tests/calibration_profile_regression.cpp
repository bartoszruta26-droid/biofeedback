#include "sensor/CalibrationProfile.hpp"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

int main()
{
    const auto directory = std::filesystem::temp_directory_path() / "biofeedback-calibration-regression";
    std::filesystem::create_directories(directory);
    const auto path = (directory / "device-123.json").string();

    sensor::CalibrationProfile profile;
    profile.deviceId = "device-123";
    profile.firmwareVersion = "1.2.0";
    profile.zeroOffsetRaw = 100000.0;
    profile.scaleFactor = 0.00042;
    profile.referenceValue = 9.81;
    profile.referenceRaw = 123357;
    profile.sampleCount = 21;
    profile.createdAtUtc = "2026-11-07T00:00:00Z";

    std::string error;
    assert(profile.isValidFor("device-123"));
    const auto oddMedian = sensor::CalibrationMath::medianRaw({10, 100, 20});
    const auto evenMedian = sensor::CalibrationMath::medianRaw({10, 20, 30, 40});
    assert(oddMedian && *oddMedian == 20.0);
    assert(evenMedian && *evenMedian == 25.0);
    assert(!sensor::CalibrationMath::medianRaw({}).has_value());
    const auto scale = sensor::CalibrationMath::scaleFromReference(100000.0, 123357, 9.81);
    assert(scale && *scale > 0.0);
    assert(!sensor::CalibrationMath::scaleFromReference(100.0, 100, 9.81).has_value());
    const auto converted = sensor::CalibrationMath::convertRaw(123357, 100000.0, *scale);
    assert(converted && std::abs(*converted - 9.81) < 1e-9);
    assert(!profile.isValidFor("different-device"));
    assert(sensor::CalibrationProfileStore::save(path, profile, &error));

    const auto loaded = sensor::CalibrationProfileStore::load(path, "device-123", &error);
    assert(loaded.has_value());
    assert(loaded->schemaVersion == sensor::CalibrationProfile::CurrentSchemaVersion);
    assert(loaded->deviceId == profile.deviceId);
    assert(loaded->zeroOffsetRaw == profile.zeroOffsetRaw);
    assert(loaded->scaleFactor == profile.scaleFactor);

    assert(!sensor::CalibrationProfileStore::load(path, "different-device", &error).has_value());

    profile.schemaVersion = 99;
    assert(!sensor::CalibrationProfileStore::save(path, profile, &error));

    std::filesystem::remove_all(directory);
    std::cout << "calibration_profile_regression: PASS\\n";
    return 0;
}
