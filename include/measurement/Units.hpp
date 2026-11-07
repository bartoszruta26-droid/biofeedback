#pragma once

namespace biofeedback::measurement {

constexpr double standardGravityMetersPerSecondSquared = 9.80665;

// The HX711 firmware reports calibrated mass-equivalent values in grams.
// Convert that calibration output to SI force before using targets expressed in N.
constexpr double gramsToNewtons(double grams) noexcept {
    return grams * standardGravityMetersPerSecondSquared / 1000.0;
}

} // namespace biofeedback::measurement
