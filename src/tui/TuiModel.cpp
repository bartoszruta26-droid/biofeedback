#include "tui/TuiModel.hpp"
#include <algorithm>
#include <stdexcept>
namespace tui {
const std::vector<ScreenInfo>& TuiModel::screens() {
    static const std::vector<ScreenInfo> value{
        {Screen::Dashboard, "Pulpit", "Przegląd systemu i bieżącego stanu"},
        {Screen::Measurements, "Pomiary", "Podgląd sesji i parametrów pomiarowych"},
        {Screen::Patients, "Pacjenci", "Moduł kart i wyboru pacjenta"},
        {Screen::Training, "Trening", "Ćwiczenia i sesje biofeedback"},
        {Screen::Devices, "Urządzenia", "Porty szeregowe i stan czujników"},
        {Screen::Diagnostics, "Diagnostyka", "Stan aplikacji i komunikaty"},
        {Screen::Settings, "Ustawienia", "Konfiguracja interfejsu terminalowego"}
    };
    return value;
}
std::size_t TuiModel::screenCount() { return screens().size(); }
std::size_t TuiModel::clampSelection(int selection) {
    if (selection < 0) return 0;
    const auto last = static_cast<int>(screenCount()) - 1;
    if (selection > last) return static_cast<std::size_t>(last);
    return static_cast<std::size_t>(selection);
}
Screen TuiModel::screenAt(std::size_t index) {
    if (index >= screenCount()) throw std::out_of_range("TUI screen index out of range");
    return screens()[index].id;
}
const ScreenInfo& TuiModel::info(Screen screen) {
    const auto& all = screens();
    const auto it = std::find_if(all.begin(), all.end(),
        [screen](const ScreenInfo& item) { return item.id == screen; });
    if (it == all.end()) throw std::out_of_range("Unknown TUI screen");
    return *it;
}
Screen TuiModel::next(Screen current) {
    const auto& all = screens();
    for (std::size_t i = 0; i < all.size(); ++i)
        if (all[i].id == current) return all[(i + 1) % all.size()].id;
    return all.front().id;
}
Screen TuiModel::previous(Screen current) {
    const auto& all = screens();
    for (std::size_t i = 0; i < all.size(); ++i)
        if (all[i].id == current) return all[(i + all.size() - 1) % all.size()].id;
    return all.front().id;
}
}
