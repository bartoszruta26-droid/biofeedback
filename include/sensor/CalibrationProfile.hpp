#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace sensor {

struct CalibrationProfile {
    static constexpr int CurrentSchemaVersion = 1;

    int schemaVersion{CurrentSchemaVersion};
    std::string deviceId;
    std::string firmwareVersion;
    std::string sensorModel{"HX711"};
    std::string unit{"N"};
    double zeroOffsetRaw{0.0};
    double scaleFactor{0.0};
    double referenceValue{0.0};
    int32_t referenceRaw{0};
    int sampleCount{0};
    std::string createdAtUtc;

    bool isValidFor(const std::string& expectedDeviceId) const noexcept;
};

class CalibrationProfileStore {
public:
    static bool save(const std::string& path,
                     const CalibrationProfile& profile,
                     std::string* error = nullptr);

    static std::optional<CalibrationProfile> load(
        const std::string& path,
        const std::string& expectedDeviceId,
        std::string* error = nullptr);
};

} // namespace sensor
