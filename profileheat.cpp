#include "profileheat.h"
#include "ui_profileheat.h"
#include <QDataStream>

ProfileHeat::ProfileHeat(QString from_file, QWidget *parent) : QWidget(parent), ui(new Ui::ProfileHeat){
    ui->setupUi(this);


    ui->pushbutton_addpoint->setEnabled(false);
    ui->pushbutton_clearpoints->setEnabled(false);
    ui->pushbutton_removepoint->setEnabled(false);
    ui->pushbutton_run->setEnabled(false);
    ui->pushbutton_stop->setEnabled(false);
    ui->lineedit_pointspeed->setEnabled(false);
    ui->lineedit_pointtemp->setEnabled(false);

    this->profile_plot_menu = new QMenu;
    this->profile_plot_menu->addAction("Save Image", this, &ProfileHeat::slSaveProfileImage);
    this->profile_plot_menu->addAction("Save Data", this, &ProfileHeat::slSaveProfileData);
    this->profile_plot_menu->addAction("Save All", this, &ProfileHeat::slSaveAll);

    this->real_plot_menu = new QMenu;
    this->real_plot_menu->addAction("Save Image", this, &ProfileHeat::slSaveRealImage);
    this->real_plot_menu->addAction("Save Data", this, &ProfileHeat::slSaveRealData);
    this->real_plot_menu->addAction("Save All", this, &ProfileHeat::slSaveAll);

    ui->widget_profileplot->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->widget_realplot->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->pushbutton_run, &QPushButton::released, this, &ProfileHeat::slRun);
    connect(ui->pushbutton_stop, &QPushButton::released, this, &ProfileHeat::slStop);
    connect(ui->widget_profileplot, &QWidget::customContextMenuRequested, this, &ProfileHeat::slShowProfilePlotMenu);
    connect(ui->widget_realplot, &QWidget::customContextMenuRequested, this, &ProfileHeat::slShowRealPlotMenu);

    ui->widget_profileplot->xAxis->setLabel("Time, s");
    ui->widget_profileplot->yAxis->setLabel("Temperature, C");
    ui->widget_profileplot->addGraph()->setPen(QPen(Qt::red));

    if (!(from_file.isEmpty())){
        QFile file(from_file);
        if (file.open(QIODevice::ReadOnly)){
            QDataStream stream(&file);
            int data_count;
            double key, value;
            stream >> data_count;
            if (data_count){
                stream >> key;
                stream >> value;
                ui->widget_profileplot->graph(0)->addData(key, value);
                ui->lineedit_starttemp->setText(QString::asprintf("%.2f", value));
                ui->lineedit_starttemp->setEnabled(false);
                ui->pushbutton_setstarttemp->setEnabled(false);
                ui->lineedit_pointtemp->setEnabled(true);
                ui->lineedit_pointtemp->setText("");
                ui->lineedit_pointspeed->setEnabled(true);
                ui->lineedit_pointspeed->setText("");
                ui->pushbutton_addpoint->setEnabled(true);
                ui->pushbutton_clearpoints->setEnabled(true);
                ui->pushbutton_removepoint->setEnabled(true);
                ui->pushbutton_run->setEnabled(true);
                ui->pushbutton_stop->setEnabled(false);
                data_count--;
                while(data_count--){
                    stream >> key;
                    stream >> value;
                    ui->widget_profileplot->graph(0)->addData(key, value);
                }
            }
            ui->widget_profileplot->rescaleAxes();
            ui->widget_profileplot->xAxis->setRangeLower(0);
            ui->widget_profileplot->yAxis->setRangeLower(0);
            ui->widget_profileplot->replot();
            file.close();
        }
    }

    ui->widget_realplot->xAxis->setLabel("Time, s");
    ui->widget_realplot->yAxis->setLabel("Temperature, C");
    ui->widget_realplot->addGraph()->setPen(QPen(Qt::red));

    connect(ui->pushbutton_setstarttemp, &QPushButton::released, this, &ProfileHeat::slSetStartingTemperature);
    connect(ui->pushbutton_addpoint, &QPushButton::released, this, &ProfileHeat::slAddProfilePoint);
    connect(ui->pushbutton_removepoint, &QPushButton::released, this, &ProfileHeat::slRemoveProfilePoint);
    connect(ui->pushbutton_clearpoints, &QPushButton::released, this, &ProfileHeat::slClearProfile);
}

