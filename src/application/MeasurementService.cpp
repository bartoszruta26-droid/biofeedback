#include "application/MeasurementService.hpp"

#include <stdexcept>
#include <utility>

namespace biofeedback::application {

MeasurementService::MeasurementService(
    std::shared_ptr<measurement::MeasurementEngine> engine)
    : m_engine(std::move(engine)) {
    if (!m_engine) {
        throw std::invalid_argument("MeasurementService requires a measurement engine");
    }
}

void MeasurementService::start() noexcept {
    m_running.store(true, std::memory_order_release);
}

void MeasurementService::stop() noexcept {
    m_running.store(false, std::memory_order_release);
}

bool MeasurementService::isRunning() const noexcept {
    return m_running.load(std::memory_order_acquire);
}

bool MeasurementService::ingest(const measurement::MeasurementInput& input,
                                measurement::MeasurementSample& output) {
    if (!isRunning()) return false;
    return m_engine->ingest(input, output);
}

void MeasurementService::setCommitHook(measurement::MeasurementEngine::CommitHook hook) {
    m_engine->setCommitHook(std::move(hook));
}

MeasurementService::CallbackToken MeasurementService::subscribe(SampleCallback callback) {
    return m_engine->subscribe(std::move(callback));
}

void MeasurementService::unsubscribe(CallbackToken token) {
    m_engine->unsubscribe(token);
}

std::vector<measurement::MeasurementSample> MeasurementService::history() const {
    return m_engine->history();
}

void MeasurementService::clearHistory() {
    m_engine->clearHistory();
}

} // namespace biofeedback::application
