#include "measurement/Units.hpp"

#include <cmath>
#include <iostream>

int main() {
    using biofeedback::measurement::gramsToNewtons;
    using biofeedback::measurement::newtonsToKilogramForce;
    const auto near = [](double actual, double expected) {
        return std::abs(actual - expected) < 1e-9;
    };
    if (!near(gramsToNewtons(0.0), 0.0) ||
        !near(gramsToNewtons(1000.0), 9.80665) ||
        !near(gramsToNewtons(100.0), 0.980665) ||
        !near(gramsToNewtons(-100.0), -0.980665) ||
        !near(std::abs(gramsToNewtons(-100.0)), 0.980665) ||
        !near(newtonsToKilogramForce(9.80665), 1.0) ||
        !near(newtonsToKilogramForce(0.980665), 0.1)) {
        std::cerr << "SI force conversion regression failed\n";
        return 1;
    }
    std::cout << "SI force conversion regression passed\n";
    return 0;
}
