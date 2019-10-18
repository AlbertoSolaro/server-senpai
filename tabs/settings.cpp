#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <QDebug>
#include <db/Db_original.h>
#include <atltime.h>
#include <ctime>
#include <regex>
#include <QtGlobal>

void MainWindow::InsertButtonClicked(QSpinBox *integerSpinBox, QLineEdit *MACEdit, QLineEdit *XEdit, QLineEdit *YEdit, QTextEdit *DevicesList) {
    //DevicesList->clear();
    if (this->n_roots == integerSpinBox->value()) {
        QMessageBox msgbox;
        msgbox.setText("Max roots reached");
        msgbox.exec();
        return;
    }
    if (!regex_match(MACEdit->text().toStdString(), regex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"))) {
        QMessageBox msgbox;
        msgbox.setText("Enter valid MAC address");
        msgbox.exec();
        return;
    }
    if (!regex_match(XEdit->text().toStdString(), regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$")) || !regex_match(YEdit->text().toStdString(), regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))) {
        QMessageBox msgbox;
        msgbox.setText("Enter valid coordinates");
        msgbox.exec();
        return;
    }
    Point point = Point(XEdit->text().toFloat(), YEdit->text().toFloat());
    pair<string, Point> pair(MACEdit->text().toStdString(), point);
    this->roots.insert(pair);
    n_roots++;
    DevicesList->append(MACEdit->text() + " " + XEdit->text() + " " + YEdit->text());
}

void MainWindow::CheckNRoots(QSpinBox *integerSpinBox) {
    if (this->triang_started) {
        QMessageBox msgbox;
        msgbox.setText("Can't change n. of roots while triangulation is not stopped.");
        msgbox.exec();
        integerSpinBox->setValue(this->n_roots);
        return;
    }
    if (this->n_roots > integerSpinBox->value()) {
        QMessageBox msgbox;
        msgbox.setText("Can't lower the number of roots without removing them");
        msgbox.exec();
        integerSpinBox->setValue(this->n_roots);
        return;
    }
}