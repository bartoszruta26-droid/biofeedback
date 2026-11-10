#include "training/RepetitionDetector.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace biofeedback::training;

int main() {
    RepetitionDetector detector({50.0, 0.20, 0.10, 2.0, 250, 30000, 500});
    RepetitionRecord rep;

    // A complete contraction: activation, peak, then release below hysteresis.
    assert(!detector.process(100, 0.0, true, rep));
    assert(!detector.process(200, 12.0, true, rep));
    assert(!detector.process(300, 30.0, true, rep));
    assert(!detector.process(400, 50.0, true, rep));
    assert(!detector.process(500, 42.0, true, rep));
    assert(detector.process(600, 4.0, true, rep));
    assert(rep.startTimestampMs == 200);
    assert(rep.endTimestampMs == 600);
    assert(rep.durationMs == 400);
    assert(rep.timeToPeakMs == 200);
    assert(std::abs(rep.peakForceN - 50.0) < 1e-9);
    assert(rep.meanForceN > 0.0);
    assert(rep.sampleCount == 5);
    assert(rep.samples.size() == rep.sampleCount);
    assert(rep.samples.front().timestampMs == rep.startTimestampMs);
    assert(rep.samples.back().timestampMs == rep.endTimestampMs);
    assert(!detector.isActive());

    // Simulate a process restart in the middle of a contraction.
    detector.reset();
    assert(!detector.process(2000, 15.0, true, rep));
    assert(!detector.process(2100, 28.0, true, rep));
    const auto snapshot = detector.snapshot();
    assert(snapshot.active && snapshot.samples.size() == 2);

    RepetitionDetector recovered({50.0, 0.20, 0.10, 2.0, 250, 30000, 500});
    assert(recovered.restore(snapshot));
    assert(recovered.isActive());
    assert(!recovered.process(2200, 45.0, true, rep));
    assert(recovered.process(2300, 4.0, true, rep));
    assert(rep.startTimestampMs == 2000);
    assert(rep.endTimestampMs == 2300);
    assert(rep.sampleCount == 4);
    assert(std::abs(rep.peakForceN - 45.0) < 1e-9);

    // A malformed checkpoint is rejected without modifying a separate
    // detector that was already restored from the valid snapshot.
    RepetitionDetector guarded({50.0, 0.20, 0.10, 2.0, 250, 30000, 500});
    assert(guarded.restore(snapshot));
    auto corrupt = snapshot;
    ++corrupt.sampleCount;
    assert(!guarded.restore(corrupt));
    auto unorderedSamples = snapshot;
    unorderedSamples.samples[1].timestampMs = unorderedSamples.samples[0].timestampMs;
    assert(!guarded.restore(unorderedSamples));
    assert(guarded.isActive());
    assert(guarded.process(2200, 45.0, true, rep) == false);
    assert(guarded.process(2300, 4.0, true, rep));
    assert(rep.startTimestampMs == 2000 && rep.endTimestampMs == 2300);
    assert(!guarded.isActive());
    assert(!guarded.process(2400, 0.0, true, rep));

    // A short spike is discarded, not counted as a repetition.
    detector.reset();
    assert(!detector.process(700, 15.0, true, rep));
    assert(!detector.process(800, 0.0, true, rep));
    assert(!detector.isActive());

    // Invalid samples terminate an incomplete contraction; no fabricated rep.
    assert(!detector.process(900, 20.0, true, rep));
    assert(!detector.process(1000, 30.0, true, rep));
    assert(!detector.process(1010, 30.0, false, rep));
    assert(!detector.isActive());

    // Missing sample intervals reset the active repetition.
    assert(!detector.process(1100, 20.0, true, rep));
    assert(!detector.process(1800, 25.0, true, rep));
    assert(!detector.isActive());

    // Device timestamp wrap is handled by unsigned elapsed-time arithmetic.
    detector.reset();
    assert(!detector.process(0xfffffff0u, 15.0, true, rep));
    assert(!detector.process(0x00000090u, 25.0, true, rep));
    assert(detector.process(0x00000130u, 4.0, true, rep));
    assert(rep.durationMs == 320);

    // At 80 SPS, a complete short tubing repetition spans at least 10
    // acquired samples. Verify repeated contractions are segmented exactly once.
    RepetitionDetector fastDetector({50.0, 0.20, 0.10, 2.0, 120, 30000, 100});
    std::vector<RepetitionRecord> fastRepetitions;
    std::uint32_t fastTimestamp = 0;
    const double contraction[] = {0.0, 12.0, 20.0, 30.0, 40.0, 50.0,
                                  50.0, 40.0, 30.0, 20.0, 10.0, 4.0};
    for (int repetitionIndex = 0; repetitionIndex < 3; ++repetitionIndex) {
        for (double force : contraction) {
            if (fastDetector.process(fastTimestamp, force, true, rep))
                fastRepetitions.push_back(rep);
            fastTimestamp += 12; // 12.5 ms nominal period, integer device clock
        }
        // A confirmed low baseline rearms the detector between repetitions.
        assert(!fastDetector.process(fastTimestamp, 0.0, true, rep));
        fastTimestamp += 12;
    }
    assert(fastRepetitions.size() == 3);
    for (const auto& fastRep : fastRepetitions) {
        assert(fastRep.durationMs >= 120);
        assert(fastRep.peakForceN == 50.0);
    }
    assert(summarizeSeries(fastRepetitions, 50.0).repetitionCount == 3);

    std::vector<RepetitionRecord> series{
        {0, 400, 400, 200, 40.0, 20.0, 5, {}},
        {500, 900, 400, 180, 50.0, 22.0, 5, {}},
        {1000, 1400, 400, 220, 60.0, 24.0, 5, {}}
    };
    const auto summary = summarizeSeries(series, 50.0);
    assert(summary.repetitionCount == 3);
    assert(std::abs(summary.averagePeakForceN - 50.0) < 1e-9);
    assert(std::abs(summary.maximumPeakForceN - 60.0) < 1e-9);
    assert(std::abs(summary.averageDurationMs - 400.0) < 1e-9);
    assert(std::abs(summary.targetAchievementPercent - (200.0 / 3.0)) < 1e-9);
    assert(summary.peakForceCoefficientOfVariationPercent > 0.0);

    return 0;
}