ProfileHeat::~ProfileHeat()
{
    delete this->profile_plot_menu;
    delete this->real_plot_menu;
    delete ui;
}

void ProfileHeat::SetRelayLabel(bool state){
    if (state)
        ui->label_relay->setStyleSheet("QLabel { background-color : rgb(0, 255, 0); color : black; }");
    else
        ui->label_relay->setStyleSheet("QLabel { background-color : rgb(255, 0, 0); color : black; }");
}

void ProfileHeat::SetRunningLabel(bool state){
    if (state)
        ui->label_isrunning->setStyleSheet("QLabel { background-color : rgb(0, 255, 0); color : black; }");
    else
        ui->label_isrunning->setStyleSheet("QLabel { background-color : rgb(255, 0, 0); color : black; }");
}

void ProfileHeat::slRun(void){
    this->SetRelayLabel(true);
}

void ProfileHeat::slStop(void){
    this->SetRelayLabel(false);
    this->SetRunningLabel(false);
}

void ProfileHeat::slSaveRealImage(void){
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

void ProfileHeat::slSaveRealData(void){
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

void ProfileHeat::slSaveProfileImage(void){
    QString filter;
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Plot Image To...",
                ".",
                "PNG (*.png);;JPG (*.jpg);;BMP (*.bmp)",
                &filter);
    if (filename.isEmpty()) return;

    if (filter == "PNG (*.png)"){
        ui->widget_profileplot->savePng(filename, 1920, 1080);
    }
    else if (filter == "JPG (*.jpg)"){
        ui->widget_profileplot->saveJpg(filename, 1920, 1080);
    }
    else if (filter == "BMP (*.bmp)"){
        ui->widget_profileplot->saveBmp(filename, 1920, 1080);
    }
}

void ProfileHeat::slSaveProfileData(void){
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
            QSharedPointer<QCPGraphDataContainer> datacontainer = ui->widget_profileplot->graph(0)->data();
            for (auto iter = datacontainer->begin(); iter != datacontainer->end(); iter++){
                stream << iter->key << ";" << iter->value << endl;
            }
            file.close();
        }
    }
}

void ProfileHeat::slSaveAll(void){
    QString filter;
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save Experiment Data To...",
                ".",
                "PNG CSV_Comma;;JPG CSV_Comma;;BMP CSV_Comma",
                &filter);
    if (filename.isEmpty()) return;

    QDir().mkdir(filename);
    QFile file_real(filename + "/Data_Real.csv");
    if (filter.endsWith("CSV_Comma")){
        if (file_real.open(QIODevice::WriteOnly)){
            QTextStream stream(&file_real);
            stream << "Time, s;Temperature, C" << endl;
            QSharedPointer<QCPGraphDataContainer> datacontainer = ui->widget_realplot->graph(0)->data();
            for (auto iter = datacontainer->begin(); iter != datacontainer->end(); iter++){
                stream << iter->key << ";" << iter->value << endl;
            }
            file_real.close();
        }
    }
    QFile file_profile(filename + "/Data_Profile.csv");
    if (filter.endsWith("CSV_Comma")){
        if (file_profile.open(QIODevice::WriteOnly)){
            QTextStream stream(&file_profile);
            stream << "Time, s;Temperature, C" << endl;
            QSharedPointer<QCPGraphDataContainer> datacontainer = ui->widget_profileplot->graph(0)->data();
            for (auto iter = datacontainer->begin(); iter != datacontainer->end(); iter++){
                stream << iter->key << ";" << iter->value << endl;
            }
            file_profile.close();
        }
    }


    if (filter.startsWith("PNG")){
        ui->widget_realplot->savePng(filename + "/Image_Real.png", 1920, 1080);
        ui->widget_profileplot->savePng(filename + "/Image_Profile.png", 1920, 1080);
    }
    else if (filter.startsWith("JPG")){
        ui->widget_realplot->saveJpg(filename + "/Image_Real.jpg", 1920, 1080);
        ui->widget_profileplot->saveJpg(filename + "/Image_Profile.jpg", 1920, 1080);
    }
    else if (filter.startsWith("BMP")){
        ui->widget_realplot->saveBmp(filename + "/Image_Real.bmp", 1920, 1080);
        ui->widget_profileplot->saveBmp(filename + "/Image_Profile.bmp", 1920, 1080);
    }
}

