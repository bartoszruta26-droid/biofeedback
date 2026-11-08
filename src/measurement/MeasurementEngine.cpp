#include "measurement/MeasurementEngine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

namespace biofeedback::measurement {
namespace {
std::int64_t nowUnixMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
    });
    return value;
}
}

const char* qualityName(Quality quality) noexcept {
    switch (quality) {
    case Quality::Valid: return "valid";
    case Quality::SensorRejected: return "sensor_rejected";
    case Quality::NonFiniteValue: return "non_finite_value";
    case Quality::MissingTimestamp: return "missing_timestamp";
    case Quality::InvalidUnit: return "invalid_unit";
    case Quality::InvalidSource: return "invalid_source";
    case Quality::OutsideConfiguredRange: return "outside_configured_range";
    }
    return "unknown";
}

MeasurementEngine::MeasurementEngine(MeasurementEngineConfig config)
    : m_config(config) {
    if (m_config.historyCapacity == 0) m_config.historyCapacity = 1;
    if (!std::isfinite(m_config.minimumValue) ||
        !std::isfinite(m_config.maximumValue) ||
        m_config.minimumValue > m_config.maximumValue) {
        m_config.minimumValue = -1.0e12;
        m_config.maximumValue = 1.0e12;
    }
}

bool MeasurementEngine::normalizeUnit(const std::string& inputUnit,
                                      std::string& canonicalUnit,
                                      double& multiplier) {
    const std::string unit = lowerAscii(inputUnit);
    multiplier = 1.0;
    if (unit == "n" || unit == "newton" || unit == "newtons") canonicalUnit = "N";
    else if (unit == "kg" || unit == "kilogram" || unit == "kilograms") canonicalUnit = "kg";
    else if (unit == "g" || unit == "gram" || unit == "grams") { canonicalUnit = "kg"; multiplier = 0.001; }
    else if (unit == "mg") { canonicalUnit = "kg"; multiplier = 0.000001; }
    else if (unit == "m" || unit == "meter" || unit == "metre") canonicalUnit = "m";
    else if (unit == "cm") { canonicalUnit = "m"; multiplier = 0.01; }
    else if (unit == "mm") { canonicalUnit = "m"; multiplier = 0.001; }
    else if (unit == "v" || unit == "volt" || unit == "volts") canonicalUnit = "V";
    else if (unit == "mv") { canonicalUnit = "V"; multiplier = 0.001; }
    else if (unit == "uv") { canonicalUnit = "V"; multiplier = 0.000001; }
    else if (unit == "count" || unit == "counts" || unit == "raw") canonicalUnit = "count";
    else return false;
    return true;
}

MeasurementSample MeasurementEngine::ingest(const MeasurementInput& input) {
    MeasurementSample output;
    (void)ingest(input, output);
    return output;
}

void MeasurementEngine::setCommitHook(CommitHook hook) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_commitHook = std::move(hook);
}

bool MeasurementEngine::ingest(const MeasurementInput& input, MeasurementSample& output) {
    MeasurementSample sample;
    sample.timestampUnixMs = nowUnixMilliseconds();
    sample.deviceTimestampMs = input.deviceTimestampMs;
    sample.rawValue = input.rawValue;
    sample.value = input.value;
    sample.source = input.source;

    double multiplier = 1.0;
    if (input.source.empty()) {
        sample.quality = Quality::InvalidSource;
    } else if (!normalizeUnit(input.unit, sample.unit, multiplier)) {
        sample.quality = Quality::InvalidUnit;
    } else if (!input.sensorValid) {
        sample.quality = Quality::SensorRejected;
    } else if (!input.hasTimestamp) {
        sample.quality = Quality::MissingTimestamp;
    } else if (!std::isfinite(input.value)) {
        sample.quality = Quality::NonFiniteValue;
    } else {
        sample.value = input.value * multiplier;
        if (!std::isfinite(sample.value)) {
            sample.quality = Quality::NonFiniteValue;
        } else if (sample.value < m_config.minimumValue ||
                   sample.value > m_config.maximumValue) {
            sample.quality = Quality::OutsideConfiguredRange;
        } else {
            sample.quality = Quality::Valid;
        }
    }

    std::vector<std::shared_ptr<CallbackEntry>> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        sample.sequence = m_nextSequence;
        if (m_commitHook) {
            bool committed = false;
            try {
                committed = m_commitHook(sample);
            } catch (...) {
                committed = false;
            }
            if (!committed) {
                output = MeasurementSample{};
                return false;
            }
        }
        ++m_nextSequence;
        m_history.push_back(sample);
        if (m_history.size() > m_config.historyCapacity)
            m_history.erase(m_history.begin(), m_history.begin() +
                            static_cast<std::ptrdiff_t>(m_history.size() - m_config.historyCapacity));
        callbacks.reserve(m_callbacks.size());
        for (const auto& entry : m_callbacks) callbacks.push_back(entry.second);
    }
    for (const auto& entry : callbacks) {
        SampleCallback callback;
        {
            std::lock_guard<std::mutex> lock(entry->mutex);
            if (!entry->active) continue;
            callback = entry->callback;
            ++entry->inFlight;
        }
        try { callback(sample); } catch (...) { /* Subscriber errors do not stop acquisition. */ }
        {
            std::lock_guard<std::mutex> lock(entry->mutex);
            --entry->inFlight;
            if (entry->inFlight == 0) entry->idle.notify_all();
        }
    }
    output = sample;
    return true;
}

MeasurementEngine::CallbackToken MeasurementEngine::subscribe(SampleCallback callback) {
    if (!callback) return 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    const CallbackToken token = m_nextToken++;
    m_callbacks.emplace(token, std::make_shared<CallbackEntry>(std::move(callback)));
    return token;
}

void MeasurementEngine::unsubscribe(CallbackToken token) {
    if (token == 0) return;
    std::shared_ptr<CallbackEntry> entry;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        const auto it = m_callbacks.find(token);
        if (it == m_callbacks.end()) return;
        entry = it->second;
        m_callbacks.erase(it);
    }
    // Return only after all callbacks already admitted for this subscriber finish.
    std::unique_lock<std::mutex> lock(entry->mutex);
    entry->active = false;
    entry->idle.wait(lock, [&entry] { return entry->inFlight == 0; });
}

std::vector<MeasurementSample> MeasurementEngine::history() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_history;
}

void MeasurementEngine::clearHistory() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_history.clear();
}

} // namespace biofeedback::measurement
