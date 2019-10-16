#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <QDebug>
#include <db/Db_original.h>
#include <atltime.h>
#include <ctime>
#include <regex>


void MainWindow::MqttStart(){
    QString program = "python mqtt.py";
    qDebug() << "Start python";
    this->mqtt.startDetached(program);
    //this->mqtt.
}

/*void MainWindow::DB(){
    qDebug() << "Running DB";


    db.number_of_rilevations(CTime(2019, 10, 1, 19, 00, 0).GetTime(), CTime(2019, 10, 1, 19, 30, 0).GetTime());
    db.last_positions(CTime(2019, 10, 1, 19, 01, 0).GetTime());
    db.statistics_fun(CTime(2019, 10, 1, 19, 00, 0).GetTime(), 1);
}

*/
void MainWindow::closing(){
    qDebug() << "Kill Process...";
    this->timer->stop();
    this->mqtt.kill();
}

void MainWindow::InsertButtonClicked(QSpinBox* integerSpinBox, QLineEdit *MACEdit, QLineEdit *XEdit, QLineEdit *YEdit, QTextEdit* DevicesList){
    //DevicesList->clear();
    if(this->n_roots==integerSpinBox->value()){
        QMessageBox msgbox;
        msgbox.setText("Max roots reached");
        msgbox.exec();
        return;
    }
    if(!regex_match(MACEdit->text().toStdString(),regex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"))){
        QMessageBox msgbox;
        msgbox.setText("Enter valid MAC address");
        msgbox.exec();
        return;
    }
    if(!regex_match(XEdit->text().toStdString(),regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))
            ||!regex_match(YEdit->text().toStdString(),regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))){
        QMessageBox msgbox;
        msgbox.setText("Enter valid coordinates");
        msgbox.exec();
        return;
    }
    Point point=Point(XEdit->text().toFloat(), YEdit->text().toFloat());
    pair<string,Point> pair(MACEdit->text().toStdString(),point);
    this->roots.insert(pair);
    n_roots++;
    DevicesList->append(MACEdit->text()+" "+XEdit->text()+" "+YEdit->text());

}

