#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QtGlobal>
#include <QDebug>
#include <db/Db_original.h>
#include <string>
#include <map>
#include "schema/helperclass.h"
#include "db/triangulation.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QProcess mqtt;
    QTimer* timer;
    QTimer* mapTimer;
    ~MainWindow();

private:
    Triangulation triang;
    int n_roots;
    float env_const;
    int measured_power;
    bool triang_started;
    map<string, Point> roots;
    Db_original* db;
    qint64 diffTick;
    
    void InsertButtonClicked(QSpinBox*, QLineEdit*, QLineEdit*, QLineEdit*, QTextEdit*);
    void CheckNRoots(QSpinBox*);
    void MqttStart() {
        QString program = "python mqtt.py";
        qDebug() << "Start python";
        this->mqtt.startDetached(program);
    }
    void DB();    

    // Tabs
    void show_map(QChartView *mapScatter, QString mapTitles, QDateTime currTime);
    void show_stats_graph(QLabel *statsLabel, int indexCombo, QChartView *statsChartViewBar1, QChartView *statsChartViewBar2QDateTime, QDateTime temp);
    void show_history_plot(QLabel *histLabel, QChartView *histChartViewBar, QDateTimeEdit *histDateEdit);

    Ui::MainWindow *ui;

public slots:
    void closing() {
        qDebug() << "Kill Process...";
        this->timer->stop();
        this->mqtt.kill();
    }
};
#endif // MAINWINDOW_H
