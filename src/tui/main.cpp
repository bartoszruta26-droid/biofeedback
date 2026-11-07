#include "tui/TuiModel.hpp"
#include "sensor/SerialCommunication.hpp"
#include "sensor/MeasurementSerialAdapter.hpp"
#include "measurement/MeasurementEngine.hpp"
#include "training/RepetitionDetector.hpp"
#include "core/DebugManager.hpp"

#include <ncurses.h>

#include <algorithm>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

namespace {
constexpr int kMinimumColumns = 72;
constexpr int kMinimumRows = 20;

struct LiveData {
    mutable std::mutex mutex;
    biofeedback::measurement::MeasurementSample latest;
    bool hasSample = false;
    std::vector<double> recentForceN;
    bool trainingActive = false;
    bool trainingPaused = false;
    double trainingTargetForceN = 50.0;
    int targetSeriesCount = 3;
    int targetRepsPerSeries = 8;
    int completedSeries = 0;
    int completedReps = 0;
    std::vector<biofeedback::training::RepetitionRecord> currentSeries;
    biofeedback::training::SeriesSummary lastSeries;
    biofeedback::training::RepetitionDetector repetitionDetector;
};

std::vector<std::string> scanSerialPortCandidates()
{
    auto ports = sensor::SerialCommunication::scanAvailablePorts();
    std::sort(ports.begin(), ports.end());
    return ports;
}

void drawText(int y, int x, const std::string& value, int maxWidth = -1)
{
    if (y < 0 || y >= LINES || x < 0 || x >= COLS) return;
    const int available = maxWidth < 0 ? COLS - x : std::min(maxWidth, COLS - x);
    if (available > 0) mvaddnstr(y, x, value.c_str(), available);
}

void drawPanel(int y, int x, int height, int width, const char* title)
{
    if (height < 3 || width < 4 || y < 0 || x < 0 ||
        y + height > LINES || x + width > COLS) return;
    attron(COLOR_PAIR(2));
    mvhline(y, x, ACS_HLINE, width);
    mvhline(y + height - 1, x, ACS_HLINE, width);
    mvvline(y, x, ACS_VLINE, height);
    mvvline(y, x + width - 1, ACS_VLINE, height);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER);
    mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    attroff(COLOR_PAIR(2));
    attron(A_BOLD | COLOR_PAIR(3));
    drawText(y, x + 2, title, width - 4);
    attroff(A_BOLD | COLOR_PAIR(3));
}

std::string sampleValue(const LiveData& live)
{
    std::lock_guard<std::mutex> lock(live.mutex);
    if (!live.hasSample || !live.latest.isValid()) return "-- N";
    const double forceN = live.latest.unit == "kg"
        ? live.latest.value * 9.80665 : live.latest.value;
    std::ostringstream value;
    value << std::fixed << std::setprecision(2) << forceN << " N";
    return value.str();
}

void drawDashboard(const sensor::SerialCommunication& serial, const LiveData& live)
{
    const int left = 25;
    const int contentWidth = COLS - left - 2;
    const int panelWidth = std::max(22, (contentWidth - 3) / 2);
    const bool connected = serial.isConnected();
    drawPanel(3, left, 7, panelWidth, "STAN SYSTEMU");
    drawText(5, left + 2, "Aplikacja: uruchomiona");
    drawText(6, left + 2, std::string("Czujnik: ") + (connected ? "polaczony" : "rozlaczony"));
    drawText(7, left + 2, connected ? serial.getPortName() : "Port: --");
    drawText(8, left + 2, "Sesja: oczekiwanie na pomiar");

    drawPanel(3, left + panelWidth + 2, 7, panelWidth, "PODGLAD POMIARU");
    drawText(5, left + panelWidth + 4, std::string("Sila: ") + sampleValue(live));
    drawText(6, left + panelWidth + 4, "Probkowanie: wg konfiguracji urzadzenia");
    drawText(7, left + panelWidth + 4, "Ostatnia probka: strumien szeregowy");
    drawText(8, left + panelWidth + 4, connected ? "Dane: polaczenie aktywne" : "Dane: brak polaczenia");

    drawPanel(11, left, 6, contentWidth, "SZYBKI DOSTEP");
    drawText(13, left + 2, "Wybierz modul z menu po lewej stronie.");
    drawText(14, left + 2, "Pomiary i trening otrzymuja probki z tego samego adaptera.");
    drawText(15, left + 2, "Karty pacjentow nie sa tworzone ani symulowane w TUI.");
}

