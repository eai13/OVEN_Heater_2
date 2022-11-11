#include "profileheat.h"
#include "ui_profileheat.h"
#include <QDataStream>
#include <QDoubleValidator>
#include <QPointF>

#define TIME_DISCRETE   ((float)(1.0))

void ProfileHeat::GUISetEnabled(GUI_ENABLE_STATE state){
    switch(state){
    case(GUI_ENABLE_STATE::ENB_STATE_FULL_BLOCK):{
        ui->lineedit_pointspeed->setEnabled(false);
        ui->lineedit_pointtemp->setEnabled(false);
        ui->pushbutton_addpoint->setEnabled(false);
        ui->pushbutton_clearpoints->setEnabled(false);
        ui->pushbutton_removepoint->setEnabled(false);
        ui->pushbutton_run->setEnabled(false);
        ui->pushbutton_stop->setEnabled(false);
        ui->pushbutton_setstarttemp->setEnabled(false);
        ui->lineedit_starttemp->setEnabled(false);
        this->SetRunningLabel(false);
        break;
    }
    case(GUI_ENABLE_STATE::ENB_START_TEMP_NOT_SET):{
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
        this->SetRunningLabel(false);
        break;
    }
    case(GUI_ENABLE_STATE::ENB_START_TEMP_SET):{
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
        this->SetRunningLabel(false);
        break;
    }
    case(GUI_ENABLE_STATE::ENB_RUN):{
        ui->pushbutton_addpoint->setEnabled(false);
        ui->pushbutton_clearpoints->setEnabled(false);
        ui->pushbutton_removepoint->setEnabled(false);
        ui->pushbutton_run->setEnabled(false);
        ui->pushbutton_setstarttemp->setEnabled(false);
        ui->pushbutton_stop->setEnabled(true);
        ui->lineedit_pointspeed->setText("");
        ui->lineedit_pointtemp->setText("");
        ui->lineedit_pointspeed->setEnabled(false);
        ui->lineedit_pointtemp->setEnabled(false);
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

ProfileHeat::ProfileHeat(QString from_file, QWidget *parent) : QWidget(parent), ui(new Ui::ProfileHeat){
    ui->setupUi(this);

    ui->lineedit_pointspeed->setValidator(new QDoubleValidator(0, 25, 2));
    ui->lineedit_pointtemp->setValidator(new QDoubleValidator(0, 1000, 2));
    ui->lineedit_starttemp->setValidator(new QDoubleValidator(0, 1000, 2));

    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_STATE_FULL_BLOCK);

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
                this->profile_lookup_table.push_back(QPointF(key, value));
                ui->widget_profileplot->graph(0)->addData(key, value);
                ui->lineedit_starttemp->setText(QString::asprintf("%.2f", value));
                data_count--;
                while(data_count--){
                    stream >> key;
                    stream >> value;
                    this->profile_lookup_table.push_back(QPointF(key, value));
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
    connect(ui->lineedit_pointtemp, &QLineEdit::textEdited, this, &ProfileHeat::slEditFinishedTemp);
    connect(ui->lineedit_pointspeed, &QLineEdit::textEdited, this, &ProfileHeat::slEditFinishedSpeed);

    this->global_time = new QTime;
    this->run_timer = new QTimer;
    connect(this->run_timer, &QTimer::timeout, this, &ProfileHeat::slRunProcessCallback);
}

ProfileHeat::~ProfileHeat(){
    if (this->profile_plot_menu != nullptr){
        delete this->profile_plot_menu;
    }
    if (this->real_plot_menu != nullptr){
        delete this->real_plot_menu;
    }
    if (this->global_time != nullptr){
        delete this->global_time;
    }
    if (this->run_timer != nullptr){
        if (this->run_timer->isActive()){
            this->run_timer->stop();
        }
        delete this->run_timer;
    }
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
    emit this->siStarted();

    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_RUN);
    this->profile_discrete.push_back(this->profile_lookup_table.front());
    for (auto iter = (++(this->profile_lookup_table.begin())); iter != this->profile_lookup_table.end(); iter++){
        QPointF current = this->profile_discrete.back();
        QPointF next = (*iter);
        uint32_t points_amount = (next - current).x() / TIME_DISCRETE;
        float temp_inc = (next.y() - current.y()) / points_amount;

        for (float temp = current.y(), time = current.x(); points_amount--; ){
            this->profile_discrete.push_back(QPointF(time += TIME_DISCRETE, temp += temp_inc));
        }
    }

    emit this->siSendEnable(1);
    this->global_time->restart();
    this->run_timer->start(100);
}

void ProfileHeat::slRunProcessCallback(void){
    float requested_temp;
    float current_time = (float)(this->global_time->elapsed()) / (float)(1000);
    if (this->profile_discrete.isEmpty()){
        this->is_at_plain = true;
        requested_temp = this->prev_set_temperature;
    }
    else if (!(this->is_prepared)){
        requested_temp = this->profile_discrete.front().y();
        if (std::abs(requested_temp - this->real_temperature) < 1.0001){
            this->is_prepared = true;
        }
        emit this->siSendSetPoint(requested_temp);
    }
    else{
        float requested_time = this->profile_discrete.front().x();
        requested_temp = this->profile_discrete.front().y();
        if (requested_time <= current_time){
            this->profile_discrete.pop_front();
            if (std::abs(this->prev_set_temperature - requested_temp) < 0.0001) this->is_at_plain = true;
            else this->is_at_plain = false;
            this->prev_set_temperature = requested_temp;
            emit this->siSendSetPoint(requested_temp);
        }
    }
    if (this->is_at_plain){
        if (std::abs(requested_temp - this->real_temperature) > 3){
            emit this->siSendPID(PID_NUMBER::PID_PI_PLAIN);
        }
        else{
            emit this->siSendPID(PID_NUMBER::PID_P_PLAIN);
        }
    }
    else{
        emit this->siSendPID(PID_NUMBER::PID_RAMP);
    }
    ui->widget_realplot->graph(0)->addData(current_time, this->real_temperature);
    ui->widget_realplot->rescaleAxes();
    ui->widget_realplot->xAxis->setRangeLower(0);
    ui->widget_realplot->yAxis->setRangeLower(0);
    ui->widget_realplot->replot();
}

void ProfileHeat::slStop(void){
    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_SET);

    this->profile_discrete.clear();
    this->run_timer->stop();
    emit this->siSendEnable(0);

    emit this->siStopped();
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
                QString tmp_value = QString::asprintf("%.2f", iter->value);
                for (auto chr = tmp_value.begin(); chr != tmp_value.end(); chr++)
                    if (*chr == '.') *chr = ',';
                stream << iter->key << ";" << tmp_value << endl;
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
                QString tmp_value = QString::asprintf("%.2f", iter->value);
                for (auto chr = tmp_value.begin(); chr != tmp_value.end(); chr++)
                    if (*chr == '.') *chr = ',';
                stream << iter->key << ";" << tmp_value << endl;
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
                QString tmp_value = QString::asprintf("%.2f", iter->value);
                for (auto chr = tmp_value.begin(); chr != tmp_value.end(); chr++)
                    if (*chr == '.') *chr = ',';
                stream << iter->key << ";" << tmp_value << endl;
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
    Q_UNUSED(pos);
    this->real_plot_menu->popup(QCursor::pos());
    this->real_plot_menu->show();
}

void ProfileHeat::slShowProfilePlotMenu(const QPoint & pos){
    Q_UNUSED(pos);
    this->profile_plot_menu->popup(QCursor::pos());
    this->profile_plot_menu->show();
}

void ProfileHeat::slSetStartingTemperature(void){
    if (ui->lineedit_starttemp->text().isEmpty()) return;

    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_SET);

    this->profile_lookup_table.push_back(QPointF(0, ui->lineedit_starttemp->text().toDouble()));
    ui->widget_profileplot->graph(0)->addData(0, ui->lineedit_starttemp->text().toDouble());
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();
}

