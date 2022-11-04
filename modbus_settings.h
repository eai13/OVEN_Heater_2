#ifndef MODBUS_SETTINGS_H
#define MODBUS_SETTINGS_H

#include <QWidget>

namespace Ui {
class MODBUS_Settings;
}

class MODBUS_Settings : public QWidget
{
    Q_OBJECT

public:
    explicit MODBUS_Settings(QWidget *parent = nullptr);
    ~MODBUS_Settings();

private:
    Ui::MODBUS_Settings *ui;

private slots:
    void slApply(void);
    void slCancel(void);

signals:
    void siSendNewParameters(uint8_t address, uint16_t tempaddr, uint16_t relayaddr, uint16_t pidaddr, uint16_t enanbleaddr);
};

#endif // MODBUS_SETTINGS_H
