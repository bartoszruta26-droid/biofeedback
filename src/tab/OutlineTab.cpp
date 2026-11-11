#include "tab/OutlineTab.hpp"
#include "games/GameEngine.hpp"
#include "games/PongGame.hpp"
#include "games/FlappyBirdGame.hpp"
#include "games/MarioGame.hpp"
#include "games/SinGame.hpp"
#include "gui/GraphWidget.hpp"
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QMetaObject>
#include <QFrame>
#include <cmath>
#include <QFileDialog>
#include <QPdfWriter>
#include <QFile>
#include <QFileInfo>
#include <QPageSize>
#include <QPageLayout>
#include <QTextDocument>
#include <QMarginsF>
#include <QTextStream>
#include <QDateTime>
#include <QDate>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <limits>
#include <QPainterPath>


namespace {

struct ForceSeries {
    QString label;
    QVector<gui::DataPoint> points;
};

class ForceOverlayWidget final : public QWidget {
public:
    explicit ForceOverlayWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setSeries(const QVector<ForceSeries>& series) {
        m_series = series;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), palette().base());
        const QRectF area = rect().adjusted(58, 18, -18, -38);
        p.setPen(palette().mid().color());
        p.drawRect(area);
        if (m_series.isEmpty()) {
            p.setPen(palette().text().color());
            p.drawText(area, Qt::AlignCenter, tr("Zapisz przebieg lub wybierz serie do porównania."));
            return;
        }

        qint64 maxTime = 0;
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        for (const auto& series : m_series) {
            if (series.points.isEmpty()) continue;
            const qint64 start = series.points.first().timestamp.toMSecsSinceEpoch();
            for (const auto& point : series.points) {
                maxTime = qMax(maxTime, point.timestamp.toMSecsSinceEpoch() - start);
                minValue = qMin(minValue, point.value);
                maxValue = qMax(maxValue, point.value);
            }
        }
        if (maxTime <= 0 || !std::isfinite(minValue) || !std::isfinite(maxValue)) {
            p.setPen(palette().text().color());
            p.drawText(area, Qt::AlignCenter, tr("Za mało poprawnych danych do nałożenia przebiegów."));
            return;
        }
        if (maxValue - minValue < 1e-9) { minValue -= 1.0; maxValue += 1.0; }
        const double margin = (maxValue - minValue) * 0.08;
        minValue -= margin;
        maxValue += margin;

        p.setPen(QPen(palette().mid().color(), 1, Qt::DashLine));
        for (int i = 0; i <= 4; ++i) {
            const qreal y = area.top() + area.height() * i / 4.0;
            p.drawLine(QPointF(area.left(), y), QPointF(area.right(), y));
            const double value = maxValue - (maxValue - minValue) * i / 4.0;
            p.setPen(palette().text().color());
            p.drawText(QRectF(2, y - 9, 50, 18), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(value, 'f', 1) + " N");
            p.setPen(QPen(palette().mid().color(), 1, Qt::DashLine));
        }

        const QVector<QColor> colors = {QColor("#1565c0"), QColor("#d84315"),
                                         QColor("#2e7d32"), QColor("#8e24aa"),
                                         QColor("#00838f"), QColor("#ad7a00")};
        int index = 0;
        for (const auto& series : m_series) {
            if (series.points.size() < 2) { ++index; continue; }
            const qint64 start = series.points.first().timestamp.toMSecsSinceEpoch();
            QPainterPath path;
            bool begun = false;
            for (const auto& point : series.points) {
                const double xRatio = qBound(0.0,
                    static_cast<double>(point.timestamp.toMSecsSinceEpoch() - start) / maxTime, 1.0);
                const double yRatio = qBound(0.0, (point.value - minValue) / (maxValue - minValue), 1.0);
                const QPointF pos(area.left() + xRatio * area.width(),
                                  area.bottom() - yRatio * area.height());
                if (!begun) { path.moveTo(pos); begun = true; } else path.lineTo(pos);
            }
            const QColor seriesColor = colors[index % colors.size()];
            p.setPen(QPen(seriesColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawPath(path);

            double seriesSum = 0.0;
            double seriesPeak = std::numeric_limits<double>::lowest();
            qint64 peakTime = 0;
            for (const auto& point : series.points) {
                seriesSum += point.value;
                if (point.value > seriesPeak) {
                    seriesPeak = point.value;
                    peakTime = point.timestamp.toMSecsSinceEpoch() - start;
                }
            }
            const double seriesMean = seriesSum / series.points.size();
            const double meanY = area.bottom()
                - qBound(0.0, (seriesMean - minValue) / (maxValue - minValue), 1.0) * area.height();
            p.setPen(QPen(seriesColor, 1, Qt::DashLine));
            p.drawLine(QPointF(area.left(), meanY), QPointF(area.right(), meanY));
            const double peakXRatio = qBound(0.0, static_cast<double>(peakTime) / maxTime, 1.0);
            const double peakYRatio = qBound(0.0, (seriesPeak - minValue) / (maxValue - minValue), 1.0);
            const QPointF peakPos(area.left() + peakXRatio * area.width(),
                                  area.bottom() - peakYRatio * area.height());
            p.setPen(QPen(seriesColor, 2));
            p.setBrush(palette().base());
            p.drawEllipse(peakPos, 4.5, 4.5);
            p.setPen(seriesColor);
            p.drawText(peakPos + QPointF(6, -6), QString("max %1 N").arg(seriesPeak, 0, 'f', 1));

            const QString legend = series.label;
            const int legendX = 64 + (index % 3) * qMax(110, (width() - 80) / 3);
            const int legendY = height() - 18 + (index / 3) * 14;
            p.setPen(colors[index % colors.size()]);
            p.drawText(legendX, legendY, legend.left(22));
            ++index;
        }
        p.setPen(palette().text().color());
        p.drawText(QRectF(area.left(), area.bottom() + 8, area.width(), 18),
                   Qt::AlignCenter, tr("Czas od początku próby (s)"));
    }

private:
    QVector<ForceSeries> m_series;
};

struct ForceMetrics {
    double mean{0.0};
    double peak{0.0};
    double peakMean{0.0};
    double standardDeviation{0.0};
    double coefficientOfVariation{0.0};
    double meanRiseMs{0.0};
    int repetitions{0};
    double durationSeconds{0.0};
};

ForceMetrics calculateForceMetrics(const QVector<gui::DataPoint>& points) {
    ForceMetrics result;
    if (points.size() < 3) return result;
    double sum = 0.0, sumSquares = 0.0;
    double minimum = std::numeric_limits<double>::max();
    double maximum = std::numeric_limits<double>::lowest();
    for (const auto& point : points) {
        if (!std::isfinite(point.value)) continue;
        sum += point.value;
        sumSquares += point.value * point.value;
        minimum = qMin(minimum, point.value);
        maximum = qMax(maximum, point.value);
    }
    const int count = points.size();
    result.mean = sum / count;
    result.peak = maximum;
    result.standardDeviation = std::sqrt(qMax(0.0, sumSquares / count - result.mean * result.mean));
    result.coefficientOfVariation = std::abs(result.mean) > 1e-9
        ? 100.0 * result.standardDeviation / std::abs(result.mean) : 0.0;
    result.durationSeconds = qMax<qint64>(0,
        points.last().timestamp.toMSecsSinceEpoch() - points.first().timestamp.toMSecsSinceEpoch()) / 1000.0;

    const double amplitude = maximum - minimum;
    if (amplitude <= 1e-9) return result;
    const double threshold = minimum + amplitude * 0.20;
    const double riseThreshold = minimum + amplitude * 0.10;
    const double peakThreshold = minimum + amplitude * 0.35;
    QVector<double> peakValues;
    QVector<double> riseTimes;
    qint64 lastPeakMs = std::numeric_limits<qint64>::min();
    for (int i = 1; i + 1 < points.size(); ++i) {
        const auto& before = points[i - 1];
        const auto& current = points[i];
        const auto& after = points[i + 1];
        if (current.value < threshold || current.value < peakThreshold ||
            current.value < before.value || current.value < after.value) continue;
        const qint64 peakMs = current.timestamp.toMSecsSinceEpoch();
        if (lastPeakMs != std::numeric_limits<qint64>::min() && peakMs - lastPeakMs < 300) continue;
        lastPeakMs = peakMs;
        peakValues.push_back(current.value);

        int startIndex = i;
        while (startIndex > 0 && points[startIndex].value > riseThreshold) --startIndex;
        const double ninety = minimum + (current.value - minimum) * 0.90;
        int riseIndex = startIndex;
        while (riseIndex < i && points[riseIndex].value < ninety) ++riseIndex;
        if (riseIndex <= i) {
            const double ms = points[riseIndex].timestamp.toMSecsSinceEpoch()
                            - points[startIndex].timestamp.toMSecsSinceEpoch();
            if (ms >= 0.0 && ms < 30000.0) riseTimes.push_back(ms);
        }
    }
    result.repetitions = peakValues.size();
    if (!peakValues.isEmpty()) {
        double peakSum = 0.0;
        for (double value : peakValues) peakSum += value;
        result.peakMean = peakSum / peakValues.size();
    }
    if (!riseTimes.isEmpty()) {
        double riseSum = 0.0;
        for (double value : riseTimes) riseSum += value;
        result.meanRiseMs = riseSum / riseTimes.size();
    }
    return result;
}

QVector<QVector<gui::DataPoint>> extractForceRepetitions(
    const QVector<gui::DataPoint>& points) {
    QVector<QVector<gui::DataPoint>> repetitions;
    if (points.size() < 5) return repetitions;
    double minimum = std::numeric_limits<double>::max();
    double maximum = std::numeric_limits<double>::lowest();
    for (const auto& point : points) {
        if (!std::isfinite(point.value)) continue;
        minimum = qMin(minimum, point.value);
        maximum = qMax(maximum, point.value);
    }
    const double amplitude = maximum - minimum;
    if (!std::isfinite(amplitude) || amplitude <= 1e-9) return repetitions;
    const double peakThreshold = minimum + amplitude * 0.35;
    const double boundaryThreshold = minimum + amplitude * 0.10;
    qint64 lastPeakMs = std::numeric_limits<qint64>::min();
    int lastEnd = -1;
    for (int i = 1; i + 1 < points.size(); ++i) {
        if (points[i].value < peakThreshold ||
            points[i].value < points[i - 1].value ||
            points[i].value < points[i + 1].value) continue;
        const qint64 peakMs = points[i].timestamp.toMSecsSinceEpoch();
        if (lastPeakMs != std::numeric_limits<qint64>::min() && peakMs - lastPeakMs < 300)
            continue;
        int begin = i;
        while (begin > lastEnd + 1 && points[begin].value > boundaryThreshold) --begin;
        int end = i;
        while (end + 1 < points.size() && points[end].value > boundaryThreshold) ++end;
        if (end - begin >= 2) {
            QVector<gui::DataPoint> segment;
            segment.reserve(end - begin + 1);
            for (int j = begin; j <= end; ++j) segment.push_back(points[j]);
            repetitions.push_back(segment);
            lastEnd = end;
            lastPeakMs = peakMs;
        }
    }
    return repetitions;
}

QString forceMetricsHtml(const QVector<gui::DataPoint>& points) {
    const ForceMetrics m = calculateForceMetrics(points);
    if (points.size() < 3) return QObject::tr("<p>Za mało próbek do analizy.</p>");
    auto v = [](double x, int decimals = 2) { return QString::number(x, 'f', decimals); };
    return QObject::tr(
        "<table><tr><th>Parametr</th><th>Wynik</th></tr>"
        "<tr><td>Liczba próbek</td><td>%1</td></tr>"
        "<tr><td>Czas próby</td><td>%2 s</td></tr>"
        "<tr><td>Średnia siła</td><td>%3 N</td></tr>"
        "<tr><td>Maksymalna siła</td><td>%4 N</td></tr>"
        "<tr><td>Średnia z wykrytych szczytów</td><td>%5 N</td></tr>"
        "<tr><td>Wykryte powtórzenia</td><td>%6</td></tr>"
        "<tr><td>Odchylenie standardowe</td><td>%7 N</td></tr>"
        "<tr><td>Współczynnik zmienności</td><td>%8%</td></tr>"
        "<tr><td>Średni czas narastania 10–90%</td><td>%9 ms</td></tr></table>"
        "<p><small>Wykrywanie powtórzeń: lokalne maksima z progiem amplitudy i "
        "minimalnym odstępem 300 ms. Parametry zależą od jakości próbkowania i "
        "wymagają walidacji dla danego czujnika oraz protokołu.</small></p>")
        .arg(points.size()).arg(v(m.durationSeconds)).arg(v(m.mean)).arg(v(m.peak))
        .arg(v(m.peakMean)).arg(m.repetitions).arg(v(m.standardDeviation))
        .arg(v(m.coefficientOfVariation)).arg(v(m.meanRiseMs, 0));
}

} // namespace

