#include "tui/TuiModel.hpp"
#include "sensor/SerialCommunication.hpp"
#include "sensor/MeasurementSerialAdapter.hpp"
#include "measurement/MeasurementEngine.hpp"
#include "training/RepetitionDetector.hpp"
#include "core/DebugManager.hpp"
#include "core/RuntimePaths.hpp"
#include "core/Authentication.hpp"
#include "api/HttpApiClient.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

#include <ncurses.h>

#include <algorithm>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
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

std::unique_ptr<biofeedback::api::HttpApiClient> authenticateTuiUser() {
    const QString usersPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("users.json"));
    if (usersPath.isEmpty()) {
        std::cerr << "Nie można przygotować magazynu kont.\n";
        return {};
    }

    auto authentication = std::make_unique<biofeedback::Authentication>(usersPath.toStdString());
    const QString configPath = biofeedback::runtime::ensureUserConfigFile(QStringLiteral("config.json"));
    QFile configFile(configPath);
    bool encryptionEnabled = false;
    const QString encryptionKey = QString::fromUtf8(qgetenv("BIOFEEDBACK_ENCRYPTION_KEY"));
    if (!configPath.isEmpty() && configFile.open(QIODevice::ReadOnly)) {
        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(configFile.readAll(), &parseError);
        if (parseError.error == QJsonParseError::NoError && document.isObject())
            encryptionEnabled = document.object().value(QStringLiteral("encryption"))
                .toObject().value(QStringLiteral("enabled")).toBool(false);
    }
    if (encryptionEnabled && encryptionKey.toUtf8().size() < 32) {
        std::cerr << "Szyfrowanie kont jest włączone, ale brakuje BIOFEEDBACK_ENCRYPTION_KEY.\n";
        return {};
    }
    authentication->setFileEncryption(encryptionEnabled, encryptionKey.toStdString());
    if (!authentication->loadUsers()) {
        std::cerr << "Nie można odczytać magazynu kont.\n";
        return {};
    }

    if (!authentication->hasUsers()) {
        std::cout << "Pierwsze uruchomienie TUI — utworzenie administratora.\n";
        std::string username;
        std::cout << "Nazwa administratora: ";
        if (!std::getline(std::cin, username) || username.empty()) return {};
        char* first = getpass("Nowe hasło (minimum 12 znaków): ");
        if (!first) return {};
        std::string password(first);
        std::memset(first, 0, std::strlen(first));
        char* second = getpass("Powtórz hasło: ");
        if (!second) {
            std::fill(password.begin(), password.end(), '\0');
            return {};
        }
        const bool matches = password == second;
        std::memset(second, 0, std::strlen(second));
        if (!matches || !authentication->createInitialAdministrator(username, password)) {
            std::fill(password.begin(), password.end(), '\0');
            std::cerr << "Nie udało się utworzyć administratora.\n";
            return {};
        }
        std::fill(password.begin(), password.end(), '\0');
        std::cout << "Konto administratora utworzone.\n";
    }

    std::string username;
    std::cout << "Użytkownik: ";
    if (!std::getline(std::cin, username) || username.empty()) return {};
    char* input = getpass("Hasło: ");
    if (!input) return {};
    std::string password(input);
    std::memset(input, 0, std::strlen(input));

    const bool loggedIn = authentication->login(username, password);
    if (!loggedIn || authentication->requiresPasswordChange()) {
        std::fill(password.begin(), password.end(), '\0');
        authentication->logout();
        std::cerr << "Logowanie nie powiodło się lub wymagane jest ustawienie nowego hasła.\n";
        return {};
    }

    QString apiUrl = QString::fromUtf8(qgetenv("BIOFEEDBACK_API_URL")).trimmed();
    if (apiUrl.isEmpty()) apiUrl = QStringLiteral("http://127.0.0.1:8765");
    std::unique_ptr<biofeedback::api::HttpApiClient> apiClient;
    try {
        apiClient = std::make_unique<biofeedback::api::HttpApiClient>(QUrl(apiUrl));
    } catch (const std::exception& error) {
        std::fill(password.begin(), password.end(), '\0');
        authentication->logout();
        std::cerr << "Niepoprawny adres serwera API: " << error.what() << "\n";
        return {};
    }
    QString apiError;
    const bool apiLogin = apiClient->login(
        QString::fromStdString(username), QString::fromStdString(password), &apiError);
    std::fill(password.begin(), password.end(), '\0');
    authentication->logout();
    if (!apiLogin) {
        std::cerr << "Nie można zalogować do wspólnego serwera danych: "
                  << apiError.toStdString() << "\n";
        return {};
    }
    std::cout << "Zalogowano do wspólnego serwera danych.\n";
    return apiClient;
}

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
    std::string trainingMessage = "Polacz czujnik, aby rozpoczac trening.";
    std::string exerciseName = "Uscisk dloni";
    int exercisePresetIndex = 0;
    struct SessionRep { int seriesIndex; int repetitionIndex; biofeedback::training::RepetitionRecord measurement; };
    std::vector<biofeedback::training::RepetitionRecord> currentSeries;
    std::vector<SessionRep> sessionRepetitions;
    biofeedback::training::SeriesSummary lastSeries;
    biofeedback::training::RepetitionDetector repetitionDetector;
    int restSeconds = 60;
    int restRemaining = 0;
    bool trainingResting = false;
    qint64 lastRestTickMs = 0;
    bool sessionNeedsSave = false;
    bool sessionSaved = false;
    bool sessionSaveAttempted = false;
    QString sessionStartedAt;
    QString selectedPatientId;
    QJsonArray measurementSamples;
    std::uint64_t validSampleCount = 0;
    std::uint64_t invalidSampleCount = 0;
    QString lastSampleQuality = "brak probki";
    double lastPeakForceN = 0.0;
    double lastMeanForceN = 0.0;
    std::uint32_t lastRepDurationMs = 0;
    std::uint32_t lastTimeToPeakMs = 0;
};

