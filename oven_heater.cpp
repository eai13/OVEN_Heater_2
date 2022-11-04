#include "oven_heater.h"
#include <QWidget>
#include "modbus_settings.h"
#include "serial_settings.h"

OVEN_Heater::OVEN_Heater(QWidget *parent) : QMainWindow(parent), ui(new Ui::OVEN_Heater){
    ui->setupUi(this);

    connect(ui->action_fast_heating, &QAction::toggled, this, &OVEN_Heater::slSwitchToFastHeat);
    connect(ui->action_profile_heating, &QAction::toggled, this, &OVEN_Heater::slSwitchToProfileHeat);
    connect(ui->action_save_experiment_as, &QAction::triggered, this, &OVEN_Heater::slSaveExperiment);
    connect(ui->action_load_experiment, &QAction::triggered, this, &OVEN_Heater::slUploadExperiment);
    connect(ui->action_modbus_settings, &QAction::triggered, this, &OVEN_Heater::slShowMODBUSConfig);
    connect(ui->action_serial_settings, &QAction::triggered, this, &OVEN_Heater::slShowSerialConfig);

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
}

OVEN_Heater::~OVEN_Heater(){
    delete ui;
}

void OVEN_Heater::slSwitchToFastHeat(bool state){
    if (!state) return;

    if (this->profile_heat != nullptr){
        disconnect(this->profile_heat, &ProfileHeat::siStarted, this, &OVEN_Heater::slStarted);
        disconnect(this->profile_heat, &ProfileHeat::siStopped, this, &OVEN_Heater::slStopped);
        ui->verticalLayout->removeWidget(this->profile_heat);
        delete this->profile_heat;
        this->profile_heat = nullptr;
    }
    this->fast_heat = new FastHeat;
    this->filename = "";
    ui->verticalLayout->addWidget(this->fast_heat, 0);
    connect(this->fast_heat, &FastHeat::siStarted, this, &OVEN_Heater::slStarted);
    connect(this->fast_heat, &FastHeat::siStopped, this, &OVEN_Heater::slStopped);
}

void OVEN_Heater::slSwitchToProfileHeat(bool state){
    if (!state) return;

    if (this->fast_heat != nullptr){
        disconnect(this->fast_heat, &FastHeat::siStarted, this, &OVEN_Heater::slStarted);
        disconnect(this->fast_heat, &FastHeat::siStopped, this, &OVEN_Heater::slStopped);
        ui->verticalLayout->removeWidget(this->fast_heat);
        delete this->fast_heat;
        this->fast_heat = nullptr;
    }
    if (this->profile_heat != nullptr){
        ui->verticalLayout->removeWidget(this->profile_heat);
        delete this->profile_heat;
    }
    this->profile_heat = new ProfileHeat(this->filename);
    ui->verticalLayout->addWidget(this->profile_heat, 0);
    connect(this->profile_heat, &ProfileHeat::siStarted, this, &OVEN_Heater::slStarted);
    connect(this->profile_heat, &ProfileHeat::siStopped, this, &OVEN_Heater::slStopped);
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
    MODBUS_Settings * modbus_window = new MODBUS_Settings;
    connect(modbus_window, &MODBUS_Settings::siSendNewParameters, this, &OVEN_Heater::slSaveMODBUSParameters);
    modbus_window->show();
}
void OVEN_Heater::slSaveMODBUSParameters(uint8_t address, uint16_t tempaddr, uint16_t relayaddr, uint16_t pidaddr, uint16_t enanbleaddr){
    this->slave_addr = address;
    this->temperature_addr = tempaddr;
    this->relay_addr = relayaddr;
    this->pid_addr = pidaddr;
    this->enable_addr = enanbleaddr;

    ui->label_modbusparams->setText("MODBUS " +
                                    QString::number(this->slave_addr) + " " +
                                    QString::number(this->temperature_addr) + " " +
                                    QString::number(this->relay_addr) + " " +
                                    QString::number(this->pid_addr) + " " +
                                    QString::number(this->enable_addr));
}

void OVEN_Heater::slShowSerialConfig(void){
    Serial_Settings * serial_window = new Serial_Settings;
    connect(serial_window, &Serial_Settings::siSendNewParameters, this, &OVEN_Heater::slSaveSerialParameters);
    serial_window->show();
}
void OVEN_Heater::slSaveSerialParameters(uint32_t baud){
    this->baudrate = baud;

    ui->label_serialparams->setText("SERIAL " + QString::number(this->baudrate));
}