namespace tab {

OutlineTab::OutlineTab(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_outlineGroup(nullptr)
    , m_outlineSelectorLayout(nullptr)
    , m_outlineLabel(nullptr)
    , m_outlineCombo(nullptr)
    , m_loadOutlineButton(nullptr)
    , m_outlineInfoGroup(nullptr)
    , m_outlineNameLabel(nullptr)
    , m_outlineDescriptionLabel(nullptr)
    , m_outlineAuthorLabel(nullptr)
    , m_outlineDurationLabel(nullptr)
    , m_outlineExerciseCountLabel(nullptr)
    , m_exerciseGroup(nullptr)
    , m_exerciseLayout(nullptr)
    , m_exerciseInfoWidget(nullptr)
    , m_exerciseInfoLayout(nullptr)
    , m_exerciseNameLabel(nullptr)
    , m_exerciseDescriptionLabel(nullptr)
    , m_mediaLayout(nullptr)
    , m_exerciseImageLabel(nullptr)
    , m_videoWidget(nullptr)
    , m_playVideoButton(nullptr)
    , m_gameSelectionGroup(nullptr)
    , m_gameSelectionLayout(nullptr)
    , m_gameLabel(nullptr)
    , m_gameCombo(nullptr)
    , m_configureGameButton(nullptr)
    , m_controlGroup(nullptr)
    , m_controlLayout(nullptr)
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_pauseResumeButton(nullptr)
    , m_prevButton(nullptr)
    , m_nextButton(nullptr)
    , m_progressGroup(nullptr)
    , m_progressLayout(nullptr)
    , m_progressLabel(nullptr)
    , m_sessionProgress(nullptr)
    , m_currentExerciseLabel(nullptr)
    , m_gameAreaGroup(nullptr)
    , m_gameAreaLayout(nullptr)
    , m_gameStack(nullptr)
    , m_gamePlaceholder(nullptr)
    , m_sessionTimer(nullptr)
    , m_currentOutlineIndex(-1)
    , m_currentExerciseIndex(-1)
    , m_isSessionActive(false)
    , m_isPaused(false)
    , m_completedExercises(0)
    , m_sessionStartTime(0)
    , m_exerciseStartTime(0)
    , m_outlinesDirectory("./data/outlines")
    , m_currentGame(nullptr)
    , m_serialPort(nullptr)
{
    setupUI();
    createConnections();
    
    // Domyślnie załaduj konspekty z katalogu
    loadOutlines(m_outlinesDirectory);
}

OutlineTab::~OutlineTab()
{
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    if (m_isSessionActive) {
        stopSession();
    }
}

void OutlineTab::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(6);
    m_mainLayout->setContentsMargins(8, 8, 8, 8);

    auto* workspace = new QSplitter(Qt::Horizontal, this);
    workspace->setChildrenCollapsible(false);
    workspace->setHandleWidth(6);

    auto* visualScroll = new QScrollArea(workspace);
    visualScroll->setWidgetResizable(true);
    visualScroll->setFrameShape(QFrame::NoFrame);
    auto* visualPanel = new QWidget(visualScroll);
    m_leftLayout = new QVBoxLayout(visualPanel);
    m_leftLayout->setContentsMargins(4, 4, 4, 4);
    m_leftLayout->setSpacing(8);
    visualScroll->setWidget(visualPanel);

    auto* settingsScroll = new QScrollArea(workspace);
    settingsScroll->setWidgetResizable(true);
    settingsScroll->setFrameShape(QFrame::NoFrame);
    settingsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* settingsPanel = new QWidget(settingsScroll);
    m_rightLayout = new QVBoxLayout(settingsPanel);
    m_rightLayout->setContentsMargins(4, 4, 4, 4);
    m_rightLayout->setSpacing(8);
    settingsScroll->setWidget(settingsPanel);

    workspace->addWidget(visualScroll);
    workspace->addWidget(settingsScroll);
    visualScroll->setMinimumWidth(420);
    settingsScroll->setMinimumWidth(300);
    workspace->setStretchFactor(0, 64);
    workspace->setStretchFactor(1, 36);
    workspace->setSizes({900, 500});
    m_mainLayout->addWidget(workspace, 1);

    m_patientContextLabel = new QLabel(tr("Pacjent: nie wybrano"), visualPanel);
    m_patientContextLabel->setObjectName(QStringLiteral("rehabilitationPatientContext"));
    m_patientContextLabel->setWordWrap(true);
    m_patientContextLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_leftLayout->addWidget(m_patientContextLabel);

    auto* episodeGroup = new QGroupBox(tr("Epizod rehabilitacji"), visualPanel);
    auto* episodeLayout = new QVBoxLayout(episodeGroup);
    m_episodeSelector = new QComboBox(episodeGroup);
    m_episodeSelector->setObjectName(QStringLiteral("rehabilitationEpisodeSelector"));
    m_episodeDiagnosisEdit = new QLineEdit(episodeGroup);
    m_episodeDiagnosisEdit->setPlaceholderText(tr("Rozpoznanie / problem kliniczny"));
    m_episodePathwayCombo = new QComboBox(episodeGroup);
    m_episodePathwayCombo->addItem(tr("Postępowanie zachowawcze"), QStringLiteral("conservative"));
    m_episodePathwayCombo->addItem(tr("Po meniscektomii"), QStringLiteral("post_meniscectomy"));
    m_episodePathwayCombo->addItem(tr("Po naprawie / szyciu"), QStringLiteral("post_repair"));
    m_episodePathwayCombo->addItem(tr("Inna ścieżka"), QStringLiteral("other"));
    m_createEpisodeButton = new QPushButton(tr("Utwórz epizod"), episodeGroup);
    m_episodeStatusLabel = new QLabel(tr("Wybierz pacjenta, aby zarządzać epizodami."), episodeGroup);
    m_episodeStatusLabel->setWordWrap(true);
    episodeLayout->addWidget(m_episodeSelector);
    episodeLayout->addWidget(m_episodeDiagnosisEdit);
    episodeLayout->addWidget(m_episodePathwayCombo);
    episodeLayout->addWidget(m_createEpisodeButton);
    episodeLayout->addWidget(m_episodeStatusLabel);
    m_leftLayout->addWidget(episodeGroup);

    auto* assessmentGroup = new QGroupBox(tr("Ocena kliniczna pacjenta"), visualPanel);
    auto* assessmentLayout = new QVBoxLayout(assessmentGroup);
    assessmentLayout->addWidget(new QLabel(tr("Rodzaj oceny"), assessmentGroup));
    m_assessmentTypeCombo = new QComboBox(assessmentGroup);
    m_assessmentTypeCombo->addItem(tr("Początkowa"), QStringLiteral("baseline"));
    m_assessmentTypeCombo->addItem(tr("Kontrolna"), QStringLiteral("follow_up"));
    m_assessmentTypeCombo->addItem(tr("Końcowa"), QStringLiteral("final"));
    m_assessmentTypeCombo->setObjectName(QStringLiteral("rehabilitationAssessmentType"));
    assessmentLayout->addWidget(m_assessmentTypeCombo);
    auto addAssessmentField = [assessmentGroup, assessmentLayout](const QString& label,
                                                                  QLineEdit*& field,
                                                                  const QString& placeholder) {
        assessmentLayout->addWidget(new QLabel(label, assessmentGroup));
        field = new QLineEdit(assessmentGroup);
        field->setPlaceholderText(placeholder);
        assessmentLayout->addWidget(field);
    };
    addAssessmentField(tr("Ból — NRS (0–10)"), m_assessmentPainNrsEdit,
                       tr("Pozostaw puste, jeśli nie oceniono"));
    addAssessmentField(tr("Zakres ruchu — ROM (stopnie)"), m_assessmentRomEdit,
                       tr("Wartość w stopniach; 0 jest prawidłową wartością pomiaru"));
    addAssessmentField(tr("Siła mięśniowa — MRC (0–5)"), m_assessmentStrengthEdit,
                       tr("Stopień MRC; pozostaw puste, jeśli nie oceniono"));
    addAssessmentField(tr("Obwód / obrzęk (cm)"), m_assessmentSwellingEdit,
                       tr("Wartość w cm; pozostaw puste, jeśli nie zmierzono"));
    addAssessmentField(tr("Test funkcjonalny / wynik"), m_assessmentFunctionalTestEdit,
                       tr("Nazwa testu i wynik, np. TUG: 12,4 s"));
    m_saveAssessmentButton = new QPushButton(tr("Zapisz ocenę"), assessmentGroup);
    m_exportAssessmentReportButton = new QPushButton(tr("Eksportuj raport epizodu do PDF"), assessmentGroup);
    m_exportAssessmentReportButton->setObjectName(QStringLiteral("exportRehabilitationReportPdf"));
    m_assessmentStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod, aby zapisać ocenę."), assessmentGroup);
    m_assessmentStatusLabel->setWordWrap(true);
    m_assessmentHistoryBrowser = new QTextBrowser(assessmentGroup);
    m_assessmentHistoryBrowser->setMaximumHeight(130);
    m_assessmentHistoryBrowser->setOpenExternalLinks(false);
    assessmentLayout->addWidget(m_saveAssessmentButton);
    assessmentLayout->addWidget(m_exportAssessmentReportButton);
    assessmentLayout->addWidget(m_assessmentStatusLabel);
    assessmentLayout->addWidget(m_assessmentHistoryBrowser);
    m_leftLayout->addWidget(assessmentGroup);

    auto* goalsGroup = new QGroupBox(tr("Cele terapeutyczne"), visualPanel);
    auto* goalsLayout = new QVBoxLayout(goalsGroup);
    auto addGoalField = [goalsGroup, goalsLayout](const QString& label,
                                                  QLineEdit*& field,
                                                  const QString& placeholder) {
        goalsLayout->addWidget(new QLabel(label, goalsGroup));
        field = new QLineEdit(goalsGroup);
        field->setPlaceholderText(placeholder);
        goalsLayout->addWidget(field);
    };
    addGoalField(tr("Cel funkcjonalny"), m_goalTitleEdit, tr("Np. samodzielne wejście po schodach"));
    addGoalField(tr("Wskaźnik / metoda pomiaru"), m_goalMetricEdit, tr("Np. TUG, ROM zgięcia kolana"));
    addGoalField(tr("Wartość początkowa"), m_goalBaselineEdit, tr("Wartość liczbowa"));
    addGoalField(tr("Wartość docelowa"), m_goalTargetEdit, tr("Wartość liczbowa"));
    addGoalField(tr("Jednostka"), m_goalUnitEdit, tr("Np. s, stopnie, N, wynik skali"));
    addGoalField(tr("Termin (RRRR-MM-DD, opcjonalnie)"), m_goalTargetDateEdit, tr("2026-12-01"));
    m_createGoalButton = new QPushButton(tr("Zapisz cel terapeutyczny"), goalsGroup);
    m_goalStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod, aby zarządzać celami."), goalsGroup);
    m_goalStatusLabel->setWordWrap(true);
    m_goalHistoryBrowser = new QTextBrowser(goalsGroup);
    m_goalHistoryBrowser->setMaximumHeight(150);
    m_goalHistoryBrowser->setOpenExternalLinks(false);
    goalsLayout->addWidget(m_createGoalButton);
    goalsLayout->addWidget(m_goalStatusLabel);
    goalsLayout->addWidget(m_goalHistoryBrowser);
    m_leftLayout->addWidget(goalsGroup);

    auto* sessionGroup = new QGroupBox(tr("Sesja rehabilitacyjna — ból i tolerancja"), visualPanel);
    auto* sessionLayout = new QVBoxLayout(sessionGroup);
    auto addSessionPainField = [sessionGroup, sessionLayout](const QString& label,
                                                             QLineEdit*& field,
                                                             const QString& placeholder) {
        sessionLayout->addWidget(new QLabel(label, sessionGroup));
        field = new QLineEdit(sessionGroup);
        field->setPlaceholderText(placeholder);
        sessionLayout->addWidget(field);
    };
    addSessionPainField(tr("Ból przed sesją — NRS (0–10)"), m_sessionPainBeforeEdit,
                        tr("Wartość wymagana do rozpoczęcia"));
    addSessionPainField(tr("Ból po sesji — NRS (0–10)"), m_sessionPainAfterEdit,
                        tr("Wartość wymagana do zakończenia"));
    addSessionPainField(tr("Ból po 24 godzinach — NRS (0–10)"), m_sessionPain24hEdit,
                        tr("Uzupełnij po upływie około 24 godzin"));
    m_startRehabilitationSessionButton = new QPushButton(tr("Rozpocznij sesję"), sessionGroup);
    m_finishRehabilitationSessionButton = new QPushButton(tr("Zakończ sesję"), sessionGroup);
    m_saveRehabilitation24hButton = new QPushButton(tr("Zapisz kontrolę po 24 godzinach"), sessionGroup);
    m_rehabilitationSessionStatusLabel = new QLabel(tr("Wybierz pacjenta i epizod."), sessionGroup);
    m_rehabilitationSessionStatusLabel->setWordWrap(true);
    sessionLayout->addWidget(m_startRehabilitationSessionButton);
    sessionLayout->addWidget(m_finishRehabilitationSessionButton);
    sessionLayout->addWidget(m_saveRehabilitation24hButton);
    sessionLayout->addWidget(m_rehabilitationSessionStatusLabel);
    m_leftLayout->addWidget(sessionGroup);
    connect(m_startRehabilitationSessionButton, &QPushButton::clicked,
            this, &OutlineTab::startRehabilitationSession);
    connect(m_finishRehabilitationSessionButton, &QPushButton::clicked,
            this, &OutlineTab::finishRehabilitationSession);
    connect(m_saveRehabilitation24hButton, &QPushButton::clicked,
            this, &OutlineTab::recordRehabilitation24HourFollowUp);
    m_startRehabilitationSessionButton->setEnabled(false);
    m_finishRehabilitationSessionButton->setEnabled(false);
    m_saveRehabilitation24hButton->setEnabled(false);
    m_sessionPainBeforeEdit->setEnabled(false);
    m_sessionPainAfterEdit->setEnabled(false);
    m_sessionPain24hEdit->setEnabled(false);


    auto* observationGroup = new QGroupBox(tr("Obserwacja wykonania ćwiczenia"), visualPanel);
    auto* observationLayout = new QVBoxLayout(observationGroup);
    auto addObservationField = [observationGroup, observationLayout](const QString& label,
                                                                      QLineEdit*& field,
                                                                      const QString& placeholder) {
        observationLayout->addWidget(new QLabel(label, observationGroup));
        field = new QLineEdit(observationGroup);
        field->setPlaceholderText(placeholder);
        observationLayout->addWidget(field);
    };
    addObservationField(tr("Ćwiczenie"), m_observationExerciseEdit, tr("Nazwa ćwiczenia"));
    addObservationField(tr("Wykonane serie"), m_observationSetsEdit, tr("Liczba całkowita"));
    addObservationField(tr("Wykonane powtórzenia"), m_observationRepetitionsEdit, tr("Łączna liczba powtórzeń"));
    addObservationField(tr("Czas wykonania (sekundy)"), m_observationDurationEdit, tr("Opcjonalnie"));
    addObservationField(tr("Odczuwalny wysiłek — RPE (0–10)"), m_observationRpeEdit, tr("Opcjonalnie"));
    observationLayout->addWidget(new QLabel(tr("Jakość ruchu"), observationGroup));
    m_observationMovementQualityCombo = new QComboBox(observationGroup);
    m_observationMovementQualityCombo->addItem(tr("Nie oceniono"), QStringLiteral("not_assessed"));
    m_observationMovementQualityCombo->addItem(tr("Prawidłowa"), QStringLiteral("acceptable"));
    m_observationMovementQualityCombo->addItem(tr("Nieprawidłowa"), QStringLiteral("unacceptable"));
    observationLayout->addWidget(m_observationMovementQualityCombo);
    addObservationField(tr("Ból przed ćwiczeniem — NRS (0–10)"), m_observationPainBeforeEdit,
                        tr("Wartość wymagana"));
    addObservationField(tr("Ból po ćwiczeniu — NRS (0–10)"), m_observationPainAfterEdit,
                        tr("Wartość wymagana"));
    addObservationField(tr("Ból po 24 godzinach — NRS (0–10)"), m_observationPain24hEdit,
                        tr("Opcjonalnie, po kontroli"));
    m_saveObservationButton = new QPushButton(tr("Zapisz obserwację ćwiczenia"), observationGroup);
    m_observationStatusLabel = new QLabel(tr("Wymagana aktywna sesja rehabilitacyjna."), observationGroup);
    m_observationStatusLabel->setWordWrap(true);
    m_observationHistoryBrowser = new QTextBrowser(observationGroup);
    m_observationHistoryBrowser->setMaximumHeight(160);
    m_observationHistoryBrowser->setOpenExternalLinks(false);
    observationLayout->addWidget(m_saveObservationButton);
    observationLayout->addWidget(m_observationStatusLabel);
    observationLayout->addWidget(m_observationHistoryBrowser);
    m_leftLayout->addWidget(observationGroup);
    connect(m_saveObservationButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationObservation);
    m_saveObservationButton->setEnabled(false);
    for (QLineEdit* field : {m_observationExerciseEdit, m_observationSetsEdit,
                             m_observationRepetitionsEdit, m_observationDurationEdit,
                             m_observationRpeEdit, m_observationPainBeforeEdit,
                             m_observationPainAfterEdit, m_observationPain24hEdit})
        field->setEnabled(false);

    connect(m_createGoalButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationGoal);
    m_createGoalButton->setEnabled(false);
    for (QLineEdit* field : {m_goalTitleEdit, m_goalMetricEdit, m_goalBaselineEdit,
                             m_goalTargetEdit, m_goalUnitEdit, m_goalTargetDateEdit})
        field->setEnabled(false);

    connect(m_saveAssessmentButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationAssessment);
    connect(m_exportAssessmentReportButton, &QPushButton::clicked,
            this, &OutlineTab::exportRehabilitationReportPdf);
    m_saveAssessmentButton->setEnabled(false);
    m_exportAssessmentReportButton->setEnabled(false);
    if (m_assessmentTypeCombo) m_assessmentTypeCombo->setEnabled(false);
    for (QLineEdit* field : {m_assessmentPainNrsEdit, m_assessmentRomEdit,
                             m_assessmentStrengthEdit, m_assessmentSwellingEdit,
                             m_assessmentFunctionalTestEdit}) {
        field->setEnabled(false);
    }

    connect(m_episodeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                m_currentEpisodeUuid = index >= 0
                    ? m_episodeSelector->itemData(index).toString() : QString();
                refreshRehabilitationAssessments();
                refreshRehabilitationGoals();
                refreshRehabilitationSession();
                refreshRehabilitationObservations();
            });
    connect(m_createEpisodeButton, &QPushButton::clicked,
            this, &OutlineTab::createRehabilitationEpisode);
    m_episodeSelector->setEnabled(false);
    m_episodeDiagnosisEdit->setEnabled(false);
    m_episodePathwayCombo->setEnabled(false);
    m_createEpisodeButton->setEnabled(false);

    setupOutlineSelector();
    setupExerciseView();
    setupControlPanel();
    setupGameArea();

