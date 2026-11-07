#pragma once

#include "measurement/MeasurementEngine.hpp"

#include <atomic>
#include <memory>

namespace biofeedback::application {

// Application-facing lifecycle boundary for the shared measurement pipeline.
// The same engine may be shared with acquisition adapters (for example, serial).
class MeasurementService final {
public:
    using SampleCallback = measurement::MeasurementEngine::SampleCallback;
    using CallbackToken = measurement::MeasurementEngine::CallbackToken;

    explicit MeasurementService(std::shared_ptr<measurement::MeasurementEngine> engine);

    MeasurementService(const MeasurementService&) = delete;
    MeasurementService& operator=(const MeasurementService&) = delete;

    void start() noexcept;
    void stop() noexcept;
    bool isRunning() const noexcept;

    // Returns false when the service is stopped. A true return means the input
    // was admitted; the resulting sample may still carry a validation failure.
    bool ingest(const measurement::MeasurementInput& input,
                measurement::MeasurementSample& output);

    CallbackToken subscribe(SampleCallback callback);
    void unsubscribe(CallbackToken token);
    std::vector<measurement::MeasurementSample> history() const;
    void clearHistory();

private:
    std::shared_ptr<measurement::MeasurementEngine> m_engine;
    std::atomic<bool> m_running{false};
};

} // namespace biofeedback::application
