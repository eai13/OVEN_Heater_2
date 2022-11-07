#include "fastheat.h"
#include "ui_fastheat.h"
#include <QDoubleValidator>
#include "qglobal.h"
#include <cmath>

void FastHeat::GUISetEnabled(GUI_ENABLE_STATE state){
    switch(state){
    case(GUI_ENABLE_STATE::ENB_STOP):{
        ui->lineedit_setpoint->setEnabled(true);
        ui->pushbutton_run->setEnabled(true);
        ui->pushbutton_stop->setEnabled(false);
        this->SetRunningLabel(false);
        break;
    }
    case(GUI_ENABLE_STATE::ENB_RUN):{
        ui->lineedit_setpoint->setEnabled(false);
        ui->pushbutton_run->setEnabled(false);
        ui->pushbutton_stop->setEnabled(true);
        this->SetRunningLabel(true);
        ui->widget_realplot->graph(0)->data()->clear();
        ui->widget_realplot->rescaleAxes();
        ui->widget_realplot->xAxis->setRangeLower(0);
        ui->widget_realplot->yAxis->setRangeLower(0);
        ui->widget_realplot->replot();
        break;
    }
    default:{
        break;
    }
    }
}

FastHeat::FastHeat(QWidget *parent) : QWidget(parent), ui(new Ui::FastHeat){
    ui->setupUi(this);

    ui->lineedit_setpoint->setValidator(new QDoubleValidator(0, 1000, 2));

    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_STOP);

    this->plot_menu = new QMenu;
    this->plot_menu->addAction("Save Image", this, &FastHeat::slSaveImage);
    this->plot_menu->addAction("Save Data", this, &FastHeat::slSaveData);
    this->plot_menu->addAction("Save All", this, &FastHeat::slSaveAll);
    ui->widget_realplot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->pushbutton_run, &QPushButton::released, this, &FastHeat::slRun);
    connect(ui->pushbutton_stop, &QPushButton::released, this, &FastHeat::slStop);
    connect(ui->widget_realplot, &QWidget::customContextMenuRequested, this, &FastHeat::slShowPlotMenu);

    ui->widget_realplot->xAxis->setLabel("Time, s");
    ui->widget_realplot->yAxis->setLabel("Temperature, C");
    ui->widget_realplot->addGraph()->setPen(QPen(Qt::red));
    ui->widget_realplot->rescaleAxes();
    ui->widget_realplot->xAxis->setRangeLower(0);
    ui->widget_realplot->yAxis->setRangeLower(0);

    this->run_timer = new QTimer;
    connect(this->run_timer, &QTimer::timeout, this, &FastHeat::slRunTimerProcess);

    this->global_time = new QTime;
}

FastHeat::~FastHeat(void){
    delete this->plot_menu;
    delete this->run_timer;
    delete this->global_time;
    delete ui;
}

void FastHeat::SetRelayLabel(bool state){
    if (state)
        ui->label_relay->setStyleSheet("QLabel { background-color : rgb(0, 255, 0); color : black; }");
    else
        ui->label_relay->setStyleSheet("QLabel { background-color : rgb(255, 0, 0); color : black; }");
}

void FastHeat::SetRunningLabel(bool state){
    if (state)
        ui->label_isrunning->setStyleSheet("QLabel { background-color : rgb(0, 255, 0); color : black; }");
    else
        ui->label_isrunning->setStyleSheet("QLabel { background-color : rgb(255, 0, 0); color : black; }");
}

void FastHeat::slRun(void){
    if (ui->lineedit_setpoint->text().isEmpty()) return;
    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_RUN);

    ui->widget_realplot->graph(0)->data()->clear();
    ui->widget_realplot->rescaleAxes();
    ui->widget_realplot->xAxis->setRangeLower(0);
    ui->widget_realplot->yAxis->setRangeLower(0);
    ui->widget_realplot->replot();

    emit this->siSendEnable(1);
    QString sp = ui->lineedit_setpoint->text();
    for (auto iter = sp.begin(); iter != sp.end(); iter++)
        if (*iter == ',') *iter = '.';
    emit this->siSendSetPoint(sp.toFloat());

    this->run_timer->start(100);
    this->global_time->restart();
}