    // Ustawienia konspektu, gry i sterowania mają odrębne karty. Dzięki temu
    // panel boczny nie wymaga przewijania przez wszystkie grupy jednocześnie.
    auto* settingsTabs = new QTabWidget(settingsPanel);
    settingsTabs->setObjectName(QStringLiteral("outlineSettingsTabs"));
    for (const auto& page : {
             QPair<QWidget*, QString>(m_outlineGroup, tr("Konspekt")),
             QPair<QWidget*, QString>(m_gameSelectionGroup, tr("Gra")),
             QPair<QWidget*, QString>(m_controlGroup, tr("Sterowanie"))}) {
        m_rightLayout->removeWidget(page.first);
        settingsTabs->addTab(page.first, page.second);
    }
    m_rightLayout->insertWidget(0, settingsTabs);
    m_rightLayout->addWidget(m_outlineInfoGroup);
    m_outlineInfoGroup->setMaximumHeight(220);

    auto* graphGroup = new QGroupBox(tr("Biofeedback — siła w czasie rzeczywistym"), visualPanel);
    auto* graphLayout = new QVBoxLayout(graphGroup);
    m_forceGraph = new gui::GraphWidget(graphGroup);
    m_forceGraph->setTitle(tr("Siła z czujnika"));
    m_forceGraph->setYLabel(tr("Siła"));
    m_forceGraph->setUnit(tr("N"));
    m_forceGraph->setTimeRange(30);
    m_forceGraph->setMinimumHeight(260);
    m_forceGraph->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Panel sterowania wykresem: zakres czasu, autoskalowanie, skala ręczna,
    // siatka, etykiety wartości oraz eksport surowych próbek do CSV.
    auto* graphToolbar = new QHBoxLayout();
    auto* timeRangeCombo = new QComboBox(graphGroup);
    timeRangeCombo->setObjectName(QStringLiteral("outlineGraphTimeRange"));
    timeRangeCombo->addItem(tr("10 s"), 10);
    timeRangeCombo->addItem(tr("30 s"), 30);
    timeRangeCombo->addItem(tr("60 s"), 60);
    timeRangeCombo->addItem(tr("120 s"), 120);
    timeRangeCombo->addItem(tr("300 s"), 300);
    timeRangeCombo->setCurrentIndex(1);
    auto* autoScaleCheck = new QCheckBox(tr("Autoskalowanie Y"), graphGroup);
    autoScaleCheck->setObjectName(QStringLiteral("outlineGraphAutoScale"));
    autoScaleCheck->setChecked(true);
    auto* yMinSpin = new QDoubleSpinBox(graphGroup);
    auto* yMaxSpin = new QDoubleSpinBox(graphGroup);
    yMinSpin->setObjectName(QStringLiteral("outlineGraphYMin"));
    yMaxSpin->setObjectName(QStringLiteral("outlineGraphYMax"));
    for (QDoubleSpinBox* spin : {yMinSpin, yMaxSpin}) {
        spin->setRange(-1000000.0, 1000000.0);
        spin->setDecimals(2);
        spin->setSingleStep(1.0);
        spin->setMaximumWidth(105);
        spin->setEnabled(false);
    }
    yMinSpin->setValue(0.0);
    yMaxSpin->setValue(100.0);
    auto* gridCheck = new QCheckBox(tr("Siatka"), graphGroup);
    gridCheck->setChecked(true);
    auto* valuesCheck = new QCheckBox(tr("Wartości"), graphGroup);
    valuesCheck->setChecked(false);
    auto* exportGraphButton = new QPushButton(tr("Eksport CSV"), graphGroup);
    auto* clearGraphButton = new QPushButton(tr("Wyczyść"), graphGroup);
    graphToolbar->addWidget(new QLabel(tr("Okno:"), graphGroup));
    graphToolbar->addWidget(timeRangeCombo);
    graphToolbar->addWidget(autoScaleCheck);
    graphToolbar->addWidget(new QLabel(tr("Y min:"), graphGroup));
    graphToolbar->addWidget(yMinSpin);
    graphToolbar->addWidget(new QLabel(tr("Y max:"), graphGroup));
    graphToolbar->addWidget(yMaxSpin);
    graphToolbar->addWidget(gridCheck);
    graphToolbar->addWidget(valuesCheck);
    graphToolbar->addWidget(exportGraphButton);
    graphToolbar->addWidget(clearGraphButton);
    graphLayout->addLayout(graphToolbar);

    auto* graphStatusLabel = new QLabel(tr("Brak próbek — oczekiwanie na dane z czujnika."), graphGroup);
    graphStatusLabel->setObjectName(QStringLiteral("outlineGraphStatus"));
    graphLayout->addWidget(graphStatusLabel);
    graphLayout->addWidget(m_forceGraph);

    // Analiza przebiegu: parametry próby, zapis serii i nakładanie przebiegów
    // z kolejnych powtórzeń lub sesji. Surowe próbki pozostają niezmienione.
    auto* analysisTabs = new QTabWidget(graphGroup);
    analysisTabs->setObjectName(QStringLiteral("forceAnalysisTabs"));
    auto* metricsPage = new QWidget(analysisTabs);
    auto* metricsLayout = new QVBoxLayout(metricsPage);
    auto* metricsBrowser = new QTextBrowser(metricsPage);
    metricsBrowser->setObjectName(QStringLiteral("forceAnalysisMetrics"));
    metricsBrowser->setMinimumHeight(145);
    auto* comparisonBrowser = new QTextBrowser(metricsPage);
    comparisonBrowser->setObjectName(QStringLiteral("forceSessionComparison"));
    comparisonBrowser->setMaximumHeight(125);
    auto* saveSeriesButton = new QPushButton(tr("Zapisz bieżący przebieg"), metricsPage);
    saveSeriesButton->setObjectName(QStringLiteral("saveForceSeries"));
    auto* savedSeriesCombo = new QComboBox(metricsPage);
    savedSeriesCombo->setObjectName(QStringLiteral("savedForceSeries"));
    auto* compareSeriesButton = new QPushButton(tr("Porównaj z wybraną serią"), metricsPage);
    compareSeriesButton->setObjectName(QStringLiteral("compareForceSeries"));
    auto* overlay = new ForceOverlayWidget(metricsPage);
    overlay->setObjectName(QStringLiteral("forceSeriesOverlay"));
    metricsLayout->addWidget(metricsBrowser);
    metricsLayout->addWidget(comparisonBrowser);
    auto* seriesControls = new QHBoxLayout();
    seriesControls->addWidget(saveSeriesButton);
    seriesControls->addWidget(savedSeriesCombo, 1);
    seriesControls->addWidget(compareSeriesButton);
    metricsLayout->addLayout(seriesControls);
    metricsLayout->addWidget(overlay, 1);
    analysisTabs->addTab(metricsPage, tr("Analiza i porównanie"));

    auto refreshForceMetrics = [this, metricsBrowser]() {
        if (m_forceGraph) metricsBrowser->setHtml(forceMetricsHtml(m_forceGraph->getData()));
    };
    refreshForceMetrics();
    auto* metricsTimer = new QTimer(metricsPage);
    metricsTimer->setInterval(1000);
    connect(metricsTimer, &QTimer::timeout, this, refreshForceMetrics);
    metricsTimer->start();

    const QString seriesStorePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                                  + QStringLiteral("/rehabilitation_force_series.json");
    auto loadSavedSeries = [seriesStorePath, savedSeriesCombo]() {
        savedSeriesCombo->clear();
        QFile file(seriesStorePath);
        if (!file.open(QIODevice::ReadOnly)) return;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        const QJsonArray all = doc.array();
        for (const QJsonValue& value : all) {
            const QJsonObject item = value.toObject();
            const QString label = item.value(QStringLiteral("label")).toString();
            if (!label.isEmpty()) savedSeriesCombo->addItem(label, QString::fromUtf8(QJsonDocument(item).toJson(QJsonDocument::Compact)));
        }
    };
    loadSavedSeries();
    connect(saveSeriesButton, &QPushButton::clicked, this,
            [this, seriesStorePath, savedSeriesCombo, loadSavedSeries, refreshForceMetrics]() {
        if (!m_forceGraph || m_forceGraph->dataPointCount() < 3) {
            QMessageBox::information(this, tr("Zapis przebiegu"),
                                     tr("Wymagane są co najmniej trzy próbki."));
            return;
        }
        QDir().mkpath(QFileInfo(seriesStorePath).absolutePath());
        QJsonArray all;
        QFile existing(seriesStorePath);
        if (existing.open(QIODevice::ReadOnly)) {
            all = QJsonDocument::fromJson(existing.readAll()).array();
            existing.close();
        }
        QJsonObject item;
        const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        const QString label = tr("%1 — %2 próbek")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")))
            .arg(m_forceGraph->dataPointCount());
        item.insert(QStringLiteral("label"), label);
        item.insert(QStringLiteral("patient_uuid"), m_currentPatientUuid);
        item.insert(QStringLiteral("created_at"), timestamp);
        QJsonArray points;
        const auto samples = m_forceGraph->getData();
        const qint64 origin = samples.isEmpty() ? 0 : samples.first().timestamp.toMSecsSinceEpoch();
        for (const auto& point : samples) {
            QJsonArray pair;
            pair.append(point.timestamp.toMSecsSinceEpoch() - origin);
            pair.append(point.value);
            points.append(pair);
        }
        item.insert(QStringLiteral("points"), points);
        all.append(item);
        while (all.size() > 100) all.removeAt(0);
        QFile file(seriesStorePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QMessageBox::warning(this, tr("Zapis przebiegu"),
                                 tr("Nie można zapisać serii: %1").arg(file.errorString()));
            return;
        }
        file.write(QJsonDocument(all).toJson(QJsonDocument::Indented));
        file.close();
        loadSavedSeries();
        savedSeriesCombo->setCurrentIndex(savedSeriesCombo->count() - 1);
        refreshForceMetrics();
        QMessageBox::information(this, tr("Zapis przebiegu"),
                                 tr("Zapisano serię pomiarową w lokalnym archiwum aplikacji."));
    });
    connect(compareSeriesButton, &QPushButton::clicked, this,
            [this, savedSeriesCombo, overlay, metricsBrowser, comparisonBrowser]() {
        comparisonBrowser->clear();
        QVector<ForceSeries> selected;
        if (m_forceGraph && m_forceGraph->dataPointCount() >= 2)
            selected.push_back({tr("Bieżący przebieg"), m_forceGraph->getData()});
        if (m_forceGraph && m_forceGraph->dataPointCount() >= 5) {
            const auto repetitions = extractForceRepetitions(m_forceGraph->getData());
            for (int i = 0; i < repetitions.size(); ++i)
                selected.push_back({tr("Powtórzenie %1").arg(i + 1), repetitions[i]});
        }
        const QJsonObject item = QJsonDocument::fromJson(savedSeriesCombo->currentData().toString().toUtf8()).object();
        QVector<gui::DataPoint> savedSamples;
        if (!item.isEmpty()) {
            const QString storedPatient = item.value(QStringLiteral("patient_uuid")).toString();
            if (storedPatient != m_currentPatientUuid) {
                QMessageBox::warning(this, tr("Porównanie serii"),
                    tr("Wybrana seria należy do innego kontekstu pacjenta. Wybierz serię zapisaną dla bieżącego pacjenta."));
                return;
            }
            const QJsonArray points = item.value(QStringLiteral("points")).toArray();
            const QDateTime base = QDateTime::fromString(
                item.value(QStringLiteral("created_at")).toString(), Qt::ISODate);
            for (const QJsonValue& value : points) {
                const QJsonArray pair = value.toArray();
                if (pair.size() != 2) continue;
                savedSamples.push_back(gui::DataPoint(base.addMSecs(pair.at(0).toVariant().toLongLong()),
                                                        pair.at(1).toDouble()));
            }
            if (savedSamples.size() >= 2)
                selected.push_back({item.value(QStringLiteral("label")).toString(), savedSamples});
        }
        overlay->setSeries(selected);
        if (!savedSamples.isEmpty() && m_forceGraph && m_forceGraph->dataPointCount() >= 3) {
            const ForceMetrics current = calculateForceMetrics(m_forceGraph->getData());
            const ForceMetrics previous = calculateForceMetrics(savedSamples);
            auto delta = [](double now, double before) {
                return before == 0.0 ? QStringLiteral("n/d")
                    : QString("%1%").arg((now - before) * 100.0 / std::abs(before), 0, 'f', 1);
            };
            comparisonBrowser->setHtml(tr(
                "<h4>Zmiana względem zapisanej sesji</h4><table>"
                "<tr><th>Parametr</th><th>Poprzednio</th><th>Obecnie</th><th>Zmiana względna</th></tr>"
                "<tr><td>Średnia siła</td><td>%1 N</td><td>%2 N</td><td>%3</td></tr>"
                "<tr><td>Szczyt siły</td><td>%4 N</td><td>%5 N</td><td>%6</td></tr>"
                "<tr><td>Zmienność (CV)</td><td>%7%</td><td>%8%</td><td>%9</td></tr>"
                "<tr><td>Czas narastania</td><td>%10 ms</td><td>%11 ms</td><td>%12</td></tr></table>"
                "<p><small>Zmiany procentowe są opisowe; porównanie jest miarodajne tylko przy zgodnym "
                "czujniku, kalibracji, pozycji, instrukcji, filtracji i sposobie normalizacji próby.</small></p>")
                .arg(previous.mean, 0, 'f', 2).arg(current.mean, 0, 'f', 2)
                .arg(delta(current.mean, previous.mean))
                .arg(previous.peak, 0, 'f', 2).arg(current.peak, 0, 'f', 2)
                .arg(delta(current.peak, previous.peak))
                .arg(previous.coefficientOfVariation, 0, 'f', 2)
                .arg(current.coefficientOfVariation, 0, 'f', 2)
                .arg(delta(current.coefficientOfVariation, previous.coefficientOfVariation))
                .arg(previous.meanRiseMs, 0, 'f', 0).arg(current.meanRiseMs, 0, 'f', 0)
                .arg(delta(current.meanRiseMs, previous.meanRiseMs)));
        }
        if (selected.isEmpty()) {
            metricsBrowser->append(tr("<p>Brak poprawnych przebiegów do porównania.</p>"));
        } else {
            metricsBrowser->append(tr("<p>Nałożono %1 przebiegów/powtórzeń. Każda seria jest wyrównana do początku próby; zachowano wspólną skalę siły.</p>")
                                   .arg(selected.size()));
        }
    });
    analysisTabs->addTab(new QLabel(tr("Archiwum przechowuje do 100 przebiegów. Porównuj próby wykonane tym samym protokołem, przy tej samej kalibracji i ustawieniu czujnika."), analysisTabs),
                         tr("Informacje"));
    graphLayout->addWidget(analysisTabs);
    connect(timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, timeRangeCombo](int index) {
                if (m_forceGraph && index >= 0)
                    m_forceGraph->setTimeRange(timeRangeCombo->itemData(index).toInt());
            });
    connect(autoScaleCheck, &QCheckBox::toggled, this,
            [this, yMinSpin, yMaxSpin](bool enabled) {
                yMinSpin->setEnabled(!enabled);
                yMaxSpin->setEnabled(!enabled);
                if (m_forceGraph) {
                    if (enabled) m_forceGraph->setYRange(0.0, 1.0, true);
                    else m_forceGraph->setYRange(yMinSpin->value(), yMaxSpin->value(), false);
                }
            });
    auto applyManualYRange = [this, autoScaleCheck, yMinSpin, yMaxSpin]() {
        if (autoScaleCheck->isChecked()) return;
        if (yMinSpin->value() >= yMaxSpin->value()) {
            yMaxSpin->setValue(yMinSpin->value() + 0.01);
        }
        if (m_forceGraph)
            m_forceGraph->setYRange(yMinSpin->value(), yMaxSpin->value(), false);
    };
    connect(yMinSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [applyManualYRange](double) { applyManualYRange(); });
    connect(yMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [applyManualYRange](double) { applyManualYRange(); });
    connect(gridCheck, &QCheckBox::toggled, m_forceGraph, &gui::GraphWidget::setShowGrid);
    connect(valuesCheck, &QCheckBox::toggled, m_forceGraph, &gui::GraphWidget::setShowValues);
    connect(clearGraphButton, &QPushButton::clicked, m_forceGraph, &gui::GraphWidget::clear);
    connect(exportGraphButton, &QPushButton::clicked, this, [this]() {
        if (!m_forceGraph || m_forceGraph->dataPointCount() == 0) {
            QMessageBox::information(this, tr("Eksport wykresu"),
                                     tr("Brak próbek do wyeksportowania."));
            return;
        }
        const QString path = QFileDialog::getSaveFileName(
            this, tr("Eksportuj próbki biofeedback"), QStringLiteral("biofeedback.csv"),
            tr("Pliki CSV (*.csv)"));
        if (path.isEmpty()) return;
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Eksport wykresu"),
                                 tr("Nie można zapisać pliku: %1").arg(file.errorString()));
            return;
        }
        QTextStream stream(&file);
        stream << "timestamp_iso8601;force_n\n";
        for (const gui::DataPoint& point : m_forceGraph->getData())
            stream << point.timestamp.toString(Qt::ISODateWithMs) << ';'
                   << QString::number(point.value, 'g', 15) << '\n';
        file.close();
        QMessageBox::information(this, tr("Eksport wykresu"),
                                 tr("Zapisano %1 próbek do pliku:\n%2")
                                     .arg(m_forceGraph->dataPointCount()).arg(path));
    });
    connect(m_forceGraph, &gui::GraphWidget::hovered, this,
            [graphStatusLabel](const gui::DataPoint& point, const QPointF&) {
                graphStatusLabel->setText(
                    QObject::tr("%1 | %2 N")
                        .arg(point.timestamp.toString(Qt::ISODateWithMs))
                        .arg(point.value, 0, 'f', 3));
            });
    auto updateGraphStatus = [this, graphStatusLabel]() {
        if (m_forceGraph)
            graphStatusLabel->setText(
                QObject::tr("Próbki: %1 | %2")
                    .arg(m_forceGraph->dataPointCount())
                    .arg(m_forceGraph->getRenderStats()));
    };
    auto* graphStatusTimer = new QTimer(graphGroup);
    graphStatusTimer->setInterval(1000);
    connect(graphStatusTimer, &QTimer::timeout, this, updateGraphStatus);
    graphStatusTimer->start();
    m_leftLayout->insertWidget(1, graphGroup);

    m_clinicalProtocolBrowser = new QTextBrowser(visualPanel);
    m_clinicalProtocolBrowser->setOpenExternalLinks(true);
    m_clinicalProtocolBrowser->setMinimumHeight(150);
    m_clinicalProtocolBrowser->setPlaceholderText(tr("Wybierz konspekt, aby zobaczyć protokół kliniczny."));

    // Dokumentacja pacjenta jest oddzielona od widoku ćwiczenia. Każdy obszar
    // ma własną zakładkę, dzięki czemu pola nie konkurują z wykresem i grą.
    auto* clinicalTabs = new QTabWidget(visualPanel);
    clinicalTabs->setObjectName(QStringLiteral("outlineClinicalWorkspaceTabs"));
    const QList<QPair<QWidget*, QString>> clinicalPages = {
        {episodeGroup, tr("Epizod")},
        {assessmentGroup, tr("Ocena")},
        {goalsGroup, tr("Cele")},
        {sessionGroup, tr("Sesja")},
        {observationGroup, tr("Obserwacje")},
        {m_clinicalProtocolBrowser, tr("Protokół")}
    };
    for (const auto& page : clinicalPages) {
        m_leftLayout->removeWidget(page.first);
        clinicalTabs->addTab(page.first, page.second);
    }
    clinicalTabs->setMinimumHeight(260);
    m_leftLayout->insertWidget(2, clinicalTabs);
}