void drawModule(tui::Screen screen,
                const sensor::SerialCommunication& serial,
                const std::vector<std::string>& ports,
                std::size_t selectedPort,
                const LiveData& live)
{
    const auto& info = tui::TuiModel::info(screen);
    const int x = 25;
    const int width = COLS - x - 2;
    drawPanel(3, x, LINES - 7, width, info.title);
    attron(A_BOLD | COLOR_PAIR(3));
    drawText(5, x + 2, info.subtitle, width - 4);
    attroff(A_BOLD | COLOR_PAIR(3));

    switch (screen) {
    case tui::Screen::Measurements: {
        std::vector<double> history;
        std::uint64_t sequence = 0;
        std::string quality = "brak probki";
        {
            std::lock_guard<std::mutex> lock(live.mutex);
            history = live.recentForceN;
            if (live.hasSample) {
                sequence = live.latest.sequence;
                quality = biofeedback::measurement::qualityName(live.latest.quality);
            }
        }
        drawText(8, x + 2, serial.isConnected() ? "Czujnik polaczony - odczyt na zywo." : "Brak polaczenia z czujnikiem.");
        drawText(10, x + 2, std::string("Sila: ") + sampleValue(live));
        drawText(11, x + 2, "Sekwencja: " + std::to_string(sequence) + " | Jakosc probki: " + quality);
        drawText(13, x + 2, "Historia sily [N], ostatnie probki:");
        const int graphWidth = std::max(8, width - 6);
        const std::size_t shown = std::min<std::size_t>(history.size(), static_cast<std::size_t>(graphWidth));
        double maxForce = 1.0;
        for (std::size_t i = history.size() - shown; i < history.size(); ++i)
            maxForce = std::max(maxForce, history[i]);
        for (std::size_t i = 0; i < shown && 15 + static_cast<int>(i) < LINES - 5; ++i) {
            const double ratio = std::max(0.0, history[history.size() - shown + i] / maxForce);
            const int bar = static_cast<int>(ratio * std::max(1, graphWidth - 12));
            drawText(15 + static_cast<int>(i), x + 2,
                     std::string(8, ' ') + std::string(static_cast<std::size_t>(bar), '#'));
        }
        break;
    }
    case tui::Screen::Patients:
        drawText(8, x + 2, "Karty pacjentow sa dostepne w aplikacji Desktop Qt.");
        drawText(10, x + 2, "Wspolne repozytorium pacjentow nie jest jeszcze podlaczone do TUI.");
        drawText(12, x + 2, "Widok nie wyswietla ani nie tworzy fikcyjnych rekordow.");
        break;
    case tui::Screen::Training: {
        bool active = false, paused = false;
        double target = 0.0;
        int series = 0, reps = 0, totalSeries = 0, totalReps = 0;
        biofeedback::training::SeriesSummary last;
        {
            std::lock_guard<std::mutex> lock(live.mutex);
            active = live.trainingActive;
            paused = live.trainingPaused;
            target = live.trainingTargetForceN;
            series = live.completedSeries;
            reps = live.completedReps;
            totalSeries = live.targetSeriesCount;
            totalReps = live.targetRepsPerSeries;
            last = live.lastSeries;
        }
        drawText(8, x + 2, std::string("Sesja: ") + (!active ? "zakonczona / bezczynna" : (paused ? "pauza" : "aktywna")));
        drawText(10, x + 2, "Cel: " + std::to_string(static_cast<int>(target)) + " N | Serie: " +
                 std::to_string(series) + "/" + std::to_string(totalSeries) +
                 " | Powtorzenia: " + std::to_string(reps) + "/" +
                 std::to_string(totalSeries * totalReps));
        drawText(12, x + 2, "Plan demonstracyjny: 3 serie x 8 powtorzen. Start/Stop: S, Pauza/Wznow: P.");
        drawText(14, x + 2, "Ostatnia seria: " + std::to_string(last.repetitionCount) +
                 " powt. | sredni szczyt: " +
                 std::to_string(static_cast<int>(last.averagePeakForceN)) + " N | CV: " +
                 std::to_string(static_cast<int>(last.peakForceCoefficientOfVariationPercent)) + "%");
        drawText(16, x + 2, "Zmiana celu: + / - (co 5 N). Dane wymagaja podlaczonego i skalibrowanego HX711.");
        break;
    }
    case tui::Screen::Devices: {
        drawText(8, x + 2, "Wykryte porty szeregowe:");
        const int availableRows = std::max(0, std::min(4, LINES - 16));
        if (ports.empty()) {
            drawText(10, x + 2, "Nie znaleziono dostepnych portow.");
        } else {
            for (std::size_t i = 0; i < ports.size() && i < static_cast<std::size_t>(availableRows); ++i) {
                const int y = 10 + static_cast<int>(i);
                if (i == selectedPort) attron(A_REVERSE | A_BOLD);
                drawText(y, x + 2, ports[i]);
                if (i == selectedPort) attroff(A_REVERSE | A_BOLD);
            }
        }
        const std::string state = serial.isConnected()
            ? std::string("Polaczono: ") + serial.getPortName()
            : "Stan: rozlaczono";
        drawText(LINES - 6, x + 2, state);
        break;
    }
    case tui::Screen::Diagnostics:
        drawText(8, x + 2, "Interfejs terminalowy: aktywny");
        drawText(10, x + 2, serial.isConnected() ? "Adapter szeregowy: polaczony" : "Adapter szeregowy: rozlaczony");
        drawText(12, x + 2, "Tryb: podglad, bez zapisu danych klinicznych");
        break;
    case tui::Screen::Settings:
        drawText(8, x + 2, "Nawigacja: strzalki, Home/End, Enter, Q lub Esc");
        drawText(10, x + 2, "Urzadzenia: lewo/prawo, C - polacz, D - rozlacz, R - skanuj");
        drawText(12, x + 2, "Predkosc transmisji: 115200 baud");
        break;
    case tui::Screen::Dashboard:
        drawDashboard(serial, live);
        break;
    }
}

