#include "gui/GraphWidget.hpp"
#include <QPainterPath>
#include <QGradient>
#include <QLinearGradient>

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
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    
    // Automatyczne odświeżanie co 50ms
    QTimer* pruneTimer = new QTimer(this);
    connect(pruneTimer, &QTimer::timeout, this, &GraphWidget::pruneOldData);
    pruneTimer->start(50);
}

GraphWidget::~GraphWidget()
{
}

void GraphWidget::addDataPoint(double value, const QDateTime& timestamp)
{
    m_dataPoints.append(DataPoint(timestamp, value));
    
    if (m_autoScaleY) {
        calculateAutoScale();
    }
    
    m_cacheValid = false;
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
    if (m_dataPoints.isEmpty()) {
        m_yMin = 0.0;
        m_yMax = 100.0;
        return;
    }
    
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();
    
    for (const DataPoint& point : m_dataPoints) {
        if (point.value < minVal) minVal = point.value;
        if (point.value > maxVal) maxVal = point.value;
    }
    
    // Dodaj margines 10%
    double range = maxVal - minVal;
    if (range < 0.001) range = 100.0;  // Minimalny zakres
    
    m_yMin = minVal - range * 0.1;
    m_yMax = maxVal + range * 0.1;
    
    if (m_yMin < 0) m_yMin = 0;  // Nie pokazuj ujemnych wartości dla siły
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
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
    
    drawGrid(painter);
    drawAxes(painter);
    drawDataLine(painter);
    drawDataPoints(painter);
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
    if (!m_showGrid) return;
    
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
}

void GraphWidget::drawAxes(QPainter& painter)
{
    QRectF area = plotArea();
    
    painter.setPen(QPen(m_textColor, 2));
    
    // Oś X
    painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.bottom()),
                    static_cast<int>(area.right()), static_cast<int>(area.bottom()));
    
    // Oś Y
    painter.drawLine(static_cast<int>(area.left()), static_cast<int>(area.top()),
                    static_cast<int>(area.left()), static_cast<int>(area.bottom()));
}

void GraphWidget::drawDataLine(QPainter& painter)
{
    if (m_dataPoints.size() < 2) return;
    
    QRectF area = plotArea();
    
    // Ścieżka linii
    QPainterPath path;
    QPointF firstPoint = dataToScreen(m_dataPoints.first());
    path.moveTo(firstPoint);
    
    for (int i = 1; i < m_dataPoints.size(); ++i) {
        QPointF point = dataToScreen(m_dataPoints[i]);
        path.lineTo(point);
    }
    
    // Rysowanie linii
    painter.setPen(QPen(m_lineColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawPath(path);
    
    // Wypełnienie pod wykresem
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
    if (m_dataPoints.isEmpty()) return;
    
    painter.setBrush(m_lineColor);
    painter.setPen(Qt::NoPen);
    
    // Rysuj tylko ostatni punkt lub punkty w pobliżu kursora
    if (!m_dataPoints.isEmpty()) {
        QPointF lastPoint = dataToScreen(m_dataPoints.last());
        painter.drawEllipse(lastPoint, 4, 4);
    }
    
    // Podświetl punkt pod kursorem
    if (m_isHovering) {
        QPointF hoverPoint = dataToScreen(m_hoveredPoint);
        painter.setBrush(QColor(255, 100, 100));
        painter.drawEllipse(hoverPoint, 6, 6);
    }
}

void GraphWidget::drawLabels(QPainter& painter)
{
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
}

void GraphWidget::drawCurrentValue(QPainter& painter)
{
    if (m_dataPoints.isEmpty()) return;
    
    const DataPoint& last = m_dataPoints.last();
    QPointF pos = dataToScreen(last);
    
    // Etykieta z wartością
    QString valueText = QString("%1 %2").arg(last.value, 0, 'f', 2).arg(m_unit);
    
    painter.setPen(Qt::white);
    painter.setBrush(m_lineColor);
    
    QRect textRect = painter.fontMetrics().boundingRect(valueText);
    textRect.adjust(-5, -3, 5, 3);
    textRect.moveTopLeft(QPoint(static_cast<int>(pos.x()) + 10, static_cast<int>(pos.y()) - 15));
    
    painter.drawRoundedRect(textRect, 3, 3);
    painter.drawText(textRect, Qt::AlignCenter, valueText);
}

QPointF GraphWidget::dataToScreen(const DataPoint& point) const
{
    QRectF area = plotArea();
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 msNow = now.toMSecsSinceEpoch();
    qint64 msPoint = point.timestamp.toMSecsSinceEpoch();
    qint64 msRange = m_timeRangeSeconds * 1000;
    
    double xRatio = static_cast<double>(msPoint - (msNow - msRange)) / msRange;
    xRatio = qBound(0.0, xRatio, 1.0);
    
    double yRatio = (point.value - m_yMin) / (m_yMax - m_yMin);
    yRatio = qBound(0.0, yRatio, 1.0);
    
    double x = area.left() + xRatio * area.width();
    double y = area.bottom() - yRatio * area.height();
    
    return QPointF(x, y);
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

} // namespace gui