void ProfileHeat::slShowRealPlotMenu(const QPoint & pos){
    this->real_plot_menu->popup(QCursor::pos());
    this->real_plot_menu->show();
}

void ProfileHeat::slShowProfilePlotMenu(const QPoint & pos){
    this->profile_plot_menu->popup(QCursor::pos());
    this->profile_plot_menu->show();
}

void ProfileHeat::slSetStartingTemperature(void){
    if (ui->lineedit_starttemp->text().isEmpty()) return;

    ui->widget_profileplot->graph(0)->addData(0, ui->lineedit_starttemp->text().toDouble());
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    ui->lineedit_starttemp->setEnabled(false);
    ui->pushbutton_setstarttemp->setEnabled(false);
    ui->lineedit_pointtemp->setEnabled(true);
    ui->lineedit_pointtemp->setText("");
    ui->lineedit_pointspeed->setEnabled(true);
    ui->lineedit_pointspeed->setText("");
    ui->pushbutton_addpoint->setEnabled(true);
    ui->pushbutton_clearpoints->setEnabled(true);
    ui->pushbutton_removepoint->setEnabled(true);
    ui->pushbutton_run->setEnabled(true);
    ui->pushbutton_stop->setEnabled(false);
}

void ProfileHeat::slAddProfilePoint(void){
    if ((ui->lineedit_pointspeed->text().isEmpty()) || (ui->lineedit_pointtemp->text().isEmpty())) return;

    QString txt_speed = ui->lineedit_pointspeed->text();
    QString txt_temp = ui->lineedit_pointtemp->text();
    double cur_temp = txt_temp.toDouble();
    double speed = txt_speed.toDouble();
    double prev_temp = (ui->widget_profileplot->graph(0)->data()->end() - 1)->value;
    double prev_time = (ui->widget_profileplot->graph(0)->data()->end() - 1)->key;

    ui->widget_profileplot->graph(0)->addData(prev_time + abs((cur_temp - prev_temp) / speed) * 60, cur_temp);
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();
}

void ProfileHeat::slRemoveProfilePoint(void){
    if (ui->widget_profileplot->graph(0)->dataCount() == 1){
        ui->lineedit_pointspeed->setEnabled(false);
        ui->lineedit_pointtemp->setEnabled(false);
        ui->lineedit_pointspeed->setText("");
        ui->lineedit_pointtemp->setText("");
        ui->pushbutton_addpoint->setEnabled(false);
        ui->pushbutton_clearpoints->setEnabled(false);
        ui->pushbutton_removepoint->setEnabled(false);
        ui->pushbutton_run->setEnabled(false);
        ui->pushbutton_stop->setEnabled(false);
        ui->pushbutton_setstarttemp->setEnabled(true);
        ui->lineedit_starttemp->setEnabled(true);
        ui->lineedit_starttemp->setText("");
    }

    ui->widget_profileplot->graph(0)->data()->removeAfter((ui->widget_profileplot->graph(0)->data()->end() - 1)->key - 0.000001);
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    ui->lineedit_pointspeed->setText("");
    ui->lineedit_pointtemp->setText("");
}

void ProfileHeat::slClearProfile(void){
    ui->widget_profileplot->graph(0)->data()->clear();
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    ui->lineedit_pointspeed->setEnabled(false);
    ui->lineedit_pointtemp->setEnabled(false);
    ui->lineedit_pointspeed->setText("");
    ui->lineedit_pointtemp->setText("");
    ui->pushbutton_addpoint->setEnabled(false);
    ui->pushbutton_clearpoints->setEnabled(false);
    ui->pushbutton_removepoint->setEnabled(false);
    ui->pushbutton_run->setEnabled(false);
    ui->pushbutton_stop->setEnabled(false);
    ui->pushbutton_setstarttemp->setEnabled(true);
    ui->lineedit_starttemp->setEnabled(true);
    ui->lineedit_starttemp->setText("");
}

void ProfileHeat::SaveExperiment(QString filename){
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)){
        QDataStream stream(&file);
        stream << ui->widget_profileplot->graph(0)->dataCount();
        QSharedPointer<QCPGraphDataContainer> tmp_data = ui->widget_profileplot->graph(0)->data();
        for (auto iter = tmp_data->begin(); iter != tmp_data->end(); iter++){
            stream << iter->key << iter->value;
        }
        file.close();
    }
}
