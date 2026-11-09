#include "gui/GraphWidget.hpp"
#include <QPainterPath>
#include <QLineF>
#include <iterator>
#include <QGradient>
#include <QLinearGradient>
#include <QElapsedTimer>
#include <cmath>
#include <iostream>
#include <sstream>

namespace gui {

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
    , m_yMin(0.0)
    , m_yMax(100.0)
    , m_autoScaleY(true)
    , m_timeRangeSeconds(60)
    , m_title(tr("Wykres Danych"))
    , m_yLabel(tr("Wartość"))
    , m_unit(tr("N"))
    , m_lineColor(QColor(0, 120, 215))
    , m_fillColor(QColor(0, 120, 215, 40))
    , m_gridColor(QColor(200, 200, 200))
    , m_textColor(QColor(50, 50, 50))
    , m_bgColor(QColor(255, 255, 255))
    , m_showGrid(true)
    , m_showValues(false)
    , m_showCurrent(true)
    , m_isPanning(false)
    , m_isHovering(false)
    , m_cacheValid(false)
    , m_renderCount(0)
    , m_errorCount(0)
    , m_invalidPointCount(0)
    , m_lastRenderTimeMs(0)
    , m_totalDataPointsAdded(0)
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    
    // Automatyczne odświeżanie co 50ms
    QTimer* pruneTimer = new QTimer(this);
    connect(pruneTimer, &QTimer::timeout, this, &GraphWidget::pruneOldData);
    pruneTimer->start(50);
    
    // Debug logging for initialization
    core::DebugManager::instance().sendDebugMessage(
        "GraphWidget initialized", 
        core::DebugLevel::DEBUG, 
        "GraphWidget::GraphWidget"
    );
}

GraphWidget::~GraphWidget()
{
}

void GraphWidget::addDataPoint(double value, const QDateTime& timestamp)
{
    try {
        // Validate input value - check for NaN or infinity
        if (std::isnan(value) || std::isinf(value)) {
            m_invalidPointCount++;
            core::DebugManager::instance().sendDebugMessage(
                QString("Invalid data point value (NaN/Inf): %1").arg(value),
                core::DebugLevel::WARNING,
                "GraphWidget::addDataPoint"
            );
            return;
        }
        
        // Presentation-only buffer: never allow a delayed GUI to grow without bound.
        constexpr int MaximumGraphPoints = 32000;
        if (!timestamp.isValid()) {
            ++m_invalidPointCount;
            return;
        }
        if (!m_dataPoints.isEmpty() &&
            timestamp < m_dataPoints.constLast().timestamp) {
            // Keep hover lookup correct even if a device clock is adjusted
            // or delayed samples arrive out of order.
            m_timestampsMonotonic = false;
        }
        m_dataPoints.append(DataPoint(timestamp, value));
        if (m_dataPoints.size() > MaximumGraphPoints) {
            m_dataPoints.remove(0, m_dataPoints.size() - MaximumGraphPoints);
        }
        m_totalDataPointsAdded++;

        // At 80 SPS this bounds full-buffer autoscaling to about 10 calculations/s.
        if (m_autoScaleY && (++m_samplesSinceAutoScale >= 8)) {
            calculateAutoScale();
            m_samplesSinceAutoScale = 0;
        }
        
        m_cacheValid = false;
        update();
    } catch (const std::exception& e) {
        m_errorCount++;
        core::DebugManager::instance().sendDebugMessage(
            QString("Exception in addDataPoint: %1").arg(e.what()),
            core::DebugLevel::ERROR,
            "GraphWidget::addDataPoint"
        );
    } catch (...) {
        m_errorCount++;
        core::DebugManager::instance().sendDebugMessage(
            "Unknown exception in addDataPoint",
            core::DebugLevel::ERROR,
            "GraphWidget::addDataPoint"
        );
    }
}

void GraphWidget::clear()
{
    m_dataPoints.clear();
    m_samplesSinceAutoScale = 0;
    m_timestampsMonotonic = true;
    m_yMin = 0.0;
    m_yMax = 100.0;
    m_cacheValid = false;
    update();
}

