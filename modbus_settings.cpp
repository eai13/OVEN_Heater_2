#include "modbus_settings.h"
#include "ui_modbus_settings.h"
#include <QIntValidator>

MODBUS_Settings::MODBUS_Settings(uint16_t sl_a, uint16_t te_a, uint16_t re_a, uint16_t pi_a, uint16_t en_a, uint16_t sp_a, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MODBUS_Settings)
{
    ui->setupUi(this);

    ui->lineedit_slaveaddress->setValidator(new QIntValidator(0, 127));

    ui->lineedit_enableaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_pidaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_relayaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_tempaddr->setValidator(new QIntValidator(0, 65535));
    ui->lineedit_setpointaddr->setValidator(new QIntValidator(0, 65535));

    ui->lineedit_enableaddr->setText(QString::number(en_a));
    ui->lineedit_pidaddr->setText(QString::number(pi_a));
    ui->lineedit_relayaddr->setText(QString::number(re_a));
    ui->lineedit_slaveaddress->setText(QString::number(sl_a));
    ui->lineedit_tempaddr->setText(QString::number(te_a));
    ui->lineedit_setpointaddr->setText(QString::number(sp_a));

    connect(ui->pushbutton_apply,       &QPushButton::released, this, &MODBUS_Settings::slApply);
    connect(ui->pushbutton_close,       &QPushButton::released, this, &MODBUS_Settings::slCancel);
    connect(ui->lineedit_tempaddr,      &QLineEdit::textEdited, this, &MODBUS_Settings::slUpdateRegsRealTemp);
    connect(ui->lineedit_relayaddr,     &QLineEdit::textEdited, this, &MODBUS_Settings::slUpdateRegsRelay);
    connect(ui->lineedit_pidaddr,       &QLineEdit::textEdited, this, &MODBUS_Settings::slUpdateRegsPID);
    connect(ui->lineedit_enableaddr,    &QLineEdit::textEdited, this, &MODBUS_Settings::slUpdateRegsEnable);
    connect(ui->lineedit_setpointaddr,  &QLineEdit::textEdited, this, &MODBUS_Settings::slUpdateRegsSetpoint);
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

void MODBUS_Settings::slUpdateRegsRealTemp(void){
    if (ui->lineedit_tempaddr->text().isEmpty()) return;
    int tmp = ui->lineedit_tempaddr->text().toInt();
    ui->lineedit_relayaddr->setText(QString::number(tmp + 2));
    ui->lineedit_pidaddr->setText(QString::number(tmp + 3));
    ui->lineedit_enableaddr->setText(QString::number(tmp + 4));
    ui->lineedit_setpointaddr->setText(QString::number(tmp + 5));
}

void MODBUS_Settings::slUpdateRegsRelay(void){
    if (ui->lineedit_relayaddr->text().isEmpty()) return;
    int tmp = ui->lineedit_relayaddr->text().toInt();
    if (tmp < 2) ui->pushbutton_apply->setEnabled(false);
    else ui->pushbutton_apply->setEnabled(true);
    ui->lineedit_tempaddr->setText(QString::number(tmp - 2));
    ui->lineedit_pidaddr->setText(QString::number(tmp + 1));
    ui->lineedit_enableaddr->setText(QString::number(tmp + 2));
    ui->lineedit_setpointaddr->setText(QString::number(tmp + 3));
}

void MODBUS_Settings::slUpdateRegsPID(void){
    if (ui->lineedit_pidaddr->text().isEmpty()) return;
    int tmp = ui->lineedit_pidaddr->text().toInt();
    if (tmp < 3) ui->pushbutton_apply->setEnabled(false);
    else ui->pushbutton_apply->setEnabled(true);
    ui->lineedit_tempaddr->setText(QString::number(tmp - 3));
    ui->lineedit_relayaddr->setText(QString::number(tmp - 1));
    ui->lineedit_enableaddr->setText(QString::number(tmp + 1));
    ui->lineedit_setpointaddr->setText(QString::number(tmp + 2));
}

void MODBUS_Settings::slUpdateRegsEnable(void){
    if (ui->lineedit_enableaddr->text().isEmpty()) return;
    int tmp = ui->lineedit_enableaddr->text().toInt();
    if (tmp < 4) ui->pushbutton_apply->setEnabled(false);
    else ui->pushbutton_apply->setEnabled(true);
    ui->lineedit_tempaddr->setText(QString::number(tmp - 4));
    ui->lineedit_relayaddr->setText(QString::number(tmp - 2));
    ui->lineedit_pidaddr->setText(QString::number(tmp - 1));
    ui->lineedit_setpointaddr->setText(QString::number(tmp + 1));
}

void MODBUS_Settings::slUpdateRegsSetpoint(void){
    if (ui->lineedit_setpointaddr->text().isEmpty()) return;
    int tmp = ui->lineedit_setpointaddr->text().toInt();
    if (tmp < 5) ui->pushbutton_apply->setEnabled(false);
    else ui->pushbutton_apply->setEnabled(true);
    ui->lineedit_tempaddr->setText(QString::number(tmp - 5));
    ui->lineedit_relayaddr->setText(QString::number(tmp - 3));
    ui->lineedit_pidaddr->setText(QString::number(tmp - 2));
    ui->lineedit_enableaddr->setText(QString::number(tmp - 1));
}
