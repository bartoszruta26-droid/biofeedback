#include "gui/GraphWidget.hpp"
#include <QPainterPath>
#include <QGradient>
#include <QLinearGradient>

namespace gui {

// ============================================================================
// GraphWidget - Implementacja
// ============================================================================

GraphWidget::GraphWidget(QWidget *parent)
    : QWidget(parent)
    , m_yMin(0.0)
    , m_yMax(100.0)
    , m_autoScaleY(true)
    , m_timeRangeSeconds(60)
    , m_title("Wykres Danych")
    , m_yLabel("Wartość")
    , m_unit("")
    , m_lineColor(0, 100, 200)
    , m_fillColor(0, 100, 200, 50)
    , m_gridColor(200, 200, 200)
    , m_textColor(50, 50, 50)
    , m_bgColor(255, 255, 255)
    , m_showGrid(true)
    , m_showValues(false)
    , m_showCurrent(true)
    , m_isPanning(false)
    , m_isHovering(false)
    , m_cacheValid(false)
{
    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Włącz podwójne buforowanie dla płynnego renderowania
    setAttribute(Qt::WA_OpaquePaintEvent);
}

GraphWidget::~GraphWidget()
{
}

void GraphWidget::addDataPoint(double value, const QDateTime& timestamp)
{
    m_dataPoints.append(DataPoint(timestamp, value));
    
    // Automatyczne skalowanie osi Y
    if (m_autoScaleY) {
        calculateAutoScale();
    }
    
    // Usuń stare dane poza zakresem czasowym
    pruneOldData();
    
    update();
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
    
    if (!autoScale) {
        if (min < max) {
            m_yMin = min;
            m_yMax = max;
            m_cacheValid = false;
            update();
        }
    } else {
        calculateAutoScale();
    }
}

void GraphWidget::setTitle(const QString& title)
{
    if (title != m_title) {
        m_title = title;
        update();
    }
}

void GraphWidget::setYLabel(const QString& label)
{
    if (label != m_yLabel) {
        m_yLabel = label;
        update();
    }
}

void GraphWidget::setUnit(const QString& unit)
{
    if (unit != m_unit) {
        m_unit = unit;
        update();
    }
}

void GraphWidget::setLineColor(const QColor& color)
{
    if (color != m_lineColor) {
        m_lineColor = color;
        update();
    }
}

void GraphWidget::setFillColor(const QColor& color)
{
    if (color != m_fillColor) {
        m_fillColor = color;
        update();
    }
}

void GraphWidget::setShowGrid(bool enabled)
{
    if (enabled != m_showGrid) {
        m_showGrid = enabled;
        update();
    }
}

void GraphWidget::setShowValues(bool enabled)
{
    if (enabled != m_showValues) {
        m_showValues = enabled;
        update();
    }
}

void GraphWidget::refresh()
{
    update();
}

void GraphWidget::pruneOldData()
{
    if (m_dataPoints.isEmpty()) return;
    
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    
    // Usuń punkty starsze niż zakres czasowy
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
    }
}

