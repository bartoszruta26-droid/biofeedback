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
        SensorSample(),
        SensorSample(200, 40.0)
    };
    assert(near(formulas.calculateImpulse(invalid), 1.5));

    // Device uint32 timestamp wrap is valid for a short interval.
    const std::vector<SensorSample> wrapped{
        SensorSample(0xfffffff0u, 10.0),
        SensorSample(0x00000010u, 10.0)
    };
    assert(near(formulas.calculateImpulse(wrapped), 0.32));
    assert(near(Formulas::calculateRepDuration(0xfffffff0u, 0x00000010u), 0.032));
    assert(near(Formulas::calculateInterRepRest(0xfffffff0u, 0x00000010u), 0.032));
    assert(near(Formulas::calculateRepDuration(200u, 100u), 0.0));

    BiomechanicalParams empty;
    assert(empty.availabilityAt(0) == ParameterAvailability::InsufficientData);
    assert(std::string(BiomechanicalParams::parameterName(0)) == "F_peak");
    assert(std::string(BiomechanicalParams::parameterName(64)) == "LPT");
    assert(std::string(BiomechanicalParams::parameterUnit(0)) == "N");
    assert(std::string(BiomechanicalParams::parameterUnit(65)).empty());
    assert(std::string(BiomechanicalParams::parameterName(65)).empty());

    std::set<std::string> names;
    for (std::size_t i = 0; i < BiomechanicalParams::ParameterCount; ++i) {
        const std::string name = BiomechanicalParams::parameterName(i);
        assert(!name.empty());
        assert(names.insert(name).second);
        assert(std::string(BiomechanicalParams::parameterUnit(i)).size() > 0);
    }

    SeriesData series;
    series.seriesNumber = 1;
    series.startTime = 0;
    series.endTime = 450;

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
    second.startTime = 250;
    second.peakTime = 350;
    second.endTime = 450;
    second.startForce = 0.0;
    second.peakForce = 25.0;
    second.endForce = 0.0;
    second.samples = {
        SensorSample(250, 0.0),
        SensorSample(350, 25.0),
        SensorSample(450, 0.0)
    };

    RepetitionData irregularRep = first;
    irregularRep.endTime = 300;
    irregularRep.samples = {
        SensorSample(0, 0.0),
        SensorSample(100, 20.0),
        SensorSample(300, 0.0)
    };
    const BiomechanicalParams irregularResult =
        formulas.calculateRepParams(irregularRep);
    // Time-weighted mean for the non-uniform 0/100/300 ms samples:
    // (1 + 2) N*s / 0.3 s = 10 N, not the sample-count mean 6.67 N.
    assert(near(irregularResult.F_mean, 10.0));

    const BiomechanicalParams repetitionResult = formulas.calculateRepParams(first);
    // Total absolute spring work for 0 -> 20 -> 0 N at k=100 N/m:
    // 2 J loading + 2 J unloading = 4 J.
    assert(near(repetitionResult.W_total, 4.0));

    RepetitionData powerRep;
    powerRep.startTime = 0;
    powerRep.peakTime = 200;
    powerRep.endTime = 300;
    powerRep.startForce = 0.0;
    powerRep.peakForce = 20.0;
    powerRep.endForce = 0.0;
    powerRep.samples = {
        SensorSample(0, 0.0),
        SensorSample(100, 10.0),
        SensorSample(200, 20.0),
        SensorSample(300, 0.0)
    };
    const BiomechanicalParams powerResult = formulas.calculateRepParams(powerRep);
    // Mean absolute extension speed = (10+10+20)/100 / 0.3 = 1.333... m/s.
    assert(near(powerResult.v_avg, 4.0 / 3.0));
    // Instantaneous peak power is max(F_i * |dF/dt| / k) = 10 W,
    // not max(F) multiplied by a velocity peak from another instant.
    assert(near(powerResult.P_peak, 10.0));

    SensorSample invalidEccentricSample(175, 1000.0);
    invalidEccentricSample.isValid = false;
    const std::vector<SensorSample> eccentricWithGap{
        SensorSample(100, 20.0),
        invalidEccentricSample,
        SensorSample(200, 10.0),
        SensorSample(250, 0.0)
    };
    // The invalid interval is excluded; only the final valid 200-250 ms
    // segment contributes to the eccentric integral.
    assert(near(formulas.calculateCEI(eccentricWithGap, 20.0, 100, 250),
                (0.25 * 0.05) / 0.15));
    assert(repetitionResult.availabilityAt(0) == ParameterAvailability::Available);
    assert(repetitionResult.availabilityAt(20) == ParameterAvailability::Available);
    assert(repetitionResult.availabilityAt(64) == ParameterAvailability::InsufficientData);

    series.reps = {first, second};
    const BiomechanicalParams result = formulas.calculateAllParams(series);

    assert(result.availabilityAt(0) == ParameterAvailability::Available);
    assert(result.availabilityAt(4) == ParameterAvailability::Available);
    assert(result.availabilityAt(2) == ParameterAvailability::Available);
    assert(result.availabilityAt(3) == ParameterAvailability::Available);
    assert(result.availabilityAt(18) == ParameterAvailability::Available);
    assert(result.availabilityAt(14) == ParameterAvailability::Available);
    assert(near(result.T_rest_rep, 0.05));
    assert(result.availabilityAt(20) == ParameterAvailability::Available);
    assert(result.availabilityAt(29) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(39) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(52) == ParameterAvailability::RequiresAdditionalSensor);
    assert(result.availabilityAt(53) == ParameterAvailability::RequiresHistory);
    assert(result.availabilityAt(58) == ParameterAvailability::RequiresCalibration);
    assert(result.availabilityAt(64) == ParameterAvailability::RequiresHistory);
    assert(result.availabilityAt(24) == ParameterAvailability::Available);
    assert(result.availabilityAt(45) == ParameterAvailability::Available);
    assert(std::string(BiomechanicalParams::parameterUnit(45)) == "m/s");

    Formulas uncalibrated;
    assert(near(uncalibrated.getElasticConstant(), 0.0));
    assert(near(uncalibrated.getPatientMass(), 0.0));
    const BiomechanicalParams withoutCalibration = uncalibrated.calculateAllParams(series);
    assert(withoutCalibration.availabilityAt(20) == ParameterAvailability::RequiresCalibration);
    assert(withoutCalibration.availabilityAt(24) == ParameterAvailability::RequiresCalibration);
    assert(withoutCalibration.availabilityAt(45) == ParameterAvailability::RequiresCalibration);

    SeriesData undersampled = series;
    undersampled.reps[0].samples.clear();
    const BiomechanicalParams rejected = formulas.calculateAllParams(undersampled);
    assert(rejected.availabilityAt(0) == ParameterAvailability::InvalidInput);

    SeriesData outOfOrder = series;
    outOfOrder.reps[0].samples = {
        SensorSample(0, 0.0),
        SensorSample(200, 20.0),
        SensorSample(100, 0.0)
    };
    const BiomechanicalParams invalidClock = formulas.calculateAllParams(outOfOrder);
    assert(invalidClock.availabilityAt(0) == ParameterAvailability::InvalidInput);

    // A missing historical or sensor-dependent metric is never advertised as
    // a measured result merely because its reset numeric field happens to be 0.
    assert(result.availabilityAt(53) != ParameterAvailability::Available);
    assert(result.availabilityAt(58) != ParameterAvailability::Available);
    assert(result.availabilityAt(64) != ParameterAvailability::Available);


    // Session metrics use distinct sets, not a single set's proxy values.
    SeriesData set1 = series;
    set1.seriesNumber = 1;
    SeriesData set2 = series;
    set2.seriesNumber = 2;
    SeriesData set3 = series;
    set3.seriesNumber = 3;
    const auto shiftAndScale = [](SeriesData& set, std::uint32_t offset, double scale) {
        set.startTime += offset;
        set.endTime += offset;
        for (auto& rep : set.reps) {
            rep.startTime += offset;
            rep.peakTime += offset;
            rep.endTime += offset;
            rep.startForce *= scale;
            rep.peakForce *= scale;
            rep.endForce *= scale;
            for (auto& sample : rep.samples) {
                sample.timestamp += offset;
                sample.force *= scale;
            }
        }
    };
    shiftAndScale(set2, 1000u, 1.1);
    shiftAndScale(set3, 2000u, 0.75);
    const BiomechanicalParams sessionResult =
        formulas.calculateSessionParams({set3, set1, set2});
    assert(sessionResult.availabilityAt(16) == ParameterAvailability::Available);
    assert(near(sessionResult.T_rest_set, 0.55));
    assert(sessionResult.availabilityAt(33) == ParameterAvailability::Available);
    assert(sessionResult.FI_inter > 0.0);
    assert(sessionResult.availabilityAt(46) == ParameterAvailability::Available);
    assert(sessionResult.FMC > 0.0);
    assert(sessionResult.availabilityAt(47) == ParameterAvailability::Available);
    assert(std::isfinite(sessionResult.ISI));
    assert(sessionResult.availabilityAt(40) == ParameterAvailability::Available);
    assert(sessionResult.F_grand_mean > 0.0);
    assert(sessionResult.availabilityAt(41) == ParameterAvailability::Available);
    assert(sessionResult.I_total > 0.0);
    assert(sessionResult.availabilityAt(42) == ParameterAvailability::Available);
    assert(sessionResult.V_training > 0.0);
    assert(sessionResult.availabilityAt(44) == ParameterAvailability::Available);
    assert(sessionResult.D_training > 0.0);
    assert(sessionResult.availabilityAt(55) == ParameterAvailability::Available);
    assert(sessionResult.availabilityAt(57) == ParameterAvailability::Available);
    assert(sessionResult.availabilityAt(60) == ParameterAvailability::Available);

    const BiomechanicalParams oneSetSession = formulas.calculateSessionParams({set1});
    assert(oneSetSession.availabilityAt(33) == ParameterAvailability::InsufficientData);
    assert(oneSetSession.availabilityAt(46) == ParameterAvailability::InsufficientData);
    assert(oneSetSession.availabilityAt(47) == ParameterAvailability::InsufficientData);
    assert(oneSetSession.availabilityAt(60) == ParameterAvailability::InsufficientData);
    const BiomechanicalParams uncalibratedSession =
        uncalibrated.calculateSessionParams({set1, set2, set3});
    assert(uncalibratedSession.availabilityAt(57) ==
           ParameterAvailability::RequiresCalibration);
    assert(uncalibratedSession.availabilityAt(55) ==
           ParameterAvailability::RequiresCalibration);

    std::cout << "Biomechanical parameter availability and timestamp integration passed\n";
    return 0;
}
