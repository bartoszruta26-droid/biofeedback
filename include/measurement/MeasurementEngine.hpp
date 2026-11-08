#pragma once

#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
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
    bool hasTimestamp{false};
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
    // First sequence to assign; initialize from the recovered journal on server restart.
    std::uint64_t initialSequence{1};
};

class MeasurementEngine final {
public:
    using SampleCallback = std::function<void(const MeasurementSample&)>;
    using CallbackToken = std::uint64_t;
    // A commit hook must durably accept a sample before it enters history or
    // reaches subscribers. It runs under the engine mutex and must not re-enter
    // this engine. Returning false (or throwing) rejects the sample.
    using CommitHook = std::function<bool(const MeasurementSample&)>;

    explicit MeasurementEngine(MeasurementEngineConfig config = {});
    MeasurementSample ingest(const MeasurementInput& input);
    bool ingest(const MeasurementInput& input, MeasurementSample& output);
    void setCommitHook(CommitHook hook);
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
    struct CallbackEntry {
        explicit CallbackEntry(SampleCallback value) : callback(std::move(value)) {}
        SampleCallback callback;
        std::mutex mutex;
        std::condition_variable idle;
        bool active{true};
        std::size_t inFlight{0};
    };
    std::map<CallbackToken, std::shared_ptr<CallbackEntry>> m_callbacks;
    CommitHook m_commitHook;
    std::uint64_t m_nextSequence{1};
    CallbackToken m_nextToken{1};
};

} // namespace biofeedback::measurement