std::vector<std::string> scanSerialPortCandidates()
{
    auto ports = sensor::SerialCommunication::scanAvailablePorts();
    std::sort(ports.begin(), ports.end());
    return ports;
}


void saveMeasurementSession(LiveData& live, biofeedback::api::HttpApiClient& apiClient)
{
    QJsonArray samples;
    QString startedAt;
    QString patientId;
    {
        std::lock_guard<std::mutex> lock(live.mutex);
        if (live.measurementSamples.isEmpty()) return;
        samples = live.measurementSamples;
        startedAt = live.sessionStartedAt;
        patientId = live.selectedPatientId;
        live.measurementSamples = QJsonArray();
    }
    const QJsonObject payload{
        {QStringLiteral("started_at"), startedAt.isEmpty()
            ? QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs) : startedAt},
        {QStringLiteral("finished_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("patient_id"), patientId},
        {QStringLiteral("sample_count"), samples.size()},
        {QStringLiteral("samples"), samples},
        {QStringLiteral("source_client"), QStringLiteral("tui")},
        {QStringLiteral("status"), QStringLiteral("completed")}
    };
    QJsonObject stored;
    QString error;
    if (!apiClient.createResource(QStringLiteral("measurements"), payload, stored, &error)) {
        std::lock_guard<std::mutex> lock(live.mutex);
        live.measurementSamples = samples;
        live.trainingMessage = "Blad archiwizacji pomiaru: " + error.toStdString();
    }
}

void saveTrainingSession(LiveData& live, biofeedback::api::HttpApiClient& apiClient)
{
    std::vector<LiveData::SessionRep> repetitions;
    QString startedAt;
    QString exercise;
    double targetForce = 0.0;
    int targetSeries = 0;
    int targetReps = 0;
    int completedSeries = 0;
    int completedReps = 0;
    QString patientId;
    {
        std::lock_guard<std::mutex> lock(live.mutex);
        if (!live.sessionNeedsSave || live.sessionSaved || live.sessionSaveAttempted ||
            live.sessionRepetitions.empty()) return;
        live.sessionSaveAttempted = true;
        repetitions = live.sessionRepetitions;
        startedAt = live.sessionStartedAt;
        exercise = QString::fromStdString(live.exerciseName);
        targetForce = live.trainingTargetForceN;
        targetSeries = live.targetSeriesCount;
        targetReps = live.targetRepsPerSeries;
        completedSeries = live.completedSeries;
        completedReps = live.completedReps;
        patientId = live.selectedPatientId;
    }

    QJsonArray repetitionRecords;
    for (const auto& item : repetitions) {
        const auto& rep = item.measurement;
        repetitionRecords.append(QJsonObject{
            {QStringLiteral("series_index"), item.seriesIndex},
            {QStringLiteral("repetition_index"), item.repetitionIndex},
            {QStringLiteral("start_timestamp_ms"), static_cast<qint64>(rep.startTimestampMs)},
            {QStringLiteral("end_timestamp_ms"), static_cast<qint64>(rep.endTimestampMs)},
            {QStringLiteral("duration_ms"), static_cast<qint64>(rep.durationMs)},
            {QStringLiteral("time_to_peak_ms"), static_cast<qint64>(rep.timeToPeakMs)},
            {QStringLiteral("peak_force_n"), rep.peakForceN},
            {QStringLiteral("mean_force_n"), rep.meanForceN},
            {QStringLiteral("sample_count"), static_cast<qint64>(rep.sampleCount)},
            {QStringLiteral("target_achieved"), rep.peakForceN >= targetForce}
        });
    }

    const QJsonObject payload{
        {QStringLiteral("started_at"), startedAt},
        {QStringLiteral("finished_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("exercise_name"), exercise},
        {QStringLiteral("patient_id"), patientId},

        {QStringLiteral("target_force_n"), targetForce},
        {QStringLiteral("target_series_count"), targetSeries},
        {QStringLiteral("target_reps_per_series"), targetReps},
        {QStringLiteral("completed_series"), completedSeries},
        {QStringLiteral("completed_repetitions"), completedReps},
        {QStringLiteral("repetitions"), repetitionRecords},
        {QStringLiteral("source_client"), QStringLiteral("tui")}
    };

    QJsonObject created;
    QString error;
    const bool saved = apiClient.createResource(
        QStringLiteral("training-sessions"), payload, created, &error);
    QString resultWarning;
    if (saved) {
        const QJsonObject resultPayload{
            {QStringLiteral("training_session_id"), created.value(QStringLiteral("id")).toString()},
            {QStringLiteral("patient_id"), patientId},
            {QStringLiteral("exercise_name"), exercise},
            {QStringLiteral("completed_series"), completedSeries},
            {QStringLiteral("completed_repetitions"), completedReps},
            {QStringLiteral("target_force_n"), targetForce},
            {QStringLiteral("repetitions"), repetitionRecords},
            {QStringLiteral("source_client"), QStringLiteral("tui")}
        };
        QJsonObject storedResult;
        if (!apiClient.createResource(QStringLiteral("training-results"),
                                      resultPayload, storedResult, &resultWarning)) {
            resultWarning = "Szczegoly wyniku nie zostaly zapisane: " + resultWarning;
        }
    }
    {
        std::lock_guard<std::mutex> lock(live.mutex);
        if (saved) {
            live.sessionSaved = true;
            live.sessionNeedsSave = false;
            live.trainingMessage = "Sesja zapisana w serwerze danych: " +
                created.value(QStringLiteral("id")).toString().toStdString();
            if (!resultWarning.isEmpty())
                live.trainingMessage += " (" + resultWarning.toStdString() + ")";
        } else {
            live.trainingMessage = "Blad zapisu sesji w serwerze: " + error.toStdString();
        }
    }
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
                const LiveData& live,
                const std::vector<std::string>& patientSummaries)
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
        drawText(8, x + 2, "Karty pobrane z tego samego repozytorium co Desktop i WebUI.");
        if (patientSummaries.empty()) {
            drawText(10, x + 2, "Brak kart pacjentow lub brak uprawnien do ich odczytu.");
        } else {
            const int visible = std::min<int>(static_cast<int>(patientSummaries.size()), LINES - 14);
            for (int i = 0; i < visible; ++i)
                drawText(10 + i, x + 2, patientSummaries[static_cast<std::size_t>(i)], width - 4);
            if (static_cast<int>(patientSummaries.size()) > visible)
                drawText(11 + visible, x + 2, "... pozostale rekordy: " +
                         std::to_string(patientSummaries.size() - static_cast<std::size_t>(visible)));
        }
        drawText(LINES - 6, x + 2, "Odczyt odbywa sie przez uwierzytelnione API serwera.");
        break;
    case tui::Screen::Training: {
        bool active = false, paused = false;
        double target = 0.0;
        int series = 0, reps = 0, totalSeries = 0, totalReps = 0, repsInSeries = 0;
        int restSeconds = 0, restRemaining = 0;
        bool resting = false;
        std::string exerciseName;
        std::string message;
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
            repsInSeries = static_cast<int>(live.currentSeries.size());
            restSeconds = live.restSeconds;
            restRemaining = live.restRemaining;
            resting = live.trainingResting;
            exerciseName = live.exerciseName;
            message = live.trainingMessage;
            last = live.lastSeries;
        }
        drawText(8, x + 2, std::string("Sesja: ") + (!active ? "zakonczona / bezczynna" : (paused ? "pauza" : "aktywna")));
        drawText(9, x + 2, "Cwiczenie: " + exerciseName);
        drawText(10, x + 2, "Cel: " + std::to_string(static_cast<int>(target)) + " N | Serie: " +
                 std::to_string(series) + "/" + std::to_string(totalSeries) +
                 " | Powtorzenia: " + std::to_string(reps) + "/" +
                 std::to_string(totalSeries * totalReps));
        drawText(12, x + 2, "Plan: " + std::to_string(totalSeries) + " serie x " +
                 std::to_string(totalReps) + " powt. | biezaca seria: " +
                 std::to_string(repsInSeries) + "/" + std::to_string(totalReps));
        drawText(14, x + 2, "Ostatnia seria: " + std::to_string(last.repetitionCount) +
                 " powt. | sredni szczyt: " +
                 std::to_string(static_cast<int>(last.averagePeakForceN)) + " N | CV: " +
                 std::to_string(static_cast<int>(last.peakForceCoefficientOfVariationPercent)) + "%");
        drawText(16, x + 2, "Odpoczynek: " + std::to_string(restSeconds) + " s" +
                 (resting ? " | pozostalo: " + std::to_string(restRemaining) + " s" : " | gotowy"));
        drawText(17, x + 2, "Plan: [/] serie, ,/. powt., +/- cel, ;/' odpoczynek +/- 5 s.");
        drawText(19, x + 2, message);
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
    case tui::Screen::Diagnostics: {
        std::uint64_t validSamples = 0, invalidSamples = 0;
        QString quality;
        double peak = 0.0, mean = 0.0;
        std::uint32_t duration = 0, timeToPeak = 0;
        {
            std::lock_guard<std::mutex> lock(live.mutex);
            validSamples = live.validSampleCount;
            invalidSamples = live.invalidSampleCount;
            quality = live.lastSampleQuality;
            peak = live.lastPeakForceN;
            mean = live.lastMeanForceN;
            duration = live.lastRepDurationMs;
            timeToPeak = live.lastTimeToPeakMs;
        }
        drawText(8, x + 2, "Interfejs terminalowy: aktywny");
        drawText(10, x + 2, serial.isConnected() ? "Adapter szeregowy: polaczony" : "Adapter szeregowy: rozlaczony");
        drawText(11, x + 2, "Probki poprawne: " + std::to_string(validSamples) +
                 " | odrzucone: " + std::to_string(invalidSamples));
        drawText(13, x + 2, "Jakosc ostatniej probki: " + quality.toStdString());
        drawText(15, x + 2, "Ostatnie powtorzenie: szczyt " + std::to_string(peak) +
                 " N | srednia " + std::to_string(mean) + " N | czas " +
                 std::to_string(duration) + " ms | do szczytu " +
                 std::to_string(timeToPeak) + " ms");
        drawText(16, x + 2, "Historia: wspolne repozytorium serwera danych");
        break;
    }
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
          const LiveData& live,
          const std::vector<std::string>& patientSummaries)
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
    drawModule(screens[active].id, serial, ports, selectedPort, live, patientSummaries);
    attron(COLOR_PAIR(1));
    mvhline(LINES - 3, 0, ' ', COLS);
    if (active == static_cast<std::size_t>(tui::Screen::Devices)) {
        drawText(LINES - 3, 2, "LEWO/PRAWO: port   C: polacz   D: rozlacz   R: skanuj   Q: wyjscie");
    } else if (active == static_cast<std::size_t>(tui::Screen::Training)) {
        drawText(LINES - 3, 2, "S: start/stop P: pauza E: cwiczenie +/-: cel [ ]: serie , .: powt. ;: odpoczynek - / apostrof +");
    } else {
        drawText(LINES - 3, 2, "GORA/DOL: wybor   ENTER: otworz   HOME/END: skrajne   Q: wyjscie");
    }
    attroff(COLOR_PAIR(1));
    refresh();
}
}

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("Biofeedback");
    QCoreApplication::setOrganizationName("Biofeedback");
    QCoreApplication::setApplicationName("Biofeedback");
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

    auto apiClient = authenticateTuiUser();
    if (!apiClient) return EXIT_FAILURE;

    std::vector<std::string> patientSummaries;
    QStringList patientIds;
    QJsonArray sharedPatients;
    QString patientLoadError;
    if (apiClient->listResources(QStringLiteral("patients"), sharedPatients, &patientLoadError)) {
        for (const QJsonValue& value : sharedPatients) {
            if (!value.isObject()) continue;
            const QJsonObject patient = value.toObject();
            const QString label = patient.value(QStringLiteral("firstName")).toString() +
                QStringLiteral(" ") + patient.value(QStringLiteral("lastName")).toString();
            const QString id = patient.value(QStringLiteral("id")).toString();
            patientSummaries.push_back((label + QStringLiteral(" [") + id + QStringLiteral("]")).toStdString());
            patientIds.push_back(id);
        }
    } else {
        patientSummaries.push_back("Blad odczytu repozytorium: " + patientLoadError.toStdString());
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
        live.lastSampleQuality = QString::fromLatin1(biofeedback::measurement::qualityName(sample.quality));
        if (!sample.isValid() || sample.unit != "kg") {
            ++live.invalidSampleCount;
            return;
        }
        ++live.validSampleCount;
        const double forceN = sample.value * 9.80665;
        if (!std::isfinite(forceN) || forceN < 0.0) return;
        if (live.measurementSamples.size() < 100000) {
            live.measurementSamples.append(QJsonObject{
                {QStringLiteral("timestamp_ms"), static_cast<qint64>(sample.deviceTimestampMs)},
                {QStringLiteral("force_n"), forceN},
                {QStringLiteral("quality"), QString::fromLatin1(biofeedback::measurement::qualityName(sample.quality))}
            });
        }
        live.recentForceN.push_back(forceN);
        if (live.recentForceN.size() > 120) live.recentForceN.erase(live.recentForceN.begin());
        if (!live.trainingActive || live.trainingPaused || live.trainingResting) return;
        biofeedback::training::RepetitionRecord repetition;
        if (!live.repetitionDetector.process(sample.deviceTimestampMs, forceN, true, repetition))
            return;
        live.currentSeries.push_back(repetition);
        live.sessionRepetitions.push_back({live.completedSeries + 1,
            static_cast<int>(live.currentSeries.size()), repetition});
        live.lastPeakForceN = repetition.peakForceN;
        live.lastMeanForceN = repetition.meanForceN;
        live.lastRepDurationMs = repetition.durationMs;
        live.lastTimeToPeakMs = repetition.timeToPeakMs;
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
                live.trainingMessage = "Plan treningowy wykonany.";
                live.sessionNeedsSave = true;
            } else if (live.restSeconds > 0) {
                live.trainingResting = true;
                live.restRemaining = live.restSeconds;
                live.lastRestTickMs = QDateTime::currentMSecsSinceEpoch();
                live.trainingMessage = "Seria zakonczona - trwa odpoczynek.";
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
        draw(selected, active, serial, ports, selectedPort, live, patientSummaries);
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
                if (serial.connect(ports[selectedPort], sensor::protocol::DefaultBaudRate)) serial.startAsyncReading();
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
                    if (!serial.isConnected()) {
                        live.trainingMessage = "Najpierw polacz czujnik w module Urzadzenia.";
                    } else {
                        live.trainingActive = true;
                        live.trainingPaused = false;
                        live.completedSeries = 0;
                        live.completedReps = 0;
                        live.currentSeries.clear();
                        live.sessionRepetitions.clear();
                        live.measurementSamples = QJsonArray();
                        live.sessionStartedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
                        if (live.selectedPatientId.isEmpty() && !patientIds.isEmpty())
                            live.selectedPatientId = patientIds.first();
                        live.lastSeries = {};
                        live.trainingResting = false;
                        live.restRemaining = 0;
                        live.sessionStartedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
                        live.sessionNeedsSave = false;
                        live.sessionSaved = false;
                        live.sessionSaveAttempted = false;
                        live.trainingMessage = "Trening aktywny - wykonuj pelne cykle napiecia i zwolnienia.";
                        live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
                    }
                } else {
                    live.trainingActive = false;
                    live.trainingPaused = false;
                    live.trainingResting = false;
                    live.restRemaining = 0;
                    live.sessionNeedsSave = !live.sessionRepetitions.empty();
                    live.trainingMessage = "Sesja zatrzymana przez operatora.";
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
                    live.trainingMessage = live.trainingPaused ? "Trening wstrzymany." : "Trening wznowiony.";
                    if (!live.trainingPaused && live.trainingResting)
                        live.lastRestTickMs = QDateTime::currentMSecsSinceEpoch();
                    live.repetitionDetector.reset();
                }
            }
            break;
        case '+':
        case '=':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) {
                    live.trainingTargetForceN = std::min(500.0, live.trainingTargetForceN + 5.0);
                    live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
                }
            }
            break;
        case '-':
        case '_':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) {
                    live.trainingTargetForceN = std::max(5.0, live.trainingTargetForceN - 5.0);
                    live.repetitionDetector.setTargetForce(live.trainingTargetForceN);
                }
            }
            break;
        case '[':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.targetSeriesCount = std::max(1, live.targetSeriesCount - 1);
            }
            break;
        case ']':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.targetSeriesCount = std::min(10, live.targetSeriesCount + 1);
            }
            break;
        case 'e':
        case 'E':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) {
                    static const char* names[] = {
                        "Uscisk dloni", "Zgiecie nadgarstka",
                        "Prostowanie nadgarstka", "Cwiczenie ogolne"
                    };
                    live.exercisePresetIndex = (live.exercisePresetIndex + 1) % 4;
                    live.exerciseName = names[live.exercisePresetIndex];
                }
            }
            break;
        case ',':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.targetRepsPerSeries = std::max(1, live.targetRepsPerSeries - 1);
            }
            break;
        case '.':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.targetRepsPerSeries = std::min(50, live.targetRepsPerSeries + 1);
            }
            break;
        case ';':
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.restSeconds = std::max(0, live.restSeconds - 5);
            }
            break;
        case 39:
            if (active == static_cast<std::size_t>(tui::Screen::Training)) {
                std::lock_guard<std::mutex> lock(live.mutex);
                if (!live.trainingActive) live.restSeconds = std::min(600, live.restSeconds + 5);
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
        {
            std::lock_guard<std::mutex> lock(live.mutex);
            if (live.trainingResting && !live.trainingPaused) {
                const qint64 now = QDateTime::currentMSecsSinceEpoch();
                const qint64 elapsedSeconds = (now - live.lastRestTickMs) / 1000;
                if (elapsedSeconds > 0) {
                    live.restRemaining = std::max(0, live.restRemaining - static_cast<int>(elapsedSeconds));
                    live.lastRestTickMs += elapsedSeconds * 1000;
                    if (live.restRemaining == 0) {
                        live.trainingResting = false;
                        live.trainingMessage = "Odpoczynek zakonczony - rozpoczyna sie kolejna seria.";
                    }
                }
            }
        }
        saveMeasurementSession(live, *apiClient);
        saveTrainingSession(live, *apiClient);
    }

    {
        std::lock_guard<std::mutex> lock(live.mutex);
        if (live.trainingActive) {
            live.trainingActive = false;
            live.trainingPaused = false;
            live.sessionNeedsSave = !live.sessionRepetitions.empty();
        }
    }
    saveMeasurementSession(live, *apiClient);
    saveTrainingSession(live, *apiClient);
    endwin();
    serial.disconnect();
    measurementEngine->unsubscribe(measurementToken);
    apiClient->logout();
    return EXIT_SUCCESS;
}