void OutlineTab::setupOutlineSelector()
{
    // Sekcja wyboru konspektu
    m_outlineGroup = new QGroupBox(tr("Wybór Konspektu"), this);
    m_outlineGroup->setFont(QFont("Arial", 8, QFont::Bold));
    m_outlineSelectorLayout = new QHBoxLayout();
    
    m_outlineLabel = new QLabel(tr("Konspekt:"), this);
    m_outlineLabel->setFont(QFont("Arial", 7, QFont::Bold));
    m_outlineCombo = new QComboBox(this);
    m_outlineCombo->setMinimumWidth(300);
    m_outlineCombo->setMinimumHeight(35);
    m_outlineCombo->setFont(QFont("Arial", 7));
    m_outlineCombo->addItem(tr("-- Wybierz konspekt --"), QString());
    
    m_loadOutlineButton = new QPushButton(tr("Wczytaj..."), this);
    m_loadOutlineButton->setToolTip(tr("Wczytaj plik JSON konspektu"));
    m_loadOutlineButton->setMinimumHeight(35);
    m_loadOutlineButton->setFont(QFont("Arial", 7));
    
    m_outlineSelectorLayout->addWidget(m_outlineLabel);
    m_outlineSelectorLayout->addWidget(m_outlineCombo, 1);
    m_outlineSelectorLayout->addWidget(m_loadOutlineButton);
    
    m_outlineGroup->setLayout(m_outlineSelectorLayout);
    m_rightLayout->addWidget(m_outlineGroup);
    
    // Panel informacji o konspekcie
    m_outlineInfoGroup = new QGroupBox(tr("Informacje o Konspekcie"), this);
    QVBoxLayout* infoLayout = new QVBoxLayout();
    
    m_outlineNameLabel = new QLabel(tr("Nazwa: -"), this);
    m_outlineNameLabel->setFont(QFont("Sans Serif", 8, QFont::Bold));
    m_outlineNameLabel->setWordWrap(true);
    
    m_outlineDescriptionLabel = new QLabel(tr("Opis: -"), this);
    m_outlineDescriptionLabel->setWordWrap(true);
    
    QHBoxLayout* metaLayout = new QHBoxLayout();
    m_outlineAuthorLabel = new QLabel(tr("Autor: -"), this);
    m_outlineDurationLabel = new QLabel(tr("Czas: -"), this);
    m_outlineExerciseCountLabel = new QLabel(tr("Ćwiczenia: 0"), this);
    
    metaLayout->addWidget(m_outlineAuthorLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_outlineDurationLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_outlineExerciseCountLabel);
    
    infoLayout->addWidget(m_outlineNameLabel);
    infoLayout->addWidget(m_outlineDescriptionLabel);
    infoLayout->addLayout(metaLayout);
    
    m_outlineInfoGroup->setLayout(infoLayout);
    m_rightLayout->addWidget(m_outlineInfoGroup);
}

void OutlineTab::setupExerciseView()
{
    m_exerciseGroup = new QGroupBox(tr("Ćwiczenie"), this);
    m_exerciseLayout = new QVBoxLayout();
    
    // Informacje o ćwiczeniu
    m_exerciseInfoWidget = new QWidget(this);
    m_exerciseInfoLayout = new QVBoxLayout(m_exerciseInfoWidget);
    m_exerciseInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    m_exerciseNameLabel = new QLabel(tr("Nazwa ćwiczenia"), this);
    m_exerciseNameLabel->setFont(QFont("Sans Serif", 8, QFont::Bold));
    m_exerciseNameLabel->setWordWrap(true);
    
    m_exerciseDescriptionLabel = new QLabel(tr("Opis ćwiczenia pojawi się tutaj."), this);
    m_exerciseDescriptionLabel->setWordWrap(true);
    m_exerciseDescriptionLabel->setMinimumHeight(60);
    
    m_exerciseInfoLayout->addWidget(m_exerciseNameLabel);
    m_exerciseInfoLayout->addWidget(m_exerciseDescriptionLabel);
    
    m_exerciseLayout->addWidget(m_exerciseInfoWidget);
    
    // Multimedia (obraz i wideo)
    m_mediaLayout = new QHBoxLayout();
    
    m_exerciseImageLabel = new QLabel(this);
    m_exerciseImageLabel->setMinimumSize(180, 140);
    m_exerciseImageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_exerciseImageLabel->setAlignment(Qt::AlignCenter);
    m_exerciseImageLabel->setText(tr("Brak obrazu"));
    m_exerciseImageLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    
    m_videoWidget = new QWidget(this);
    m_videoWidget->setMinimumSize(180, 140);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_videoWidget->setStyleSheet("QWidget { background-color: #000; }");
    
    QVBoxLayout* videoLayout = new QVBoxLayout(m_videoWidget);
    m_playVideoButton = new QPushButton(tr("▶ Odtwórz film"), this);
    m_playVideoButton->setMinimumHeight(40);
    videoLayout->addWidget(m_playVideoButton);
    
    m_mediaLayout->addWidget(m_exerciseImageLabel);
    m_mediaLayout->addWidget(m_videoWidget);
    
    m_exerciseLayout->addLayout(m_mediaLayout);
    
    // Wybór gry
    m_gameSelectionGroup = new QGroupBox(tr("Gra Biofeedback"), this);
    m_gameSelectionLayout = new QHBoxLayout();
    
    m_gameLabel = new QLabel(tr("Gra:"), this);
    m_gameCombo = new QComboBox(this);
    m_gameCombo->setMinimumWidth(200);
    populateGameCombo();
    
    m_configureGameButton = new QPushButton(tr("Konfiguruj..."), this);
    m_configureGameButton->setEnabled(false);
    
    m_gameSelectionLayout->addWidget(m_gameLabel);
    m_gameSelectionLayout->addWidget(m_gameCombo, 1);
    m_gameSelectionLayout->addWidget(m_configureGameButton);
    
    m_gameSelectionGroup->setLayout(m_gameSelectionLayout);
    // Gra jest ustawieniem protokołu i dlatego pozostaje w prawym panelu.
    m_rightLayout->addWidget(m_gameSelectionGroup);

    m_exerciseGroup->setLayout(m_exerciseLayout);
    m_leftLayout->addWidget(m_exerciseGroup);
}

void OutlineTab::setupControlPanel()
{
    m_controlGroup = new QGroupBox(tr("Sterowanie Sesją"), this);
    m_controlLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton(tr("▶ Start"), this);
    m_startButton->setToolTip(tr("Rozpocznij sesję z wybranym konspektem"));
    m_startButton->setEnabled(false);
    
    m_stopButton = new QPushButton(tr("■ Stop"), this);
    m_stopButton->setToolTip(tr("Zatrzymaj sesję"));
    m_stopButton->setEnabled(false);
    
    m_pauseResumeButton = new QPushButton(tr("❚❚ Pauza"), this);
    m_pauseResumeButton->setToolTip(tr("Pauzuj/Wznów sesję"));
    m_pauseResumeButton->setEnabled(false);
    
    m_prevButton = new QPushButton(tr("◀ Wstecz"), this);
    m_prevButton->setToolTip(tr("Poprzednie ćwiczenie"));
    m_prevButton->setEnabled(false);
    
    m_nextButton = new QPushButton(tr("Dalej ▶"), this);
    m_nextButton->setToolTip(tr("Następne ćwiczenie"));
    m_nextButton->setEnabled(false);
    
    m_controlLayout->addWidget(m_startButton);
    m_controlLayout->addWidget(m_stopButton);
    m_controlLayout->addWidget(m_pauseResumeButton);
    m_controlLayout->addSpacing(20);
    m_controlLayout->addWidget(m_prevButton);
    m_controlLayout->addWidget(m_nextButton);
    m_controlLayout->addStretch();
    
    m_controlGroup->setLayout(m_controlLayout);
    m_rightLayout->addWidget(m_controlGroup);
    
    // Pasek postępu
    m_progressGroup = new QGroupBox(tr("Postęp"), this);
    m_progressLayout = new QHBoxLayout();
    
    m_progressLabel = new QLabel(tr("Postęp:"), this);
    m_sessionProgress = new QProgressBar(this);
    m_sessionProgress->setMinimum(0);
    m_sessionProgress->setMaximum(100);
    m_sessionProgress->setValue(0);
    m_sessionProgress->setFormat("%p%");
    
    m_currentExerciseLabel = new QLabel(tr("Ćwiczenie: - / -"), this);
    
    m_progressLayout->addWidget(m_progressLabel);
    m_progressLayout->addWidget(m_sessionProgress, 1);
    m_progressLayout->addWidget(m_currentExerciseLabel);
    
    m_progressGroup->setLayout(m_progressLayout);
    m_leftLayout->addWidget(m_progressGroup);
}

void OutlineTab::setupGameArea()
{
    m_gameAreaGroup = new QGroupBox(tr("Obszar Gry"), this);
    m_gameAreaLayout = new QVBoxLayout();
    
    m_gameStack = new QStackedWidget(this);
    
    // Placeholder gdy gra nie jest aktywna
    m_gamePlaceholder = new QWidget(this);
    QVBoxLayout* placeholderLayout = new QVBoxLayout(m_gamePlaceholder);
    QLabel* placeholderLabel = new QLabel(tr("Gra zostanie uruchomiona po rozpoczęciu sesji"), this);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setMinimumHeight(200);
    placeholderLayout->addWidget(placeholderLabel);
    
    m_gameStack->addWidget(m_gamePlaceholder);
    
    m_gameAreaLayout->addWidget(m_gameStack);
    m_gameAreaGroup->setLayout(m_gameAreaLayout);
    m_leftLayout->addWidget(m_gameAreaGroup, 1);
}

void OutlineTab::createConnections()
{
    connect(m_outlineCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutlineTab::onOutlineSelected);
    
    connect(m_loadOutlineButton, &QPushButton::clicked,
            this, [this]() {
                QString filename = QFileDialog::getOpenFileName(
                    this, tr("Wczytaj Konspekt"), m_outlinesDirectory,
                    tr("Pliki JSON (*.json)"));
                if (!filename.isEmpty()) {
                    loadOutlineFromFile(filename);
                }
            });
    
    connect(m_startButton, &QPushButton::clicked,
            this, &OutlineTab::onStartSessionClicked);
    
    connect(m_stopButton, &QPushButton::clicked,
            this, &OutlineTab::onStopSessionClicked);
    
    connect(m_pauseResumeButton, &QPushButton::clicked,
            this, &OutlineTab::onPauseResumeClicked);
    
    connect(m_prevButton, &QPushButton::clicked,
            this, &OutlineTab::onPreviousExerciseClicked);
    
    connect(m_nextButton, &QPushButton::clicked,
            this, &OutlineTab::onNextExerciseClicked);
    
    connect(m_gameCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutlineTab::onGameSelectionChanged);
    
    connect(m_playVideoButton, &QPushButton::clicked,
            this, &OutlineTab::onVideoPlayerButtonClicked);
    
    // Timer sesji
    m_sessionTimer = new QTimer(this);
    connect(m_sessionTimer, &QTimer::timeout,
            this, &OutlineTab::onSessionTimerTick);
}

void OutlineTab::loadOutlines(const QString& directory)
{
    m_outlinesDirectory = directory;
    m_outlines.clear();
    
    loadOutlineFiles(directory);
    populateOutlineCombo();
}

void OutlineTab::loadOutlineFiles(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        return;
    }
    
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    for (const QString& file : files) {
        QString fullPath = dir.absoluteFilePath(file);
        OutlineData outline;
        if (parseOutlineFile(fullPath, outline)) {
            m_outlines.append(outline);
        }
    }
}

bool OutlineTab::parseOutlineFile(const QString& filename, OutlineData& outline)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    
    // Parsowanie podstawowych danych konspektu
    outline.id = root["id"].toString();
    outline.name = root["name"].toString();
    outline.description = root["description"].toString();
    outline.author = root["author"].toString();
    outline.category = root["category"].toString();
    outline.estimatedDurationMinutes = root["estimatedDurationMinutes"].toInt(0);
    outline.clinicalProtocolHtml = root["clinicalProtocolHtml"].toString();
    
    // Parsowanie ćwiczeń
    QJsonArray exercisesArray = root["exercises"].toArray();
    
    for (const QJsonValue& exerciseValue : exercisesArray) {
        QJsonObject exerciseObj = exerciseValue.toObject();
        
        ExerciseData exercise;
        exercise.id = exerciseObj["id"].toString();
        exercise.name = exerciseObj["name"].toString();
        exercise.description = exerciseObj["description"].toString();
        exercise.imagePath = exerciseObj["imagePath"].toString();
        exercise.videoPath = exerciseObj["videoPath"].toString();
        exercise.gameId = exerciseObj["gameId"].toString();
        if (exercise.gameId == QStringLiteral("scored_earth"))
            exercise.gameId = QStringLiteral("scorched_earth");
        exercise.durationSeconds = exerciseObj["durationSeconds"].toInt(0);
        exercise.targetRepetitions = exerciseObj["targetRepetitions"].toInt(0);
        exercise.difficulty = exerciseObj["difficulty"].toString("medium");
        
        outline.exercises.append(exercise);
    }
    
    return !outline.id.isEmpty() && !outline.exercises.isEmpty();
}

