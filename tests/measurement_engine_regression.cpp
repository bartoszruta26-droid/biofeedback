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

    // Source changes discard partial work and require a fresh first sample.
    Sample start{2.0, 3.0, ForceUnit::Newton, SampleSource::Device, true};
    assert(!engine.addSample(start));
    Sample mixed{2.1, 5.0, ForceUnit::Newton, SampleSource::Replay, true};
    assert(!engine.addSample(mixed));
    assert(engine.rejectedSampleCount() == 3);
    Sample afterSwitch{2.2, 4.0, ForceUnit::Newton, SampleSource::Replay, true};
    assert(!engine.addSample(afterSwitch));
    assert(engine.isInContraction());

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
    assert(std::abs(seriesResults.front().maximumPeakForceN - 8.0) < 1e-9);
    assert(std::abs(seriesResults.front().minimumPeakForceN - 8.0) < 1e-9);
    assert(seriesResults.front().averageTimeToPeakSeconds > 0.0);
    assert(seriesResults.front().averageForceN > 0.0);
    assert(seriesResults.front().averageRepetitionDurationSeconds > 0.0);
    assert(seriesResults.front().totalImpulseNs > 0.0);
    assert(seriesResults.front().targetAccuracyPercent > 0.0);

    // Device acquisition and replay must produce identical analytics for identical samples.
    auto runSource = [](SampleSource source) {
        MeasurementEngine engine({2.0, 1.0, 5.0, 0.20, 8});
        const double forces[] = {3.0, 5.0, 8.0, 5.0, 1.0};
        std::optional<RepetitionStats> result;
        double time = 0.0;
        for (double force : forces) {
            result = engine.addSample(Sample{time, force, ForceUnit::Newton, source, true});
            time += 0.1;
        }
        return result;
    };
    const auto deviceResult = runSource(SampleSource::Device);
    const auto replayResult = runSource(SampleSource::Replay);
    assert(deviceResult && replayResult);
    assert(deviceResult->source == SampleSource::Device);
    assert(replayResult->source == SampleSource::Replay);
    assert(std::abs(deviceResult->peakForceN - replayResult->peakForceN) < 1e-12);
    assert(std::abs(deviceResult->impulseNs - replayResult->impulseNs) < 1e-12);

    MeasurementEngine configurable({2.0, 1.0});
    assert(configurable.setRepetitionsPerSeries(1));
    const double oneRep[] = {3.0, 6.0, 1.0};
    double configurableTime = 0.0;
    for (double force : oneRep) {
        configurable.addSample(Sample{configurableTime, force, ForceUnit::Newton,
                                      SampleSource::Simulation, true});
        configurableTime += 0.1;
    }
    assert(configurable.takeCompletedSeries().size() == 1);

    std::cout << "measurement_engine_regression: PASS\n";
    return 0;
}
