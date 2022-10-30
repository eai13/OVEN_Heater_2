#include "fastheat.h"
#include "ui_fastheat.h"

FastHeat::FastHeat(QWidget *parent) : QWidget(parent), ui(new Ui::FastHeat){
    ui->setupUi(this);

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
    ui->widget_realplot->graph(0)->addData(1, 1);
    ui->widget_realplot->graph(0)->addData(2, 4);
    ui->widget_realplot->graph(0)->addData(3, 9);
    ui->widget_realplot->rescaleAxes();
}

FastHeat::~FastHeat(){
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
    this->SetRunningLabel(true);
    this->SetRelayLabel(true);
}

void FastHeat::slStop(void){
    this->SetRunningLabel(false);
    this->SetRelayLabel(false);
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

    this->plot_menu->popup(QCursor::pos());
    this->plot_menu->show();
}
