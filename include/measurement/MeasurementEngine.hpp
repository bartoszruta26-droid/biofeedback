#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace biofeedback::measurement {

enum class Quality {
    Valid,
    SensorRejected,
    NonFiniteValue,
    MissingTimestamp,
    InvalidUnit,
    InvalidSource,
    OutsideConfiguredRange
};

const char* qualityName(Quality quality) noexcept;

struct MeasurementInput {
    std::uint32_t deviceTimestampMs{0};
    std::int32_t rawValue{0};
    double value{0.0};
    bool sensorValid{false};
    std::string unit;
    std::string source;
};

struct MeasurementSample {
    std::uint64_t sequence{0};
    std::int64_t timestampUnixMs{0};
    std::uint32_t deviceTimestampMs{0};
    std::int32_t rawValue{0};
    double value{0.0};
    std::string unit;
    std::string source;
    Quality quality{Quality::SensorRejected};

    bool isValid() const noexcept { return quality == Quality::Valid; }
};

struct MeasurementEngineConfig {
    double minimumValue{-1.0e12};
    double maximumValue{1.0e12};
    std::size_t historyCapacity{512};
};

class MeasurementEngine final {
public:
    using SampleCallback = std::function<void(const MeasurementSample&)>;
    using CallbackToken = std::uint64_t;

    explicit MeasurementEngine(MeasurementEngineConfig config = {});
    MeasurementSample ingest(const MeasurementInput& input);
    CallbackToken subscribe(SampleCallback callback);
    void unsubscribe(CallbackToken token);
    std::vector<MeasurementSample> history() const;
    void clearHistory();

    static bool normalizeUnit(const std::string& inputUnit,
                              std::string& canonicalUnit,
                              double& multiplier);

private:
    MeasurementEngineConfig m_config;
    mutable std::mutex m_mutex;
    std::vector<MeasurementSample> m_history;
    std::map<CallbackToken, SampleCallback> m_callbacks;
    std::uint64_t m_nextSequence{1};
    CallbackToken m_nextToken{1};
};

} // namespace biofeedback::measurement