bool OutlineTab::loadOutlineFromFile(const QString& filename)
{
    OutlineData outline;
    if (!parseOutlineFile(filename, outline)) {
        QMessageBox::warning(this, tr("Błąd ładowania"),
                            tr("Nie udało się wczytać pliku konspektu."));
        return false;
    }
    
    // Sprawdź czy konspekt już istnieje, jeśli tak to zaktualizuj
    int existingIndex = -1;
    for (int i = 0; i < m_outlines.size(); ++i) {
        if (m_outlines[i].id == outline.id) {
            existingIndex = i;
            break;
        }
    }
    
    if (existingIndex >= 0) {
        m_outlines[existingIndex] = outline;
    } else {
        m_outlines.append(outline);
    }
    
    populateOutlineCombo();
    
    // Wybierz wczytany konspekt
    for (int i = 0; i < m_outlineCombo->count(); ++i) {
        if (m_outlineCombo->itemData(i).toString() == outline.id) {
            m_outlineCombo->setCurrentIndex(i);
            break;
        }
    }
    
    return true;
}

void OutlineTab::populateOutlineCombo()
{
    m_outlineCombo->blockSignals(true);
    m_outlineCombo->clear();
    m_outlineCombo->addItem(tr("-- Wybierz konspekt --"), QString());
    
    for (int i = 0; i < m_outlines.size(); ++i) {
        m_outlineCombo->addItem(m_outlines[i].name, m_outlines[i].id);
    }
    
    m_outlineCombo->blockSignals(false);
}

void OutlineTab::populateGameCombo()
{
    m_gameCombo->blockSignals(true);
    m_gameCombo->clear();
    m_gameCombo->addItem(tr("-- Wybierz grę --"), QString());
    m_gameCombo->addItem(tr("Sin Game"), "sin_game");
    m_gameCombo->addItem(tr("Tan Game"), "tan_game");
    m_gameCombo->addItem(tr("Flappy Bird"), "flappy_bird");
    m_gameCombo->addItem(tr("Pong"), "pong");
    m_gameCombo->addItem(tr("Arkanoid"), "arkanoid");
    m_gameCombo->addItem(tr("Mario"), "mario");
    m_gameCombo->addItem(tr("Galaga"), "galaga");
    m_gameCombo->addItem(tr("Archer"), "archer");
    m_gameCombo->addItem(tr("Scorched Earth"), "scorched_earth");
    m_gameCombo->addItem(tr("Car Race"), "car_race");
    m_gameCombo->addItem(tr("Precision Crane"), "precision_crane");
    m_gameCombo->addItem(tr("Reaction Force — Refleks siły"), "reaction_force");

    // Nie pokazuj jako dostępnych gier, których implementacje nie są jeszcze
    // dołączone do aplikacji. Identyfikatory pozostają rozpoznawalne przy
    // odczycie starszych konspektów, ale użytkownik nie może ich wybrać.
    const int unavailableIndexes[] = {7, 8, 9, 10};
    if (auto* model = qobject_cast<QStandardItemModel*>(m_gameCombo->model())) {
        for (const int index : unavailableIndexes) {
            if (QStandardItem* item = model->item(index)) {
                item->setEnabled(false);
                item->setText(item->text() + tr(" (w przygotowaniu)"));
            }
        }
    }
    m_gameCombo->blockSignals(false);
}

void OutlineTab::onOutlineSelected(int index)
{
    if (index <= 0 || index > m_outlines.size()) {
        m_currentOutline = OutlineData();
        m_currentOutlineIndex = -1;
        
        // Wyczyść wyświetlanie
        m_outlineNameLabel->setText(tr("Nazwa: -"));
        m_outlineDescriptionLabel->setText(tr("Opis: -"));
        if (m_clinicalProtocolBrowser)
            m_clinicalProtocolBrowser->setHtml(tr("<p>Wybierz konspekt, aby wyświetlić protokół.</p>"));
        m_outlineAuthorLabel->setText(tr("Autor: -"));
        m_outlineDurationLabel->setText(tr("Czas: -"));
        m_outlineExerciseCountLabel->setText(tr("Ćwiczenia: 0"));
        
        m_startButton->setEnabled(false);
        return;
    }
    
    m_currentOutlineIndex = index - 1;
    m_currentOutline = m_outlines[m_currentOutlineIndex];
    
    // Wyświetl informacje o konspekcie
    m_outlineNameLabel->setText(tr("Nazwa: %1").arg(m_currentOutline.name));
    m_outlineDescriptionLabel->setText(m_currentOutline.description);
    if (m_clinicalProtocolBrowser) {
        if (!m_currentOutline.clinicalProtocolHtml.isEmpty())
            m_clinicalProtocolBrowser->setHtml(m_currentOutline.clinicalProtocolHtml);
        else
            m_clinicalProtocolBrowser->setHtml(tr("<p>Ten konspekt nie zawiera rozszerzonego protokołu klinicznego.</p>"));
    }
    m_outlineAuthorLabel->setText(tr("Autor: %1").arg(
        m_currentOutline.author.isEmpty() ? tr("Nieznany") : m_currentOutline.author));
    m_outlineDurationLabel->setText(tr("Czas: %1 min").arg(
        m_currentOutline.estimatedDurationMinutes));
    m_outlineExerciseCountLabel->setText(tr("Ćwiczenia: %1").arg(
        m_currentOutline.exercises.size()));
    
    m_startButton->setEnabled(!m_currentOutline.exercises.isEmpty());
    
    emit outlineSelected(m_currentOutline.id);
    
    // Reset do pierwszego ćwiczenia
    setCurrentExerciseIndex(0);
}

void OutlineTab::setCurrentExerciseIndex(int index)
{
    if (m_currentOutline.exercises.isEmpty()) {
        m_currentExerciseIndex = -1;
        return;
    }
    
    if (index < 0) {
        index = 0;
    }
    if (index >= m_currentOutline.exercises.size()) {
        index = m_currentOutline.exercises.size() - 1;
    }
    
    m_currentExerciseIndex = index;
    updateExerciseDisplay();
    updateProgressIndicators();
}

void OutlineTab::updateExerciseDisplay()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        m_exerciseNameLabel->setText(tr("Brak ćwiczenia"));
        m_exerciseDescriptionLabel->setText(tr("Wybierz konspekt aby zobaczyć ćwiczenia."));
        m_exerciseImageLabel->setText(tr("Brak obrazu"));
        m_exerciseImageLabel->setPixmap(QPixmap());
        return;
    }
    
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    
    m_exerciseNameLabel->setText(exercise.name);
    m_exerciseDescriptionLabel->setText(exercise.description);
    
    // Załaduj obraz jeśli istnieje
    if (!exercise.imagePath.isEmpty()) {
        QPixmap pixmap(exercise.imagePath);
        if (!pixmap.isNull()) {
            m_exerciseImageLabel->setPixmap(pixmap.scaled(
                m_exerciseImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_exerciseImageLabel->setText(tr("Nie można załadować obrazu"));
        }
    } else {
        m_exerciseImageLabel->setText(tr("Brak obrazu"));
        m_exerciseImageLabel->setPixmap(QPixmap());
    }
    
    // Ustaw grę w combo jeśli przypisana
    if (!exercise.gameId.isEmpty()) {
        int gameIndex = m_gameCombo->findData(exercise.gameId);
        if (gameIndex >= 0) {
            m_gameCombo->setCurrentIndex(gameIndex);
        }
    }
    
    emit exerciseChanged(m_currentExerciseIndex, exercise);
}

void OutlineTab::updateProgressIndicators()
{
    if (m_currentOutline.exercises.isEmpty()) {
        m_sessionProgress->setValue(0);
        m_currentExerciseLabel->setText(tr("Ćwiczenie: - / -"));
        return;
    }
    
    int total = m_currentOutline.exercises.size();
    int current = m_currentExerciseIndex + 1;
    
    int progressPercent = (m_completedExercises * 100) / total;
    m_sessionProgress->setValue(progressPercent);
    
    m_currentExerciseLabel->setText(tr("Ćwiczenie: %1 / %2").arg(current).arg(total));
    
    // Aktualizuj dostępność przycisków nawigacji
    m_prevButton->setEnabled(m_currentExerciseIndex > 0 && m_isSessionActive && !m_isPaused);
    m_nextButton->setEnabled(m_currentExerciseIndex < total - 1 && m_isSessionActive && !m_isPaused);
}

void OutlineTab::onStartSessionClicked()
{
    if (!validateCurrentSetup()) {
        return;
    }
    
    m_isSessionActive = true;
    m_isPaused = false;
    m_completedExercises = 0;
    m_exerciseScores.clear();
    m_exerciseScores.resize(m_currentOutline.exercises.size());
    std::fill(m_exerciseScores.begin(), m_exerciseScores.end(), 0);
    m_sessionStartTime = QDateTime::currentMSecsSinceEpoch();
    
    // Zablokuj wybór konspektu podczas sesji
    m_outlineCombo->setEnabled(false);
    m_loadOutlineButton->setEnabled(false);
    
    // Włącz kontrolki sesji
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_pauseResumeButton->setEnabled(true);
    m_pauseResumeButton->setText(tr("❚❚ Pauza"));
    
    // Rozpocznij pierwsze ćwiczenie
    setCurrentExerciseIndex(0);
    
    // Uruchom timer sesji
    m_sessionTimer->start(1000); // aktualizacja co sekundę
    
    emit sessionStarted(m_currentOutline.id);
    
    // Automatycznie uruchom grę dla pierwszego ćwiczenia
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    if (!exercise.gameId.isEmpty()) {
        emit requestGameStart(exercise.gameId, exercise);
    }
}

void OutlineTab::onStopSessionClicked()
{
    endSession();
}

void OutlineTab::onPauseResumeClicked()
{
    if (!m_isSessionActive) {
        return;
    }
    
    m_isPaused = !m_isPaused;
    
    if (m_isPaused) {
        m_pauseResumeButton->setText(tr("▶ Wznów"));
        m_sessionTimer->stop();
        
        // Pauzuj grę jeśli istnieje
        if (m_currentGame) {
            m_currentGame->pause();
        }
    } else {
        m_pauseResumeButton->setText(tr("❚❚ Pauza"));
        m_sessionTimer->start(1000);
        
        // Wznów grę jeśli istnieje
        if (m_currentGame) {
            m_currentGame->resume();
        }
    }
}

void OutlineTab::onPreviousExerciseClicked()
{
    if (m_currentExerciseIndex > 0) {
        setCurrentExerciseIndex(m_currentExerciseIndex - 1);
    }
}

void OutlineTab::onNextExerciseClicked()
{
    if (m_currentExerciseIndex < m_currentOutline.exercises.size() - 1) {
        finishCurrentExercise();
        moveToNextExercise();
    }
}

void OutlineTab::onGameSelectionChanged(int index)
{
    QString gameId = m_gameCombo->itemData(index).toString();
    m_configureGameButton->setEnabled(index > 0);
    
    // Jeśli trwa sesja, zapisz wybór gry dla bieżącego ćwiczenia
    if (m_isSessionActive && m_currentExerciseIndex >= 0 && 
        m_currentExerciseIndex < m_currentOutline.exercises.size()) {
        m_currentOutline.exercises[m_currentExerciseIndex].gameId = gameId;
    }
}

void OutlineTab::onVideoPlayerButtonClicked()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        return;
    }
    
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    
    if (exercise.videoPath.isEmpty()) {
        QMessageBox::information(this, tr("Brak wideo"),
                                tr("Nie ma filmu instruktażowego dla tego ćwiczenia."));
        return;
    }
    
    // Spróbuj otworzyć plik wideo
    QUrl url = QUrl::fromLocalFile(exercise.videoPath);
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::warning(this, tr("Błąd"),
                            tr("Nie można otworzyć pliku wideo."));
    }
}

void OutlineTab::onSessionTimerTick()
{
    if (!m_isSessionActive || m_isPaused) {
        return;
    }
    
    // Aktualizuj czas sesji (można dodać wyświetlanie czasu)
    // qint64 elapsed = (QDateTime::currentMSecsSinceEpoch() - m_sessionStartTime) / 1000;
    // Można dodać label do wyświetlania czasu
}

void OutlineTab::startSession()
{
    onStartSessionClicked();
}

void OutlineTab::stopSession()
{
    onStopSessionClicked();
}

void OutlineTab::pauseSession()
{
    if (m_isSessionActive && !m_isPaused) {
        onPauseResumeClicked();
    }
}

void OutlineTab::resumeSession()
{
    if (m_isSessionActive && m_isPaused) {
        onPauseResumeClicked();
    }
}

void OutlineTab::nextExercise()
{
    onNextExerciseClicked();
}

void OutlineTab::previousExercise()
{
    onPreviousExerciseClicked();
}

void OutlineTab::setGameForCurrentExercise(const QString& gameId)
{
    int index = m_gameCombo->findData(gameId);
    if (index >= 0) {
        m_gameCombo->setCurrentIndex(index);
    }
}

bool OutlineTab::validateCurrentSetup()
{
    if (m_currentOutline.exercises.isEmpty()) {
        QMessageBox::warning(this, tr("Błąd"),
                            tr("Wybrany konspekt nie zawiera ćwiczeń."));
        return false;
    }
    
    // Sprawdź czy wszystkie ćwiczenia mają przypisaną grę (opcjonalne)
    for (int i = 0; i < m_currentOutline.exercises.size(); ++i) {
        if (m_currentOutline.exercises[i].gameId.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Brak gry"),
                tr("Ćwiczenie '%1' nie ma przypisanej gry. Czy na pewno kontynuować?")
                  .arg(m_currentOutline.exercises[i].name),
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::No) {
                return false;
            }
        }
    }
    
    return true;
}

void OutlineTab::finishCurrentExercise()
{
    if (m_currentExerciseIndex < 0 || 
        m_currentExerciseIndex >= m_currentOutline.exercises.size()) {
        return;
    }
    
    m_completedExercises++;
    
    emit exerciseCompleted(m_currentExerciseIndex, m_exerciseScores[m_currentExerciseIndex]);
}

void OutlineTab::moveToNextExercise()
{
    int nextIndex = m_currentExerciseIndex + 1;
    
    if (nextIndex >= m_currentOutline.exercises.size()) {
        // Koniec sesji
        endSession();
        return;
    }
    
    setCurrentExerciseIndex(nextIndex);
    
    // Automatycznie uruchom grę dla następnego ćwiczenia
    const ExerciseData& exercise = m_currentOutline.exercises[m_currentExerciseIndex];
    if (!exercise.gameId.isEmpty()) {
        emit requestGameStart(exercise.gameId, exercise);
    }
}

void OutlineTab::endSession()
{
    m_isSessionActive = false;
    m_isPaused = false;
    
    m_sessionTimer->stop();
    
    // Przywróć dostępność wyboru konspektu
    m_outlineCombo->setEnabled(true);
    m_loadOutlineButton->setEnabled(true);
    
    // Zaktualizuj stan przycisków
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_pauseResumeButton->setEnabled(false);
    m_prevButton->setEnabled(false);
    m_nextButton->setEnabled(false);
    
    emit sessionEnded(m_completedExercises);
    
    // Wyczyść obszar gry
    m_gameStack->setCurrentWidget(m_gamePlaceholder);
    m_currentGame = nullptr;
}

void OutlineTab::onGameFinished(int score)
{
    // Zapisz wynik
    if (m_currentExerciseIndex >= 0 && 
        m_currentExerciseIndex < m_exerciseScores.size()) {
        m_exerciseScores[m_currentExerciseIndex] = score;
    }
    
    emit gameFinished(score);
    
    // Automatycznie przejdź do następnego ćwiczenia
    if (m_isSessionActive && !m_isPaused) {
        finishCurrentExercise();
        moveToNextExercise();
    }
}