void GraphWidget::calculateAutoScale()
{
    if (m_dataPoints.isEmpty()) {
        m_yMin = 0.0;
        m_yMax = 100.0;
        return;
    }
    
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    
    // Znajdź min/max z widocznych danych
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    
    for (const auto& point : m_dataPoints) {
        if (point.timestamp >= cutoff) {
            if (point.value < min) min = point.value;
            if (point.value > max) max = point.value;
        }
    }
    
    // Dodaj margines 10%
    double range = max - min;
    if (range < 0.001) range = 1.0;  // Minimalny zakres
    
    m_yMin = min - range * 0.1;
    m_yMax = max + range * 0.1;
    
    // Zaokrąglij do ładnych wartości
    double step = (m_yMax - m_yMin) / 10.0;
    if (step > 0) {
        double magnitude = std::pow(10.0, std::floor(std::log10(step)));
        double normalized = step / magnitude;
        
        if (normalized <= 1.0) step = 1.0 * magnitude;
        else if (normalized <= 2.0) step = 2.0 * magnitude;
        else if (normalized <= 5.0) step = 5.0 * magnitude;
        else step = 10.0 * magnitude;
        
        m_yMin = std::floor(m_yMin / step) * step;
        m_yMax = std::ceil(m_yMax / step) * step;
    }
    
    m_cacheValid = false;
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Tło
    painter.fillRect(rect(), m_bgColor);
    
    // Obszar wykresu
    QRectF plot = plotArea();
    
    if (plot.width() <= 0 || plot.height() <= 0) {
        return;
    }
    
    // Rysuj elementy wykresu
    if (m_showGrid) {
        drawGrid(painter);
    }
    
    drawAxes(painter);
    
    if (!m_dataPoints.isEmpty()) {
        drawDataLine(painter);
        drawDataPoints(painter);
    }
    
    drawLabels(painter);
    
    if (m_showCurrent && !m_dataPoints.isEmpty()) {
        drawCurrentValue(painter);
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
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        
        // Przesuwanie widoku (można rozszerzyć)
        update();
    } else {
        // Sprawdź hover nad punktami
        QPointF dataPos = screenToData(event->pos());
        m_isHovering = false;
        
        for (const auto& point : m_dataPoints) {
            QPointF screenPos = dataToScreen(point);
            if (QPointF(screenPos - event->pos()).manhattanLength() < 10) {
                m_isHovering = true;
                m_hoveredPoint = point;
                emit hovered(point, dataPos);
                update();
                break;
            }
        }
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
    // Zoomowanie kółkiem myszy
    qreal delta = event->angleDelta().y() / 120.0;
    
    if (delta > 0) {
        // Przybliżenie - zmniejsz zakres czasu
        m_timeRangeSeconds = std::max(5, m_timeRangeSeconds - 5);
    } else {
        // Oddalenie - zwiększ zakres czasu
        m_timeRangeSeconds = std::min(300, m_timeRangeSeconds + 5);
    }
    
    pruneOldData();
    update();
    emit yRangeChanged(m_yMin, m_yMax);
}

void GraphWidget::leaveEvent(QEvent *event)
{
    m_isHovering = false;
    QWidget::leaveEvent(event);
}

void GraphWidget::drawGrid(QPainter& painter)
{
    QRectF plot = plotArea();
    painter.setPen(QPen(m_gridColor, 1, Qt::DashLine));
    
    // Linie poziome (oś Y)
    int gridLinesY = 5;
    for (int i = 0; i <= gridLinesY; ++i) {
        double y = plot.top() + (plot.height() * i / gridLinesY);
        painter.drawLine(plot.left(), static_cast<int>(y), 
                        plot.right(), static_cast<int>(y));
    }
    
    // Linie pionowe (oś X)
    int gridLinesX = 6;
    for (int i = 0; i <= gridLinesX; ++i) {
        double x = plot.left() + (plot.width() * i / gridLinesX);
        painter.drawLine(static_cast<int>(x), plot.top(), 
                        static_cast<int>(x), plot.bottom());
    }
}

void GraphWidget::drawAxes(QPainter& painter)
{
    QRectF plot = plotArea();
    painter.setPen(QPen(m_textColor, 2));
    
    // Oś Y
    painter.drawLine(plot.topLeft(), plot.bottomLeft());
    
    // Oś X
    painter.drawLine(plot.bottomLeft(), plot.bottomRight());
}

void GraphWidget::drawDataLine(QPainter& painter)
{
    if (m_dataPoints.size() < 2) return;
    
    QRectF plot = plotArea();
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    
    // Ścieżka linii
    QPainterPath path;
    bool firstPoint = true;
    
    for (const auto& point : m_dataPoints) {
        if (point.timestamp < cutoff) continue;
        
        QPointF pos = dataToScreen(point);
        
        if (firstPoint) {
            path.moveTo(pos);
            firstPoint = false;
        } else {
            path.lineTo(pos);
        }
    }
    
    // Rysuj linię
    painter.setPen(QPen(m_lineColor, 2, Qt::SolidLine));
    painter.drawPath(path);
    
    // Wypełnienie pod wykresem
    if (m_fillColor.alpha() > 0) {
        QPainterPath fillPath = path;
        fillPath.lineTo(plot.bottomRight());
        fillPath.lineTo(plot.bottomLeft());
        fillPath.closeSubpath();
        
        painter.fillPath(fillPath, m_fillColor);
    }
}

void GraphWidget::drawDataPoints(QPainter& painter)
{
    if (m_dataPoints.isEmpty()) return;
    
    QRectF plot = plotArea();
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-m_timeRangeSeconds);
    
    painter.setBrush(QBrush(m_lineColor));
    painter.setPen(Qt::NoPen);
    
    for (const auto& point : m_dataPoints) {
        if (point.timestamp < cutoff) continue;
        
        QPointF pos = dataToScreen(point);
        
        // Rysuj punkt tylko jeśli jest w obszarze wykresu
        if (plot.contains(pos)) {
            painter.drawEllipse(pos, 3, 3);
        }
    }
    
    // Wyróżniony punkt (hover)
    if (m_isHovering) {
        QPointF pos = dataToScreen(m_hoveredPoint);
        painter.setBrush(QBrush(Qt::red));
        painter.drawEllipse(pos, 6, 6);
    }
}