void GraphWidget::setTimeRange(int seconds)
{
    if (seconds > 0 && seconds != m_timeRangeSeconds) {
        m_timeRangeSeconds = seconds;
        pruneOldData();
        m_cacheValid = false;
        update();
    }
}

void GraphWidget::setYRange(double min, double max, bool autoScale)
{
    m_autoScaleY = autoScale;
    
    if (!autoScale && min < max) {
        m_yMin = min;
        m_yMax = max;
        m_cacheValid = false;
        update();
    }
}

void GraphWidget::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void GraphWidget::setYLabel(const QString& label)
{
    m_yLabel = label;
    update();
}

void GraphWidget::setUnit(const QString& unit)
{
    m_unit = unit;
    update();
}

void GraphWidget::setLineColor(const QColor& color)
{
    m_lineColor = color;
    update();
}

void GraphWidget::setFillColor(const QColor& color)
{
    m_fillColor = color;
    update();
}

void GraphWidget::setShowGrid(bool enabled)
{
    m_showGrid = enabled;
    update();
}

void GraphWidget::setShowValues(bool enabled)
{
    m_showValues = enabled;
    update();
}

void GraphWidget::refresh()
{
    update();
}

void GraphWidget::pruneOldData()
{
    if (m_dataPoints.isEmpty()) return;

    const QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    int removeCount = 0;

    if (m_timestampsMonotonic) {
        // Fast path: monotonic acquisition timestamps form an expired prefix.
        while (removeCount < m_dataPoints.size() &&
               m_dataPoints[removeCount].timestamp < cutoff) {
            ++removeCount;
        }
        if (removeCount > 0) {
            m_dataPoints.remove(0, removeCount);
        }
    } else {
        // Clock adjustments or delayed samples can put expired points anywhere.
        const auto newEnd = std::remove_if(
            m_dataPoints.begin(), m_dataPoints.end(),
            [&cutoff](const DataPoint& point) {
                return point.timestamp < cutoff;
            });
        removeCount = static_cast<int>(std::distance(newEnd, m_dataPoints.end()));
        if (removeCount > 0) {
            m_dataPoints.erase(newEnd, m_dataPoints.end());
        }
    }

    if (removeCount > 0) {
        if (m_autoScaleY) {
            calculateAutoScale();
            m_samplesSinceAutoScale = 0;
        }
        m_cacheValid = false;
        update();
    }
}

void GraphWidget::calculateAutoScale()
{
    try {
        if (m_dataPoints.isEmpty()) {
            m_yMin = 0.0;
            m_yMax = 100.0;
            return;
        }
        
        double minVal = std::numeric_limits<double>::max();
        double maxVal = std::numeric_limits<double>::lowest();
        
        for (const DataPoint& point : m_dataPoints) {
            // Skip invalid values
            if (std::isnan(point.value) || std::isinf(point.value)) {
                continue;
            }
            if (point.value < minVal) minVal = point.value;
            if (point.value > maxVal) maxVal = point.value;
        }
        
        // Dodaj margines 10%
        double range = maxVal - minVal;
        if (range < 0.001) range = 100.0;  // Minimalny zakres
        
        m_yMin = minVal - range * 0.1;
        m_yMax = maxVal + range * 0.1;
        
        // Dla wartości raw (bardzo dużych) lub ujemnych - nie ograniczaj do zera
        // Pozwól na wyświetlanie wartości ujemnych dla tensometrów
    } catch (const std::exception& e) {

        // Set safe defaults on error
        m_yMin = 0.0;
        m_yMax = 100.0;
    } catch (...) {

        m_yMin = 0.0;
        m_yMax = 100.0;
    }
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    try {

        Q_UNUSED(event);
        // One time reference for the entire frame prevents horizontal jitter
        // and avoids repeated system-clock queries for every plotted sample.
        m_renderNow = QDateTime::currentDateTime();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Tło
        painter.fillRect(rect(), m_bgColor);
        
        if (m_dataPoints.isEmpty()) {
            // Wyświetl komunikat o braku danych

            painter.setPen(m_textColor);
            painter.setFont(QFont("Arial", 14));
            painter.drawText(rect(), Qt::AlignCenter, tr("Brak danych"));
            return;
        }

        try {
            drawGrid(painter);

        } catch (const std::exception& e) {

        }
        
        try {
            drawAxes(painter);

        } catch (const std::exception& e) {

        }
        
        try {
            drawDataLine(painter);

        } catch (const std::exception& e) {

        }
        
        try {
            drawDataPoints(painter);

        } catch (const std::exception& e) {

        }
        
        try {
            drawLabels(painter);

        } catch (const std::exception& e) {

        }
        
        if (m_showCurrent && !m_dataPoints.isEmpty()) {
            try {
                drawCurrentValue(painter);

            } catch (const std::exception& e) {

            }
        }

    } catch (const std::exception& e) {

    } catch (...) {

    }
}

void GraphWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_cacheValid = false;
}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void GraphWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        // Implementacja przesuwania wykresu
        m_lastMousePos = event->pos();
    }
    
    // Use one consistent time reference for hit-testing this mouse event.
    m_renderNow = QDateTime::currentDateTime();

    // Detekcja najechania na punkt danych
    QPointF dataPos = screenToData(event->pos());
    m_isHovering = false;
    
    // Fast binary-search path is used only while timestamps are monotonic.
    // If the device clock changes or delayed frames arrive out of order,
    // fall back to a correct bounded-buffer scan rather than mis-hitting.
    const QRectF area = plotArea();
    if (area.width() > 0.0 && !m_dataPoints.isEmpty()) {
        const qint64 centerMs = static_cast<qint64>(dataPos.x());
        const qint64 radiusMs = qMax<qint64>(
            1, static_cast<qint64>(
                (5.0 * m_timeRangeSeconds * 1000.0) / area.width()));

        auto testPoint = [&](const DataPoint& point) {
            const QPointF screenPos = dataToScreen(point);
            if (QLineF(screenPos, event->pos()).length() < 5.0) {
                m_isHovering = true;
                m_hoveredPoint = point;
                emit hovered(point, dataPos);
                return true;
            }
            return false;
        };

        if (m_timestampsMonotonic) {
            const auto first = std::lower_bound(
                m_dataPoints.cbegin(), m_dataPoints.cend(), centerMs - radiusMs,
                [](const DataPoint& point, qint64 timestampMs) {
                    return point.timestamp.toMSecsSinceEpoch() < timestampMs;
                });
            const auto last = std::upper_bound(
                first, m_dataPoints.cend(), centerMs + radiusMs,
                [](qint64 timestampMs, const DataPoint& point) {
                    return timestampMs < point.timestamp.toMSecsSinceEpoch();
                });
            for (auto it = first; it != last; ++it) {
                if (testPoint(*it)) break;
            }
        } else {
            for (const DataPoint& point : m_dataPoints) {
                if (testPoint(point)) break;
            }
        }
    }    
    if (!m_isHovering) {
        update();
    }
}

void GraphWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void GraphWidget::wheelEvent(QWheelEvent *event)
{
    // Zoomowanie za pomocą kółka myszy
    if (event->angleDelta().y() > 0) {
        // Przybliżenie
        m_timeRangeSeconds = qMax(5, m_timeRangeSeconds - 5);
    } else {
        // Oddalenie
        m_timeRangeSeconds = qMin(300, m_timeRangeSeconds + 5);
    }
    
    pruneOldData();
    emit yRangeChanged(m_yMin, m_yMax);
    update();
}

void GraphWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isHovering = false;
    update();
}

