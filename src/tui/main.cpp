#include "tui/TuiModel.hpp"
#include <ncurses.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
namespace {
constexpr int kMinimumColumns = 72;
constexpr int kMinimumRows = 20;
void drawText(int y, int x, const std::string& value, int maxWidth = -1) {
    if (y < 0 || y >= LINES || x < 0 || x >= COLS) return;
    const int available = maxWidth < 0 ? COLS - x : std::min(maxWidth, COLS - x);
    if (available > 0) mvaddnstr(y, x, value.c_str(), available);
}
void drawPanel(int y, int x, int height, int width, const char* title) {
    if (height < 3 || width < 4 || y < 0 || x < 0 || y + height > LINES || x + width > COLS) return;
    attron(COLOR_PAIR(2));
    mvhline(y, x, ACS_HLINE, width); mvhline(y + height - 1, x, ACS_HLINE, width);
    mvvline(y, x, ACS_VLINE, height); mvvline(y, x + width - 1, ACS_VLINE, height);
    mvaddch(y, x, ACS_ULCORNER); mvaddch(y, x + width - 1, ACS_URCORNER);
    mvaddch(y + height - 1, x, ACS_LLCORNER); mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
    attroff(COLOR_PAIR(2));
    attron(A_BOLD | COLOR_PAIR(3)); drawText(y, x + 2, title, width - 4); attroff(A_BOLD | COLOR_PAIR(3));
}
void drawDashboard() {
    const int left = 25;
    const int contentWidth = COLS - left - 2;
    const int panelWidth = std::max(22, (contentWidth - 3) / 2);
    drawPanel(3, left, 7, panelWidth, "STAN SYSTEMU");
    drawText(5, left + 2, "Aplikacja: uruchomiona");
    drawText(6, left + 2, "Tryb danych: DEMONSTRACYJNY");
    drawText(7, left + 2, "Czujnik: niepodlaczony");
    drawText(8, left + 2, "Sesja: brak aktywnego pomiaru");
    drawPanel(3, left + panelWidth + 2, 7, panelWidth, "PODGLAD POMIARU");
    drawText(5, left + panelWidth + 4, "Sila: -- N");
    drawText(6, left + panelWidth + 4, "Probkowanie: -- Hz");
    drawText(7, left + panelWidth + 4, "Ostatnia probka: --");
    drawText(8, left + panelWidth + 4, "Dane rzeczywiste: niedostepne");
    drawPanel(11, left, 7, contentWidth, "SZYBKI DOSTEP");
    drawText(13, left + 2, "Wybierz modul z menu po lewej stronie.");
    drawText(14, left + 2, "Strzalki gora/dol: nawigacja   Enter: wybierz modul");
    drawText(15, left + 2, "Dane demonstracyjne nie sa danymi pacjenta ani odczytem czujnika.");
}
void drawModule(tui::Screen screen) {
    const auto& info = tui::TuiModel::info(screen);
    const int x = 25, width = COLS - x - 2;
    drawPanel(3, x, LINES - 7, width, info.title);
    attron(A_BOLD | COLOR_PAIR(3)); drawText(5, x + 2, info.subtitle, width - 4); attroff(A_BOLD | COLOR_PAIR(3));
    switch (screen) {
    case tui::Screen::Measurements:
        drawText(8, x + 2, "Brak aktywnego pomiaru.");
        drawText(10, x + 2, "Podlacz i zweryfikuj czujnik przed rozpoczeciem sesji.");
        drawText(12, x + 2, "Integracja z MeasurementEngine jest kolejnym etapem.");
        break;
    case tui::Screen::Patients:
        drawText(8, x + 2, "Karty pacjentow sa obecnie dostepne w aplikacji Desktop Qt.");
        drawText(10, x + 2, "Integracja wspolnego repozytorium z TUI jest kolejnym etapem.");
        drawText(12, x + 2, "Widok nie wyswietla ani nie tworzy fikcyjnych rekordow.");
        break;
    case tui::Screen::Training:
        drawText(8, x + 2, "Brak aktywnej sesji treningowej.");
        drawText(10, x + 2, "Moduly treningowe i gry sa obecnie dostepne w aplikacji Desktop Qt.");
        break;
    case tui::Screen::Devices:
        drawText(8, x + 2, "Port szeregowy: nieustawiony");
        drawText(10, x + 2, "Stan czujnika: niepodlaczony");
        drawText(12, x + 2, "Ta wersja TUI nie wysyla komend do urzadzenia.");
        break;
    case tui::Screen::Diagnostics:
        drawText(8, x + 2, "Interfejs terminalowy: aktywny");
        drawText(10, x + 2, "Backend pomiarowy: oczekuje na adapter aplikacyjny");
        drawText(12, x + 2, "Tryb: podglad, bez zapisu danych klinicznych");
        break;
    case tui::Screen::Settings:
        drawText(8, x + 2, "Kolorystyka: terminalowa");
        drawText(10, x + 2, "Nawigacja: strzalki, Home/End, Q lub Esc");
        drawText(12, x + 2, "Konfiguracja urzadzen bedzie udostepniona po integracji uslug.");
        break;
    case tui::Screen::Dashboard: drawDashboard(); break;
    }
}
void draw(std::size_t selected) {
    erase();
    attron(A_BOLD | COLOR_PAIR(1)); mvhline(0, 0, ' ', COLS);
    drawText(0, 2, "BIOFEEDBACK  /  TERMINAL WORKSPACE"); attroff(A_BOLD | COLOR_PAIR(1));
    drawText(1, 2, "Interfejs TUI | C++17 | tryb demonstracyjny");
    drawPanel(3, 1, LINES - 7, 22, "MODULY");
    const auto& screens = tui::TuiModel::screens();
    for (std::size_t i = 0; i < screens.size(); ++i) {
        const int y = 5 + static_cast<int>(i);
        if (i == selected) { attron(A_REVERSE | A_BOLD); drawText(y, 3, std::string("> ") + screens[i].title, 18); attroff(A_REVERSE | A_BOLD); }
        else drawText(y, 3, std::string("  ") + screens[i].title, 18);
    }
    drawModule(screens[selected].id);
    attron(COLOR_PAIR(1)); mvhline(LINES - 3, 0, ' ', COLS);
    drawText(LINES - 3, 2, "GORA/DOL: wybor   HOME/END: skrajne moduly   Q: wyjscie"); attroff(COLOR_PAIR(1));
    refresh();
}
}
int main(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--version") {
        std::puts("Biofeedback TUI 0.1.0");
        return EXIT_SUCCESS;
    }
    initscr();
    if (has_colors()) {
        start_color(); use_default_colors();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_CYAN, -1);
        init_pair(3, COLOR_YELLOW, -1);
    }
    cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    if (LINES < kMinimumRows || COLS < kMinimumColumns) {
        endwin();
        std::fprintf(stderr, "Terminal must be at least %dx%d characters.\\n", kMinimumColumns, kMinimumRows);
        return EXIT_FAILURE;
    }
    std::size_t selected = 0;
    bool running = true;
    while (running) {
        draw(selected);
        switch (getch()) {
        case KEY_UP: selected = tui::TuiModel::clampSelection(static_cast<int>(selected) - 1); break;
        case KEY_DOWN: selected = tui::TuiModel::clampSelection(static_cast<int>(selected) + 1); break;
        case KEY_HOME: selected = 0; break;
        case KEY_END: selected = tui::TuiModel::screenCount() - 1; break;
        case 'q': case 'Q': case 27: running = false; break;
        default: break;
        }
    }
    endwin();
    return EXIT_SUCCESS;
}
