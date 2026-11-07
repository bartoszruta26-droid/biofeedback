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
    assert(!detector.isActive());

    // A short spike is discarded, not counted as a repetition.
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
    assert(!detector.process(0x00000130u, 4.0, true, rep));
    assert(rep.durationMs == 320);

    std::vector<RepetitionRecord> series{
        {0, 400, 400, 200, 40.0, 20.0, 5},
        {500, 900, 400, 180, 50.0, 22.0, 5},
        {1000, 1400, 400, 220, 60.0, 24.0, 5}
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
