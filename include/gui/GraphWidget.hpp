#ifndef GUI_GRAPHWIDGET_HPP
#define GUI_GRAPHWIDGET_HPP

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDateTime>
#include <QTimer>
#include <cmath>
#include <algorithm>
#include <limits>

namespace gui {

/**
 * @struct DataPoint
 * @brief Struktura reprezentująca pojedynczy punkt danych na wykresie
 */
struct DataPoint {
    QDateTime timestamp;  ///< Czas pomiaru
    double value;         ///< Wartość pomiaru (waga)
    
    DataPoint() : timestamp(QDateTime::currentDateTime()), value(0.0) {}
    DataPoint(const QDateTime& ts, double val) : timestamp(ts), value(val) {}
};

/**
 * @class GraphWidget
 * @brief Widget do wizualizacji danych w czasie rzeczywistym
 * 
 * GraphWidget zapewnia wydajne renderowanie wykresu liniowego z danymi
 * z wagi w czasie rzeczywistym. Obsługuje:
 * - Automatyczne skalowanie osi Y
 * - Przewijanie i zoomowanie
 * - Wyróżnianie punktów maksymalnych/minimalnych
 * - Siatkę pomocniczą i etykiety osi
 * - Tryb ciemny/jasny
 */
class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget() override;

    /**
     * @brief Dodaje nowy punkt danych do wykresu
     * @param value Wartość do dodania
     * @param timestamp Czas pomiaru (domyślnie obecny czas)
     */
    void addDataPoint(double value, const QDateTime& timestamp = QDateTime::currentDateTime());

    /**
     * @brief Czyści wszystkie dane z wykresu
     */
    void clear();

    /**
     * @brief Ustawia zakres widocznych danych (w sekundach)
     * @param seconds Liczba sekund widocznych na wykresie
     */
    void setTimeRange(int seconds);

    /**
     * @brief Pobiera aktualny zakres czasowy
     * @return Liczba sekund widocznych na wykresie
     */
    int timeRange() const { return m_timeRangeSeconds; }

    /**
     * @brief Ustawia ręczny zakres osi Y
     * @param min Wartość minimalna
     * @param max Wartość maksymalna
     * @param autoScale true dla automatycznego skalowania
     */
    void setYRange(double min, double max, bool autoScale = false);

    /**
     * @brief Sprawdza czy autoskalowanie jest włączone
     * @return true jeśli autoskalowanie jest aktywne
     */
    bool isAutoScaling() const { return m_autoScaleY; }

    /**
     * @brief Ustawia tytuł wykresu
     * @param title Tekst tytułu
     */
    void setTitle(const QString& title);

    /**
     * @brief Ustawia etykietę osi Y
     * @param label Tekst etykiety
     */
    void setYLabel(const QString& label);

    /**
     * @brief Ustawia jednostkę wyświetlanych wartości
     * @param unit Symbol jednostki (np. "g", "kg")
     */
    void setUnit(const QString& unit);

    /**
     * @brief Zwraca liczbę przechowywanych punktów danych
     * @return Liczba punktów
     */
    int dataPointCount() const { return m_dataPoints.size(); }

    /**
     * @brief Eksportuje dane do wektora
     * @return Wektor wszystkich punktów danych
     */
    QVector<DataPoint> getData() const { return m_dataPoints; }

    /**
     * @brief Ustawia kolor linii wykresu
     * @param color Nowy kolor
     */
    void setLineColor(const QColor& color);

    /**
     * @brief Ustawia kolor wypełnienia pod wykresem
     * @param color Nowy kolor
     */
    void setFillColor(const QColor& color);

    /**
     * @brief Włącza/wyłącza wyświetlanie siatki
     * @param enabled true aby włączyć siatkę
     */
    void setShowGrid(bool enabled);

    /**
     * @brief Włącza/wyłącza wyświetlanie wartości na wykresie
     * @param enabled true aby włączyć etykiety wartości
     */
    void setShowValues(bool enabled);

public slots:
    /**
     * @brief Wymusza odświeżenie wykresu
     */
    void refresh();

    /**
     * @brief Usuwa stare punkty danych spoza zakresu czasowego
     */
    void pruneOldData();

signals:
    /**
     * @brief Sygnał emitowany gdy najechano kursorem nad punkt danych
     * @param point Punkt danych
     * @param pos Pozycja kursora
     */
    void hovered(const DataPoint& point, const QPointF& pos);

    /**
     * @brief Sygnał emitowany przy zmianie zakresu osi Y
     * @param min Nowa wartość minimalna
     * @param max Nowa wartość maksymalna
     */
    void yRangeChanged(double min, double max);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void calculateAutoScale();
    void drawGrid(QPainter& painter);
    void drawAxes(QPainter& painter);
    void drawDataLine(QPainter& painter);
    void drawDataPoints(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawCurrentValue(QPainter& painter);
    
    QPointF dataToScreen(const DataPoint& point) const;
    QPointF screenToData(const QPointF& screenPos) const;
    
    QRectF plotArea() const;
    int leftMargin() const { return 60; }
    int rightMargin() const { return 20; }
    int topMargin() const { return 40; }
    int bottomMargin() const { return 50; }

    // Dane wykresu
    QVector<DataPoint> m_dataPoints;
    
    // Zakresy osi
    double m_yMin;
    double m_yMax;
    bool m_autoScaleY;
    int m_timeRangeSeconds;
    
    // Wygląd
    QString m_title;
    QString m_yLabel;
    QString m_unit;
    QColor m_lineColor;
    QColor m_fillColor;
    QColor m_gridColor;
    QColor m_textColor;
    QColor m_bgColor;
    
    bool m_showGrid;
    bool m_showValues;
    bool m_showCurrent;
    
    // Interakcja
    bool m_isPanning;
    QPoint m_lastMousePos;
    bool m_isHovering;
    DataPoint m_hoveredPoint;
    
    // Cache obliczeń
    mutable QRectF m_plotAreaCache;
    mutable bool m_cacheValid;
};

} // namespace gui

#endif // GUI_GRAPHWIDGET_HPP
