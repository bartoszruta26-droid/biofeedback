#pragma once
#include "application/DeviceService.hpp"
#include "application/MeasurementService.hpp"
#include <memory>
namespace biofeedback::application {
// Single process-level composition shared by all local interfaces and API bridges.
class ApplicationServices final {
public:
    explicit ApplicationServices(measurement::MeasurementEngineConfig config = {});
    ApplicationServices(const ApplicationServices&) = delete;
    ApplicationServices& operator=(const ApplicationServices&) = delete;
    DeviceService& devices() noexcept { return m_devices; }
    const DeviceService& devices() const noexcept { return m_devices; }
    MeasurementService& measurements() noexcept { return m_measurements; }
    const MeasurementService& measurements() const noexcept { return m_measurements; }
private:
    std::shared_ptr<measurement::MeasurementEngine> m_measurementEngine;
    DeviceService m_devices;
    MeasurementService m_measurements;
};
} // namespace biofeedback::application
