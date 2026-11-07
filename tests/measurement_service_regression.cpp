#include "application/MeasurementService.hpp"

#include <cassert>
#include <memory>

using namespace biofeedback;

int main() {
    auto engine = std::make_shared<measurement::MeasurementEngine>();
    application::MeasurementService service(engine);

    measurement::MeasurementInput input;
    input.deviceTimestampMs = 42;
    input.rawValue = 125;
    input.value = 1.25;
    input.sensorValid = true;
    input.unit = "N";
    input.source = "service-regression";

    measurement::MeasurementSample output;
    assert(!service.isRunning());
    assert(!service.ingest(input, output));
    assert(service.history().empty());

    int callbacks = 0;
    const auto token = service.subscribe([&callbacks](const measurement::MeasurementSample&) {
        ++callbacks;
    });
    assert(token != 0);

    service.start();
    assert(service.isRunning());
    assert(service.ingest(input, output));
    assert(output.isValid());
    assert(output.sequence == 1);
    assert(callbacks == 1);
    assert(service.history().size() == 1);

    input.unit = "unsupported";
    assert(service.ingest(input, output));
    assert(!output.isValid());
    assert(output.quality == measurement::Quality::InvalidUnit);
    assert(callbacks == 2);
    assert(service.history().size() == 2);

    service.stop();
    assert(!service.isRunning());
    assert(!service.ingest(input, output));
    assert(service.history().size() == 2);

    service.unsubscribe(token);
    service.clearHistory();
    assert(service.history().empty());
    return 0;
}
