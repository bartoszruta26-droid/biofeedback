#include "formulas.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {
bool near(double actual, double expected, double tolerance = 1e-9)
{
    return std::abs(actual - expected) <= tolerance;
}
}

int main()
{
    Formulas formulas(100.0, 70.0, 100.0);

    // Non-uniform intervals: trapezoidal integral is 1.5 + 4.0 = 5.5 N*s.
    const std::vector<SensorSample> irregular{
        SensorSample(0, 10.0),
        SensorSample(100, 20.0),
        SensorSample(300, 20.0)
    };
    assert(near(formulas.calculateImpulse(irregular), 5.5));

    // Invalid and out-of-order segments must not contribute to the integral.
    const std::vector<SensorSample> invalid{
        SensorSample(0, 10.0),
        SensorSample(100, 20.0),
        SensorSample(90, 30.0),
        SensorSample(200, 40.0)
    };
    assert(near(formulas.calculateImpulse(invalid), 1.5));

    // Device uint32 timestamp wrap is valid for a short interval.
    const std::vector<SensorSample> wrapped{
        SensorSample(0xfffffff0u, 10.0),
        SensorSample(0x00000010u, 10.0)
    };
    assert(near(formulas.calculateImpulse(wrapped), 0.32));

    BiomechanicalParams empty;
    assert(empty.availabilityAt(0) == ParameterAvailability::InsufficientData);
    assert(std::string(BiomechanicalParams::parameterName(0)) == "F_peak");
    assert(std::string(BiomechanicalParams::parameterName(64)) == "LPT");
    assert(std::string(BiomechanicalParams::parameterName(65)).empty());

    std::set<std::string> names;
    for (std::size_t i = 0; i < BiomechanicalParams::ParameterCount; ++i) {
        const std::string name = BiomechanicalParams::parameterName(i);
        assert(!name.empty());
        assert(names.insert(name).second);
    }

    SeriesData series;
    series.seriesNumber = 1;
    series.startTime = 0;
    series.endTime = 400;

    RepetitionData first;
    first.repNumber = 1;
    first.startTime = 0;
    first.peakTime = 100;
    first.endTime = 200;
    first.startForce = 0.0;
    first.peakForce = 20.0;
    first.endForce = 0.0;
    first.samples = {
        SensorSample(0, 0.0),
        SensorSample(100, 20.0),
        SensorSample(200, 0.0)
    };

    RepetitionData second;
    second.repNumber = 2;
    second.startTime = 200;
    second.peakTime = 300;
    second.endTime = 400;
    second.startForce = 0.0;
    second.peakForce = 25.0;
    second.endForce = 0.0;
    second.samples = {
        SensorSample(200, 0.0),
        SensorSample(300, 25.0),
        SensorSample(400, 0.0)
    };

    series.reps = {first, second};
    const BiomechanicalParams result = formulas.calculateAllParams(series);

    assert(result.availabilityAt(0) == ParameterAvailability::Available);
    assert(result.availabilityAt(4) == ParameterAvailability::Available);
    assert(result.availabilityAt(20) == ParameterAvailability::RequiresCalibration);
    assert(result.availabilityAt(29) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(39) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(52) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(53) == ParameterAvailability::RequiresHistory);
    assert(result.availabilityAt(57) == ParameterAvailability::RequiresCalibration);
    assert(result.availabilityAt(64) == ParameterAvailability::RequiresHistory);

    // A missing historical or sensor-dependent metric is never advertised as
    // a measured result merely because its reset numeric field happens to be 0.
    assert(result.availabilityAt(53) != ParameterAvailability::Available);
    assert(result.availabilityAt(57) != ParameterAvailability::Available);
    assert(result.availabilityAt(64) != ParameterAvailability::Available);

    std::cout << "Biomechanical parameter availability and timestamp integration passed\n";
    return 0;
}
