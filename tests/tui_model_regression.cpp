#include "tui/TuiModel.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
namespace {
void expect(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}
int main() {
    try {
        using tui::Screen; using tui::TuiModel;
        expect(TuiModel::screenCount() == 7, "TUI should expose seven modules");
        expect(TuiModel::clampSelection(-10) == 0, "selection must clamp at first module");
        expect(TuiModel::clampSelection(100) == 6, "selection must clamp at last module");
        expect(TuiModel::screenAt(0) == Screen::Dashboard, "dashboard should be first");
        expect(TuiModel::next(Screen::Settings) == Screen::Dashboard, "next should wrap");
        expect(TuiModel::previous(Screen::Dashboard) == Screen::Settings, "previous should wrap");
        expect(std::string(TuiModel::info(Screen::Devices).title) == "Urządzenia",
               "device screen title should be localized");
        std::cout << "OK: tui_model_regression\\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\\n';
        return EXIT_FAILURE;
    }
}
