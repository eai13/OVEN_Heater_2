#include "modbus_settings.h"
#include "ui_modbus_settings.h"
#include <QIntValidator>

MODBUS_Settings::MODBUS_Settings(uint16_t sl_a, uint16_t te_a, uint16_t re_a, uint16_t pi_a, uint16_t en_a, uint16_t sp_a, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MODBUS_Settings)
{
    ui->setupUi(this);

    ui->lineedit_enableaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_pidaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_relayaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_slaveaddress->setValidator(new QIntValidator(0, 127));
    ui->lineedit_tempaddr->setValidator(new QIntValidator(0, 65535));

    ui->lineedit_enableaddr->setText(QString::number(en_a));
    ui->lineedit_pidaddr->setText(QString::number(pi_a));
    ui->lineedit_relayaddr->setText(QString::number(re_a));
    ui->lineedit_slaveaddress->setText(QString::number(sl_a));
    ui->lineedit_tempaddr->setText(QString::number(te_a));
    ui->lineedit_setpointaddr->setText(QString::number(sp_a));

    connect(ui->pushbutton_apply, &QPushButton::released, this, &MODBUS_Settings::slApply);
    connect(ui->pushbutton_close, &QPushButton::released, this, &MODBUS_Settings::slCancel);
}

MODBUS_Settings::~MODBUS_Settings()
{
    delete ui;
}

void MODBUS_Settings::slApply(void){
    if (ui->lineedit_slaveaddress->text().isEmpty()) return;
    int address = ui->lineedit_slaveaddress->text().toUInt();
    if (ui->lineedit_tempaddr->text().isEmpty()) return;
    int tempaddr = ui->lineedit_tempaddr->text().toUInt();
    if (ui->lineedit_relayaddr->text().isEmpty()) return;
    int relayaddr = ui->lineedit_relayaddr->text().toUInt();
    if (ui->lineedit_pidaddr->text().isEmpty()) return;
    int pidaddr = ui->lineedit_pidaddr->text().toUInt();
    if (ui->lineedit_enableaddr->text().isEmpty()) return;
    int enableaddr = ui->lineedit_enableaddr->text().toUInt();
    if (ui->lineedit_setpointaddr->text().isEmpty()) return;
    int setpointaddr = ui->lineedit_setpointaddr->text().toUInt();
    emit this->siSendNewParameters(address, tempaddr, relayaddr, pidaddr, enableaddr, setpointaddr);
    this->deleteLater();
}

void MODBUS_Settings::slCancel(void){
    this->deleteLater();
}
