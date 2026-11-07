#pragma once

namespace biofeedback::measurement {

constexpr double standardGravityMetersPerSecondSquared = 9.80665;

// HX711 firmware reports calibrated mass-equivalent values in grams.
// Convert those values to SI force before comparing them with N targets.
constexpr double gramsToNewtons(double grams) noexcept {
    return grams * standardGravityMetersPerSecondSquared / 1000.0;
}

} // namespace biofeedback::measurement
