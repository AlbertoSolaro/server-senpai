#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "schema/helperclass.h"
#include <atltime.h>
#include <ctime>
#include <regex>
#include <QtGlobal>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->triang_started=false;
    this->timer = new QTimer(this);
    this->mapTimer = new QTimer(this);
    this->db = new Db_original();

    //---------------------
    // Create Main Window
    //---------------------

    ui->setupUi(this);

    // SETTINGS TAB

    this->n_roots=3;
    Point a(0.0,2.5), b(3.8,0.0), c(0.0,0.0);
    this->roots.insert(pair<string,Point>("30:AE:A4:1D:52:BC",a));
    this->roots.insert(pair<string,Point>("30:AE:A4:75:23:E8",b));
    this->roots.insert(pair<string,Point>("A4:CF:12:55:88:F0",c));

    QGroupBox *NDevicesGroup=new QGroupBox();

    QLabel *NLabel=new QLabel("Number of devices: ");
    QSpinBox *integerSpinBox = new QSpinBox;
    integerSpinBox->setRange(2, numeric_limits<int>::max());
    integerSpinBox->setSingleStep(1);
    integerSpinBox->setValue(3);
    connect(integerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, integerSpinBox](){CheckNRoots(integerSpinBox);});

    QTextEdit *DevicesList=new QTextEdit();
    //DevicesList->setText("No devices inserted yet.");

    QGridLayout *NDevicesLayout=new QGridLayout();
    NDevicesLayout->addWidget(DevicesList, 0, 0);
    NDevicesGroup->setLayout(NDevicesLayout);

    for(map<string,Point>::iterator it=this->roots.begin(); it!=this->roots.end(); it++){
        DevicesList->append(QString::fromStdString(it->first+" "+to_string(it->second.x)+" "+to_string(it->second.y)));
    }


    QGroupBox *InsertGroup=new QGroupBox();

    QGroupBox *MACGroup=new QGroupBox(tr("MAC"));
    QLineEdit *MACEdit=new QLineEdit();
    QGroupBox *XGroup=new QGroupBox(tr("X"));
    QLineEdit *XEdit=new QLineEdit();
    QGroupBox *YGroup=new QGroupBox(tr("Y"));
    QLineEdit *YEdit=new QLineEdit();
    QGroupBox *MPGroup=new QGroupBox(tr("Measured power"));
    QLineEdit *MPEdit=new QLineEdit(tr("-50"));
    QGroupBox *ENGroup=new QGroupBox(tr("Enviromental constant"));
    QLineEdit *ENEdit=new QLineEdit(tr("2.5"));

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
        qDebug()<<this->measured_power;
        qDebug()<<this->env_const;

        DevicesList->append(QString::fromStdString("Constants changed: measured power is "+to_string(this->measured_power)+" and enviromental constant is "+to_string(this->env_const)));

    });

    QPushButton *StartButton=new QPushButton("START TRIANGULATION", this);
    connect(StartButton, &QPushButton::released, this, [this, integerSpinBox, StartButton,MPEdit,ENEdit](){
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
        this->measured_power=MPEdit->text().toInt();
        this->env_const=ENEdit->text().toFloat();

        // Start MQTT
        MqttStart();

        // Start DB
        db->triang=Triangulation();
        // Init triangulation
        // TODO - read configuration
        //Point root1(0.0, 2.5), root2(3.8,0.0); //root3(0.0,0.0);
        //pair<string,Point> a("30:AE:A4:1D:52:BC",root1),b("30:AE:A4:75:23:E8",root2);//,c("A4:CF:12:55:88:F0",root3);
        //this->roots = { a,b};

        db->triang.initTriang(this->roots, this->measured_power, this->env_const, integerSpinBox->value());

        this->triang_started=true;
        StartButton->setText("STOP TRIANGULATION");
        int n_sec_history=30;
        this->timer->setInterval(n_sec_history*1000);
        connect(this->timer, &QTimer::timeout,this, [db]() {
          /*  time_t timev;
            time(&timev);
            db->loop1(timev);
        */
        db->loop1(CTime(2019,10,16,00,55,00).GetTime());
        });
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
    InsertLayout->addWidget(StartButton, 3, 2);
    InsertLayout->addWidget(InsertButton, 3, 0);
    InsertLayout->addWidget(RemoveButton, 3, 1);

    InsertGroup->setLayout(InsertLayout);


    QVBoxLayout *settingsLayout=new QVBoxLayout();
    settingsLayout->addWidget(InsertGroup);
    settingsLayout->addWidget(NDevicesGroup);
    QWidget *settingsWidget = new QWidget;
    settingsWidget->setLayout(settingsLayout);


    // MAP TAB


    QChartView *mapScatter = new QChartView();
    string mapTitle = "Real time map of detected devices";

    show_map(this, mapScatter, mapTitle);

    int n_sec_last=10;
    this->mapTimer->setInterval(n_sec_last*1000);

    connect(this->mapTimer, &QTimer::timeout,this, [&,mapTitle,mapScatter,db]() {

        show_map(this, mapScatter, mapTitle);

    });
    this->mapTimer->start();





    //HISTORY TAB



    // Create time and date picker

    QDateTimeEdit *histDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    histDateEdit->setMaximumDate(QDate::currentDate());
    histDateEdit->setDisplayFormat("yyyy.MM.dd hh:mm");


    QPushButton * update_button = new QPushButton("Update", this);
    QLabel *histFormatLabel = new QLabel(tr("Pick start time"));


    QString histText = QString("Date selected: %1").arg(histDateEdit->date().toString("d/M/yyyy"));

    QLabel* histLabel = new QLabel(histText);

    // Create time series using current time
    QBarSet *set0hist = new QBarSet("Private MAC");
    QBarSet *set1hist = new QBarSet("Public MAC");



    map<string,num_ril> histMap;

    time_t histStart;
    time_t histEnd;

    histStart = histDateEdit->dateTime().addSecs(-1800).toTime_t();
    histEnd = histDateEdit->dateTime().toTime_t();
    QDateTime t = QDateTime::fromTime_t(histStart);
    histDateEdit->setDateTime(t);
    histMap = db->number_of_rilevations(histStart, histEnd);

    for(map<string,num_ril>::iterator itMap=histMap.begin(); itMap!=histMap.end();++itMap){
       *set0hist << itMap->second.n_priv;
       *set1hist << itMap->second.n_pub;
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


    // Update chart with enter press
    connect(histDateEdit, &QAbstractSpinBox::editingFinished, this, [histLabel, histChartViewBar,histDateEdit,db] (){
        show_history_plot(histLabel, histChartViewBar,histDateEdit);
    });

    // Update chart with update function
    connect(update_button, &QPushButton::released, this, [histLabel, histChartViewBar,histDateEdit,db] (){

        show_history_plot(histLabel, histChartViewBar,histDateEdit);
    });

    QHBoxLayout *changeDataLayout= new QHBoxLayout;
    QVBoxLayout *histLayout = new QVBoxLayout;
    histLayout->addWidget(histFormatLabel);
    changeDataLayout->addWidget(histDateEdit,5);
    changeDataLayout->addWidget(update_button,Qt::AlignRight);
    histLayout->addLayout(changeDataLayout);
    histLayout->addWidget(histChartViewBar);
    histLayout->addWidget(histLabel);
    QWidget *histWidget = new QWidget;
    histWidget->setLayout(histLayout);



    // STATS TAB



    // Create time and date picker

    QDateTimeEdit *statsDateEdit = new QDateTimeEdit(QDateTime::currentDateTime());
    statsDateEdit->setMaximumDate(QDate::currentDate());
    statsDateEdit->setDisplayFormat("yyyy.MM.dd hh:mm");


    QLabel *statsEndLabel = new QLabel(tr("Pick start time"));

    QPushButton *stats_update_button = new QPushButton("Update", this);

    QLabel *statsFormatLabel = new QLabel(tr("Pick frequency period"));

    QString statsText = QString("Date selected: %1").arg(statsDateEdit->date().toString("d/M/yyyy"));

    QLabel* statsLabel = new QLabel(statsText);

    QComboBox *statsComboBox = new QComboBox;
        statsComboBox->addItem(tr("Next 2 hours"));
        statsComboBox->addItem(tr("Next 6 hours"));
        statsComboBox->addItem(tr("Next day"));
        statsComboBox->addItem(tr("Next week"));

    best_k_mac bestStat;
    time_t statsStart;

    statsStart = statsDateEdit->dateTime().addSecs(-7200).toTime_t();
   // QDateTime r = QDateTime::fromTime_t(statsStart);
   // statsDateEdit->setDateTime(r);

    bestStat = db->statistics_fun(statsStart,1);

    QString mac1Text;
    QString mac2Text;

    if(bestStat.best_mac[0].first=="0"){
        mac1Text = QString("No device detected");
    }
    else {
        mac1Text = QString("MAC: %1").arg(QString::fromStdString(bestStat.best_mac[0].first));
    }

    if(bestStat.best_mac[1].first=="0"){
        mac2Text = QString("No device detected");
    }
    else {
        mac2Text = QString("MAC: %1").arg(QString::fromStdString(bestStat.best_mac[1].first));
    }


    // Create time series using current time for first mac
    QLineSeries *statsLineseries1 = new QLineSeries();
        statsLineseries1->setName("Number of times tracked");
        for(int i = 0; i<6; i++) {
            statsLineseries1->append(QPoint(i, bestStat.best_mac[0].second[i]));
        }


    // Configure the chart using current time for first mac
    QChart *statsChartBar1 = new QChart();
    statsChartBar1->addSeries(statsLineseries1);
    statsChartBar1->setTitle(mac1Text);
    statsChartBar1->setAnimationOptions(QChart::SeriesAnimations);

    QStringList statsCategories;
    statsCategories << statsDateEdit->dateTime().addSecs(-6000).toString("hh:mm") << statsDateEdit->dateTime().addSecs(-4800).toString("hh:mm") << statsDateEdit->dateTime().addSecs(-3600).toString("hh:mm") << statsDateEdit->dateTime().addSecs(-2400).toString("hh:mm") << statsDateEdit->dateTime().addSecs(-1200).toString("hh:mm") << statsDateEdit->dateTime().toString("hh:mm");
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
    QChartView *statsChartViewBar1 = new QChartView(statsChartBar1);
    statsChartViewBar1->setRenderHint(QPainter::Antialiasing);

    // Create time series using current time for second mac
    QLineSeries *statsLineseries2 = new QLineSeries();
        statsLineseries2->setName("Number of times tracked");
        for(int i = 0; i<6; i++) {
            statsLineseries2->append(QPoint(i, bestStat.best_mac[1].second[i]));
        }


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
    QChartView *statsChartViewBar2 = new QChartView(statsChartBar2);
    statsChartViewBar2->setRenderHint(QPainter::Antialiasing);



    // Update chart with selected time
    connect(stats_update_button, &QPushButton::released, this, [statsLabel, statsDateEdit, statsComboBox, statsChartViewBar1, statsChartViewBar2, db] (){

        QDateTime temp=statsDateEdit->dateTime();
        int indexCombo=statsComboBox->currentIndex();
        show_stats_graph (statsLabel, indexCombo, statsChartViewBar1, statsChartViewBar2, temp);

    });

    // Update chart with enter press
    connect(statsDateEdit, &QAbstractSpinBox::editingFinished, this, [statsLabel, statsDateEdit, statsComboBox, statsChartViewBar1, statsChartViewBar2, db] (){

        QDateTime temp=statsDateEdit->dateTime();
        int indexCombo=statsComboBox->currentIndex();
        show_stats_graph (statsLabel, indexCombo, statsChartViewBar1, statsChartViewBar2, temp);

    });

    // Update chart with selected frequency
    connect(statsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [statsLabel, statsDateEdit, statsChartViewBar1, statsChartViewBar2, db] (int indexCombo){

        QDateTime temp=statsDateEdit->dateTime();
        show_stats_graph (statsLabel, indexCombo, statsChartViewBar1, statsChartViewBar2, temp);

    });

    QVBoxLayout *statsLayout = new QVBoxLayout;
    QHBoxLayout *changeStatsLayout= new QHBoxLayout;
    statsLayout->addWidget(statsEndLabel);
    changeStatsLayout->addWidget(statsDateEdit,5);
    changeStatsLayout->addWidget(stats_update_button,Qt::AlignRight);
    statsLayout->addLayout(changeStatsLayout);
    statsLayout->addWidget(statsFormatLabel);
    statsLayout->addWidget(statsComboBox);
    statsLayout->addWidget(statsChartViewBar1);
    statsLayout->addWidget(statsChartViewBar2);
    statsLayout->addWidget(statsLabel);
    QWidget *statsWidget = new QWidget;
    statsWidget->setLayout(statsLayout);



    // TIMELAPSE TAB



    QVBoxLayout *timeLayout = new QVBoxLayout;
    QWidget *timeWidget = new QWidget;
    timeWidget->setLayout(timeLayout);




    // LOAD CREATED TABS



    auto tw = new QTabWidget (this);
    tw->addTab(settingsWidget, "Settings");
    tw->addTab(mapScatter, "Map");
    tw->addTab(histWidget, "History");
    tw->addTab(statsWidget, "Stats");
    tw->addTab(timeWidget, "Time lapse");


    setCentralWidget(tw);


    /*connect(tw, QOverload<int>::of(&QTabWidget::currentChanged), this, [graphicsViewScatter] (int i) {

        if(i==0){
            graphicsViewScatter->update();
        }


    });*/



}

MainWindow::~MainWindow() {
    delete timer;
    delete mapTimer;
    delete db;
    delete ui;
}