bool OutlineTab::isSessionActive() const
{
    return m_isSessionActive;
}

int OutlineTab::outlineCount() const
{
    return m_outlines.size();
}

QVector<OutlineData> OutlineTab::getOutlines() const
{
    return m_outlines;
}

// ==================== Serial Communication Methods ====================

void OutlineTab::setSerialConnection(std::shared_ptr<sensor::SerialCommunication> serial)
{
    // MainWindow owns serial acquisition. OutlineTab consumes only the shared
    // calibrated MeasurementEngine stream and never subscribes to raw serial data.
    if (m_serialPort && m_dataCallbackToken) {
        m_serialPort->removeDataCallback(m_dataCallbackToken);
        m_dataCallbackToken = 0;
    }
    m_serialPort = std::move(serial);
}

void OutlineTab::setCurrentPatientContext(const QString& patientUuid, const QString& displayName)
{
    m_currentPatientUuid = patientUuid.trimmed();
    if (m_currentPatientUuid.isEmpty()) {
        if (m_patientContextLabel)
            m_patientContextLabel->setText(tr("Pacjent: nie wybrano"));
        m_currentEpisodeUuid.clear();
        refreshRehabilitationEpisodes();
        refreshRehabilitationAssessments();
        refreshRehabilitationGoals();
        refreshRehabilitationSession();
        return;
    }
    const QString safeName = displayName.trimmed().isEmpty()
        ? tr("Pacjent bez nazwy") : displayName.trimmed();
    if (m_patientContextLabel)
        m_patientContextLabel->setText(tr("Pacjent: %1").arg(safeName));
    refreshRehabilitationEpisodes();
}

void OutlineTab::setResourceRepository(
    std::shared_ptr<biofeedback::data::JsonResourceRepository> repository)
{
    m_resourceRepository = std::move(repository);
    refreshRehabilitationEpisodes();
}

void OutlineTab::refreshRehabilitationEpisodes()
{
    if (!m_episodeSelector) return;
    const QSignalBlocker blocker(m_episodeSelector);
    m_episodeSelector->clear();
    m_currentEpisodeUuid.clear();
    const bool canUseRepository = m_resourceRepository && !m_currentPatientUuid.isEmpty();
    m_episodeSelector->setEnabled(canUseRepository);
    m_episodeDiagnosisEdit->setEnabled(canUseRepository);
    m_episodePathwayCombo->setEnabled(canUseRepository);
    m_createEpisodeButton->setEnabled(canUseRepository);
    if (!canUseRepository) {
        if (m_episodeStatusLabel)
            m_episodeStatusLabel->setText(tr("Wybierz pacjenta, aby zarządzać epizodami."));
        // Clear every patient-scoped panel immediately when patient context is removed.
        refreshRehabilitationAssessments();
        refreshRehabilitationGoals();
        refreshRehabilitationSession();
        refreshRehabilitationObservations();
        return;
    }
    QString error;
    const QJsonArray episodes = m_resourceRepository->list(
        QStringLiteral("rehabilitation-episodes"), &error);
    if (!error.isEmpty()) {
        m_episodeStatusLabel->setText(tr("Nie udało się odczytać epizodów rehabilitacji."));
        m_episodeSelector->setEnabled(false);
        m_createEpisodeButton->setEnabled(false);
        refreshRehabilitationAssessments();
        refreshRehabilitationGoals();
        refreshRehabilitationSession();
        refreshRehabilitationObservations();
        return;
    }
    for (const QJsonValue& value : episodes) {
        if (!value.isObject()) continue;
        const QJsonObject episode = value.toObject();
        if (episode.value(QStringLiteral("patient_id")).toString().compare(
                m_currentPatientUuid, Qt::CaseInsensitive) != 0) continue;
        const QString diagnosis = episode.value(QStringLiteral("diagnosis_code")).toString();
        const QString pathway = episode.value(QStringLiteral("care_pathway")).toString();
        const QString label = diagnosis.isEmpty() ? pathway : diagnosis + QStringLiteral(" — ") + pathway;
        m_episodeSelector->addItem(label, episode.value(QStringLiteral("id")).toString());
    }
    if (m_episodeSelector->count() > 0) {
        m_episodeSelector->setCurrentIndex(0);
        m_currentEpisodeUuid = m_episodeSelector->currentData().toString();
        m_episodeStatusLabel->setText(tr("Wybrano istniejący epizod. Możesz utworzyć kolejny."));
    } else {
        m_episodeStatusLabel->setText(tr("Brak epizodów. Wprowadź rozpoznanie i utwórz epizod."));
    }
    refreshRehabilitationAssessments();
    refreshRehabilitationGoals();
    refreshRehabilitationSession();
    refreshRehabilitationObservations();
}

void OutlineTab::createRehabilitationEpisode()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty()) {
        QMessageBox::warning(this, tr("Epizod rehabilitacji"),
                             tr("Najpierw wybierz pacjenta."));
        return;
    }
    const QString diagnosis = m_episodeDiagnosisEdit->text().trimmed();
    if (diagnosis.isEmpty()) {
        QMessageBox::warning(this, tr("Epizod rehabilitacji"),
                             tr("Wprowadź rozpoznanie lub problem kliniczny."));
        m_episodeDiagnosisEdit->setFocus();
        return;
    }
    const QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("diagnosis_code"), diagnosis},
        {QStringLiteral("care_pathway"), m_episodePathwayCombo->currentData().toString()},
        {QStringLiteral("status"), QStringLiteral("active")}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-episodes"),
                                      payload, created, &error)) {
        QMessageBox::critical(this, tr("Epizod rehabilitacji"),
                              tr("Nie udało się zapisać epizodu. Kod błędu: %1").arg(error));
        return;
    }
    m_episodeDiagnosisEdit->clear();
    refreshRehabilitationEpisodes();
    const QString createdId = created.value(QStringLiteral("id")).toString();
    const int index = m_episodeSelector->findData(createdId);
    if (index >= 0) {
        m_episodeSelector->setCurrentIndex(index);
        m_currentEpisodeUuid = createdId;
    }
    m_episodeStatusLabel->setText(tr("Epizod zapisany w repozytorium."));
}

void OutlineTab::refreshRehabilitationAssessments()
{
    const bool canSave = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                         !m_currentEpisodeUuid.isEmpty();
    if (m_saveAssessmentButton) m_saveAssessmentButton->setEnabled(canSave);
    if (m_exportAssessmentReportButton) m_exportAssessmentReportButton->setEnabled(canSave);
    if (m_assessmentTypeCombo) m_assessmentTypeCombo->setEnabled(canSave);
    for (QLineEdit* field : {m_assessmentPainNrsEdit, m_assessmentRomEdit,
                             m_assessmentStrengthEdit, m_assessmentSwellingEdit,
                             m_assessmentFunctionalTestEdit}) {
        if (field) field->setEnabled(canSave);
    }
    if (!m_assessmentHistoryBrowser) return;
    m_assessmentHistoryBrowser->clear();
    if (!canSave) {
        if (m_assessmentStatusLabel)
            m_assessmentStatusLabel->setText(tr("Wybierz pacjenta i epizod, aby zapisać ocenę."));
        return;
    }

    QString error;
    const QJsonArray assessments = m_resourceRepository->list(
        QStringLiteral("rehabilitation-assessments"), &error);
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie udało się odczytać historii ocen."));
        m_saveAssessmentButton->setEnabled(false);
        if (m_exportAssessmentReportButton) m_exportAssessmentReportButton->setEnabled(false);
        return;
    }
    QStringList entries;
    int assessmentCount = 0;
    QJsonObject baselineAssessment;
    QJsonObject latestAssessment;
    QString latestAssessmentAt;
    for (const QJsonValue& value : assessments) {
        if (!value.isObject()) continue;
        const QJsonObject item = value.toObject();
        if (item.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            item.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        ++assessmentCount;
        const QString assessedAt = item.value(QStringLiteral("assessed_at")).toString();
        if (item.value(QStringLiteral("assessment_type")).toString() == QStringLiteral("baseline") &&
            (baselineAssessment.isEmpty() ||
             assessedAt < baselineAssessment.value(QStringLiteral("assessed_at")).toString()))
            baselineAssessment = item;
        if (latestAssessment.isEmpty() || assessedAt > latestAssessmentAt) {
            latestAssessment = item;
            latestAssessmentAt = assessedAt;
        }
        QStringList fields;
        const QString assessmentType = item.value(QStringLiteral("assessment_type")).toString();
        if (assessmentType == QStringLiteral("baseline")) fields << tr("Ocena początkowa");
        else if (assessmentType == QStringLiteral("follow_up")) fields << tr("Ocena kontrolna");
        else if (assessmentType == QStringLiteral("final")) fields << tr("Ocena końcowa");
        if (item.contains(QStringLiteral("pain_nrs")))
            fields << tr("NRS: %1").arg(item.value(QStringLiteral("pain_nrs")).toInt());
        if (item.contains(QStringLiteral("rom_degrees")))
            fields << tr("ROM: %1°").arg(item.value(QStringLiteral("rom_degrees")).toDouble());
        if (item.contains(QStringLiteral("strength_mrc")))
            fields << tr("MRC: %1/5").arg(item.value(QStringLiteral("strength_mrc")).toDouble());
        if (item.contains(QStringLiteral("swelling_cm")))
            fields << tr("Obwód: %1 cm").arg(item.value(QStringLiteral("swelling_cm")).toDouble());
        if (item.contains(QStringLiteral("functional_test")))
            fields << item.value(QStringLiteral("functional_test")).toString();
        if (item.contains(QStringLiteral("icf_assessment_ref")))
            fields << tr("Powiązano z konkretną wersją oceny ICF");
        entries << QStringLiteral("• %1 — %2")
                       .arg(item.value(QStringLiteral("assessed_at")).toString(),
                            fields.join(QStringLiteral("; ")));
    }
    if (!baselineAssessment.isEmpty() && !latestAssessment.isEmpty() &&
        baselineAssessment.value(QStringLiteral("id")).toString() !=
            latestAssessment.value(QStringLiteral("id")).toString()) {
        QStringList comparisons;
        struct Metric { const char* key; const char* label; const char* unit; };
        static const Metric metrics[] = {
            {"pain_nrs", "NRS", "/10"}, {"rom_degrees", "ROM", "°"},
            {"strength_mrc", "MRC", "/5"}, {"swelling_cm", "Obwód", "cm"}
        };
        for (const Metric& metric : metrics) {
            const QString key = QString::fromLatin1(metric.key);
            if (!baselineAssessment.contains(key) || !latestAssessment.contains(key)) continue;
            const double initial = baselineAssessment.value(key).toDouble();
            const double current = latestAssessment.value(key).toDouble();
            const double delta = current - initial;
            comparisons << tr("%1: %2 → %3 %4 (zmiana: %5%6)")
                .arg(QString::fromLatin1(metric.label),
                     QString::number(initial, 'g', 5),
                     QString::number(current, 'g', 5),
                     QString::fromLatin1(metric.unit),
                     delta > 0.0 ? QStringLiteral("+") : QString(),
                     QString::number(delta, 'g', 5));
        }
        if (!comparisons.isEmpty()) {
            entries << QString();
            entries << tr("Porównanie: ocena początkowa → najnowsza");
            entries.append(comparisons);
            entries << tr("Zmiany liczbowe wymagają interpretacji klinicznej; kierunek poprawy zależy od mierzonego parametru.");
        }
    }
    m_assessmentHistoryBrowser->setPlainText(entries.isEmpty()
        ? tr("Brak zapisanych ocen dla tego epizodu.") : entries.join(QStringLiteral("\n")));
    m_assessmentStatusLabel->setText(tr("Historia ocen: %1").arg(assessmentCount));
}

void OutlineTab::createRehabilitationAssessment()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() ||
        m_currentEpisodeUuid.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Wybierz pacjenta i epizod rehabilitacji."));
        return;
    }

    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("assessment_type"), m_assessmentTypeCombo
            ? m_assessmentTypeCombo->currentData().toString() : QStringLiteral("baseline")},
        {QStringLiteral("assessed_at"),
         QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    };

    // Link the rehabilitation assessment to the exact ICF assessment version.
    // New records use UUID; legacy assessments without UUID retain timestamp compatibility.
    QJsonObject patientRecord;
    QString patientReadError;
    bool linkedIcfAssessment = false;
    if (!m_resourceRepository->get(QStringLiteral("patients"), m_currentPatientUuid,
                                   patientRecord, &patientReadError)) {
        m_assessmentStatusLabel->setText(
            tr("Nie można odczytać kartoteki pacjenta. Ocena nie została zapisana: %1")
                .arg(patientReadError));
        return;
    }
    const QJsonObject currentIcf = patientRecord.value(QStringLiteral("icf_assessment")).toObject();
    QString icfAssessmentReference = currentIcf.value(QStringLiteral("id")).toString().trimmed();
    if (icfAssessmentReference.isEmpty())
        icfAssessmentReference = currentIcf.value(QStringLiteral("assessed_at")).toString().trimmed();
    if (!icfAssessmentReference.isEmpty()) {
        payload.insert(QStringLiteral("icf_assessment_ref"), icfAssessmentReference);
        linkedIcfAssessment = true;
    }

    auto parseOptionalNumber = [this, &payload](QLineEdit* field, const QString& key,
                                      double minimum, double maximum,
                                      bool integerOnly, QString& error) -> bool {
        const QString text = field->text().trimmed();
        if (text.isEmpty()) return true; // Missing measurement is not zero.
        bool ok = false;
        const double number = text.toDouble(&ok);
        if (!ok || !std::isfinite(number) || number < minimum || number > maximum ||
            (integerOnly && std::floor(number) != number)) {
            error = tr("Nieprawidłowa wartość pola: %1").arg(key);
            return false;
        }
        payload.insert(key, integerOnly ? QJsonValue(static_cast<int>(number))
                                           : QJsonValue(number));
        return true;
    };

    QString validationError;
    if (!parseOptionalNumber(m_assessmentPainNrsEdit, QStringLiteral("pain_nrs"), 0, 10, true, validationError) ||
        !parseOptionalNumber(m_assessmentRomEdit, QStringLiteral("rom_degrees"), 0, 360, false, validationError) ||
        !parseOptionalNumber(m_assessmentStrengthEdit, QStringLiteral("strength_mrc"), 0, 5, true, validationError) ||
        !parseOptionalNumber(m_assessmentSwellingEdit, QStringLiteral("swelling_cm"), 0, 100, false, validationError)) {
        m_assessmentStatusLabel->setText(validationError);
        return;
    }
    const QString functionalTest = m_assessmentFunctionalTestEdit->text().trimmed();
    if (!functionalTest.isEmpty())
        payload.insert(QStringLiteral("functional_test"), functionalTest.left(500));
    const bool hasClinicalValue =
        payload.contains(QStringLiteral("pain_nrs")) ||
        payload.contains(QStringLiteral("rom_degrees")) ||
        payload.contains(QStringLiteral("strength_mrc")) ||
        payload.contains(QStringLiteral("swelling_cm")) ||
        payload.contains(QStringLiteral("functional_test"));
    if (!hasClinicalValue) {
        m_assessmentStatusLabel->setText(tr("Wprowadź co najmniej jeden oceniony parametr kliniczny."));
        return;
    }

    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-assessments"),
                                      payload, created, &error)) {
        m_assessmentStatusLabel->setText(tr("Nie udało się zapisać oceny: %1").arg(error));
        return;
    }
    m_assessmentPainNrsEdit->clear();
    m_assessmentRomEdit->clear();
    m_assessmentStrengthEdit->clear();
    m_assessmentSwellingEdit->clear();
    m_assessmentFunctionalTestEdit->clear();
    m_assessmentStatusLabel->setText(linkedIcfAssessment
        ? tr("Ocena została zapisana i powiązana z aktualną wersją oceny ICF.")
        : tr("Ocena została zapisana. Brak aktualnej oceny ICF do powiązania."));
    refreshRehabilitationAssessments();
}


