#include "sensor/AcquisitionSerialDeviceAdapter.hpp"

#include <utility>

namespace sensor {

AcquisitionSerialDeviceAdapter::AcquisitionSerialDeviceAdapter(
    std::shared_ptr<biofeedback::application::AcquisitionService> acquisition,
    std::string deviceId,
    SerialConfig config,
    std::string source,
    std::string unit)
    : m_acquisition(std::move(acquisition)),
      m_deviceId(std::move(deviceId)),
      m_source(std::move(source)),
      m_unit(std::move(unit)),
      m_config(std::move(config)) {
    if (!m_acquisition || m_deviceId.empty() || m_source.empty() || m_unit.empty()) return;
    m_active = std::make_shared<std::atomic<bool>>(true);
    m_dataToken = m_serial.addDataCallback([this, active = m_active](const SensorData& data) {
        if (!active->load(std::memory_order_acquire)) return;
        biofeedback::measurement::MeasurementSample ignored;
        (void)processSample(data, ignored);
    });
}

AcquisitionSerialDeviceAdapter::~AcquisitionSerialDeviceAdapter() {
    disconnect();
    if (m_active) m_active->store(false, std::memory_order_release);
    if (m_dataToken != 0) m_serial.removeDataCallback(m_dataToken);
}

bool AcquisitionSerialDeviceAdapter::connect(std::string& error) {
    if (!m_acquisition || !m_active || m_dataToken == 0) {
        error = "INVALID_SERIAL_ACQUISITION_ADAPTER";
        return false;
    }
    if (m_serial.isConnected()) return true;
    if (!m_serial.connect(m_config)) {
        error = m_serial.getLastError();
        if (error.empty()) error = "SERIAL_CONNECTION_FAILED";
        return false;
    }
    m_serial.startAsyncReading();
    error.clear();
    return true;
}

void AcquisitionSerialDeviceAdapter::disconnect() noexcept {
    m_serial.stopAsyncReading();
    m_serial.disconnect();
}

bool AcquisitionSerialDeviceAdapter::isConnected() const noexcept {
    return m_serial.isConnected();
}

bool AcquisitionSerialDeviceAdapter::processSample(
    const SensorData& data,
    biofeedback::measurement::MeasurementSample& output) {
    if (!m_acquisition || !m_active || !m_active->load(std::memory_order_acquire) ||
        !data.hasFrame) return false;

    biofeedback::measurement::MeasurementInput input;
    input.deviceTimestampMs = data.timestamp;
    input.hasTimestamp = data.hasTimestamp;
    input.rawValue = data.value;
    input.value = m_unit == "count" ? static_cast<double>(data.value) : data.calibratedValue;
    input.sensorValid = data.isValid && (m_unit == "count" || data.hasCalibratedValue);
    input.unit = m_unit;
    input.source = m_source;
    return m_acquisition->ingest(m_deviceId, input, output);
}

} // namespace sensor