void GraphWidget::drawGrid(QPainter& painter)
{
    try {

        if (!m_showGrid) {

            return;
        }
        
        QRectF area = plotArea();
        
        painter.setPen(QPen(m_gridColor, 1, Qt::DashLine));
        
        // Linie poziome
        int gridLines = 5;
        for (int i = 0; i <= gridLines; ++i) {
            double y = area.top() + (area.height() * i / gridLines);
            painter.drawLine(area.left(), static_cast<int>(y), 
                            static_cast<int>(area.right()), static_cast<int>(y));
        }
        
        // Linie pionowe
        int timeSteps = 6;
        for (int i = 0; i <= timeSteps; ++i) {
            double x = area.left() + (area.width() * i / timeSteps);
            painter.drawLine(static_cast<int>(x), static_cast<int>(area.top()),
                            static_cast<int>(x), static_cast<int>(area.bottom()));
        }

    } catch (const std::exception& e) {

        throw;
    } catch (...) {

        throw;
    }
}

void GraphWidget::drawAxes(QPainter& painter)
{
    try {

        QRectF area = plotArea();
        
        painter.setPen(QPen(m_textColor, 2));
        
        // Oś X
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.bottom()),
                        static_cast<int>(area.right()), static_cast<int>(area.bottom()));

        // Oś Y
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.top()),
                        static_cast<int>(area.left()), static_cast<int>(area.bottom()));


    } catch (const std::exception& e) {

        throw;
    } catch (...) {

        throw;
    }
}