void OutlineTab::exportRehabilitationReportPdf()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() || m_currentEpisodeUuid.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Wybierz pacjenta i epizod przed eksportem raportu."));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this, tr("Zapisz raport rehabilitacyjny"), QString(),
        tr("Dokument PDF (*.pdf)"));
    if (fileName.isEmpty()) return;
    const QString outputPath = fileName.endsWith(QStringLiteral(".pdf"), Qt::CaseInsensitive)
        ? fileName : fileName + QStringLiteral(".pdf");

    QJsonObject patient;
    QJsonObject episode;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("patients"), m_currentPatientUuid, patient, &error)) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać kartoteki pacjenta: %1").arg(error));
        return;
    }
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-episodes"), m_currentEpisodeUuid,
                                   episode, &error)) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać epizodu rehabilitacji: %1").arg(error));
        return;
    }

    auto recordsForEpisode = [this, &error](const QString& collection) {
        QJsonArray filtered;
        const QJsonArray records = m_resourceRepository->list(collection, &error);
        if (!error.isEmpty()) return filtered;
        for (const QJsonValue& value : records) {
            if (!value.isObject()) continue;
            const QJsonObject record = value.toObject();
            if (record.value(QStringLiteral("patient_id")).toString() == m_currentPatientUuid &&
                record.value(QStringLiteral("episode_id")).toString() == m_currentEpisodeUuid)
                filtered.append(record);
        }
        return filtered;
    };

    const QJsonArray assessments = recordsForEpisode(QStringLiteral("rehabilitation-assessments"));
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać ocen: %1").arg(error));
        return;
    }
    const QJsonArray goals = recordsForEpisode(QStringLiteral("rehabilitation-goals"));
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać celów: %1").arg(error));
        return;
    }
    const QJsonArray sessions = recordsForEpisode(QStringLiteral("rehabilitation-sessions"));
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać sesji: %1").arg(error));
        return;
    }
    const QJsonArray observations = recordsForEpisode(QStringLiteral("rehabilitation-observations"));
    if (!error.isEmpty()) {
        m_assessmentStatusLabel->setText(tr("Nie można odczytać obserwacji: %1").arg(error));
        return;
    }

    const auto safe = [](const QString& value) { return value.toHtmlEscaped(); };
    const QString patientLabel = m_patientContextLabel
        ? m_patientContextLabel->text() : tr("Pacjent");
    const QString diagnosis = episode.value(QStringLiteral("diagnosis_code")).toString();
    const QString pathway = episode.value(QStringLiteral("care_pathway")).toString();
    const QJsonObject currentIcf = patient.value(QStringLiteral("icf_assessment")).toObject();
    const QJsonArray icfHistory = patient.value(QStringLiteral("icf_assessment_history")).toArray();
    auto resolveIcfReference = [&currentIcf, &icfHistory](const QString& reference) {
        if (reference.isEmpty()) return QJsonObject();
        const QString currentId = currentIcf.value(QStringLiteral("id")).toString();
        if (currentId == reference ||
            (currentId.isEmpty() && currentIcf.value(QStringLiteral("assessed_at")).toString() == reference))
            return currentIcf;
        for (const QJsonValue& value : icfHistory) {
            const QJsonObject historical = value.toObject();
            const QString historicalId = historical.value(QStringLiteral("id")).toString();
            if (historicalId == reference ||
                (historicalId.isEmpty() &&
                 historical.value(QStringLiteral("assessed_at")).toString() == reference))
                return historical;
        }
        return QJsonObject();
    };
    QJsonArray linkedIcfAssessments;
    for (const QJsonValue& value : assessments) {
        const QString reference = value.toObject().value(QStringLiteral("icf_assessment_ref")).toString();
        const QJsonObject linked = resolveIcfReference(reference);
        if (linked.isEmpty()) continue;
        bool alreadyIncluded = false;
        for (const QJsonValue& existing : linkedIcfAssessments) {
            const QJsonObject item = existing.toObject();
            if (item.value(QStringLiteral("id")).toString() == linked.value(QStringLiteral("id")).toString() &&
                item.value(QStringLiteral("assessed_at")).toString() == linked.value(QStringLiteral("assessed_at")).toString()) {
                alreadyIncluded = true;
                break;
            }
        }
        if (!alreadyIncluded) linkedIcfAssessments.append(linked);
    }

    QString html;
    html += QStringLiteral("<html><head><meta charset='utf-8'><style>"
        "body{font-family:'DejaVu Sans',sans-serif;font-size:9pt;color:#202124;}"
        "h1{font-size:18pt;}h2{font-size:12pt;margin-top:18px;border-bottom:1px solid #777;}"
        "table{width:100%;border-collapse:collapse;margin-top:6px;}"
        "th,td{border:1px solid #bbb;padding:5px;text-align:left;vertical-align:top;}"
        "th{background:#eee;} .muted{color:#555;font-size:8pt;}"
        "</style></head><body>");
    html += QStringLiteral("<h1>Raport z epizodu rehabilitacyjnego</h1>");
    html += QStringLiteral("<p><b>%1</b><br>Wygenerowano: %2<br>Rozpoznanie / problem: %3<br>Ścieżka postępowania: %4</p>")
        .arg(safe(patientLabel),
             safe(QDateTime::currentDateTimeUtc().toString(Qt::ISODate)),
             safe(diagnosis.isEmpty() ? tr("nie podano") : diagnosis),
             safe(pathway.isEmpty() ? tr("nie określono") : pathway));
    html += QStringLiteral("<p class='muted'>Identyfikator epizodu: %1. Raport jest zestawieniem zapisanych danych, nie automatycznym rozpoznaniem ani zaleceniem terapeutycznym.</p>")
        .arg(safe(m_currentEpisodeUuid));

    html += QStringLiteral("<h2>Oceny kliniczne</h2><table><tr><th>Data</th><th>Rodzaj</th><th>Wyniki</th><th>Powiązanie ICF</th></tr>");
    for (const QJsonValue& value : assessments) {
        const QJsonObject item = value.toObject();
        QStringList results;
        if (item.contains(QStringLiteral("pain_nrs")))
            results << tr("NRS: %1/10").arg(item.value(QStringLiteral("pain_nrs")).toInt());
        if (item.contains(QStringLiteral("rom_degrees")))
            results << tr("ROM: %1°").arg(item.value(QStringLiteral("rom_degrees")).toDouble());
        if (item.contains(QStringLiteral("strength_mrc")))
            results << tr("MRC: %1/5").arg(item.value(QStringLiteral("strength_mrc")).toInt());
        if (item.contains(QStringLiteral("swelling_cm")))
            results << tr("Obwód: %1 cm").arg(item.value(QStringLiteral("swelling_cm")).toDouble());
        if (item.contains(QStringLiteral("functional_test")))
            results << item.value(QStringLiteral("functional_test")).toString();
        QString type = item.value(QStringLiteral("assessment_type")).toString();
        if (type == QStringLiteral("baseline")) type = tr("Początkowa");
        else if (type == QStringLiteral("follow_up")) type = tr("Kontrolna");
        else if (type == QStringLiteral("final")) type = tr("Końcowa");
        const QJsonObject linkedIcf = resolveIcfReference(
            item.value(QStringLiteral("icf_assessment_ref")).toString());
        const QString linkedIcfDate = linkedIcf.value(QStringLiteral("assessed_at")).toString();
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
            .arg(safe(item.value(QStringLiteral("assessed_at")).toString()),
                 safe(type), safe(results.join(QStringLiteral("; "))),
                 safe(linkedIcfDate.isEmpty() ? tr("Brak") : linkedIcfDate));
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>Cele terapeutyczne</h2><table><tr><th>Cel</th><th>Wskaźnik</th><th>Wartość początkowa</th><th>Cel</th><th>Termin / status</th></tr>");
    for (const QJsonValue& value : goals) {
        const QJsonObject item = value.toObject();
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3 %4</td><td>%5 %4</td><td>%6 / %7</td></tr>")
            .arg(safe(item.value(QStringLiteral("title")).toString()),
                 safe(item.value(QStringLiteral("metric_id")).toString()),
                 safe(QString::number(item.value(QStringLiteral("baseline_value")).toDouble())),
                 safe(item.value(QStringLiteral("unit")).toString()),
                 safe(QString::number(item.value(QStringLiteral("target_value")).toDouble())),
                 safe(item.value(QStringLiteral("target_date")).toString()),
                 safe(item.value(QStringLiteral("status")).toString()));
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>Sesje rehabilitacyjne</h2><table><tr><th>Rozpoczęcie</th><th>Zakończenie</th><th>Status</th><th>Ból NRS przed / po / 24 h</th></tr>");
    for (const QJsonValue& value : sessions) {
        const QJsonObject item = value.toObject();
        const QString pain = QStringLiteral("%1 / %2 / %3")
            .arg(item.value(QStringLiteral("pre_session_pain_nrs")).toVariant().toString(),
                 item.value(QStringLiteral("post_session_pain_nrs")).toVariant().toString(),
                 item.value(QStringLiteral("pain_at_24h_nrs")).toVariant().toString());
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
            .arg(safe(item.value(QStringLiteral("started_at")).toString()),
                 safe(item.value(QStringLiteral("ended_at")).toString()),
                 safe(item.value(QStringLiteral("session_status")).toString()), safe(pain));
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>Obserwacje ćwiczeń</h2><table><tr><th>Data</th><th>Ćwiczenie</th><th>Dawka</th><th>Ból przed / po / 24 h</th><th>RPE / jakość</th></tr>");
    for (const QJsonValue& value : observations) {
        const QJsonObject item = value.toObject();
        const QString dose = tr("%1 serii × %2 powtórzeń; %3 s")
            .arg(item.value(QStringLiteral("completed_sets")).toInt())
            .arg(item.value(QStringLiteral("completed_repetitions")).toInt())
            .arg(item.value(QStringLiteral("duration_seconds")).toInt());
        const QString pain = QStringLiteral("%1 / %2 / %3")
            .arg(item.value(QStringLiteral("pain_before")).toVariant().toString(),
                 item.value(QStringLiteral("pain_after")).toVariant().toString(),
                 item.value(QStringLiteral("pain_at_24h")).toVariant().toString());
        const QString effort = QStringLiteral("%1 / %2")
            .arg(item.value(QStringLiteral("rpe")).toVariant().toString(),
                 item.value(QStringLiteral("movement_quality")).toString());
        html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td></tr>")
            .arg(safe(item.value(QStringLiteral("observed_at")).toString()),
                 safe(item.value(QStringLiteral("exercise_name")).toString()),
                 safe(dose), safe(pain), safe(effort));
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>Oceny ICF powiązane z ocenami klinicznymi epizodu</h2>");
    if (linkedIcfAssessments.isEmpty()) {
        html += QStringLiteral("<p>Brak powiązanych wersji oceny ICF.</p>");
    }
    for (const QJsonValue& value : linkedIcfAssessments) {
        const QJsonObject icf = value.toObject();
        const QJsonObject responses = icf.value(QStringLiteral("responses")).toObject();
        html += QStringLiteral("<h3>Ocena z dnia %1</h3><p>Liczba domen: %2<br>Opis: %3</p>")
            .arg(safe(icf.value(QStringLiteral("assessed_at")).toString()),
                 QString::number(responses.size()),
                 safe(icf.value(QStringLiteral("narrative")).toString()));
        html += QStringLiteral("<table><tr><th>Domena ICF</th><th>Kwalifikator</th><th>Wykonanie</th><th>Zdolność</th><th>Zakres</th><th>Charakter</th><th>Lokalizacja</th></tr>");
        for (auto it = responses.constBegin(); it != responses.constEnd(); ++it) {
            const QJsonObject response = it.value().toObject();
            const auto valueFor = [this, &response](const QString& key) {
                if (!response.contains(key)) return QString();
                const QJsonValue raw = response.value(key);
                if (raw.isDouble() && raw.toInt() == -99) return tr("Nie oceniono");
                return raw.toVariant().toString();
            };
            html += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
                .arg(safe(it.key()), safe(valueFor(QStringLiteral("qualifier"))),
                     safe(valueFor(QStringLiteral("performance"))), safe(valueFor(QStringLiteral("capacity"))),
                     safe(valueFor(QStringLiteral("extent"))), safe(valueFor(QStringLiteral("nature"))),
                     safe(valueFor(QStringLiteral("location"))));
        }
        html += QStringLiteral("</table>");
    }
    html += QStringLiteral("<p class='muted'>Dokument wygenerowano z danych dostępnych w repozytorium w chwili eksportu. Brak pomiaru pozostaje brakiem danych i nie jest interpretowany jako zero.</p></body></html>");

    QFile pdfFile(outputPath);
    if (!pdfFile.open(QIODevice::WriteOnly)) {
        m_assessmentStatusLabel->setText(
            tr("Nie można utworzyć pliku PDF: %1").arg(pdfFile.errorString()));
        return;
    }
    {
        QPdfWriter writer(&pdfFile);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
        writer.setTitle(tr("Raport rehabilitacyjny"));
        writer.setCreator(QStringLiteral("Biofeedback"));
        QTextDocument document;
        document.setHtml(html);
        document.print(&writer);
    }
    pdfFile.close();
    if (!QFileInfo(outputPath).exists() || QFileInfo(outputPath).size() <= 0) {
        m_assessmentStatusLabel->setText(tr("Eksport PDF nie powiódł się — plik jest pusty."));
        return;
    }
    m_assessmentStatusLabel->setText(tr("Raport PDF zapisano: %1").arg(outputPath));
}

void OutlineTab::refreshRehabilitationGoals()
{
    const bool canUse = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                        !m_currentEpisodeUuid.isEmpty();
    if (m_createGoalButton) m_createGoalButton->setEnabled(canUse);
    for (QLineEdit* field : {m_goalTitleEdit, m_goalMetricEdit, m_goalBaselineEdit,
                             m_goalTargetEdit, m_goalUnitEdit, m_goalTargetDateEdit})
        if (field) field->setEnabled(canUse);
    if (!m_goalHistoryBrowser) return;
    m_goalHistoryBrowser->clear();
    if (!canUse) {
        m_goalStatusLabel->setText(tr("Wybierz pacjenta i epizod, aby zarządzać celami."));
        return;
    }
    QString error;
    const QJsonArray goals = m_resourceRepository->list(QStringLiteral("rehabilitation-goals"), &error);
    if (!error.isEmpty()) {
        m_goalStatusLabel->setText(tr("Nie udało się odczytać celów terapeutycznych."));
        m_createGoalButton->setEnabled(false);
        return;
    }
    QStringList entries;
    for (const QJsonValue& value : goals) {
        if (!value.isObject()) continue;
        const QJsonObject goal = value.toObject();
        if (goal.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            goal.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        entries << QStringLiteral("%1 — %2: %3 %4 (cel: %5 %4; termin: %6; status: %7)")
            .arg(goal.value(QStringLiteral("title")).toString(),
                 goal.value(QStringLiteral("metric_id")).toString(),
                 QString::number(goal.value(QStringLiteral("baseline_value")).toDouble()),
                 goal.value(QStringLiteral("unit")).toString(),
                 QString::number(goal.value(QStringLiteral("target_value")).toDouble()),
                 goal.value(QStringLiteral("target_date")).toString().isEmpty()
                    ? tr("nie określono") : goal.value(QStringLiteral("target_date")).toString(),
                 goal.value(QStringLiteral("status")).toString());
    }
    m_goalHistoryBrowser->setPlainText(entries.isEmpty()
        ? tr("Brak zapisanych celów dla tego epizodu.") : entries.join(QStringLiteral("\n")));
    m_goalStatusLabel->setText(tr("Zapisane cele: %1").arg(entries.size()));
}

void OutlineTab::createRehabilitationGoal()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() || m_currentEpisodeUuid.isEmpty()) {
        m_goalStatusLabel->setText(tr("Wybierz pacjenta i epizod rehabilitacji."));
        return;
    }
    const QString title = m_goalTitleEdit->text().trimmed();
    const QString metric = m_goalMetricEdit->text().trimmed();
    const QString unit = m_goalUnitEdit->text().trimmed();
    bool baselineOk = false, targetOk = false;
    const double baseline = m_goalBaselineEdit->text().trimmed().toDouble(&baselineOk);
    const double target = m_goalTargetEdit->text().trimmed().toDouble(&targetOk);
    if (title.isEmpty() || title.size() > 200 || metric.isEmpty() || metric.size() > 120 ||
        unit.isEmpty() || unit.size() > 32 || !baselineOk || !targetOk ||
        !std::isfinite(baseline) || !std::isfinite(target)) {
        m_goalStatusLabel->setText(tr("Uzupełnij cel, wskaźnik, wartości liczbowe i jednostkę."));
        return;
    }
    const QString targetDate = m_goalTargetDateEdit->text().trimmed();
    if (!targetDate.isEmpty()) {
        const QDate parsed = QDate::fromString(targetDate, Qt::ISODate);
        if (!parsed.isValid() || parsed.toString(Qt::ISODate) != targetDate) {
            m_goalStatusLabel->setText(tr("Termin musi mieć format RRRR-MM-DD."));
            return;
        }
    }
    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("title"), title},
        {QStringLiteral("metric_id"), metric},
        {QStringLiteral("baseline_value"), baseline},
        {QStringLiteral("target_value"), target},
        {QStringLiteral("unit"), unit},
        {QStringLiteral("status"), QStringLiteral("active")},
        {QStringLiteral("target_date"), targetDate}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-goals"), payload, created, &error)) {
        m_goalStatusLabel->setText(tr("Nie udało się zapisać celu: %1").arg(error));
        return;
    }
    m_goalTitleEdit->clear();
    m_goalMetricEdit->clear();
    m_goalBaselineEdit->clear();
    m_goalTargetEdit->clear();
    m_goalUnitEdit->clear();
    m_goalTargetDateEdit->clear();
    m_goalStatusLabel->setText(tr("Cel terapeutyczny zapisany w repozytorium."));
    refreshRehabilitationGoals();
}