void ProfileHeat::slAddProfilePoint(void){
    if ((ui->lineedit_pointspeed->text().isEmpty()) || (ui->lineedit_pointtemp->text().isEmpty())) return;

    QString txt_speed = ui->lineedit_pointspeed->text();
    for (auto iter = txt_speed.begin(); iter != txt_speed.end(); iter++)
        if (*iter == ',') *iter = '.';
    QString txt_temp = ui->lineedit_pointtemp->text();
    for (auto iter = txt_temp.begin(); iter != txt_temp.end(); iter++)
        if (*iter == ',') *iter = '.';
    double cur_temp = txt_temp.toDouble();
    double speed = txt_speed.toDouble();
    double prev_temp = this->profile_lookup_table.back().y();
    double prev_time = this->profile_lookup_table.back().x();

    if (std::abs(speed - 0) < 0.0001){
        this->profile_lookup_table.push_back(QPointF(prev_time + cur_temp * 60, prev_temp));
    }
    else if (std::abs(cur_temp - this->profile_lookup_table.back().y()) < 0.0001){
        this->profile_lookup_table.push_back(QPointF(prev_time + speed * 60, prev_temp));
    }
    else{
        this->profile_lookup_table.push_back(QPointF(prev_time + abs((cur_temp - prev_temp) / speed) * 60, cur_temp));
    }
    ui->widget_profileplot->graph(0)->addData(this->profile_lookup_table.back().x(), this->profile_lookup_table.back().y());
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    ui->lineedit_pointspeed->setText("");
    ui->lineedit_pointtemp->setText("");
}

