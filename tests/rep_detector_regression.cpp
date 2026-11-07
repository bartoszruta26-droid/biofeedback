#include "tab/RepDetector.hpp"

#include <cassert>
#include <cmath>
#include <limits>

int main()
{
    training::RepDetector detector;

    // Below the start threshold does not begin a repetition.
    assert(!detector.update(79.9, 100.0));
    assert(!detector.inProgress());

    // Exactly 80% starts; the peak follows the maximum force.
    assert(!detector.update(80.0, 100.0));
    assert(detector.inProgress());
    assert(detector.peakForce() == 80.0);
    assert(!detector.update(95.0, 100.0));
    assert(!detector.update(90.0, 100.0));
    assert(detector.peakForce() == 95.0);

    // 40% completes and preserves the completed peak.
    assert(detector.update(40.0, 100.0));
    assert(!detector.inProgress());
    assert(detector.peakForce() == 95.0);

    // A second repetition starts independently and records its own peak.
    assert(!detector.update(81.0, 100.0));
    assert(detector.inProgress());
    assert(!detector.update(110.0, 100.0));
    assert(detector.update(39.0, 100.0));
    assert(detector.peakForce() == 110.0);

    // Invalid targets and non-finite samples reset an incomplete repetition.
    assert(!detector.update(90.0, 100.0));
    assert(detector.inProgress());
    assert(!detector.update(std::numeric_limits<double>::quiet_NaN(), 100.0));
    assert(!detector.inProgress());
    assert(detector.peakForce() == 0.0);
    assert(!detector.update(90.0, 0.0));
    assert(!detector.update(std::numeric_limits<double>::infinity(), 100.0));

    // Reset clears both active and completed state.
    assert(!detector.update(85.0, 100.0));
    detector.reset();
    assert(!detector.inProgress());
    assert(detector.peakForce() == 0.0);
    return 0;
}
