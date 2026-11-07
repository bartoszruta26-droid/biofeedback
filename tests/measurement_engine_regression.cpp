#include "measurement/MeasurementEngine.hpp"

#include <cassert>
#include <cmath>
#include <limits>

using namespace biofeedback::measurement;

int main() {
    MeasurementEngine engine({-100.0, 100.0, 2});
    int callbackCount = 0;
    const auto token = engine.subscribe([&callbackCount](const MeasurementSample&) {
        ++callbackCount;
    });
    assert(token != 0);

    MeasurementInput input;
    input.deviceTimestampMs = 1234;
    input.hasTimestamp = true;
    input.rawValue = 250;
    input.value = 2500.0;
    input.sensorValid = true;
    input.unit = "g";
    input.source = "sensor-test";

    const auto grams = engine.ingest(input);
    assert(grams.isValid());
    assert(grams.unit == "kg");
    assert(std::abs(grams.value - 2.5) < 1e-9);
    assert(grams.timestampUnixMs > 0);
    assert(grams.sequence == 1);

    MeasurementInput wrappedClockInput = input;
    wrappedClockInput.deviceTimestampMs = 0;
    const auto wrappedClockSample = engine.ingest(wrappedClockInput);
    assert(wrappedClockSample.isValid()); // uint32 device-clock wrap is not a missing timestamp
    assert(wrappedClockSample.deviceTimestampMs == 0);

    input.unit = "mm";
    input.value = 25.0;
    const auto millimetres = engine.ingest(input);
    assert(millimetres.isValid());
    assert(millimetres.unit == "m");
    assert(std::abs(millimetres.value - 0.025) < 1e-9);

    input.unit = "not-a-unit";
    assert(engine.ingest(input).quality == Quality::InvalidUnit);
    input.unit = "N";
    input.source.clear();
    assert(engine.ingest(input).quality == Quality::InvalidSource);
    input.source = "sensor-test";
    input.sensorValid = false;
    assert(engine.ingest(input).quality == Quality::SensorRejected);
    input.sensorValid = true;
    input.hasTimestamp = false;
    assert(engine.ingest(input).quality == Quality::MissingTimestamp);
    input.hasTimestamp = true;
    input.deviceTimestampMs = 1235;
    input.value = std::numeric_limits<double>::infinity();
    assert(engine.ingest(input).quality == Quality::NonFiniteValue);
    input.value = 101.0;
    assert(engine.ingest(input).quality == Quality::OutsideConfiguredRange);

    assert(callbackCount == 9);
    const auto history = engine.history();
    assert(history.size() == 2);
    assert(history.front().sequence == 8);
    assert(history.back().sequence == 9);

    engine.unsubscribe(token);
    engine.ingest(input);
    assert(callbackCount == 9);
    engine.clearHistory();
    assert(engine.history().empty());
    return 0;
}