void draw(std::size_t selected,
          std::size_t active,
          const sensor::SerialCommunication& serial,
          const std::vector<std::string>& ports,
          std::size_t selectedPort,
          const LiveData& live)
{
    erase();
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(0, 0, ' ', COLS);
    drawText(0, 2, "BIOFEEDBACK  /  TERMINAL WORKSPACE");
    attroff(A_BOLD | COLOR_PAIR(1));
    drawText(1, 2, "Interfejs TUI | wspolny adapter szeregowy");
    drawPanel(3, 1, LINES - 7, 22, "MODULY");
    const auto& screens = tui::TuiModel::screens();
    for (std::size_t i = 0; i < screens.size(); ++i) {
        const int y = 5 + static_cast<int>(i);
        const std::string marker = (i == active ? "* " : "  ");
        if (i == selected) {
            attron(A_REVERSE | A_BOLD);
            drawText(y, 3, std::string("> ") + marker + screens[i].title, 18);
            attroff(A_REVERSE | A_BOLD);
        } else {
            drawText(y, 3, marker + screens[i].title, 18);
        }
    }
    drawModule(screens[active].id, serial, ports, selectedPort, live);
    attron(COLOR_PAIR(1));
    mvhline(LINES - 3, 0, ' ', COLS);
    if (active == static_cast<std::size_t>(tui::Screen::Devices)) {
        drawText(LINES - 3, 2, "LEWO/PRAWO: port   C: polacz   D: rozlacz   R: skanuj   Q: wyjscie");
    } else if (active == static_cast<std::size_t>(tui::Screen::Training)) {
        drawText(LINES - 3, 2, "S: start/stop   P: pauza/wznow   +/-: cel sily   Q: wyjscie");
    } else {
        drawText(LINES - 3, 2, "GORA/DOL: wybor   ENTER: otworz   HOME/END: skrajne   Q: wyjscie");
    }
    attroff(COLOR_PAIR(1));
    refresh();
}
}

