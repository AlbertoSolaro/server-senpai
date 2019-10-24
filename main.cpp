#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

#include <QApplication>
#include <QProcess>
#include <QScatterSeries>

QT_CHARTS_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.resize(800, 700);
    w.show();
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(closing()));
    return a.exec();
}