void FastHeat::slStop(void){
    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_STOP);

    emit this->siSendEnable(0);

    this->run_timer->stop();
}

void FastHeat::slRunTimerProcess(void){
    QString temp_txt = ui->lineedit_setpoint->text();
    for (auto iter = temp_txt.begin(); iter != temp_txt.end(); iter++)
        if (*iter == ',') *iter = '.';
    ui->widget_realplot->graph(0)->addData((float)(this->global_time->elapsed()) / float(1000), this->temperature);
    ui->widget_realplot->rescaleAxes();
    ui->widget_realplot->xAxis->setRangeLower(0);
    ui->widget_realplot->yAxis->setRangeLower(0);
    ui->widget_realplot->replot();
    float setpoint_temp = temp_txt.toFloat();
    if (this->is_at_plane){
        if (std::abs(temperature - setpoint_temp) > 3){
            emit this->siSendPID(PID_NUMBER::PID_PI_PLAIN);
        }
        else{
            emit this->siSendPID(PID_NUMBER::PID_P_PLAIN);
        }
    }
    else{
        if (std::abs(temperature - setpoint_temp) < 5){
            this->is_at_plane = true;
            emit this->siSendPID(PID_NUMBER::PID_P_PLAIN);
        }
        else{
            this->is_at_plane = false;
            emit this->siSendPID(PID_NUMBER::PID_RAMP);
        }
    }
}

void FastHeat::slSaveImage(void){
    QString filter;
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Plot Image To...",
                ".",
                "PNG (*.png);;JPG (*.jpg);;BMP (*.bmp)",
                &filter);
    if (filename.isEmpty()) return;

    if (filter == "PNG (*.png)"){
        ui->widget_realplot->savePng(filename, 1920, 1080);
    }
    else if (filter == "JPG (*.jpg)"){
        ui->widget_realplot->saveJpg(filename, 1920, 1080);
    }
    else if (filter == "BMP (*.bmp)"){
        ui->widget_realplot->saveBmp(filename, 1920, 1080);
    }
}

void FastHeat::slSaveData(void){
    QString filter;
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Plot Data To...",
                ".",
                "CSV (*.csv)",
                &filter);
    if (filename.isEmpty()) return;

    if (filter == "CSV (*.csv)"){
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << "Time, s;Temperature, C" << endl;
            QSharedPointer<QCPGraphDataContainer> datacontainer = ui->widget_realplot->graph(0)->data();
            for (auto iter = datacontainer->begin(); iter != datacontainer->end(); iter++){
                stream << iter->key << ";" << iter->value << endl;
            }
            file.close();
        }
    }
}

void FastHeat::slSaveAll(void){
    QString filter;
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Experiment Data To...",
                ".",
                "PNG CSV_Comma;;JPG CSV_Comma;;BMP CSV_Comma",
                &filter);
    if (filename.isEmpty()) return;

    QDir().mkdir(filename);
    QFile file(filename + "/Data.csv");
    if (filter.endsWith("CSV_Comma")){
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << "Time, s;Temperature, C" << endl;
            QSharedPointer<QCPGraphDataContainer> datacontainer = ui->widget_realplot->graph(0)->data();
            for (auto iter = datacontainer->begin(); iter != datacontainer->end(); iter++){
                stream << iter->key << ";" << iter->value << endl;
            }
            file.close();
        }
    }


    if (filter.startsWith("PNG")){
        ui->widget_realplot->savePng(filename + "/Image.png", 1920, 1080);
    }
    else if (filter.startsWith("JPG")){
        ui->widget_realplot->saveJpg(filename + "/Image.jpg", 1920, 1080);
    }
    else if (filter.startsWith("BMP")){
        ui->widget_realplot->saveBmp(filename + "/Image.bmp", 1920, 1080);
    }
}

void FastHeat::slShowPlotMenu(const QPoint & pos){
    Q_UNUSED(pos);
    this->plot_menu->popup(QCursor::pos());
    this->plot_menu->show();
}

void FastHeat::slReceiveRelay(uint16_t relay){
    this->SetRelayLabel(relay);
}

void FastHeat::slReceiveTemp(float temp){
    this->temperature = temp;
    ui->lcdnumber_actualvalue->display(temp);
}
