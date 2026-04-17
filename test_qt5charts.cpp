#include <QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Checking Qt5Charts linkage...";
    
    // Create simple line series
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < 10; i++) {
        series->append(i, i * i);
    }
    
    // Create chart
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Qt5Charts Test - Simple Line Chart");
    
    // Create axis
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0, 10);
    axisX->setTitleText("X Axis");
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 100);
    axisY->setTitleText("Y Axis");
    
    chart->setAxisX(axisX, series);
    chart->setAxisY(axisY, series);
    
    // Create chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(800, 600);
    chartView->show();
    
    qDebug() << "Qt5Charts successfully linked and working!";
    qDebug() << "Chart view created and displayed.";
    
    return app.exec();
}
