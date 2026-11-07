#include "measurement/Units.hpp"

#include <cmath>
#include <iostream>

int main() {
    using biofeedback::measurement::gramsToNewtons;
    const auto near = [](double actual, double expected) {
        return std::abs(actual - expected) < 1e-9;
    };
    if (!near(gramsToNewtons(0.0), 0.0) ||
        !near(gramsToNewtons(1000.0), 9.80665) ||
        !near(gramsToNewtons(100.0), 0.980665) ||
        !near(gramsToNewtons(-100.0), -0.980665)) {
        std::cerr << "gramsToNewtons conversion regression failed\n";
        return 1;
    }
    std::cout << "Force unit conversion regression passed\n";
    return 0;
}