void GraphWidget::drawLabels(QPainter& painter)
{
    QRectF plot = plotArea();
    painter.setPen(m_textColor);
    painter.setFont(QFont("Arial", 10));
    
    // Tytuł
    if (!m_title.isEmpty()) {
        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(12);
        painter.setFont(titleFont);
        painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignTop, m_title);
        painter.setFont(painter.font());
    }
    
    // Etykieta osi Y
    if (!m_yLabel.isEmpty()) {
        painter.save();
        painter.translate(15, plot.center().y());
        painter.rotate(-90);
        painter.drawText(QRectF(-20, -100, 200, 200), Qt::AlignCenter, m_yLabel);
        painter.restore();
    }
    
    // Wartości osi Y
    int gridLinesY = 5;
    painter.setFont(QFont("Arial", 9));
    for (int i = 0; i <= gridLinesY; ++i) {
        double value = m_yMax - (m_yMax - m_yMin) * i / gridLinesY;
        double y = plot.top() + (plot.height() * i / gridLinesY);
        QString label = QString::number(value, 'f', 2);
        if (!m_unit.isEmpty()) {
            label += " " + m_unit;
        }
        painter.drawText(5, static_cast<int>(y) + 3, label);
    }
    
    // Etykiety osi X (czas)
    int gridLinesX = 6;
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i <= gridLinesX; ++i) {
        int seconds = -m_timeRangeSeconds + (m_timeRangeSeconds * i / gridLinesX);
        QDateTime labelTime = now.addSecs(seconds);
        QString label = labelTime.toString("hh:mm:ss");
        
        double x = plot.left() + (plot.width() * i / gridLinesX);
        painter.drawText(static_cast<int>(x) - 30, plot.bottom() + 15, 60, 20, 
                        Qt::AlignCenter, label);
    }
}

void GraphWidget::drawCurrentValue(QPainter& painter)
{
    if (m_dataPoints.isEmpty()) return;
    
    DataPoint current = m_dataPoints.last();
    QRectF plot = plotArea();
    
    // Etykieta z aktualną wartością
    QString valueText = QString::number(current.value, 'f', 2);
    if (!m_unit.isEmpty()) {
        valueText += " " + m_unit;
    }
    
    QFont valueFont = painter.font();
    valueFont.setBold(true);
    valueFont.setPointSize(14);
    painter.setFont(valueFont);
    
    // Tło etykiety
    QRectF labelRect(plot.right() - 100, plot.top() + 10, 90, 25);
    painter.setBrush(QBrush(QColor(255, 255, 255, 200)));
    painter.setPen(QPen(m_lineColor, 2));
    painter.drawRoundedRect(labelRect, 5, 5);
    
    painter.drawText(labelRect, Qt::AlignCenter, valueText);
}

QPointF GraphWidget::dataToScreen(const DataPoint& point) const
{
    QRectF plot = plotArea();
    
    double xRatio = 0.0;
    if (m_timeRangeSeconds > 0) {
        qint64 nowMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
        qint64 pointMs = point.timestamp.toMSecsSinceEpoch();
        qint64 cutoffMs = nowMs - m_timeRangeSeconds * 1000;
        
        if (pointMs >= cutoffMs) {
            xRatio = static_cast<double>(pointMs - cutoffMs) / 
                    (m_timeRangeSeconds * 1000.0);
        }
    }
    
    double yRatio = 0.0;
    if (m_yMax > m_yMin) {
        yRatio = (point.value - m_yMin) / (m_yMax - m_yMin);
    }
    
    double x = plot.left() + xRatio * plot.width();
    double y = plot.bottom() - yRatio * plot.height();
    
    return QPointF(x, y);
}

QPointF GraphWidget::screenToData(const QPointF& screenPos) const
{
    QRectF plot = plotArea();
    
    double xRatio = (screenPos.x() - plot.left()) / plot.width();
    double yRatio = 1.0 - (screenPos.y() - plot.top()) / plot.height();
    
    qint64 nowMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 cutoffMs = nowMs - m_timeRangeSeconds * 1000;
    qint64 pointMs = static_cast<qint64>(cutoffMs + xRatio * m_timeRangeSeconds * 1000);
    
    double value = m_yMin + yRatio * (m_yMax - m_yMin);
    
    QDateTime timestamp;
    timestamp.setMSecsSinceEpoch(pointMs);
    
    return QPointF(value, 0);  // Zwracamy wartość, druga współrzędna to placeholder
}

QRectF GraphWidget::plotArea() const
{
    if (m_cacheValid) {
        return m_plotAreaCache;
    }
    
    int left = leftMargin();
    int right = width() - rightMargin();
    int top = topMargin();
    int bottom = height() - bottomMargin();
    
    if (!m_title.isEmpty()) {
        top += 20;
    }
    
    m_plotAreaCache = QRectF(left, top, right - left, bottom - top);
    m_cacheValid = true;
    
    return m_plotAreaCache;
}

} // namespace gui
