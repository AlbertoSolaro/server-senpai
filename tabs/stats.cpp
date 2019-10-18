#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <QDebug>
#include <db/Db_original.h>
#include <atltime.h>
#include <ctime>
#include <regex>
#include <QtGlobal>

void MainWindow::show_stats_graph(QLabel *statsLabel, int indexCombo, QChartView *statsChartViewBar1, QChartView *statsChartViewBar2, QDateTime temp) {
    QString dateText = QString("Date selected: %1").arg(temp.toString("d/M/yyyy"));
    statsLabel->setText(dateText);

    best_k_mac bestStat;
    time_t statsStart;

    statsStart = temp.toTime_t();

    bestStat = db->statistics_fun(statsStart, indexCombo + 1);

    QString mac1Text;
    QString mac2Text;

    if (bestStat.best_mac[0].first == "0")
        mac1Text = QString("No device detected");
    else
        mac1Text = QString("MAC: %1").arg(QString::fromStdString(bestStat.best_mac[0].first));

    if (bestStat.best_mac[1].first == "0")
        mac2Text = QString("No device detected");
    else
        mac2Text = QString("MAC: %1").arg(QString::fromStdString(bestStat.best_mac[1].first));

    // Create time series using current time for first mac
    QLineSeries *statsLineseries1 = new QLineSeries();
    statsLineseries1->setName("Number of times tracked");
    if (indexCombo == 3)
        for (int i = 0; i < 7; i++)
            statsLineseries1->append(QPoint(i, bestStat.best_mac[0].second[i]));
    else
        for (int i = 0; i < 6; i++)
            statsLineseries1->append(QPoint(i, bestStat.best_mac[0].second[i]));

    // Configure the chart using current time for first mac
    QChart *statsChartBar1 = new QChart();
    statsChartBar1->addSeries(statsLineseries1);
    statsChartBar1->setTitle(mac1Text);
    statsChartBar1->setAnimationOptions(QChart::SeriesAnimations);
    CTime time_fin;
    CTime time_in(temp.toTime_t());
    QStringList statsCategories;
    switch (indexCombo) {
        case 0:
            /* time_fin=time_in+ CTimeSpan(0,2,0,0);
        CTimeSpan  range(0,0,20,0);
        for(int i=0;i<6;i++)
        {

            statsCategories<< QString((time_fin-range).Format(_T("%H:%M")));
        }*/
            statsCategories << temp.toString("hh:mm") << temp.addSecs(1200).toString("hh:mm") << temp.addSecs(2400).toString("hh:mm") << temp.addSecs(3600).toString("hh:mm") << temp.addSecs(4800).toString("hh:mm") << temp.addSecs(6000).toString("hh:mm");
            statsCategories.sort();
            break;
        case 1:
            statsCategories << temp.toString("hh:mm") << temp.addSecs(3600).toString("hh:mm") << temp.addSecs(7200).toString("hh:mm") << temp.addSecs(10800).toString("hh:mm") << temp.addSecs(14400).toString("hh:mm") << temp.addSecs(18000).toString("hh:mm");
            break;
        case 2:
            statsCategories << temp.toString("hh:mm") << temp.addSecs(14400).toString("hh:mm") << temp.addSecs(28800).toString("hh:mm") << temp.addSecs(43200).toString("hh:mm") << temp.addSecs(57600).toString("hh:mm") << temp.addSecs(72000).toString("hh:mm");
            break;
        case 3:
            statsCategories << temp.toString("d/M/yyyy") << temp.addDays(1).toString("d/M/yyyy") << temp.addDays(2).toString("d/M/yyyy") << temp.addDays(3).toString("d/M/yyyy") << temp.addDays(4).toString("d/M/yyyy") << temp.addDays(5).toString("d/M/yyyy") << temp.addDays(6).toString("d/M/yyyy");
            break;
    }

    QBarCategoryAxis *axisXstats1 = new QBarCategoryAxis();
    axisXstats1->append(statsCategories);
    statsChartBar1->addAxis(axisXstats1, Qt::AlignBottom);
    statsLineseries1->attachAxis(axisXstats1);
    QValueAxis *axisYstats1 = new QValueAxis();
    statsChartBar1->addAxis(axisYstats1, Qt::AlignLeft);
    statsLineseries1->attachAxis(axisYstats1);

    statsChartBar1->legend()->setVisible(true);
    statsChartBar1->legend()->setAlignment(Qt::AlignBottom);

    // Create chart view for first mac
    statsChartViewBar1->setChart(statsChartBar1);
    statsChartViewBar1->setRenderHint(QPainter::Antialiasing);

    // Create time series using current time for second mac
    QLineSeries *statsLineseries2 = new QLineSeries();
    statsLineseries2->setName("Number of times tracked");
    if (indexCombo == 3)
        for (int i = 0; i < 7; i++)
            statsLineseries2->append(QPoint(i, bestStat.best_mac[1].second[i]));
    else
        for (int i = 0; i < 6; i++)
            statsLineseries2->append(QPoint(i, bestStat.best_mac[1].second[i]));

    // Configure the chart using current time for second mac
    QChart *statsChartBar2 = new QChart();
    statsChartBar2->addSeries(statsLineseries2);
    statsChartBar2->setTitle(mac2Text);
    statsChartBar2->setAnimationOptions(QChart::SeriesAnimations);

    QBarCategoryAxis *axisXstats2 = new QBarCategoryAxis();
    axisXstats2->append(statsCategories);
    statsChartBar2->addAxis(axisXstats2, Qt::AlignBottom);
    statsLineseries2->attachAxis(axisXstats2);
    QValueAxis *axisYstats2 = new QValueAxis();
    statsChartBar2->addAxis(axisYstats2, Qt::AlignLeft);
    statsLineseries2->attachAxis(axisYstats2);

    statsChartBar2->legend()->setVisible(true);
    statsChartBar2->legend()->setAlignment(Qt::AlignBottom);

    // Create chart view for second mac

    statsChartViewBar2->setChart(statsChartBar2);
    statsChartViewBar2->setRenderHint(QPainter::Antialiasing);
};