int main(int argc, char** argv)
{
    if (argc > 1) {
        const std::string option(argv[1]);
        if (option == "--version") {
            std::puts("Biofeedback TUI 0.2.0");
            return EXIT_SUCCESS;
        }
        if (option == "--help") {
            std::puts("Biofeedback TUI 0.2.0");
            std::puts("Usage: biofeedback_tui [--help|--version|--status|--list-ports]");
            return EXIT_SUCCESS;
        }
        if (option == "--status") {
            std::puts("TUI: ready");
            std::puts("Sensor: disconnected (interactive connection required)");
            std::puts("Measurement: idle");
            return EXIT_SUCCESS;
        }
        if (option == "--list-ports") {
            const auto ports = scanSerialPortCandidates();
            if (ports.empty()) std::puts("No serial port candidates found.");
            else for (const auto& port : ports) std::cout << port << std::endl;
            return EXIT_SUCCESS;
        }
    }

    std::setlocale(LC_ALL, "");
    core::DebugManager::instance().setConsoleOutputEnabled(false);
    sensor::SerialCommunication serial;
    serial.setVerbose(false);
    LiveData live;
    auto measurementEngine = std::make_shared<biofeedback::measurement::MeasurementEngine>();
    // Firmware calibration is reported in grams. The shared engine normalizes
    // it to kilograms; presentation and training convert kg to SI force (N).
    sensor::MeasurementSerialAdapter measurementAdapter(serial, measurementEngine, "tui-hx711", "g");
    const auto measurementToken = measurementEngine->subscribe([&live](const biofeedback::measurement::MeasurementSample& sample) {
        std::lock_guard<std::mutex> lock(live.mutex);
        live.latest = sample;
        live.hasSample = true;
        if (!sample.isValid() || sample.unit != "kg") return;
        const double forceN = sample.value * 9.80665;
        if (!std::isfinite(forceN) || forceN < 0.0) return;
        live.recentForceN.push_back(forceN);
        if (live.recentForceN.size() > 120) live.recentForceN.erase(live.recentForceN.begin());
        if (!live.trainingActive || live.trainingPaused) return;
        biofeedback::training::RepetitionRecord repetition;
        if (!live.repetitionDetector.process(sample.deviceTimestampMs, forceN, true, repetition))
            return;
        live.currentSeries.push_back(repetition);
        ++live.completedReps;
        if (static_cast<int>(live.currentSeries.size()) >= live.targetRepsPerSeries) {
            live.lastSeries = biofeedback::training::summarizeSeries(
                live.currentSeries, live.trainingTargetForceN);
            live.currentSeries.clear();
            ++live.completedSeries;
            live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
            if (live.completedSeries >= live.targetSeriesCount) {
                live.trainingActive = false;
                live.trainingPaused = false;
            }
        }
    });
    auto ports = scanSerialPortCandidates();
    std::size_t selectedPort = 0;

    initscr();
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_CYAN, -1);
        init_pair(3, COLOR_YELLOW, -1);
    }
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(250);

    if (LINES < kMinimumRows || COLS < kMinimumColumns) {
        endwin();
        measurementEngine->unsubscribe(measurementToken);
        std::fprintf(stderr, "Terminal must be at least %dx%d characters.\n", kMinimumColumns, kMinimumRows);
        return EXIT_FAILURE;
    }

    std::size_t selected = 0;
    std::size_t active = 0;
    bool running = true;
    while (running) {
        draw(selected, active, serial, ports, selectedPort, live);
        const int key = getch();
        switch (key) {
        case KEY_UP:
            selected = tui::TuiModel::clampSelection(static_cast<int>(selected) - 1);
            break;
        case KEY_DOWN:
            selected = tui::TuiModel::clampSelection(static_cast<int>(selected) + 1);
            break;
        case KEY_HOME: selected = 0; break;
        case KEY_END: selected = tui::TuiModel::screenCount() - 1; break;
        case KEY_ENTER:
        case 10:
        case 13: active = selected; break;
        case KEY_LEFT:
            if (active == static_cast<std::size_t>(tui::Screen::Devices) && selectedPort > 0) --selectedPort;
            break;
        case KEY_RIGHT:
            if (active == static_cast<std::size_t>(tui::Screen::Devices) && selectedPort + 1 < ports.size()) ++selectedPort;
            break;
        case 'r':
        case 'R':
            if (active == static_cast<std::size_t>(tui::Screen::Devices)) {
                ports = scanSerialPortCandidates();
                selectedPort = std::min(selectedPort, ports.empty() ? std::size_t(0) : ports.size() - 1);
            }
            break;
        case 'c':
        case 'C':
            if (active == static_cast<std::size_t>(tui::Screen::Devices) &&
                !serial.isConnected() && selectedPort < ports.size()) {
                if (serial.connect(ports[selectedPort], DEFAULT_BAUD_RATE)) serial.startAsyncReading();
            }
            break;
        case 'd':
        case 'D':
            if (active == static_cast<std::size_t>(tui::Screen::Devices) && serial.isConnected()) serial.disconnect();
            break;
        case 's':
        case 'S':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) {
                    live.trainingActive = true;
                    live.trainingPaused = false;
                    live.completedSeries = 0;
                    live.completedReps = 0;
                    live.currentSeries.clear();
                    live.lastSeries = {};
                    live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
                } else {
                    live.trainingActive = false;
                    live.trainingPaused = false;
                    live.repetitionDetector.reset();
                }
            }
            break;
        case 'p':
        case 'P':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (live.trainingActive) {
                    live.trainingPaused = !live.trainingPaused;
                    live.repetitionDetector.reset();
                }
            }
            break;
        case '+':
        case '=':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                live.trainingTargetForceN = std::min(500.0, live.trainingTargetForceN + 5.0);
                live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
            }
            break;
        case '-':
        case '_':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                live.trainingTargetForceN = std::max(5.0, live.trainingTargetForceN - 5.0);
                live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
            }
            break;
        case 'q':
        case 'Q':
        case 27:
            running = false;
            break;
        default:
            break;
        }
    }

    endwin();
    serial.disconnect();
    measurementEngine->unsubscribe(measurementToken);
    return EXIT_SUCCESS;
}
