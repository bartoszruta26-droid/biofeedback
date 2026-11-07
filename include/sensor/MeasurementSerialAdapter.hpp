#pragma once

#include "measurement/MeasurementEngine.hpp"
#include "sensor/SerialCommunication.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace sensor {

// Bridges the existing serial acquisition callback to the shared measurement
// pipeline. The adapter owns a shared engine reference and unregisters on teardown.
class MeasurementSerialAdapter final {
public:
    MeasurementSerialAdapter(SerialCommunication& serial,
                             std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine,
                             std::string source,
                             std::string unit);
    ~MeasurementSerialAdapter();

    MeasurementSerialAdapter(const MeasurementSerialAdapter&) = delete;
    MeasurementSerialAdapter& operator=(const MeasurementSerialAdapter&) = delete;

    bool isAttached() const noexcept;

private:
    SerialCommunication& m_serial;
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> m_engine;
    SerialCommunication::CallbackToken m_token{0};
    std::string m_source;
    std::string m_unit;
    std::shared_ptr<std::atomic<bool>> m_active;
};

} // namespace sensor
