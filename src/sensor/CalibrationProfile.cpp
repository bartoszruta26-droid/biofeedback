#include "sensor/CalibrationProfile.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <utility>
#include <cstdio>
#include <cerrno>
#include <system_error>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace sensor {
namespace {
using json = nlohmann::json;

void setError(std::string* error, const std::string& message)
{
    if (error) *error = message;
}

bool atomicReplace(const std::filesystem::path& temporary,
                   const std::filesystem::path& target,
                   std::string* error)
{
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), target.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        setError(error, std::system_error(
            static_cast<int>(GetLastError()), std::system_category()).what());
        return false;
    }
#else
    if (std::rename(temporary.c_str(), target.c_str()) != 0) {
        setError(error, std::system_error(errno, std::generic_category()).what());
        return false;
    }
#endif
    return true;
}

json toJson(const CalibrationProfile& profile)
{
    return json{
        {"schemaVersion", profile.schemaVersion},
        {"deviceId", profile.deviceId},
        {"firmwareVersion", profile.firmwareVersion},
        {"sensorModel", profile.sensorModel},
        {"unit", profile.unit},
        {"zeroOffsetRaw", profile.zeroOffsetRaw},
        {"scaleFactor", profile.scaleFactor},
        {"referenceValue", profile.referenceValue},
        {"referenceRaw", profile.referenceRaw},
        {"sampleCount", profile.sampleCount},
        {"createdAtUtc", profile.createdAtUtc}
    };
}
}

std::optional<double> CalibrationMath::medianRaw(const std::vector<int32_t>& samples) noexcept
{
    if (samples.empty() || samples.size() > 1000) return std::nullopt;
    std::vector<int32_t> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const std::size_t middle = sorted.size() / 2;
    if (sorted.size() % 2 != 0) return static_cast<double>(sorted[middle]);
    return (static_cast<double>(sorted[middle - 1]) + static_cast<double>(sorted[middle])) * 0.5;
}

std::optional<double> CalibrationMath::scaleFromReference(double zeroOffsetRaw,
                                                           int32_t referenceRaw,
                                                           double referenceValue) noexcept
{
    const double delta = static_cast<double>(referenceRaw) - zeroOffsetRaw;
    if (!std::isfinite(zeroOffsetRaw) || !std::isfinite(referenceValue) ||
        delta <= 0.0 || referenceValue <= 0.0) return std::nullopt;
    const double scale = referenceValue / delta;
    if (!std::isfinite(scale) || scale <= 0.0) return std::nullopt;
    return scale;
}

std::optional<double> CalibrationMath::convertRaw(int32_t raw,
                                                   double zeroOffsetRaw,
                                                   double scaleFactor) noexcept
{
    if (!std::isfinite(zeroOffsetRaw) || !std::isfinite(scaleFactor) || scaleFactor <= 0.0)
        return std::nullopt;
    const double result = (static_cast<double>(raw) - zeroOffsetRaw) * scaleFactor;
    if (!std::isfinite(result)) return std::nullopt;
    return result;
}

bool CalibrationProfile::isValidFor(const std::string& expectedDeviceId) const noexcept
{
    if (schemaVersion != CurrentSchemaVersion || deviceId.empty() ||
        deviceId != expectedDeviceId || unit != "g" ||
        !std::isfinite(zeroOffsetRaw) || !std::isfinite(scaleFactor) ||
        scaleFactor <= 0.0 || !std::isfinite(referenceValue) ||
        referenceValue <= 0.0 || sampleCount < 1 || sampleCount > 1000 ||
        createdAtUtc.empty()) return false;

    const double rawDelta = static_cast<double>(referenceRaw) - zeroOffsetRaw;
    if (rawDelta <= 0.0) return false;
    const double expectedScale = referenceValue / rawDelta;
    const double scaleTolerance = std::max(1e-12, std::abs(expectedScale) * 1e-6);
    return std::abs(scaleFactor - expectedScale) <= scaleTolerance;
}

bool CalibrationProfileStore::save(const std::string& path,
                                    const CalibrationProfile& profile,
                                    std::string* error)
{
    if (path.empty()) {
        setError(error, "Calibration profile path is empty");
        return false;
    }
    if (!profile.isValidFor(profile.deviceId)) {
        setError(error, "Calibration profile is invalid or unsupported");
        return false;
    }

    const std::filesystem::path target(path);
    const std::filesystem::path temporary = target.string() + ".tmp";
    try {
        if (target.has_parent_path()) {
            std::filesystem::create_directories(target.parent_path());
        }
        {
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
            if (!output) {
                setError(error, "Cannot open temporary calibration profile for writing");
                return false;
            }
            output << toJson(profile).dump(2) << '\n';
            output.flush();
            if (!output.good()) {
                setError(error, "Failed while writing calibration profile");
                output.close();
                std::filesystem::remove(temporary);
                return false;
            }
        }
        if (!atomicReplace(temporary, target, error)) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
        if (error) error->clear();
        return true;
    } catch (const std::exception& exception) {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        setError(error, exception.what());
        return false;
    }
}

std::optional<CalibrationProfile> CalibrationProfileStore::load(
    const std::string& path,
    const std::string& expectedDeviceId,
    std::string* error)
{
    if (path.empty() || expectedDeviceId.empty()) {
        setError(error, "Profile path and expected device identifier are required");
        return std::nullopt;
    }

    try {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            setError(error, "Cannot open calibration profile");
            return std::nullopt;
        }
        const json document = json::parse(input);
        CalibrationProfile profile;
        profile.schemaVersion = document.at("schemaVersion").get<int>();
        profile.deviceId = document.at("deviceId").get<std::string>();
        profile.firmwareVersion = document.value("firmwareVersion", std::string{});
        profile.sensorModel = document.value("sensorModel", std::string{"HX711"});
        profile.unit = document.at("unit").get<std::string>();
        profile.zeroOffsetRaw = document.at("zeroOffsetRaw").get<double>();
        profile.scaleFactor = document.at("scaleFactor").get<double>();
        profile.referenceValue = document.at("referenceValue").get<double>();
        profile.referenceRaw = document.at("referenceRaw").get<int32_t>();
        profile.sampleCount = document.at("sampleCount").get<int>();
        profile.createdAtUtc = document.at("createdAtUtc").get<std::string>();

        if (!profile.isValidFor(expectedDeviceId)) {
            setError(error, "Calibration profile version, values, or device identifier do not match");
            return std::nullopt;
        }
        if (error) error->clear();
        return profile;
    } catch (const std::exception& exception) {
        setError(error, exception.what());
        return std::nullopt;
    }
}

} // namespace sensor
