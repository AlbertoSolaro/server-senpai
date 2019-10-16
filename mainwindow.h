#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <string>
#include <map>
#include "schema/helperclass.h"
#include "db/triangulation.h"

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

public slots:
    void closing();

private:
    Triangulation triang;
    int n_roots;
    float env_const;
    int measured_power;
    bool triang_started;
    map<string, Point> roots;
    void InsertButtonClicked(QSpinBox*, QLineEdit*, QLineEdit*, QLineEdit*, QTextEdit*);
    void CheckNRoots(QSpinBox*);
    void MqttStart();
    void DB();    
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