void MainWindow::CheckNRoots(QSpinBox* integerSpinBox){
    if(this->triang_started){
        QMessageBox msgbox;
        msgbox.setText("Can't change n. of roots while triangulation is not stopped.");
        msgbox.exec();
        integerSpinBox->setValue(this->n_roots);
        return;
    }
    if(this->n_roots>integerSpinBox->value()){
        QMessageBox msgbox;
        msgbox.setText("Can't lower the number of roots without removing them");
        msgbox.exec();
        integerSpinBox->setValue(this->n_roots);
        return;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->triang_started=false;
    this->timer = new QTimer(this);
    this->mapTimer = new QTimer(this);
    Db_original db;
    int n_sec=1;


/*
    // Start MQTT
    MqttStart();

    // Start DB
    Db_original db;
    db.triang=Triangulation();
    // Init triangulation
    // TODO - read configuration
    Point root1(0.0, 0.0), root2(0.8,0.0); //root3(0.0,5.0);
    pair<string,Point> a("30:AE:A4:1D:52:BC",root1),b("30:AE:A4:75:23:E8",root2);//,c("a",root3);
    map<string, Point> roots = { a,b};

    db.triang.initTriang(roots);

    int n_sec=1;
    this->timer->setInterval(n_sec*1000);
    connect(this->timer, &QTimer::timeout,this, []() {
        Db_original db;
        db.triang=Triangulation();
        db.loop(CTime(2019, 10, 4, 13, 30, 00).GetTime());});
    this->timer->start();
    */

    //---------------------
    // Create Main Window
    //---------------------

    ui->setupUi(this);

    // SETTINGS TAB

    this->n_roots=0;

    QGroupBox *NDevicesGroup=new QGroupBox();

    QLabel *NLabel=new QLabel("Number of devices: ");
    QSpinBox *integerSpinBox = new QSpinBox;
    integerSpinBox->setRange(2, numeric_limits<int>::max());
    integerSpinBox->setSingleStep(1);
    integerSpinBox->setValue(0);
    connect(integerSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [this, integerSpinBox](){CheckNRoots(integerSpinBox);});

    QTextEdit *DevicesList=new QTextEdit();
    //DevicesList->setText("No devices inserted yet.");

    QGridLayout *NDevicesLayout=new QGridLayout();
    NDevicesLayout->addWidget(DevicesList, 0, 0);
    NDevicesGroup->setLayout(NDevicesLayout);


    QGroupBox *InsertGroup=new QGroupBox();

    QGroupBox *MACGroup=new QGroupBox(tr("MAC"));
    QLineEdit *MACEdit=new QLineEdit();
    QGroupBox *XGroup=new QGroupBox(tr("X"));
    QLineEdit *XEdit=new QLineEdit();
    QGroupBox *YGroup=new QGroupBox(tr("Y"));
    QLineEdit *YEdit=new QLineEdit();
    QGroupBox *MPGroup=new QGroupBox(tr("Measured power"));
    QLineEdit *MPEdit=new QLineEdit();
    QGroupBox *ENGroup=new QGroupBox(tr("Enviromental constant"));
    QLineEdit *ENEdit=new QLineEdit();

    QGridLayout *MACLayout=new QGridLayout;
    MACLayout->addWidget(MACEdit, 1, 0, 1, 2);
    MACGroup->setLayout(MACLayout);
    QGridLayout *XLayout=new QGridLayout;
    XLayout->addWidget(XEdit, 1, 0, 1, 2);
    XGroup->setLayout(XLayout);
    QGridLayout *YLayout=new QGridLayout;
    YLayout->addWidget(YEdit, 1, 0, 1, 2);
    YGroup->setLayout(YLayout);
    QGridLayout *MPLayout=new QGridLayout;
    MPLayout->addWidget(MPEdit, 1, 0, 1, 2);
    MPGroup->setLayout(MPLayout);
    QGridLayout *ENLayout=new QGridLayout;
    ENLayout->addWidget(ENEdit, 1, 0, 1, 2);
    ENGroup->setLayout(ENLayout);

    QPushButton *InsertButton=new QPushButton("Insert root", this);
    connect(InsertButton, &QPushButton::released, this, [integerSpinBox, MACEdit, XEdit, YEdit, DevicesList, this](){InsertButtonClicked(integerSpinBox, MACEdit, XEdit, YEdit, DevicesList);});



    QPushButton *ChangeButton=new QPushButton("Set constants", this);
    connect(ChangeButton, &QPushButton::released, this, [ENEdit, MPEdit, DevicesList, this](){
        if(MPEdit->text().toInt()>0||
                !regex_match(MPEdit->text().toStdString(),regex("^[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)$"))){
            QMessageBox msgbox;
            msgbox.setText("Measured power can only be a negative integer.");
            msgbox.exec();
            return;
        }
        if(ENEdit->text().toFloat()<2||ENEdit->text().toFloat()>4){
            QMessageBox msgbox;
            msgbox.setText("Enviromental constant should be between 2 and 4");
            msgbox.exec();
            return;
        }
        this->measured_power=MPEdit->text().toInt();
        this->env_const=ENEdit->text().toFloat();
        DevicesList->append(QString::fromStdString("Constants changed: measured power is "+to_string(this->measured_power)+" and enviromental constant is "+to_string(this->env_const)));

    });

    QPushButton *StartButton=new QPushButton("START TRIANGULATION", this);
    connect(StartButton, &QPushButton::released, this, [this, n_sec, integerSpinBox, StartButton](){
        if(this->triang_started){
            delete timer;
            this->mqtt.kill();
            this->timer = new QTimer(this);
            this->triang_started=false;
            StartButton->setText("START TRIANGULATION");
            return;
        }
        if(this->n_roots!=integerSpinBox->value()){
            QMessageBox msgbox;
            msgbox.setText("Number of roots is not the same as declared.");
            msgbox.exec();
            return;
        }
        // Start MQTT
        MqttStart();

        // Start DB
        Db_original db;
        db.triang=Triangulation();
        // Init triangulation
        // TODO - read configuration
        //Point root1(0.0, 0.0), root2(0.8,0.0); //root3(0.0,5.0);
        //pair<string,Point> a("30:AE:A4:1D:52:BC",root1),b("30:AE:A4:75:23:E8",root2);//,c("a",root3);
        //this->roots = { a,b};

        db.triang.initTriang(this->roots, this->measured_power, this->env_const, integerSpinBox->value());

        this->triang_started=true;
        StartButton->setText("STOP TRIANGULATION");

        this->timer->setInterval(n_sec*1000);
        connect(this->timer, &QTimer::timeout,this, []() {
            Db_original db;
            db.triang=Triangulation();
            db.loop(CTime(2019, 10, 4, 13, 30, 00).GetTime());});
        this->timer->start();
    });

    QPushButton *RemoveButton=new QPushButton("Remove roots", this);
    connect(RemoveButton, &QPushButton::released, this, [DevicesList, this, StartButton](){
        DevicesList->clear();
        this->roots.clear();
        this->n_roots=0;
        if(this->triang_started){
            delete timer;
            this->mqtt.kill();
            this->timer = new QTimer(this);
            this->triang_started=false;
            StartButton->setText("START TRIANGULATION");
        }
    });

    QGridLayout *InsertLayout=new QGridLayout;
    InsertLayout->addWidget(integerSpinBox, 0, 1);
    InsertLayout->addWidget(NLabel, 0, 0);
    InsertLayout->addWidget(MACGroup, 1, 0);
    InsertLayout->addWidget(XGroup, 1, 1);
    InsertLayout->addWidget(YGroup, 1, 2);
    InsertLayout->addWidget(MPGroup, 2, 0);
    InsertLayout->addWidget(ENGroup, 2, 1);
    InsertLayout->addWidget(StartButton, 2, 2);
    InsertLayout->addWidget(InsertButton, 3, 0);
    InsertLayout->addWidget(RemoveButton, 3, 1);
    InsertLayout->addWidget(ChangeButton, 3, 2);
    InsertGroup->setLayout(InsertLayout);


    QVBoxLayout *settingsLayout=new QVBoxLayout();
    settingsLayout->addWidget(InsertGroup);
    settingsLayout->addWidget(NDevicesGroup);
    QWidget *settingsWidget = new QWidget;
    settingsWidget->setLayout(settingsLayout);


    // MAP TAB



    // Create your time series
    QScatterSeries *boardScatter = new QScatterSeries();
    boardScatter->setName("Boards");
    boardScatter->setPointLabelsVisible(true);
    boardScatter->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    boardScatter->setMarkerSize(10.0);

    *boardScatter<<QPointF(3,4);

    QScatterSeries *phoneScatter = new QScatterSeries();
    phoneScatter->setName("Phones");
    phoneScatter->setPointLabelsVisible(true);
    phoneScatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    phoneScatter->setMarkerSize(10.0);

    time_t timev;
    time(&timev);
    vector<schema_triang> vlast;

    // Usare timev invece di ctime

    vlast = db.last_positions(CTime(2019, 10, 4, 13, 30, 30).GetTime());

    for(vector<schema_triang>::iterator it=vlast.begin(); it!=vlast.end();++it){
       *phoneScatter<<QPointF(it->x,it->y);
    }


    // Configure your chart
    QChart *chartScatter = new QChart();
    chartScatter->addSeries(boardScatter);
    chartScatter->addSeries(phoneScatter);
    QValueAxis *axisYmap = new QValueAxis();
    axisYmap->setRange(0, 20);
    chartScatter->addAxis(axisYmap, Qt::AlignLeft);
    boardScatter->attachAxis(axisYmap);
    phoneScatter->attachAxis(axisYmap);
    QValueAxis *axisXmap = new QValueAxis();
    axisXmap->setRange(0, 20);
    chartScatter->addAxis(axisXmap, Qt::AlignBottom);
    boardScatter->attachAxis(axisXmap);
    phoneScatter->attachAxis(axisXmap);
    chartScatter->setTitle("Real time map of detected devices");
    chartScatter->setDropShadowEnabled(false);
    chartScatter->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    // Create your chart view
    QChartView *graphicsViewScatter = new QChartView(chartScatter);
    graphicsViewScatter->setRenderHint(QPainter::Antialiasing);



    this->mapTimer->setInterval(n_sec*1000);
    connect(this->mapTimer, &QTimer::timeout,this, [phoneScatter,boardScatter,graphicsViewScatter]() {
        Db_original db;
        time_t timev;
        time(&timev);
        vector<schema_triang> vlast;

        //Usare timev invece di ctime
        vlast = db.last_positions(CTime(2019, 10, 4, 13, 30, 00).GetTime());

        for(vector<schema_triang>::iterator it=vlast.begin(); it!=vlast.end();++it){
           *phoneScatter<<QPointF(it->x,it->y);
        }

        // Configure your chart
        QChart *chartScatter = new QChart();
        chartScatter->addSeries(boardScatter);
        chartScatter->addSeries(phoneScatter);
        QValueAxis *axisYmap = new QValueAxis();
        axisYmap->setRange(-20, 20);
        chartScatter->addAxis(axisYmap, Qt::AlignLeft);
        boardScatter->attachAxis(axisYmap);
        phoneScatter->attachAxis(axisYmap);
        QValueAxis *axisXmap = new QValueAxis();
        axisXmap->setRange(-20, 20);
        chartScatter->addAxis(axisXmap, Qt::AlignBottom);
        boardScatter->attachAxis(axisXmap);
        phoneScatter->attachAxis(axisXmap);
        chartScatter->setTitle("Real time map of detected devices");
        chartScatter->setDropShadowEnabled(false);
        chartScatter->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

        // Create your chart view
        graphicsViewScatter->setChart(chartScatter);
        graphicsViewScatter->setRenderHint(QPainter::Antialiasing);
    });
    this->mapTimer->start();





    //HISTORY TAB



    // Create time and date picker

    QDateTimeEdit *histDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    histDateEdit->setMaximumDate(QDate::currentDate());
    histDateEdit->setDisplayFormat("yyyy.MM.dd hh:mm");


    QLabel *histFormatLabel = new QLabel(tr("Pick start time"));

    QString histText = QString("Date selected: %1").arg(histDateEdit->date().toString("d/M/yyyy"));

    QLabel* histLabel = new QLabel(histText);

    // Create time series using current time
    QBarSet *set0hist = new QBarSet("Private MAC");
    QBarSet *set1hist = new QBarSet("Public MAC");


    map<string,int*> histMap;

    time_t histStart;
    time_t histEnd;

    histStart = histDateEdit->dateTime().addSecs(-1800).toTime_t();
    histEnd = histDateEdit->dateTime().toTime_t();

    histMap = db.number_of_rilevations(histStart, histEnd);

    for(map<string,int*>::iterator itMap=histMap.begin(); itMap!=histMap.end();++itMap){
       *set0hist << itMap->second[0];
       *set1hist << itMap->second[1];
    }


    QStackedBarSeries *histSeriesBar = new QStackedBarSeries();
    histSeriesBar->append(set0hist);
    histSeriesBar->append(set1hist);

    // Configure the chart using current time
    QChart *histChartBar = new QChart();
    histChartBar->addSeries(histSeriesBar);
    histChartBar->setTitle("Number of devices tracked");
    histChartBar->setAnimationOptions(QChart::SeriesAnimations);

    QStringList categories;
    categories << histDateEdit->time().addSecs(-1500).toString("hh:mm") << histDateEdit->time().addSecs(-1200).toString("hh:mm") << histDateEdit->time().addSecs(-900).toString("hh:mm") << histDateEdit->time().addSecs(-600).toString("hh:mm") << histDateEdit->time().addSecs(-300).toString("hh:mm") << histDateEdit->time().toString("hh:mm");
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    histChartBar->addAxis(axisX, Qt::AlignBottom);
    histSeriesBar->attachAxis(axisX);
    QValueAxis *axisY = new QValueAxis();
    histChartBar->addAxis(axisY, Qt::AlignLeft);
    histSeriesBar->attachAxis(axisY);

    histChartBar->legend()->setVisible(true);
    histChartBar->legend()->setAlignment(Qt::AlignBottom);

    // Create chart view
    QChartView *histChartViewBar = new QChartView(histChartBar);
    histChartViewBar->setRenderHint(QPainter::Antialiasing);


    // Update chart with selected time
    connect(histDateEdit, &QDateTimeEdit::dateTimeChanged, this, [histLabel, histChartViewBar] (QDateTime temp){

        Db_original db;

        QString dateText = QString("Date selected: %1").arg(temp.toString("d/M/yyyy"));
        histLabel->setText(dateText);
        QBarSet *set0hist = new QBarSet("Private MAC");
        QBarSet *set1hist = new QBarSet("Public MAC");

        map<string,int*> histMap;

        time_t histStart;
        time_t histEnd;

        histStart = temp.toTime_t();
        histEnd = temp.addSecs(1800).toTime_t();

        histMap = db.number_of_rilevations(histStart, histEnd);

        for(map<string,int*>::iterator itMap=histMap.begin(); itMap!=histMap.end();++itMap){
           *set0hist << itMap->second[0];
           *set1hist << itMap->second[1];
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


    });

    QVBoxLayout *histLayout = new QVBoxLayout;
    histLayout->addWidget(histFormatLabel);
    histLayout->addWidget(histDateEdit);
    histLayout->addWidget(histChartViewBar);
    histLayout->addWidget(histLabel);
    QWidget *histWidget = new QWidget;
    histWidget->setLayout(histLayout);



    // STATS TAB



    // Create time and date picker

    QDateTimeEdit *statsDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    statsDateEdit->setMaximumDate(QDate::currentDate());
    statsDateEdit->setDisplayFormat("yyyy.MM.dd hh:mm");

    QLabel *statsEndLabel = new QLabel(tr("Pick finish time"));

    QLabel *statsFormatLabel = new QLabel(tr("Pick frequency period"));

    QString statsText = QString("Date selected: %1").arg(statsDateEdit->date().toString("d/M/yyyy"));

    QLabel* statsLabel = new QLabel(statsText);

    QComboBox *statsComboBox = new QComboBox;
        statsComboBox->addItem(tr("Last 2 hours"));
        statsComboBox->addItem(tr("Last day"));
        statsComboBox->addItem(tr("Last week"));

    // Create time series using current time
    QLineSeries *statsLineseries = new QLineSeries();
        statsLineseries->setName("Number of devices tracked");
        statsLineseries->append(QPoint(0, 4));
        statsLineseries->append(QPoint(1, 15));
        statsLineseries->append(QPoint(2, 20));
        statsLineseries->append(QPoint(3, 4));
        statsLineseries->append(QPoint(4, 12));
        statsLineseries->append(QPoint(5, 17));


    // Configure the chart using current time
    QChart *statsChartBar = new QChart();
    statsChartBar->addSeries(statsLineseries);
    statsChartBar->setTitle("Number of devices tracked");
    statsChartBar->setAnimationOptions(QChart::SeriesAnimations);

    QStringList statsCategories;
    statsCategories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QBarCategoryAxis *axisXstats = new QBarCategoryAxis();
    axisXstats->append(statsCategories);
    statsChartBar->addAxis(axisXstats, Qt::AlignBottom);
    statsLineseries->attachAxis(axisXstats);
    axisXstats->setRange(QString("Jan"), QString("Jun"));
    QValueAxis *axisYstats = new QValueAxis();
    statsChartBar->addAxis(axisYstats, Qt::AlignLeft);
    statsLineseries->attachAxis(axisYstats);

    statsChartBar->legend()->setVisible(true);
    statsChartBar->legend()->setAlignment(Qt::AlignBottom);

    // Create chart view
    QChartView *statsChartViewBar = new QChartView(statsChartBar);
    statsChartViewBar->setRenderHint(QPainter::Antialiasing);


    // Update chart with selected time
    connect(statsDateEdit, &QDateTimeEdit::dateTimeChanged, this, [statsLabel, statsChartViewBar] (QDateTime temp){
        QString dateText = QString("Date selected: %1").arg(temp.toString("d/M/yyyy"));
        statsLabel->setText(dateText);
        QLineSeries *statsLineseries = new QLineSeries();
            statsLineseries->setName("Number of devices tracked");
            statsLineseries->append(QPoint(0, 4));
            statsLineseries->append(QPoint(1, 15));
            statsLineseries->append(QPoint(2, 20));
            statsLineseries->append(QPoint(3, 4));
            statsLineseries->append(QPoint(4, 12));
            statsLineseries->append(QPoint(5, 17));

            // Configure the chart using current time
            QChart *statsChartBar = new QChart();
            statsChartBar->addSeries(statsLineseries);
            statsChartBar->setTitle("Number of devices tracked");
            statsChartBar->setAnimationOptions(QChart::SeriesAnimations);

            QStringList statsCategories;
            statsCategories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
            QBarCategoryAxis *axisXstats = new QBarCategoryAxis();
            axisXstats->append(statsCategories);
            statsChartBar->addAxis(axisXstats, Qt::AlignBottom);
            statsLineseries->attachAxis(axisXstats);
            axisXstats->setRange(QString("Jan"), QString("Jun"));
            QValueAxis *axisYstats = new QValueAxis();
            statsChartBar->addAxis(axisYstats, Qt::AlignLeft);
            statsLineseries->attachAxis(axisYstats);

            statsChartBar->legend()->setVisible(true);
            statsChartBar->legend()->setAlignment(Qt::AlignBottom);

            statsChartViewBar->setChart(statsChartBar);
            statsChartViewBar->setRenderHint(QPainter::Antialiasing);



    });


    // Update chart with selected frequency
    connect(statsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [statsChartViewBar] (){
        QLineSeries *statsLineseries = new QLineSeries();
            statsLineseries->setName("Number of devices tracked");
            statsLineseries->append(QPoint(0, 4));
            statsLineseries->append(QPoint(1, 15));
            statsLineseries->append(QPoint(2, 20));
            statsLineseries->append(QPoint(3, 4));
            statsLineseries->append(QPoint(4, 12));
            statsLineseries->append(QPoint(5, 17));

            // Configure the chart using current time
            QChart *statsChartBar = new QChart();
            statsChartBar->addSeries(statsLineseries);
            statsChartBar->setTitle("Number of devices tracked");
            statsChartBar->setAnimationOptions(QChart::SeriesAnimations);

            QStringList statsCategories;
            statsCategories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
            QBarCategoryAxis *axisXstats = new QBarCategoryAxis();
            axisXstats->append(statsCategories);
            statsChartBar->addAxis(axisXstats, Qt::AlignBottom);
            statsLineseries->attachAxis(axisXstats);
            axisXstats->setRange(QString("Jan"), QString("Jun"));
            QValueAxis *axisYstats = new QValueAxis();
            statsChartBar->addAxis(axisYstats, Qt::AlignLeft);
            statsLineseries->attachAxis(axisYstats);

            statsChartBar->legend()->setVisible(true);
            statsChartBar->legend()->setAlignment(Qt::AlignBottom);

            statsChartViewBar->setChart(statsChartBar);
            statsChartViewBar->setRenderHint(QPainter::Antialiasing);



    });

    QVBoxLayout *statsLayout = new QVBoxLayout;
    statsLayout->addWidget(statsEndLabel);
    statsLayout->addWidget(statsDateEdit);
    statsLayout->addWidget(statsFormatLabel);
    statsLayout->addWidget(statsComboBox);
    statsLayout->addWidget(statsChartViewBar);
    statsLayout->addWidget(statsLabel);
    QWidget *statsWidget = new QWidget;
    statsWidget->setLayout(statsLayout);




    // LOAD CREATED TABS



    auto tw = new QTabWidget (this);
    tw->addTab(settingsWidget, "Settings");
    tw->addTab(graphicsViewScatter, "Map");
    tw->addTab(histWidget, "History");
    tw->addTab(statsWidget, "Stats");

    setCentralWidget(tw);

    connect(tw, QOverload<int>::of(&QTabWidget::currentChanged), this, [graphicsViewScatter] (int i) {
        if(i==0){
            graphicsViewScatter->update();
        }

    });


}

MainWindow::~MainWindow()
{
    delete timer;
    delete ui;
}