void OutlineTab::refreshRehabilitationSession()
{
    const bool hasContext = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                            !m_currentEpisodeUuid.isEmpty();
    m_currentRehabilitationSessionUuid.clear();
    if (m_sessionPainBeforeEdit) m_sessionPainBeforeEdit->clear();
    if (m_sessionPainAfterEdit) m_sessionPainAfterEdit->clear();
    if (m_sessionPain24hEdit) m_sessionPain24hEdit->clear();
    if (!hasContext) {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Wybierz pacjenta i epizod."));
        return;
    }
    QString error;
    const QJsonArray sessions = m_resourceRepository->list(
        QStringLiteral("rehabilitation-sessions"), &error);
    if (!error.isEmpty()) {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(
            tr("Nie udało się odczytać sesji rehabilitacyjnych. Operacje sesji zostały zablokowane."));
        return;
    }
    QJsonObject current;
    QString latestStartedAt;
    for (const QJsonValue& value : sessions) {
        if (!value.isObject()) continue;
        const QJsonObject candidate = value.toObject();
        if (candidate.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            candidate.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        const QString status = candidate.value(QStringLiteral("session_status")).toString();
        if (status != QStringLiteral("active") && status != QStringLiteral("awaiting_24h"))
            continue;
        const QString startedAt = candidate.value(QStringLiteral("started_at")).toString();
        if (current.isEmpty() || startedAt > latestStartedAt) {
            current = candidate;
            latestStartedAt = startedAt;
        }
    }
    if (current.isEmpty()) {
        m_startRehabilitationSessionButton->setEnabled(true);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(true);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Brak otwartej sesji. Wprowadź ból początkowy i rozpocznij sesję."));
        return;
    }
    m_currentRehabilitationSessionUuid = current.value(QStringLiteral("id")).toString();
    const QString status = current.value(QStringLiteral("session_status")).toString();
    if (status == QStringLiteral("active")) {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(true);
        m_saveRehabilitation24hButton->setEnabled(false);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(true);
        m_sessionPain24hEdit->setEnabled(false);
        m_rehabilitationSessionStatusLabel->setText(tr("Sesja trwa. Zapisano ból przed sesją: %1/10.")
            .arg(current.value(QStringLiteral("pre_session_pain_nrs")).toInt()));
    } else {
        m_startRehabilitationSessionButton->setEnabled(false);
        m_finishRehabilitationSessionButton->setEnabled(false);
        m_saveRehabilitation24hButton->setEnabled(true);
        m_sessionPainBeforeEdit->setEnabled(false);
        m_sessionPainAfterEdit->setEnabled(false);
        m_sessionPain24hEdit->setEnabled(true);
        m_rehabilitationSessionStatusLabel->setText(tr("Sesja zakończona. Oczekuje na kontrolę bólu po 24 godzinach."));
    }
}

void OutlineTab::startRehabilitationSession()
{
    bool ok = false;
    const int pain = m_sessionPainBeforeEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból przed sesją jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("session_type"), QStringLiteral("treatment")},
        {QStringLiteral("session_status"), QStringLiteral("active")},
        {QStringLiteral("started_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
        {QStringLiteral("pre_session_pain_nrs"), pain}
    };
    QJsonObject created;
    QString error;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-sessions"), payload, created, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się rozpocząć sesji: %1").arg(error));
        return;
    }
    m_sessionPainBeforeEdit->clear();
    refreshRehabilitationSession();
}

void OutlineTab::finishRehabilitationSession()
{
    bool ok = false;
    const int pain = m_sessionPainAfterEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból po sesji jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject current;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
                                   m_currentRehabilitationSessionUuid, current, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się odczytać sesji: %1").arg(error));
        return;
    }
    current.insert(QStringLiteral("post_session_pain_nrs"), pain);
    current.insert(QStringLiteral("ended_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    current.insert(QStringLiteral("session_status"), QStringLiteral("awaiting_24h"));
    QJsonObject updated;
    if (!m_resourceRepository->update(QStringLiteral("rehabilitation-sessions"),
            m_currentRehabilitationSessionUuid, current.value(QStringLiteral("version")).toInt(),
            current, updated, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się zakończyć sesji: %1").arg(error));
        return;
    }
    m_sessionPainAfterEdit->clear();
    refreshRehabilitationSession();
}

void OutlineTab::recordRehabilitation24HourFollowUp()
{
    bool ok = false;
    const int pain = m_sessionPain24hEdit->text().trimmed().toInt(&ok);
    if (!ok || pain < 0 || pain > 10) {
        m_rehabilitationSessionStatusLabel->setText(tr("Podaj ból po 24 godzinach jako liczbę całkowitą 0–10."));
        return;
    }
    QJsonObject current;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
                                   m_currentRehabilitationSessionUuid, current, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się odczytać sesji: %1").arg(error));
        return;
    }
    current.insert(QStringLiteral("pain_at_24h_nrs"), pain);
    current.insert(QStringLiteral("follow_up_24h_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    current.insert(QStringLiteral("session_status"), QStringLiteral("completed"));
    QJsonObject updated;
    if (!m_resourceRepository->update(QStringLiteral("rehabilitation-sessions"),
            m_currentRehabilitationSessionUuid, current.value(QStringLiteral("version")).toInt(),
            current, updated, &error)) {
        m_rehabilitationSessionStatusLabel->setText(tr("Nie udało się zapisać kontroli: %1").arg(error));
        return;
    }
    m_sessionPain24hEdit->clear();
    m_currentRehabilitationSessionUuid.clear();
    refreshRehabilitationSession();
}


void OutlineTab::refreshRehabilitationObservations()
{
    const bool hasContext = m_resourceRepository && !m_currentPatientUuid.isEmpty() &&
                            !m_currentEpisodeUuid.isEmpty();
    QJsonObject session;
    QString error;
    const bool hasActiveSession = hasContext && !m_currentRehabilitationSessionUuid.isEmpty() &&
        m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
                                  m_currentRehabilitationSessionUuid, session, &error) &&
        session.value(QStringLiteral("session_status")).toString() == QStringLiteral("active");
    if (m_saveObservationButton) m_saveObservationButton->setEnabled(hasActiveSession);
    for (QLineEdit* field : {m_observationExerciseEdit, m_observationSetsEdit,
                             m_observationRepetitionsEdit, m_observationDurationEdit,
                             m_observationRpeEdit, m_observationPainBeforeEdit,
                             m_observationPainAfterEdit, m_observationPain24hEdit})
        if (field) field->setEnabled(hasActiveSession);
    if (m_observationMovementQualityCombo)
        m_observationMovementQualityCombo->setEnabled(hasActiveSession);
    if (!m_observationHistoryBrowser) return;
    m_observationHistoryBrowser->clear();
    if (!hasContext) {
        m_observationStatusLabel->setText(tr("Wybierz pacjenta i epizod."));
        return;
    }
    const QJsonArray records = m_resourceRepository->list(
        QStringLiteral("rehabilitation-observations"), &error);
    if (!error.isEmpty()) {
        m_observationStatusLabel->setText(tr("Nie udało się odczytać obserwacji."));
        if (m_saveObservationButton) m_saveObservationButton->setEnabled(false);
        return;
    }
    QStringList entries;
    for (const QJsonValue& value : records) {
        if (!value.isObject()) continue;
        const QJsonObject record = value.toObject();
        if (record.value(QStringLiteral("patient_id")).toString() != m_currentPatientUuid ||
            record.value(QStringLiteral("episode_id")).toString() != m_currentEpisodeUuid)
            continue;
        entries << QStringLiteral("%1 | serie: %2 | powtórzenia: %3 | ból: %4 → %5 → %6 | RPE: %7 | jakość: %8")
            .arg(record.value(QStringLiteral("exercise_name")).toString(),
                 record.value(QStringLiteral("completed_sets")).toVariant().toString(),
                 record.value(QStringLiteral("completed_repetitions")).toVariant().toString(),
                 record.value(QStringLiteral("pain_before")).toVariant().toString(),
                 record.value(QStringLiteral("pain_after")).toVariant().toString(),
                 record.value(QStringLiteral("pain_at_24h")).isUndefined()
                    ? tr("brak") : record.value(QStringLiteral("pain_at_24h")).toVariant().toString(),
                 record.value(QStringLiteral("rpe")).isUndefined()
                    ? tr("brak") : record.value(QStringLiteral("rpe")).toVariant().toString(),
                 record.value(QStringLiteral("movement_quality")).toString());
    }
    m_observationHistoryBrowser->setPlainText(entries.isEmpty()
        ? tr("Brak zapisanych obserwacji dla tego epizodu.") : entries.join(QStringLiteral("\n")));
    m_observationStatusLabel->setText(hasActiveSession
        ? tr("Aktywna sesja: można zapisać obserwację ćwiczenia. Zapisane obserwacje: %1").arg(entries.size())
        : tr("Wymagana aktywna sesja. Zapisane obserwacje: %1").arg(entries.size()));
}

void OutlineTab::createRehabilitationObservation()
{
    if (!m_resourceRepository || m_currentPatientUuid.isEmpty() ||
        m_currentEpisodeUuid.isEmpty() || m_currentRehabilitationSessionUuid.isEmpty()) {
        m_observationStatusLabel->setText(tr("Wybierz pacjenta, epizod i rozpocznij sesję."));
        return;
    }
    QJsonObject session;
    QString error;
    if (!m_resourceRepository->get(QStringLiteral("rehabilitation-sessions"),
            m_currentRehabilitationSessionUuid, session, &error) ||
        session.value(QStringLiteral("session_status")).toString() != QStringLiteral("active")) {
        m_observationStatusLabel->setText(tr("Obserwacje można zapisywać wyłącznie podczas aktywnej sesji."));
        refreshRehabilitationObservations();
        return;
    }
    const QString exercise = m_observationExerciseEdit->text().trimmed();
    bool setsOk = false, repsOk = false, beforeOk = false, afterOk = false;
    const int sets = m_observationSetsEdit->text().trimmed().toInt(&setsOk);
    const int reps = m_observationRepetitionsEdit->text().trimmed().toInt(&repsOk);
    const int painBefore = m_observationPainBeforeEdit->text().trimmed().toInt(&beforeOk);
    const int painAfter = m_observationPainAfterEdit->text().trimmed().toInt(&afterOk);
    if (exercise.isEmpty() || exercise.size() > 200 || !setsOk || sets < 0 || sets > 100 ||
        !repsOk || reps < 0 || reps > 10000 || !beforeOk || painBefore < 0 || painBefore > 10 ||
        !afterOk || painAfter < 0 || painAfter > 10) {
        m_observationStatusLabel->setText(tr("Wymagane: ćwiczenie, serie 0–100, powtórzenia 0–10000 oraz ból przed i po 0–10."));
        return;
    }
    QJsonObject payload{
        {QStringLiteral("patient_id"), m_currentPatientUuid},
        {QStringLiteral("episode_id"), m_currentEpisodeUuid},
        {QStringLiteral("session_id"), m_currentRehabilitationSessionUuid},
        {QStringLiteral("exercise_name"), exercise},
        {QStringLiteral("completed_sets"), sets},
        {QStringLiteral("completed_repetitions"), reps},
        {QStringLiteral("pain_before"), painBefore},
        {QStringLiteral("pain_after"), painAfter},
        {QStringLiteral("movement_quality"), m_observationMovementQualityCombo->currentData().toString()},
        {QStringLiteral("observed_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)}
    };
    const QString durationText = m_observationDurationEdit->text().trimmed();
    if (!durationText.isEmpty()) {
        bool ok = false; const int duration = durationText.toInt(&ok);
        if (!ok || duration < 1 || duration > 86400) {
            m_observationStatusLabel->setText(tr("Czas ćwiczenia musi wynosić 1–86400 sekund."));
            return;
        }
        payload.insert(QStringLiteral("duration_seconds"), duration);
    }
    const QString rpeText = m_observationRpeEdit->text().trimmed();
    if (!rpeText.isEmpty()) {
        bool ok = false; const int rpe = rpeText.toInt(&ok);
        if (!ok || rpe < 0 || rpe > 10) {
            m_observationStatusLabel->setText(tr("RPE musi być liczbą całkowitą 0–10."));
            return;
        }
        payload.insert(QStringLiteral("rpe"), rpe);
    }
    const QString pain24Text = m_observationPain24hEdit->text().trimmed();
    if (!pain24Text.isEmpty()) {
        bool ok = false; const int pain24 = pain24Text.toInt(&ok);
        if (!ok || pain24 < 0 || pain24 > 10) {
            m_observationStatusLabel->setText(tr("Ból po 24 godzinach musi być liczbą całkowitą 0–10."));
            return;
        }
        payload.insert(QStringLiteral("pain_at_24h"), pain24);
    }
    QJsonObject created;
    if (!m_resourceRepository->create(QStringLiteral("rehabilitation-observations"),
                                      payload, created, &error)) {
        m_observationStatusLabel->setText(tr("Nie udało się zapisać obserwacji: %1").arg(error));
        return;
    }
    m_observationExerciseEdit->clear();
    m_observationSetsEdit->clear();
    m_observationRepetitionsEdit->clear();
    m_observationDurationEdit->clear();
    m_observationRpeEdit->clear();
    m_observationPainBeforeEdit->clear();
    m_observationPainAfterEdit->clear();
    m_observationPain24hEdit->clear();
    m_observationMovementQualityCombo->setCurrentIndex(0);
    refreshRehabilitationObservations();
}

void OutlineTab::setMeasurementEngine(
    std::shared_ptr<biofeedback::measurement::MeasurementEngine> engine)
{
    if (m_measurementEngine && m_measurementCallbackToken) {
        m_measurementEngine->unsubscribe(m_measurementCallbackToken);
        m_measurementCallbackToken = 0;
    }
    m_measurementEngine = std::move(engine);
    if (!m_measurementEngine) return;

    m_measurementCallbackToken = m_measurementEngine->subscribe(
        [this](const biofeedback::measurement::MeasurementSample& sample) {
            if (!sample.isValid() || sample.unit != "N" || !std::isfinite(sample.value))
                return;
            const auto timestamp = QDateTime::fromMSecsSinceEpoch(sample.timestampUnixMs);
            QMetaObject::invokeMethod(this, [this, value = std::abs(sample.value), timestamp]() {
                if (m_forceGraph)
                    m_forceGraph->addDataPoint(value, timestamp);
            }, Qt::QueuedConnection);
        });
}

bool OutlineTab::isSerialConnected() const
{
    return m_serialPort && m_serialPort->isConnected();
}


} // namespace tab
