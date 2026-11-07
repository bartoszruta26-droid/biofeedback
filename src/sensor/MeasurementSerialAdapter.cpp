#include "sensor/MeasurementSerialAdapter.hpp"

#include <utility>

namespace sensor {

MeasurementSerialAdapter::MeasurementSerialAdapter(
    SerialCommunication& serial,
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine,
    std::string source,
    std::string unit)
    : m_serial(serial),
      m_engine(std::move(engine)),
      m_source(std::move(source)),
      m_unit(std::move(unit)) {
    if (!m_engine || m_source.empty() || m_unit.empty()) return;

    m_active = std::make_shared<std::atomic<bool>>(true);

    const std::weak_ptr<biofeedback::measurement::MeasurementEngine> weakEngine = m_engine;
    const std::string sourceCopy = m_source;
    const std::string unitCopy = m_unit;
    const auto active = m_active;
    m_token = m_serial.addDataCallback(
        [weakEngine, sourceCopy, unitCopy, active](const SensorData& data) {
            if (!active->load()) return;
            const auto engineLocked = weakEngine.lock();
            if (!engineLocked) return;

            biofeedback::measurement::MeasurementInput input;
            input.deviceTimestampMs = data.timestamp;
            input.rawValue = data.value;
            input.value = unitCopy == "count" ? static_cast<double>(data.value)
                                               : data.calibratedValue;
            input.sensorValid = data.isValid;
            input.unit = unitCopy;
            input.source = sourceCopy;
            engineLocked->ingest(input);
        });
}

MeasurementSerialAdapter::~MeasurementSerialAdapter() {
    if (m_active) m_active->store(false);
    if (m_token != 0) m_serial.removeDataCallback(m_token);
}

biofeedback::measurement::MeasurementSample MeasurementSerialAdapter::processSample(const SensorData& data) {
    if (!m_engine) return {};
    biofeedback::measurement::MeasurementInput input;
    input.deviceTimestampMs = data.timestamp;
    input.rawValue = data.value;
    input.value = m_unit == "count" ? static_cast<double>(data.value) : data.calibratedValue;
    input.sensorValid = data.isValid;
    input.unit = m_unit;
    input.source = m_source;
    return m_engine->ingest(input);
}

bool MeasurementSerialAdapter::isAttached() const noexcept {
    return m_token != 0;
}

} // namespace sensor
