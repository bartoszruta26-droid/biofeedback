#include "application/ApplicationServices.hpp"
#include <cassert>
#include <memory>
#include <string>
using namespace biofeedback::application;
class FakeDevice final : public DeviceAdapter {
public:
    bool connect(std::string& error) override {
        if (fail) { error = "synthetic failure"; return false; }
        connected = true; return true;
    }
    void disconnect() noexcept override { connected = false; }
    bool isConnected() const noexcept override { return connected; }
    bool connected{false}, fail{false};
};
int main() {
    ApplicationServices services;
    auto adapter = std::make_shared<FakeDevice>();
    DeviceDescriptor descriptor{"sensor-01", "Czujnik siły", "serial"};
    int events = 0;
    const auto token = services.devices().subscribe([&events](const DeviceDescriptor&) { ++events; });
    assert(token != 0);
    assert(services.devices().registerDevice(descriptor, adapter));
    assert(!services.devices().registerDevice(descriptor, adapter));
    assert(services.devices().connectDevice("sensor-01"));
    DeviceDescriptor current;
    assert(services.devices().device("sensor-01", current));
    assert(current.state == DeviceState::Connected);
    assert(events >= 2);
    assert(services.devices().disconnectDevice("sensor-01"));
    assert(services.devices().unregisterDevice("sensor-01"));
    services.devices().unsubscribe(token);
    auto failing = std::make_shared<FakeDevice>(); failing->fail = true;
    descriptor.id = "sensor-02";
    assert(services.devices().registerDevice(descriptor, failing));
    assert(!services.devices().connectDevice("sensor-02"));
    assert(services.devices().device("sensor-02", current));
    assert(current.state == DeviceState::Error && current.lastError == "synthetic failure");
    auto& measurements = services.measurements();
    biofeedback::measurement::MeasurementInput input;
    input.hasTimestamp = true; input.deviceTimestampMs = 0; input.rawValue = 42;
    input.value = 4.2; input.sensorValid = true; input.unit = "N"; input.source = "sensor-01";
    biofeedback::measurement::MeasurementSample sample;
    assert(!measurements.ingest(input, sample));
    measurements.start();
    assert(measurements.ingest(input, sample) && sample.isValid() && sample.sequence == 1);
    assert(measurements.history().size() == 1);
    measurements.stop();
    assert(!measurements.ingest(input, sample));
    return 0;
}
