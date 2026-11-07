#ifndef BIOFEEDBACK_TUI_MODEL_HPP
#define BIOFEEDBACK_TUI_MODEL_HPP
#include <cstddef>
#include <string>
#include <vector>
namespace tui {
enum class Screen { Dashboard = 0, Measurements, Patients, Training, Devices, Diagnostics, Settings };
struct ScreenInfo { Screen id; const char* title; const char* subtitle; };
class TuiModel {
public:
    static const std::vector<ScreenInfo>& screens();
    static std::size_t screenCount();
    static std::size_t clampSelection(int selection);
    static Screen screenAt(std::size_t index);
    static const ScreenInfo& info(Screen screen);
    static Screen next(Screen current);
    static Screen previous(Screen current);
};
}
#endif
