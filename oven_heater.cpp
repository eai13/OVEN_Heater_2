#include "oven_heater.h"
#include <QWidget>
#include "modbus_settings.h"
#include "serial_settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QDebug>

#define CONFIG_FILENAME ("CONFIG.CNF")
#define MODBUS_CONFIG   ("MODBUS_CONFIG")
#define SERIAL_CONFIG   ("SERIAL_CONFIG")
#define SLAVE_ADDR      ("SLAVE_ADDR")
#define TEMP_ADDR       ("TEMPERATURE_ADDR")
#define RELAY_ADDR      ("RELAY_ADDR")
#define PID_ADDR        ("PID_ADDR")
#define ENABLE_ADDR     ("ENABLE_ADDR")
#define SETPOINT_ADDR   ("SETPOINT_ADDR")
#define BAUDRATE_VAL    ("BAUDRATE")

OVEN_Heater::OVEN_Heater(QWidget *parent) : QMainWindow(parent), ui(new Ui::OVEN_Heater){
    ui->setupUi(this);

    QFile json_file(CONFIG_FILENAME);
    if ((json_file.exists()) && (json_file.open(QIODevice::ReadOnly | QIODevice::Text))){
        QString json_text = json_file.readAll();
        json_file.close();
        QJsonDocument json_doc = QJsonDocument::fromJson(json_text.toUtf8());
        QJsonObject main_obj = json_doc.object();
        QJsonValue modbus_config_obj = main_obj[MODBUS_CONFIG];
        QJsonValue serial_config_obj = main_obj[SERIAL_CONFIG];
        this->slave_addr = modbus_config_obj[SLAVE_ADDR].toInt();
        this->temperature_addr = modbus_config_obj[TEMP_ADDR].toInt();
        this->relay_addr = modbus_config_obj[RELAY_ADDR].toInt();
        this->pid_addr = modbus_config_obj[PID_ADDR].toInt();
        this->enable_addr = modbus_config_obj[ENABLE_ADDR].toInt();
        this->setpoint_addr = modbus_config_obj[SETPOINT_ADDR].toInt();
        this->baudrate = serial_config_obj[BAUDRATE_VAL].toInt();

        ui->label_modbusparams->setText("MODBUS " +
                                        QString::number(this->slave_addr) + " " +
                                        QString::number(this->temperature_addr) + " " +
                                        QString::number(this->relay_addr) + " " +
                                        QString::number(this->pid_addr) + " " +
                                        QString::number(this->enable_addr) + " " +
                                        QString::number(this->setpoint_addr));
        ui->label_serialparams->setText("SERIAL " + QString::number(this->baudrate));

        qDebug() << SLAVE_ADDR << " " << this->slave_addr;
        qDebug() << TEMP_ADDR << " " << this->temperature_addr;
        qDebug() << RELAY_ADDR << " " << this->relay_addr;
        qDebug() << PID_ADDR << " " << this->pid_addr;
        qDebug() << ENABLE_ADDR << " " << this->enable_addr;
        qDebug() << SETPOINT_ADDR << " " << this->setpoint_addr;
        qDebug() << BAUDRATE_VAL << " " << this->baudrate;
    }
    else{
        json_file.open(QIODevice::WriteOnly | QIODevice::Text);
        QJsonDocument json_doc;
        QJsonObject modbus_config_obj;
        modbus_config_obj.insert(SLAVE_ADDR, 1);
        modbus_config_obj.insert(TEMP_ADDR, 0);
        modbus_config_obj.insert(RELAY_ADDR, 1);
        modbus_config_obj.insert(PID_ADDR, 2);
        modbus_config_obj.insert(ENABLE_ADDR, 3);
        modbus_config_obj.insert(SETPOINT_ADDR, 4);
        QJsonObject serial_config_obj;
        serial_config_obj.insert(BAUDRATE_VAL, 9600);
        QJsonObject main_obj;
        main_obj.insert(MODBUS_CONFIG, modbus_config_obj);
        main_obj.insert(SERIAL_CONFIG, serial_config_obj);
        json_doc.setObject(main_obj);
        json_file.write(json_doc.toJson());
        json_file.close();
    }

    this->serial_timeout_timer = new QTimer();
    connect(this->serial_timeout_timer, &QTimer::timeout, this, &OVEN_Heater::slSerialTimeout);

    this->serial_queue_timer = new QTimer();
    connect(this->serial_queue_timer, &QTimer::timeout, this, &OVEN_Heater::slSerialQueueProcess);
    this->serial_queue_timer->start(250);

    this->serial_ping_timer = new QTimer();
    connect(this->serial_ping_timer, &QTimer::timeout, this, &OVEN_Heater::slSerialPing);
    this->serial_ping_timer->start(1000);

    connect(ui->action_fast_heating, &QAction::toggled, this, &OVEN_Heater::slSwitchToFastHeat);
    connect(ui->action_profile_heating, &QAction::toggled, this, &OVEN_Heater::slSwitchToProfileHeat);
    connect(ui->action_save_experiment_as, &QAction::triggered, this, &OVEN_Heater::slSaveExperiment);
    connect(ui->action_load_experiment, &QAction::triggered, this, &OVEN_Heater::slUploadExperiment);
    connect(ui->action_modbus_settings, &QAction::triggered, this, &OVEN_Heater::slShowMODBUSConfig);
    connect(ui->action_serial_settings, &QAction::triggered, this, &OVEN_Heater::slShowSerialConfig);

    this->setWindowIcon(                QIcon(":/ICONS/ICONS/MAIN_icon.png"));
    ui->action_profile_heating->setIcon(QIcon(":/ICONS/ICONS/PH_icon.png"));
    ui->action_fast_heating->setIcon(   QIcon(":/ICONS/ICONS/FH_icon.png"));
    ui->action_load_experiment->setIcon(QIcon(":/ICONS/ICONS/UPLOAD_icon.png"));
    ui->action_modbus_settings->setIcon(QIcon(":/ICONS/ICONS/MODBUS_icon.png"));
    ui->action_serial_settings->setIcon(QIcon(":/ICONS/ICONS/RS485_icon.png"));
    ui->action_save_experiment->setIcon(QIcon(":/ICONS/ICONS/SAVE_icon.png"));
    ui->action_save_experiment_as->setIcon(QIcon(":/ICONS/ICONS/SAVEAS_icon.png"));

    QActionGroup * action_group = new QActionGroup(this);
    action_group->setExclusive(true);
    ui->action_fast_heating->setActionGroup(action_group);
    ui->action_profile_heating->setActionGroup(action_group);

    this->fast_heat = new FastHeat;
    ui->verticalLayout->addWidget(this->fast_heat, 0);
    connect(this->fast_heat, &FastHeat::siStarted, this, &OVEN_Heater::slStarted);
    connect(this->fast_heat, &FastHeat::siStopped, this, &OVEN_Heater::slStopped);
    connect(this, &OVEN_Heater::siSendRelay, this->fast_heat, &FastHeat::slReceiveRelay);
    connect(this, &OVEN_Heater::siSendTemperature, this->fast_heat, &FastHeat::slReceiveTemp);
    connect(this->fast_heat, &FastHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
    connect(this->fast_heat, &FastHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
    connect(this->fast_heat, &FastHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
    connect(this, &OVEN_Heater::siConnectionLost, this->fast_heat, &FastHeat::slStop);
}

OVEN_Heater::~OVEN_Heater(){
    if (this->serial_ping_timer != nullptr){
        if (this->serial_ping_timer->isActive()){
            this->serial_ping_timer->stop();
        }
        delete this->serial_ping_timer;
    }
    if (this->serial_queue_timer != nullptr){
        if (this->serial_queue_timer->isActive()){
            this->serial_queue_timer->stop();
        }
        delete this->serial_queue_timer;
    }
    if (this->serial_timeout_timer != nullptr){
        if (this->serial_timeout_timer->isActive()){
            this->serial_timeout_timer->stop();
        }
        delete this->serial_timeout_timer;
    }
    if (this->fast_heat != nullptr){
        delete this->fast_heat;
    }
    if (this->profile_heat != nullptr){
        delete this->profile_heat;
    }
    delete ui;
}

void OVEN_Heater::slSwitchToFastHeat(bool state){
    if (!state) return;

    if (this->profile_heat != nullptr){
        disconnect(this->profile_heat, &ProfileHeat::siStarted, this, &OVEN_Heater::slStarted);
        disconnect(this->profile_heat, &ProfileHeat::siStopped, this, &OVEN_Heater::slStopped);
        disconnect(this, &OVEN_Heater::siSendRelay, this->profile_heat, &ProfileHeat::slReceiveRelay);
        disconnect(this, &OVEN_Heater::siSendTemperature, this->profile_heat, &ProfileHeat::slReceiveTemp);
        disconnect(this->profile_heat, &ProfileHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
        disconnect(this->profile_heat, &ProfileHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
        disconnect(this->profile_heat, &ProfileHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
        disconnect(this, &OVEN_Heater::siConnectionLost, this->profile_heat, &ProfileHeat::slStop);
        ui->verticalLayout->removeWidget(this->profile_heat);
        delete this->profile_heat;
        this->profile_heat = nullptr;
    }
    this->fast_heat = new FastHeat;
    this->filename = "";
    ui->verticalLayout->addWidget(this->fast_heat, 0);
    connect(this->fast_heat, &FastHeat::siStarted, this, &OVEN_Heater::slStarted);
    connect(this->fast_heat, &FastHeat::siStopped, this, &OVEN_Heater::slStopped);
    connect(this, &OVEN_Heater::siSendRelay, this->fast_heat, &FastHeat::slReceiveRelay);
    connect(this, &OVEN_Heater::siSendTemperature, this->fast_heat, &FastHeat::slReceiveTemp);
    connect(this->fast_heat, &FastHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
    connect(this->fast_heat, &FastHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
    connect(this->fast_heat, &FastHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
    connect(this, &OVEN_Heater::siConnectionLost, this->fast_heat, &FastHeat::slStop);
}

void OVEN_Heater::slSwitchToProfileHeat(bool state){
    if (!state) return;

    if (this->fast_heat != nullptr){
        disconnect(this->fast_heat, &FastHeat::siStarted, this, &OVEN_Heater::slStarted);
        disconnect(this->fast_heat, &FastHeat::siStopped, this, &OVEN_Heater::slStopped);
        disconnect(this, &OVEN_Heater::siSendRelay, this->fast_heat, &FastHeat::slReceiveRelay);
        disconnect(this, &OVEN_Heater::siSendTemperature, this->fast_heat, &FastHeat::slReceiveTemp);
        disconnect(this->fast_heat, &FastHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
        disconnect(this->fast_heat, &FastHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
        disconnect(this->fast_heat, &FastHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
        disconnect(this, &OVEN_Heater::siConnectionLost, this->fast_heat, &FastHeat::slStop);
        ui->verticalLayout->removeWidget(this->fast_heat);
        delete this->fast_heat;
        this->fast_heat = nullptr;
    }
    if (this->profile_heat != nullptr){
        disconnect(this->profile_heat, &ProfileHeat::siStarted, this, &OVEN_Heater::slStarted);
        disconnect(this->profile_heat, &ProfileHeat::siStopped, this, &OVEN_Heater::slStopped);
        disconnect(this, &OVEN_Heater::siSendRelay, this->profile_heat, &ProfileHeat::slReceiveRelay);
        disconnect(this, &OVEN_Heater::siSendTemperature, this->profile_heat, &ProfileHeat::slReceiveTemp);
        disconnect(this->profile_heat, &ProfileHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
        disconnect(this->profile_heat, &ProfileHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
        disconnect(this->profile_heat, &ProfileHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
        disconnect(this, &OVEN_Heater::siConnectionLost, this->profile_heat, &ProfileHeat::slStop);
        ui->verticalLayout->removeWidget(this->profile_heat);
        delete this->profile_heat;
        this->profile_heat = nullptr;
    }
    this->profile_heat = new ProfileHeat((this->filename.isEmpty()) ? ("") : (this->filename));
    ui->verticalLayout->addWidget(this->profile_heat, 0);
    connect(this->profile_heat, &ProfileHeat::siStarted, this, &OVEN_Heater::slStarted);
    connect(this->profile_heat, &ProfileHeat::siStopped, this, &OVEN_Heater::slStopped);
    connect(this, &OVEN_Heater::siSendRelay, this->profile_heat, &ProfileHeat::slReceiveRelay);
    connect(this, &OVEN_Heater::siSendTemperature, this->profile_heat, &ProfileHeat::slReceiveTemp);
    connect(this->profile_heat, &ProfileHeat::siSendEnable, this, &OVEN_Heater::slReceiveEnable);
    connect(this->profile_heat, &ProfileHeat::siSendPID, this, &OVEN_Heater::slReceivePID);
    connect(this->profile_heat, &ProfileHeat::siSendSetPoint, this, &OVEN_Heater::slReceiveSetPoint);
    connect(this, &OVEN_Heater::siConnectionLost, this->profile_heat, &ProfileHeat::slStop);
}

void OVEN_Heater::slSaveExperiment(void){
    if (this->profile_heat != nullptr){
        QString fname = QFileDialog::getSaveFileName(
                    this,
                    "Save Experiment To...",
                    ".",
                    "OHE Files (*.ohe)");
        if (fname.isEmpty()) return;
        this->filename = fname;
        this->profile_heat->SaveExperiment(fname);
    }
}

void OVEN_Heater::slUploadExperiment(void){
    QString fname = QFileDialog::getOpenFileName(
                this,
                "Upload Experiment...",
                ".",
                "OHE Files (*.ohe)");
    if (fname.isEmpty()) return;

    this->filename = fname;
    if (ui->action_profile_heating->isChecked())
        this->slSwitchToProfileHeat(true);
    else
        ui->action_profile_heating->setChecked(true);
}

void OVEN_Heater::slStarted(void){
    ui->menubar->setEnabled(false);
}

void OVEN_Heater::slStopped(void){
    ui->menubar->setEnabled(true);
}

void OVEN_Heater::slShowMODBUSConfig(void){
    MODBUS_Settings * modbus_window = new MODBUS_Settings(this->slave_addr,
                                                          this->temperature_addr,
                                                          this->relay_addr,
                                                          this->pid_addr,
                                                          this->enable_addr,
                                                          this->setpoint_addr);
    connect(modbus_window, &MODBUS_Settings::siSendNewParameters, this, &OVEN_Heater::slSaveMODBUSParameters);
    modbus_window->show();
}
void OVEN_Heater::slSaveMODBUSParameters(uint8_t address, uint16_t tempaddr, uint16_t relayaddr, uint16_t pidaddr, uint16_t enanbleaddr, uint16_t setpointaddr){
    this->slave_addr = address;
    this->temperature_addr = tempaddr;
    this->relay_addr = relayaddr;
    this->pid_addr = pidaddr;
    this->enable_addr = enanbleaddr;
    this->setpoint_addr = setpointaddr;

    QFile file(CONFIG_FILENAME);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QJsonDocument json_doc;
        QJsonObject modbus_config_obj;
        modbus_config_obj.insert(SLAVE_ADDR, this->slave_addr);
        modbus_config_obj.insert(TEMP_ADDR, this->temperature_addr);
        modbus_config_obj.insert(RELAY_ADDR, this->relay_addr);
        modbus_config_obj.insert(PID_ADDR, this->pid_addr);
        modbus_config_obj.insert(ENABLE_ADDR, this->enable_addr);
        modbus_config_obj.insert(SETPOINT_ADDR, this->setpoint_addr);
        QJsonObject serial_config_obj;
        serial_config_obj.insert(BAUDRATE_VAL, this->baudrate);
        QJsonObject main_obj;
        main_obj.insert(MODBUS_CONFIG, modbus_config_obj);
        main_obj.insert(SERIAL_CONFIG, serial_config_obj);
        json_doc.setObject(main_obj);
        file.write(json_doc.toJson());
        file.close();
    }

    ui->label_modbusparams->setText("MODBUS " +
                                    QString::number(this->slave_addr) + " " +
                                    QString::number(this->temperature_addr) + " " +
                                    QString::number(this->relay_addr) + " " +
                                    QString::number(this->pid_addr) + " " +
                                    QString::number(this->enable_addr) + " " +
                                    QString::number(this->setpoint_addr));
}

void OVEN_Heater::slShowSerialConfig(void){
    Serial_Settings * serial_window = new Serial_Settings(this->baudrate);
    connect(serial_window, &Serial_Settings::siSendNewParameters, this, &OVEN_Heater::slSaveSerialParameters);
    serial_window->show();
}
void OVEN_Heater::slSaveSerialParameters(uint32_t baud){
    this->baudrate = baud;

    QFile file(CONFIG_FILENAME);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QJsonDocument json_doc;
        QJsonObject modbus_config_obj;
        modbus_config_obj.insert(SLAVE_ADDR, this->slave_addr);
        modbus_config_obj.insert(TEMP_ADDR, this->temperature_addr);
        modbus_config_obj.insert(RELAY_ADDR, this->relay_addr);
        modbus_config_obj.insert(PID_ADDR, this->pid_addr);
        modbus_config_obj.insert(ENABLE_ADDR, this->enable_addr);
        modbus_config_obj.insert(SETPOINT_ADDR, this->setpoint_addr);
        QJsonObject serial_config_obj;
        serial_config_obj.insert(BAUDRATE_VAL, this->baudrate);
        QJsonObject main_obj;
        main_obj.insert(MODBUS_CONFIG, modbus_config_obj);
        main_obj.insert(SERIAL_CONFIG, serial_config_obj);
        json_doc.setObject(main_obj);
        file.write(json_doc.toJson());
        file.close();
    }

    ui->label_serialparams->setText("SERIAL " + QString::number(this->baudrate));
}

void OVEN_Heater::slSerialPing(void){
    if (this->ConnectionOK == false){
        if (this->serial != nullptr){
            disconnect(this->serial, &QSerialPort::readyRead, this, &OVEN_Heater::slSerialRxProcess);
            delete this->serial;
            this->serial = nullptr;
        }
        if (this->SerialPortsToScan.isEmpty()){
            this->SerialPortsToScan = QSerialPortInfo::availablePorts();
        }
        if (this->SerialPortsToScan.isEmpty()) return;
        QString serial_name = this->SerialPortsToScan.takeFirst().portName();
        ui->label_activeport->setText(serial_name);
        this->serial = new QSerialPort(serial_name);
        this->serial->setBaudRate(this->baudrate);
        if (this->serial->open(QIODevice::ReadWrite)){
            connect(this->serial, &QSerialPort::readyRead, this, &OVEN_Heater::slSerialRxProcess);
            this->C_ReadTemperatureRelay();
        }
        else{
            disconnect(this->serial, &QSerialPort::readyRead, this, &OVEN_Heater::slSerialRxProcess);
            delete this->serial;
            this->serial = nullptr;
        }
    }
    else{
        this->C_ReadTemperatureRelay();
        this->C_WritePIDEnableSetPoint();
    }
}

void OVEN_Heater::slSerialRxProcess(void){
    if ((this->bytes_awaited) && (this->serial->bytesAvailable() >= this->bytes_awaited)){
        if (this->serial_timeout_timer->isActive())
            this->serial_timeout_timer->stop();
        QByteArray data = this->serial->read(this->bytes_awaited);
        this->bytes_awaited = 0;
        this->serial->readAll();

        uint8_t cmd = data.at(1);
        switch(cmd){
        case(0x03):{
            uint16_t temp = ((((uint16_t)data.at(3)) << 8) & 0xFF00) | (((uint16_t)data.at(4)) & 0xFF);
            uint16_t relay = data.at(6);
            qDebug() << "TEMPERATURE " << temp << " RELAY: " << relay;
            emit this->siSendTemperature(temp);
            emit this->siSendRelay(relay);
            this->ConnectionOK = true;
            break;
        }
        case(0x10):{
            if ((((uint16_t)data.at(4) << 8) | (uint16_t)data.at(5)) == 3){
                qDebug() << "WRITE OK";
            }
            else{
                qDebug() << "WRITE ERROR";
            }
            break;
        }
        default:{
            qDebug() << "UNKNOWN CMD: " << data;
            break;
        }
        }

    }
}

void OVEN_Heater::C_ReadTemperatureRelay(void){
    uint8_t data_to_send[8] = {
        this->slave_addr,
        0x03,
        (uint8_t)(this->temperature_addr >> 8),
        (uint8_t)(this->temperature_addr),
        0x00, 0x02,
        0, 0
    };
    uint16_t crc = this->crc16(data_to_send, 6);
    data_to_send[6] = crc;
    data_to_send[7] = crc >> 8;
    QByteArray b_arr;
    QDataStream stream(&b_arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    for (uint16_t iter = 0; iter < 8; iter++)
        stream << data_to_send[iter];

    qDebug() << b_arr;
    this->serial_queue.push_back(Pack(b_arr, 9));
}

void OVEN_Heater::C_WritePIDEnableSetPoint(void){
    uint8_t data_to_send[15] = {
        this->slave_addr,
        0x10,
        (uint8_t)(this->pid_addr >> 8), (uint8_t)(this->pid_addr),
        0x00, 0x03,
        0x06,
        (uint8_t)(this->pid >> 8), (uint8_t)(this->pid),
        (uint8_t)(this->enable >> 8), (uint8_t)(this->enable),
        (uint8_t)(this->setpoint >> 8), (uint8_t)(this->setpoint),
        0, 0
    };
    uint16_t crc = this->crc16(data_to_send, 13);
    data_to_send[13] = crc;
    data_to_send[14] = crc >> 8;
    QByteArray b_arr;
    QDataStream stream(&b_arr, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    for (uint16_t iter = 0; iter < 15; iter++)
        stream << data_to_send[iter];

    qDebug() << b_arr;
    this->serial_queue.push_back(Pack(b_arr, 8));
}

void OVEN_Heater::slSerialQueueProcess(void){
    if (this->serial_queue.isEmpty()) return;
    else{
        Pack new_pack = this->serial_queue.takeFirst();
        this->bytes_awaited = new_pack.bytes_awaited;
        if (this->serial_timeout_timer->isActive()) this->serial_timeout_timer->stop();
        this->serial_timeout_timer->start(200);
        this->serial->write(new_pack.data);
    }
}

void OVEN_Heater::slSerialTimeout(void){
    this->bytes_awaited = 0;
//    if (this->serial->bytesAvailable()) this->serial->readAll();
    this->ConnectionOK = false;

    emit this->siConnectionLost();
}
