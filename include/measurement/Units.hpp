#pragma once

namespace biofeedback::measurement {

constexpr double standardGravityMetersPerSecondSquared = 9.80665;

// HX711 firmware reports calibrated mass-equivalent values in grams.
// Convert once at the acquisition boundary; all movement metrics use newtons.
constexpr double gramsToNewtons(double grams) noexcept {
    return grams * standardGravityMetersPerSecondSquared / 1000.0;
}

constexpr double newtonsToKilogramForce(double newtons) noexcept {
    return newtons / standardGravityMetersPerSecondSquared;
}

} // namespace biofeedback::measurement
