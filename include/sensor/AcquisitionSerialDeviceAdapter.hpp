#pragma once

#include "application/AcquisitionService.hpp"
#include "application/DeviceService.hpp"
#include "sensor/SerialCommunication.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace sensor {

// Server-side USB/UART adapter. All accepted samples pass through the active
// server-owned AcquisitionService session; no direct MeasurementEngine path.
class AcquisitionSerialDeviceAdapter final : public biofeedback::application::DeviceAdapter {
public:
    AcquisitionSerialDeviceAdapter(std::shared_ptr<biofeedback::application::AcquisitionService> acquisition,
                                   std::string deviceId,
                                   SerialConfig config,
                                   std::string source,
                                   std::string unit);
    ~AcquisitionSerialDeviceAdapter() override;

    AcquisitionSerialDeviceAdapter(const AcquisitionSerialDeviceAdapter&) = delete;
    AcquisitionSerialDeviceAdapter& operator=(const AcquisitionSerialDeviceAdapter&) = delete;

    bool connect(std::string& error) override;
    void disconnect() noexcept override;
    bool isConnected() const noexcept override;

    bool processSample(const SensorData& data,
                       biofeedback::measurement::MeasurementSample& output);

private:
    SerialCommunication m_serial;
    std::shared_ptr<biofeedback::application::AcquisitionService> m_acquisition;
    std::string m_deviceId;
    std::string m_source;
    std::string m_unit;
    SerialConfig m_config;
    SerialCommunication::CallbackToken m_dataToken{0};
    std::shared_ptr<std::atomic<bool>> m_active;
};

} // namespace sensor
