#include "gui/GraphWidget.hpp"
#include <QPainterPath>
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
        
        m_dataPoints.append(DataPoint(timestamp, value));
        m_totalDataPointsAdded++;
        
        if (m_autoScaleY) {
            calculateAutoScale();
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
    
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    
    int removeCount = 0;
    for (int i = 0; i < m_dataPoints.size(); ++i) {
        if (m_dataPoints[i].timestamp < cutoff) {
            removeCount++;
        } else {
            break;
        }
    }
    
    if (removeCount > 0) {
        m_dataPoints.remove(0, removeCount);
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
        std::cerr << "[GraphWidget] ERROR in calculateAutoScale: " << e.what() << std::endl;
        // Set safe defaults on error
        m_yMin = 0.0;
        m_yMax = 100.0;
    } catch (...) {
        std::cerr << "[GraphWidget] ERROR in calculateAutoScale: Unknown exception" << std::endl;
        m_yMin = 0.0;
        m_yMax = 100.0;
    }
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    try {
        std::cout << "[GraphWidget::paintEvent] START - Rendering graph with " 
                  << m_dataPoints.size() << " data points" << std::endl;
        
        Q_UNUSED(event);
        
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Tło
        painter.fillRect(rect(), m_bgColor);
        
        if (m_dataPoints.isEmpty()) {
            // Wyświetl komunikat o braku danych
            std::cout << "[GraphWidget::paintEvent] No data points to render, showing 'Brak danych' message" << std::endl;
            painter.setPen(m_textColor);
            painter.setFont(QFont("Arial", 14));
            painter.drawText(rect(), Qt::AlignCenter, tr("Brak danych"));
            return;
        }
        
        std::cout << "[GraphWidget::paintEvent] Y-axis range: [" << m_yMin << ", " << m_yMax 
                  << "], Time range: " << m_timeRangeSeconds << "s" << std::endl;
        
        try {
            drawGrid(painter);
            std::cout << "[GraphWidget::paintEvent] Grid drawn successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[GraphWidget::paintEvent] ERROR drawing grid: " << e.what() << std::endl;
        }
        
        try {
            drawAxes(painter);
            std::cout << "[GraphWidget::paintEvent] Axes drawn successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[GraphWidget::paintEvent] ERROR drawing axes: " << e.what() << std::endl;
        }
        
        try {
            drawDataLine(painter);
            std::cout << "[GraphWidget::paintEvent] Data line drawn successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[GraphWidget::paintEvent] ERROR drawing data line: " << e.what() << std::endl;
        }
        
        try {
            drawDataPoints(painter);
            std::cout << "[GraphWidget::paintEvent] Data points drawn successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[GraphWidget::paintEvent] ERROR drawing data points: " << e.what() << std::endl;
        }
        
        try {
            drawLabels(painter);
            std::cout << "[GraphWidget::paintEvent] Labels drawn successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[GraphWidget::paintEvent] ERROR drawing labels: " << e.what() << std::endl;
        }
        
        if (m_showCurrent && !m_dataPoints.isEmpty()) {
            try {
                drawCurrentValue(painter);
                std::cout << "[GraphWidget::paintEvent] Current value label drawn successfully" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[GraphWidget::paintEvent] ERROR drawing current value: " << e.what() << std::endl;
            }
        }
        
        std::cout << "[GraphWidget::paintEvent] END - Rendering complete" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::paintEvent] CRITICAL ERROR: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[GraphWidget::paintEvent] CRITICAL ERROR: Unknown exception during rendering" << std::endl;
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
    
    // Detekcja najechania na punkt danych
    QPointF dataPos = screenToData(event->pos());
    m_isHovering = false;
    
    for (const DataPoint& point : m_dataPoints) {
        QPointF screenPos = dataToScreen(point);
        double dist = QLineF(screenPos, event->pos()).length();
        
        if (dist < 5.0) {
            m_isHovering = true;
            m_hoveredPoint = point;
            emit hovered(point, dataPos);
            break;
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
        std::cout << "[GraphWidget::drawGrid] START" << std::endl;
        
        if (!m_showGrid) {
            std::cout << "[GraphWidget::drawGrid] Grid disabled, skipping" << std::endl;
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
        
        std::cout << "[GraphWidget::drawGrid] END - Grid drawn successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawGrid] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawGrid] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

void GraphWidget::drawAxes(QPainter& painter)
{
    try {
        std::cout << "[GraphWidget::drawAxes] START" << std::endl;
        
        QRectF area = plotArea();
        
        painter.setPen(QPen(m_textColor, 2));
        
        // Oś X
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.bottom()),
                        static_cast<int>(area.right()), static_cast<int>(area.bottom()));
        std::cout << "[GraphWidget::drawAxes] X-axis drawn from (" 
                  << area.left() << ", " << area.bottom() << ") to ("
                  << area.right() << ", " << area.bottom() << ")" << std::endl;
        
        // Oś Y
        painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.top()),
                        static_cast<int>(area.left()), static_cast<int>(area.bottom()));
        std::cout << "[GraphWidget::drawAxes] Y-axis drawn from (" 
                  << area.left() << ", " << area.top() << ") to ("
                  << area.left() << ", " << area.bottom() << ")" << std::endl;
        
        std::cout << "[GraphWidget::drawAxes] END - Axes drawn successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawAxes] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawAxes] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

void GraphWidget::drawDataLine(QPainter& painter)
{
    try {
        std::cout << "[GraphWidget::drawDataLine] START - Processing " 
                  << m_dataPoints.size() << " data points" << std::endl;
        
        if (m_dataPoints.size() < 2) {
            std::cout << "[GraphWidget::drawDataLine] Not enough data points (< 2), skipping line drawing" << std::endl;
            return;
        }
        
        QRectF area = plotArea();
        
        // Ścieżka linii
        QPainterPath path;
        QPointF firstPoint = dataToScreen(m_dataPoints.first());
        std::cout << "[GraphWidget::drawDataLine] First point screen coordinates: (" 
                  << firstPoint.x() << ", " << firstPoint.y() << ")" << std::endl;
        path.moveTo(firstPoint);
        
        int validPoints = 0;
        int invalidPoints = 0;
        for (int i = 1; i < m_dataPoints.size(); ++i) {
            QPointF point = dataToScreen(m_dataPoints[i]);
            
            // Check for invalid coordinates
            if (std::isnan(point.x()) || std::isnan(point.y()) || 
                std::isinf(point.x()) || std::isinf(point.y())) {
                invalidPoints++;
                std::cerr << "[GraphWidget::drawDataLine] WARNING: Invalid point at index " << i 
                          << " (NaN/Inf coordinates), skipping" << std::endl;
                continue;
            }
            
            path.lineTo(point);
            validPoints++;
        }
        
        std::cout << "[GraphWidget::drawDataLine] Processed " << validPoints 
                  << " valid points, " << invalidPoints << " invalid points" << std::endl;
        
        // Rysowanie linii
        painter.setPen(QPen(m_lineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(path);
        std::cout << "[GraphWidget::drawDataLine] Line path drawn successfully" << std::endl;
        
        // Wypełnienie pod wykresem
        QPainterPath fillPath = path;
        fillPath.lineTo(area.right(), area.bottom());
        fillPath.lineTo(area.left(), area.bottom());
        fillPath.closeSubpath();
        
        painter.setBrush(m_fillColor);
        painter.setPen(Qt::NoPen);
        painter.drawPath(fillPath);
        std::cout << "[GraphWidget::drawDataLine] Fill path drawn successfully" << std::endl;
        
        std::cout << "[GraphWidget::drawDataLine] END - Data line rendered successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawDataLine] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawDataLine] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

void GraphWidget::drawDataPoints(QPainter& painter)
{
    try {
        std::cout << "[GraphWidget::drawDataPoints] START - Drawing data points, total points: " 
                  << m_dataPoints.size() << std::endl;
        
        if (m_dataPoints.isEmpty()) {
            std::cout << "[GraphWidget::drawDataPoints] No data points to draw" << std::endl;
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
                std::cerr << "[GraphWidget::drawDataPoints] WARNING: Last point has invalid coordinates, skipping" << std::endl;
            } else {
                std::cout << "[GraphWidget::drawDataPoints] Drawing last point at (" 
                          << lastPoint.x() << ", " << lastPoint.y() << ")" << std::endl;
                painter.drawEllipse(lastPoint, 4, 4);
            }
        }
        
        // Podświetl punkt pod kursorem
        if (m_isHovering) {
            QPointF hoverPoint = dataToScreen(m_hoveredPoint);
            
            if (std::isnan(hoverPoint.x()) || std::isnan(hoverPoint.y()) ||
                std::isinf(hoverPoint.x()) || std::isinf(hoverPoint.y())) {
                std::cerr << "[GraphWidget::drawDataPoints] WARNING: Hovered point has invalid coordinates, skipping highlight" << std::endl;
            } else {
                std::cout << "[GraphWidget::drawDataPoints] Highlighting hovered point at (" 
                          << hoverPoint.x() << ", " << hoverPoint.y() << ")" << std::endl;
                painter.setBrush(QColor(255, 100, 100));
                painter.drawEllipse(hoverPoint, 6, 6);
            }
        }
        
        std::cout << "[GraphWidget::drawDataPoints] END - Data points drawn successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawDataPoints] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawDataPoints] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

void GraphWidget::drawLabels(QPainter& painter)
{
    try {
        std::cout << "[GraphWidget::drawLabels] START" << std::endl;
        
        QRectF area = plotArea();
        
        painter.setPen(m_textColor);
        painter.setFont(QFont("Arial", 9));
        
        // Tytuł
        if (!m_title.isEmpty()) {
            std::cout << "[GraphWidget::drawLabels] Drawing title: " << m_title.toStdString() << std::endl;
            QFont titleFont = painter.font();
            titleFont.setBold(true);
            titleFont.setPointSize(12);
            painter.setFont(titleFont);
            painter.drawText(area.left(), area.top() - 10, m_title);
            painter.setFont(QFont("Arial", 9));
        }
        
        // Etykieta osi Y
        if (!m_yLabel.isEmpty()) {
            std::cout << "[GraphWidget::drawLabels] Drawing Y-axis label: " << m_yLabel.toStdString() << std::endl;
            painter.save();
            painter.translate(10, area.center().y());
            painter.rotate(-90);
            painter.drawText(0, 0, m_yLabel);
            painter.restore();
        }
        
        // Wartości na osi Y
        std::cout << "[GraphWidget::drawLabels] Drawing Y-axis values (range: " 
                  << m_yMin << " to " << m_yMax << ")" << std::endl;
        int gridLines = 5;
        for (int i = 0; i <= gridLines; ++i) {
            double value = m_yMax - (m_yMax - m_yMin) * i / gridLines;
            
            // Validate value before displaying
            if (std::isnan(value) || std::isinf(value)) {
                std::cerr << "[GraphWidget::drawLabels] WARNING: Invalid Y-axis value at index " << i << std::endl;
                continue;
            }
            
            QString label = QString::number(value, 'f', 1);
            int y = static_cast<int>(area.top() + (area.height() * i / gridLines));
            painter.drawText(5, y + 3, label);
        }
        
        // Wartości na osi X (czas)
        std::cout << "[GraphWidget::drawLabels] Drawing X-axis time labels (time range: " 
                  << m_timeRangeSeconds << "s)" << std::endl;
        int timeSteps = 6;
        QDateTime now = QDateTime::currentDateTime();
        for (int i = 0; i <= timeSteps; ++i) {
            int seconds = -m_timeRangeSeconds * (timeSteps - i) / timeSteps;
            QTime time = now.addSecs(seconds).time();
            QString label = time.toString("hh:mm:ss");
            int x = static_cast<int>(area.left() + (area.width() * i / timeSteps));
            painter.drawText(x - 30, static_cast<int>(area.bottom()) + 20, 60, 20, Qt::AlignCenter, label);
        }
        
        std::cout << "[GraphWidget::drawLabels] END - Labels drawn successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawLabels] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawLabels] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

void GraphWidget::drawCurrentValue(QPainter& painter)
{
    try {
        std::cout << "[GraphWidget::drawCurrentValue] START" << std::endl;
        
        if (m_dataPoints.isEmpty()) {
            std::cout << "[GraphWidget::drawCurrentValue] No data points, skipping current value display" << std::endl;
            return;
        }
        
        const DataPoint& last = m_dataPoints.last();
        std::cout << "[GraphWidget::drawCurrentValue] Last data point - Value: " << last.value 
                  << " " << m_unit.toStdString() 
                  << ", Timestamp: " << last.timestamp.toString().toStdString() << std::endl;
        
        QPointF pos = dataToScreen(last);
        
        // Validate position
        if (std::isnan(pos.x()) || std::isnan(pos.y()) ||
            std::isinf(pos.x()) || std::isinf(pos.y())) {
            std::cerr << "[GraphWidget::drawCurrentValue] WARNING: Invalid screen position for current value, skipping" << std::endl;
            return;
        }
        
        std::cout << "[GraphWidget::drawCurrentValue] Screen position: (" 
                  << pos.x() << ", " << pos.y() << ")" << std::endl;
        
        // Etykieta z wartością
        QString valueText = QString("%1 %2").arg(last.value, 0, 'f', 2).arg(m_unit);
        std::cout << "[GraphWidget::drawCurrentValue] Value text: " << valueText.toStdString() << std::endl;
        
        painter.setPen(Qt::white);
        painter.setBrush(m_lineColor);
        
        QRect textRect = painter.fontMetrics().boundingRect(valueText);
        textRect.adjust(-5, -3, 5, 3);
        textRect.moveTopLeft(QPoint(static_cast<int>(pos.x()) + 10, static_cast<int>(pos.y()) - 15));
        
        painter.drawRoundedRect(textRect, 3, 3);
        painter.drawText(textRect, Qt::AlignCenter, valueText);
        
        std::cout << "[GraphWidget::drawCurrentValue] END - Current value label drawn successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::drawCurrentValue] ERROR: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "[GraphWidget::drawCurrentValue] ERROR: Unknown exception" << std::endl;
        throw;
    }
}

QPointF GraphWidget::dataToScreen(const DataPoint& point) const
{
    try {
        std::cout << "[GraphWidget::dataToScreen] Converting data point - Value: " << point.value 
                  << ", Timestamp: " << point.timestamp.toString().toStdString() << std::endl;
        
        QRectF area = plotArea();
        
        // Validate plot area
        if (area.width() <= 0 || area.height() <= 0) {
            std::cerr << "[GraphWidget::dataToScreen] ERROR: Invalid plot area (width=" 
                      << area.width() << ", height=" << area.height() << ")" << std::endl;
            return QPointF(0, 0);
        }
        
        QDateTime now = QDateTime::currentDateTime();
        qint64 msNow = now.toMSecsSinceEpoch();
        qint64 msPoint = point.timestamp.toMSecsSinceEpoch();
        qint64 msRange = m_timeRangeSeconds * 1000;
        
        // Avoid division by zero
        if (msRange <= 0) {
            std::cerr << "[GraphWidget::dataToScreen] ERROR: Invalid time range (" << msRange << "ms)" << std::endl;
            return QPointF(0, 0);
        }
        
        double xRatio = static_cast<double>(msPoint - (msNow - msRange)) / msRange;
        xRatio = qBound(0.0, xRatio, 1.0);
        
        // Avoid division by zero for Y-axis
        double yRange = m_yMax - m_yMin;
        if (std::abs(yRange) < 0.0001) {
            std::cerr << "[GraphWidget::dataToScreen] WARNING: Very small Y range (" << yRange 
                      << "), using default" << std::endl;
            yRange = 100.0;
        }
        
        double yRatio = (point.value - m_yMin) / yRange;
        yRatio = qBound(0.0, yRatio, 1.0);
        
        double x = area.left() + xRatio * area.width();
        double y = area.bottom() - yRatio * area.height();
        
        // Validate result
        if (std::isnan(x) || std::isnan(y) || std::isinf(x) || std::isinf(y)) {
            std::cerr << "[GraphWidget::dataToScreen] ERROR: Result contains NaN/Inf - x=" 
                      << x << ", y=" << y << std::endl;
            return QPointF(0, 0);
        }
        
        std::cout << "[GraphWidget::dataToScreen] Screen coordinates: (" << x << ", " << y << ")" << std::endl;
        
        return QPointF(x, y);
    } catch (const std::exception& e) {
        std::cerr << "[GraphWidget::dataToScreen] ERROR: " << e.what() << std::endl;
        return QPointF(0, 0);
    } catch (...) {
        std::cerr << "[GraphWidget::dataToScreen] ERROR: Unknown exception" << std::endl;
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