void GraphWidget::drawDataLine(QPainter& painter)
{
    if (m_dataPoints.size() < 2) {
        return;
    }

    const QRectF area = plotArea();
    const int bucketCount = qMax(1, static_cast<int>(std::ceil(area.width())));
    struct PixelBucket {
        bool valid{false};
        int minIndex{0};
        int maxIndex{0};
        QPointF minPoint;
        QPointF maxPoint;
    };

    // Min/max envelope: at most two vertices per horizontal pixel column.
    // This preserves narrow peaks while avoiding thousands of off-screen
    // painter path segments when the sample rate exceeds display resolution.
    QVector<PixelBucket> buckets(bucketCount);
    for (int i = 0; i < m_dataPoints.size(); ++i) {
        const QPointF point = dataToScreen(m_dataPoints[i]);
        if (!std::isfinite(point.x()) || !std::isfinite(point.y())) {
            continue;
        }

        const int column = qBound(
            0, static_cast<int>(point.x() - area.left()), bucketCount - 1);
        PixelBucket& bucket = buckets[column];
        if (!bucket.valid) {
            bucket.valid = true;
            bucket.minIndex = bucket.maxIndex = i;
            bucket.minPoint = bucket.maxPoint = point;
            continue;
        }
        if (point.y() < bucket.minPoint.y()) {
            bucket.minPoint = point;
            bucket.minIndex = i;
        }
        if (point.y() > bucket.maxPoint.y()) {
            bucket.maxPoint = point;
            bucket.maxIndex = i;
        }
    }

    QPainterPath path;
    bool started = false;
    for (const PixelBucket& bucket : buckets) {
        if (!bucket.valid) {
            continue;
        }
        const bool minFirst = bucket.minIndex <= bucket.maxIndex;
        const QPointF first = minFirst ? bucket.minPoint : bucket.maxPoint;
        const QPointF second = minFirst ? bucket.maxPoint : bucket.minPoint;
        if (!started) {
            path.moveTo(first);
            started = true;
        } else {
            path.lineTo(first);
        }
        if (bucket.minIndex != bucket.maxIndex) {
            path.lineTo(second);
        }
    }

    if (!started) {
        return;
    }

    painter.setPen(QPen(m_lineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);

    QPainterPath fillPath = path;
    fillPath.lineTo(area.right(), area.bottom());
    fillPath.lineTo(area.left(), area.bottom());
    fillPath.closeSubpath();

    painter.setBrush(m_fillColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(fillPath);
}

void GraphWidget::drawDataPoints(QPainter& painter)
{
    try {

        if (m_dataPoints.isEmpty()) {

            return;
        }
        
        painter.setBrush(m_lineColor);
        painter.setPen(Qt::NoPen);
        
        // Rysuj tylko ostatni punkt lub punkty w pobliżu kursora
        if (!m_dataPoints.isEmpty()) {
            QPointF lastPoint = dataToScreen(m_dataPoints.last());
            
            // Validate point coordinates
            if (std::isnan(lastPoint.x()) || std::isnan(lastPoint.y()) ||
                std::isinf(lastPoint.x()) || std::isinf(lastPoint.y())) {

            } else {

                painter.drawEllipse(lastPoint, 4, 4);
            }
        }
        
        // Podświetl punkt pod kursorem
        if (m_isHovering) {
            QPointF hoverPoint = dataToScreen(m_hoveredPoint);
            
            if (std::isnan(hoverPoint.x()) || std::isnan(hoverPoint.y()) ||
                std::isinf(hoverPoint.x()) || std::isinf(hoverPoint.y())) {

            } else {

                painter.setBrush(QColor(255, 100, 100));
                painter.drawEllipse(hoverPoint, 6, 6);
            }
        }

    } catch (const std::exception& e) {

        throw;
    } catch (...) {

        throw;
    }
}

void GraphWidget::drawLabels(QPainter& painter)
{
    try {

        QRectF area = plotArea();
        
        painter.setPen(m_textColor);
        painter.setFont(QFont("Arial", 9));
        
        // Tytuł
        if (!m_title.isEmpty()) {

            QFont titleFont = painter.font();
            titleFont.setBold(true);
            titleFont.setPointSize(12);
            painter.setFont(titleFont);
            painter.drawText(area.left(), area.top() - 10, m_title);
            painter.setFont(QFont("Arial", 9));
        }
        
        // Etykieta osi Y
        if (!m_yLabel.isEmpty()) {

            painter.save();
            painter.translate(10, area.center().y());
            painter.rotate(-90);
            painter.drawText(0, 0, m_yLabel);
            painter.restore();
        }
        
        // Wartości na osi Y

        int gridLines = 5;
        for (int i = 0; i <= gridLines; ++i) {
            double value = m_yMax - (m_yMax - m_yMin) * i / gridLines;
            
            // Validate value before displaying
            if (std::isnan(value) || std::isinf(value)) {

                continue;
            }
            
            QString label = QString::number(value, 'f', 1);
            int y = static_cast<int>(area.top() + (area.height() * i / gridLines));
            painter.drawText(5, y + 3, label);
        }
        
        // Wartości na osi X (czas)

        int timeSteps = 6;
        QDateTime now = QDateTime::currentDateTime();
        for (int i = 0; i <= timeSteps; ++i) {
            int seconds = -m_timeRangeSeconds * (timeSteps - i) / timeSteps;
            QTime time = now.addSecs(seconds).time();
            QString label = time.toString("hh:mm:ss");
            int x = static_cast<int>(area.left() + (area.width() * i / timeSteps));
            painter.drawText(x - 30, static_cast<int>(area.bottom()) + 20, 60, 20, Qt::AlignCenter, label);
        }

    } catch (const std::exception& e) {

        throw;
    } catch (...) {

        throw;
    }
}

void GraphWidget::drawCurrentValue(QPainter& painter)
{
    try {

        if (m_dataPoints.isEmpty()) {

            return;
        }
        
        const DataPoint& last = m_dataPoints.last();

        QPointF pos = dataToScreen(last);
        
        // Validate position
        if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
            std::isinf(pos.x()) || std::isinf(pos.y())) {

            return;
        }

        // Etykieta z wartością
        QString valueText = QString("%1 %2").arg(last.value, 0, 'f', 2).arg(m_unit);

        painter.setPen(Qt::white);
        painter.setBrush(m_lineColor);
        
        QRect textRect = painter.fontMetrics().boundingRect(valueText);
        textRect.adjust(-5, -3, 5, 3);
        textRect.moveTopLeft(QPoint(static_cast<int>(pos.x()) + 10, static_cast<int>(pos.y()) - 15));
        
        painter.drawRoundedRect(textRect, 3, 3);
        painter.drawText(textRect, Qt::AlignCenter, valueText);

    } catch (const std::exception& e) {

        throw;
    } catch (...) {

        throw;
    }
}

QPointF GraphWidget::dataToScreen(const DataPoint& point) const
{
    try {

        QRectF area = plotArea();
        
        // Validate plot area
        if (area.width() <= 0 || area.height() <= 0) {

            return QPointF(0, 0);
        }
        
        const QDateTime now = m_renderNow.isValid()
            ? m_renderNow : QDateTime::currentDateTime();
        const qint64 msNow = now.toMSecsSinceEpoch();
        qint64 msPoint = point.timestamp.toMSecsSinceEpoch();
        qint64 msRange = m_timeRangeSeconds * 1000;
        
        // Avoid division by zero
        if (msRange <= 0) {

            return QPointF(0, 0);
        }
        
        double xRatio = static_cast<double>(msPoint - (msNow - msRange)) / msRange;
        xRatio = qBound(0.0, xRatio, 1.0);
        
        // Avoid division by zero for Y-axis
        double yRange = m_yMax - m_yMin;
        if (std::abs(yRange) < 0.0001) {

            yRange = 100.0;
        }
        
        double yRatio = (point.value - m_yMin) / yRange;
        yRatio = qBound(0.0, yRatio, 1.0);
        
        double x = area.left() + xRatio * area.width();
        double y = area.bottom() - yRatio * area.height();
        
        // Validate result
        if (std::isnan(x) || std::isnan(y) || std::isinf(x) || std::isinf(y)) {

            return QPointF(0, 0);
        }

        return QPointF(x, y);
    } catch (const std::exception& e) {

        return QPointF(0, 0);
    } catch (...) {

        return QPointF(0, 0);
    }
}

QPointF GraphWidget::screenToData(const QPointF& screenPos) const
{
    QRectF area = plotArea();
    
    double xRatio = (screenPos.x() - area.left()) / area.width();
    xRatio = qBound(0.0, xRatio, 1.0);
    
    double yRatio = 1.0 - (screenPos.y() - area.top()) / area.height();
    yRatio = qBound(0.0, yRatio, 1.0);
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 msNow = now.toMSecsSinceEpoch();
    qint64 msRange = m_timeRangeSeconds * 1000;
    qint64 msPoint = msNow - msRange + static_cast<qint64>(xRatio * msRange);
    
    double value = m_yMin + yRatio * (m_yMax - m_yMin);
    
    QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(msPoint);
    
    return QPointF(timestamp.toMSecsSinceEpoch(), value);
}

QRectF GraphWidget::plotArea() const
{
    if (!m_cacheValid) {
        m_plotAreaCache = QRectF(
            leftMargin(),
            topMargin(),
            width() - leftMargin() - rightMargin(),
            height() - topMargin() - bottomMargin()
        );
        m_cacheValid = true;
    }
    
    return m_plotAreaCache;
}

QString GraphWidget::getRenderStats() const
{
    QMutexLocker locker(&m_statsMutex);
    
    std::ostringstream oss;
    oss << "=== GraphWidget Statistics ===" << std::endl;
    oss << "Total data points added: " << m_totalDataPointsAdded.load() << std::endl;
    oss << "Current data points: " << m_dataPoints.size() << std::endl;
    oss << "Render count: " << m_renderCount.load() << std::endl;
    oss << "Error count: " << m_errorCount.load() << std::endl;
    oss << "Invalid point count: " << m_invalidPointCount.load() << std::endl;
    oss << "Last render time: " << m_lastRenderTimeMs << " ms" << std::endl;
    
    return QString::fromStdString(oss.str());
}

void GraphWidget::resetRenderStats()
{
    QMutexLocker locker(&m_statsMutex);
    
    m_renderCount = 0;
    m_errorCount = 0;
    m_invalidPointCount = 0;
    m_lastRenderTimeMs = 0;
    m_totalDataPointsAdded = 0;
    
    core::DebugManager::instance().sendDebugMessage(
        "GraphWidget statistics reset",
        core::DebugLevel::INFO,
        "GraphWidget::resetRenderStats"
    );
}

} // namespace gui
