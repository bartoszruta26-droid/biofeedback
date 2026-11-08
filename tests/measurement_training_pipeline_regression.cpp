#include "measurement/MeasurementEngine.hpp"
#include "measurement/Units.hpp"
#include "training/RepetitionDetector.hpp"

#include <cassert>
#include <cmath>

int main() {
    using namespace biofeedback::measurement;
    using namespace biofeedback::training;

    MeasurementEngine engine;
    RepetitionDetector detector({2.0, 0.20, 0.10, 0.1, 100, 5000, 500});
    RepetitionRecord repetition;

    auto ingestForce = [&](std::uint32_t timestamp, double grams) {
        MeasurementInput input;
        input.deviceTimestampMs = timestamp;
        input.hasTimestamp = true;
        input.rawValue = static_cast<std::int32_t>(grams * 10.0);
        input.value = grams;
        input.sensorValid = true;
        input.unit = "g";
        input.source = "hx711-test";
        const auto sample = engine.ingest(input);
        assert(sample.isValid());
        assert(sample.unit == "kg");
        const double forceN = sample.value * standardGravityMetersPerSecondSquared;
        assert(std::abs(forceN - gramsToNewtons(grams)) < 1e-9);
        return detector.process(timestamp, forceN, sample.isValid(), repetition);
    };

    assert(!ingestForce(100, 100.0));
    assert(!ingestForce(200, 200.0));
    assert(!ingestForce(300, 100.0));
    assert(ingestForce(400, 10.0));

    assert(repetition.durationMs == 300);
    assert(std::abs(repetition.peakForceN - gramsToNewtons(200.0)) < 1e-9);
    assert(repetition.sampleCount == 4);
    assert(repetition.samples.size() == repetition.sampleCount);
    assert(std::abs(repetition.samples.front().forceN - gramsToNewtons(100.0)) < 1e-9);
    assert(std::abs(repetition.samples.back().forceN - gramsToNewtons(10.0)) < 1e-9);
    // A sample gap must not re-arm detection in the middle of a sustained
    // contraction. A baseline sample is required before the next repetition.
    detector.reset();
    RepetitionRecord afterGap;
    assert(!detector.process(1000, 10.0, true, afterGap));
    assert(!detector.process(1100, 12.0, true, afterGap));
    assert(!detector.process(1800, 12.0, true, afterGap)); // gap > 500 ms
    assert(!detector.process(1900, 12.0, true, afterGap)); // still contracted
    assert(!detector.process(2000, 0.0, true, afterGap));  // release baseline
    assert(!detector.process(2100, 10.0, true, afterGap)); // new contraction
    assert(!detector.process(2200, 12.0, true, afterGap));
    assert(detector.process(2300, 0.0, true, afterGap));
    assert(afterGap.sampleCount == 3);
    assert(afterGap.startTimestampMs == 2100);

    return 0;
}
