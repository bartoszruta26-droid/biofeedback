#include "core/MeasurementEngine.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace biofeedback::measurement;

int main()
{
    MeasurementEngine engine({2.0, 1.0});
    std::optional<RepetitionStats> completed;

    const double forces[] = {0.0, 1.0, 3.0, 6.0, 10.0, 8.0, 4.0, 1.0};
    for (std::size_t i = 0; i < sizeof(forces) / sizeof(forces[0]); ++i) {
        Sample sample;
        sample.timestampSeconds = static_cast<double>(i) * 0.1;
        sample.force = forces[i];
        sample.source = SampleSource::Simulation;
        sample.valid = true;
        const auto result = engine.addSample(sample);
        if (result) completed = result;
    }

    assert(completed.has_value());
    assert(completed->source == SampleSource::Simulation);
    assert(completed->sampleCount == 6);
    assert(std::abs(completed->peakForceN - 10.0) < 1e-9);
    assert(std::abs(completed->durationSeconds - 0.5) < 1e-9);
    assert(completed->impulseNs > 0.0);
    assert(completed->riseRateNPerSecond > 0.0);
    assert(completed->fallRateNPerSecond > 0.0);
    assert(completed->medianForceN > 0.0);
    assert(completed->percentile95ForceN >= completed->medianForceN);
    assert(completed->standardDeviationN > 0.0);
    assert(!engine.isInContraction());

    Sample invalid;
    invalid.timestampSeconds = 1.0;
    invalid.force = std::nan("");
    invalid.valid = true;
    assert(!engine.addSample(invalid));
    assert(engine.rejectedSampleCount() == 1);

    // A source change within an active repetition is rejected.
    Sample start{2.0, 3.0, ForceUnit::Newton, SampleSource::Device, true};
    assert(!engine.addSample(start));
    Sample mixed{2.1, 5.0, ForceUnit::Newton, SampleSource::Replay, true};
    assert(!engine.addSample(mixed));
    assert(engine.rejectedSampleCount() == 2);

    // Shared series aggregation and target-zone accuracy are produced by the engine.
    MeasurementEngine seriesEngine({2.0, 1.0, 5.0, 0.20, 2});
    double timestamp = 0.0;
    for (int repetition = 0; repetition < 2; ++repetition) {
        const double repForces[] = {3.0, 5.0, 5.5, 8.0, 5.0, 1.0};
        for (double force : repForces) {
            Sample sample{timestamp, force, ForceUnit::Newton, SampleSource::Replay, true};
            seriesEngine.addSample(sample);
            timestamp += 0.1;
        }
    }
    const auto seriesResults = seriesEngine.takeCompletedSeries();
    assert(seriesResults.size() == 1);
    assert(seriesResults.front().repetitionCount == 2);
    assert(std::abs(seriesResults.front().averagePeakForceN - 8.0) < 1e-9);
    assert(seriesResults.front().totalImpulseNs > 0.0);
    assert(seriesResults.front().targetAccuracyPercent > 0.0);

    std::cout << "measurement_engine_regression: PASS\n";
    return 0;
}
