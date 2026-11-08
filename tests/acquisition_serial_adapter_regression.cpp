#include "application/AcquisitionService.hpp"
#include "application/MeasurementService.hpp"
#include "measurement/MeasurementEngine.hpp"
#include "sensor/AcquisitionSerialDeviceAdapter.hpp"

#include <cassert>
#include <memory>

int main() {
    auto engine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    auto measurements = std::make_shared<biofeedback::application::MeasurementService>(engine);
    auto acquisition = std::make_shared<biofeedback::application::AcquisitionService>(measurements);

    sensor::SerialConfig config;
    config.portName = "/dev/this-port-must-not-exist";
    auto adapter = std::make_shared<sensor::AcquisitionSerialDeviceAdapter>(
        acquisition, "nano-hx711-1", config, "hx711", "g");

    sensor::SensorData data;
    data.timestamp = 1234;
    data.hasTimestamp = true;
    data.value = 4567;
    data.calibratedValue = 12.5;
    data.hasCalibratedValue = true;
    data.isValid = true;
    data.hasFrame = true;

    biofeedback::measurement::MeasurementSample output;
    // No active server session: the adapter must reject the sample.
    assert(!adapter->processSample(data, output));
    assert(measurements->history().empty());

    // A different device cannot inject into this session.
    std::string error;
    assert(acquisition->startSession(
        {"session-1", "patient-1", "another-device", "strength", false}, error));
    assert(!adapter->processSample(data, output));
    assert(measurements->history().empty());
    assert(acquisition->stopSession("session-1", error));

    // Only the device bound to the active server session is accepted.
    assert(acquisition->startSession(
        {"session-2", "patient-1", "nano-hx711-1", "strength", false}, error));
    assert(adapter->processSample(data, output));
    assert(output.sequence == 1);
    assert(output.isValid());
    assert(output.source == "hx711");
    assert(measurements->history().size() == 1);

    // Closing the session closes the ingest gate.
    assert(acquisition->stopSession("session-2", error));
    assert(!adapter->processSample(data, output));
    assert(measurements->history().size() == 1);

    adapter->disconnect();
    return 0;
}
