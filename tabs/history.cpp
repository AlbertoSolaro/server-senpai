#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <QDebug>
#include <db/Db_original.h>
#include <atltime.h>
#include <ctime>
#include <regex>
#include <QtGlobal>

void MainWindow::show_history_plot(QLabel *histLabel, QChartView *histChartViewBar, QDateTimeEdit *histDateEdit) {
    QDateTime temp = histDateEdit->dateTime();
    QString dateText = QString("Date selected: %1").arg(temp.toString("d/M/yyyy"));
    histLabel->setText(dateText);
    QBarSet *set0hist = new QBarSet("Private MAC");
    QBarSet *set1hist = new QBarSet("Public MAC");

    map<string, num_ril> histMap;

    time_t histStart;
    time_t histEnd;

    histStart = temp.toTime_t();
    histEnd = temp.addSecs(1800).toTime_t();

    histMap = db->number_of_rilevations(histStart, histEnd);
    for (map<string, num_ril>::iterator it = histMap.begin(); it != histMap.end(); ++it)
        qDebug() << it->first.c_str() << " " << it->second.n_pub << " - " << it->second.n_priv;

    for (map<string, num_ril>::iterator itMap = histMap.begin(); itMap != histMap.end(); ++itMap) {
        *set0hist << itMap->second.n_pub;
        *set1hist << itMap->second.n_priv;
    }

    QStackedBarSeries *seriesBar = new QStackedBarSeries();
    seriesBar->append(set0hist);
    seriesBar->append(set1hist);

    // Configure updated chart
    QChart *chartBar = new QChart();
    chartBar->addSeries(seriesBar);
    chartBar->setTitle("Number of devices tracked");
    chartBar->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;

    categories << temp.time().toString("hh:mm") << temp.time().addSecs(300).toString("hh:mm") << temp.time().addSecs(600).toString("hh:mm") << temp.time().addSecs(900).toString("hh:mm") << temp.time().addSecs(1200).toString("hh:mm") << temp.time().addSecs(1500).toString("hh:mm");

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chartBar->addAxis(axisX, Qt::AlignBottom);
    seriesBar->attachAxis(axisX);
    QValueAxis *axisY = new QValueAxis();
    chartBar->addAxis(axisY, Qt::AlignLeft);
    seriesBar->attachAxis(axisY);

    chartBar->legend()->setVisible(true);
    chartBar->legend()->setAlignment(Qt::AlignBottom);

    histChartViewBar->setChart(chartBar);
    histChartViewBar->setRenderHint(QPainter::Antialiasing);
}