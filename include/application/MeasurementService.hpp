#pragma once

#include "measurement/MeasurementEngine.hpp"

#include <atomic>
#include <memory>
#include <vector>

namespace biofeedback::application {

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
