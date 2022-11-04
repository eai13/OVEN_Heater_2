#include "serial_settings.h"
#include "ui_serial_settings.h"
#include <QIntValidator>

Serial_Settings::Serial_Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Serial_Settings)
{
    ui->setupUi(this);

    ui->lineedit_baud->setValidator(new QIntValidator(0, 115200));

    connect(ui->pushbutton_apply, &QPushButton::released, this, &Serial_Settings::slApply);
    connect(ui->pushbutton_cancel, &QPushButton::released, this, &Serial_Settings::slCancel);
}

Serial_Settings::~Serial_Settings()
{
    delete ui;
}

void Serial_Settings::slApply(void){
    if (ui->lineedit_baud->text().isEmpty()) return;
    uint32_t baud = ui->lineedit_baud->text().toUInt();
    emit this->siSendNewParameters(baud);

    this->deleteLater();
}

void Serial_Settings::slCancel(void){
    this->deleteLater();
}
