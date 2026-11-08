#include "application/ApplicationServices.hpp"
namespace biofeedback::application {
ApplicationServices::ApplicationServices(measurement::MeasurementEngineConfig config)
    : m_measurementEngine(std::make_shared<measurement::MeasurementEngine>(config)),
      m_devices(), m_measurements(m_measurementEngine) {}
} // namespace biofeedback::application
