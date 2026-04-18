/**
 * @file qt5_mandatory.cpp
 * @brief Minimal mandatory Qt5 + QtCharts code example.
 */

#include <QApplication>
#include <QLabel>
#include <QString>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto* series = new QtCharts::QLineSeries();
    series->append(0.0, 0.0);
    series->append(1.0, 1.0);
    series->append(2.0, 0.5);

    auto* chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle(QStringLiteral("Biofeedback: Qt5 + QtCharts example is active."));

    QtCharts::QChartView chartView(chart);
    chartView.setMinimumSize(420, 280);
    chartView.show();

    return app.exec();
}
