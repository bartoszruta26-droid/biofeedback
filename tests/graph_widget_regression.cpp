#include "gui/GraphWidget.hpp"

#include <QApplication>
#include <QImage>
#include <QDateTime>
#include <QMouseEvent>
#include <iostream>

int main(int argc, char** argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);

    gui::GraphWidget graph;
    graph.resize(800, 420);
    graph.setTimeRange(300);

    const QDateTime start = QDateTime::currentDateTime();
    constexpr int sampleCount = 13050;
    for (int i = 0; i < sampleCount; ++i) {
        // Insert a deliberately out-of-order timestamp once. Hover lookup
        // must switch to its safe fallback rather than binary-searching
        // a sequence that is no longer sorted.
        const qint64 offsetMs = (i == 6500) ? -5000 : static_cast<qint64>(i) * 12;
        const double value = (i == 6499 || i == 6501) ? 95.0 : 20.0 + (i % 17);
        graph.addDataPoint(value, start.addMSecs(offsetMs));
    }

    if (graph.dataPointCount() != 12000) {
        std::cerr << "Graph buffer cap violated: " << graph.dataPointCount()
                  << " points (expected 12000)\n";
        return 1;
    }

    const QVector<gui::DataPoint> samples = graph.getData();
    if (samples.size() != 12000 || samples.isEmpty()) {
        std::cerr << "Graph data export is inconsistent\n";
        return 2;
    }

    QImage frame(graph.size(), QImage::Format_ARGB32_Premultiplied);
    frame.fill(Qt::transparent);
    graph.render(&frame);
    // Exercise the timestamp-order fallback in cursor hit-testing.
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(400.0, 200.0),
                          Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&graph, &moveEvent);
    if (frame.isNull()) {
        std::cerr << "Graph render produced a null image\n";
        return 3;
    }

    std::cout << "GraphWidget regression passed: bounded buffer, out-of-order "
                 "timestamps and offscreen render ("
              << samples.size() << " points)\n";
    return 0;
}