void ProfileHeat::slRemoveProfilePoint(void){
    if (this->profile_lookup_table.isEmpty()){
        this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_NOT_SET);
        return;
    }
    if (this->profile_lookup_table.size() == 1){
        this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_NOT_SET);
    }

    this->profile_lookup_table.pop_back();
    ui->widget_profileplot->graph(0)->data()->removeAfter((ui->widget_profileplot->graph(0)->data()->end() - 1)->key - 0.000001);
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    ui->lineedit_pointspeed->setText("");
    ui->lineedit_pointtemp->setText("");
}

void ProfileHeat::slClearProfile(void){
    this->profile_lookup_table.clear();
    ui->widget_profileplot->graph(0)->data()->clear();
    ui->widget_profileplot->rescaleAxes();
    ui->widget_profileplot->yAxis->setRangeLower(0);
    ui->widget_profileplot->xAxis->setRangeLower(0);
    ui->widget_profileplot->replot();

    this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_NOT_SET);
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

void ProfileHeat::slReceiveTemp(float temp){
    ui->lcdnumber_actualtemp->display(temp);
    this->real_temperature = temp;
}
void ProfileHeat::slReceiveRelay(uint16_t relay){
    this->SetRelayLabel(relay);
}

void ProfileHeat::slDisconnectionStop(void){
    if (this->is_connected){
        this->is_connected = false;
        this->slStop();
        this->GUISetEnabled(GUI_ENABLE_STATE::ENB_STATE_FULL_BLOCK);
    }
}

void ProfileHeat::slConnected(void){
    if (!(this->is_connected)){
        this->is_connected = true;
        if (!(this->profile_lookup_table.isEmpty())){
            this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_SET);
        }
        else{
            this->GUISetEnabled(GUI_ENABLE_STATE::ENB_START_TEMP_NOT_SET);
        }
    }
}

void ProfileHeat::slEditFinishedSpeed(void){
    qWarning() << "EDIT FINISHED SPEED";
    if (ui->lineedit_pointspeed->text().isEmpty()){
        ui->lineedit_pointtemp->setPlaceholderText("Point Temperature, C");
        return;
    }
    QString speed_text = ui->lineedit_pointspeed->text();
    for (auto iter = speed_text.begin(); iter != speed_text.end(); iter++)
        if (*iter == ',') *iter = '.';
    if (std::abs(speed_text.toFloat() - 0) < 0.00001){
        ui->lineedit_pointtemp->setPlaceholderText("Plane Time, min");
    }
    else{
        ui->lineedit_pointtemp->setPlaceholderText("Point Temperature, C");
    }
}
void ProfileHeat::slEditFinishedTemp(void){
    qWarning() << "EDIT FINISHED TEMP";
    if (ui->lineedit_pointtemp->text().isEmpty()){
        ui->lineedit_pointspeed->setPlaceholderText("Point Speed, C/min");
        return;
    }
    QString temp_text = ui->lineedit_pointtemp->text();
    for (auto iter = temp_text.begin(); iter != temp_text.end(); iter++)
        if (*iter == ',') *iter = '.';
    if (std::abs(temp_text.toFloat() - this->profile_lookup_table.back().y()) < 0.0001){
        ui->lineedit_pointspeed->setPlaceholderText("Plane Time, min");
    }
    else{
        ui->lineedit_pointspeed->setPlaceholderText("Point Speed, C/min");
    }
}
