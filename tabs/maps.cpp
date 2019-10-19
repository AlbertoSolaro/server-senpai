#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <QDebug>
#include <db/Db_original.h>
#include <atltime.h>
#include <ctime>
#include <regex>
#include <QtGlobal>

void MainWindow::show_map(QChartView *mapScatter, QString mapTitle, QDateTime currTime,int n_last_sec) {

    time_t timev;
    time(&timev);
    qDebug() << "Start map function.Time: "<<timev;

    vector<QScatterSeries *> vSeries;
    vector<QScatterSeries *> vBoards;
    float xMax = 0, xMin = 0;
    for (map<string, Point>::iterator it2 = roots.begin(); it2 != roots.end(); ++it2) {
        QScatterSeries *boardScatter = new QScatterSeries();
        boardScatter->setPointLabelsVisible(false);
        MainWindow::connect(boardScatter, &QXYSeries::hovered, this, [boardScatter](const QPointF &waste, bool check) {
            if (check == true)
                boardScatter->setPointLabelsVisible(true);
            else
                boardScatter->setPointLabelsVisible(false);
        });
        boardScatter->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
        boardScatter->setMarkerSize(20.0);
        boardScatter->setColor("blue");
        QString boardLabel = it2->first.c_str();

        boardLabel.append("\n test");
        boardScatter->setPointLabelsFormat(boardLabel);
        *boardScatter << QPointF(it2->second.x, it2->second.y);
        if (xMax < it2->second.x)
            xMax = it2->second.x;
        if (xMax < it2->second.y)
            xMax = it2->second.y;
        if (xMin > it2->second.x)
            xMin = it2->second.x;
        if (xMin > it2->second.y)
            xMin = it2->second.y;
        vBoards.push_back(boardScatter);
    }

    //time(&timev);
    vector<schema_triang> vlast;


    //vlast = db->last_positions(timev);
    time(&timev);
    qDebug() << "Finish last_positions.Time: "<<timev;
    vlast = db->last_positions(currTime.toTime_t(),n_last_sec);

    for (vector<schema_triang>::iterator it = vlast.begin(); it != vlast.end(); ++it) {
        QScatterSeries *phoneScatter = new QScatterSeries();
        phoneScatter->setPointLabelsVisible(false);
        MainWindow::connect(phoneScatter, &QXYSeries::hovered, this, [phoneScatter](const QPointF &waste, bool check) {
            if (check == true)
                phoneScatter->setPointLabelsVisible(true);
            else
                phoneScatter->setPointLabelsVisible(false);
        });
        phoneScatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        phoneScatter->setMarkerSize(10.0);
        phoneScatter->setColor("green");
        phoneScatter->setPointLabelsFormat(it->MAC);
        *phoneScatter << QPointF(it->x, it->y);
       /* if (xMax < it->x)
            xMax = it->x;
        if (xMax < it->y)
            xMax = it->y;
        if (xMin > it->x)
            xMin = it->x;
        if (xMin > it->y)
            xMin = it->y;*/
        vSeries.push_back(phoneScatter);
    }

    // Configure your chart
    QChart *chartScatter = new QChart();
    QValueAxis *axisYmap = new QValueAxis();
    xMax += 2;
    xMin -= 2;
    axisYmap->setRange(xMin, xMax);

    chartScatter->addAxis(axisYmap, Qt::AlignLeft);
    QValueAxis *axisXmap = new QValueAxis();
    axisXmap->setRange(xMin, xMax);

    chartScatter->addAxis(axisXmap, Qt::AlignBottom);

    for (int i = 0; i < vBoards.size(); i++) {
        chartScatter->addSeries(vBoards.at(i));
        vBoards.at(i)->attachAxis(axisYmap);
        vBoards.at(i)->attachAxis(axisXmap);
    }

    for (int i = 0; i < vSeries.size(); i++) {
        chartScatter->addSeries(vSeries.at(i));
        vSeries.at(i)->attachAxis(axisYmap);
        vSeries.at(i)->attachAxis(axisXmap);
    }

    chartScatter->setTitle(mapTitle);
    chartScatter->setDropShadowEnabled(false);
    chartScatter->legend()->setVisible(false);

    // Create your chart view
    mapScatter->setChart(chartScatter);
    mapScatter->setRenderHint(QPainter::Antialiasing);

    time(&timev);
    qDebug() << "Finish show map.Time: "<<timev;
};

void MainWindow::show_map(QChartView *mapScatter, QString mapTitle) {

    vector<QScatterSeries *> vSeries;
    vector<QScatterSeries *> vBoards;
    float xMax = 0, xMin = 0;

    //time(&timev);
    vector<schema_triang> vlast;



    for (vector<schema_triang>::iterator it = vlast.begin(); it != vlast.end(); ++it) {
        QScatterSeries *phoneScatter = new QScatterSeries();
        phoneScatter->setPointLabelsVisible(false);
        MainWindow::connect(phoneScatter, &QXYSeries::hovered, this, [phoneScatter](const QPointF &waste, bool check) {
            if (check == true)
                phoneScatter->setPointLabelsVisible(true);
            else
                phoneScatter->setPointLabelsVisible(false);
        });
        phoneScatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        phoneScatter->setMarkerSize(10.0);
        phoneScatter->setColor("green");
        phoneScatter->setPointLabelsFormat(it->MAC);
        *phoneScatter << QPointF(it->x, it->y);
        if (xMax < it->x)
            xMax = it->x;
        if (xMax < it->y)
            xMax = it->y;
        if (xMin > it->x)
            xMin = it->x;
        if (xMin > it->y)
            xMin = it->y;
        vSeries.push_back(phoneScatter);
    }

    // Configure your chart
    QChart *chartScatter = new QChart();
    QValueAxis *axisYmap = new QValueAxis();
    xMax += 2;
    xMin -= 2;
    axisYmap->setRange(xMin, xMax);

    chartScatter->addAxis(axisYmap, Qt::AlignLeft);
    QValueAxis *axisXmap = new QValueAxis();
    axisXmap->setRange(xMin, xMax);

    chartScatter->addAxis(axisXmap, Qt::AlignBottom);

    for (int i = 0; i < vBoards.size(); i++) {
        chartScatter->addSeries(vBoards.at(i));
        vBoards.at(i)->attachAxis(axisYmap);
        vBoards.at(i)->attachAxis(axisXmap);
    }

    for (int i = 0; i < vSeries.size(); i++) {
        chartScatter->addSeries(vSeries.at(i));
        vSeries.at(i)->attachAxis(axisYmap);
        vSeries.at(i)->attachAxis(axisXmap);
    }

    chartScatter->setTitle(mapTitle);
    chartScatter->setDropShadowEnabled(false);
    chartScatter->legend()->setVisible(false);

    // Create your chart view
    mapScatter->setChart(chartScatter);
    mapScatter->setRenderHint(QPainter::Antialiasing);


};
