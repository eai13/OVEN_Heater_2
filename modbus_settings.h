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
    explicit MODBUS_Settings(uint16_t sl_a, uint16_t te_a, uint16_t re_a, uint16_t pi_a, uint16_t en_a, uint16_t sp_a, QWidget *parent = nullptr);
    ~MODBUS_Settings();

private:
    Ui::MODBUS_Settings *ui;

private slots:
    void slApply(void);
    void slCancel(void);
    void slUpdateRegsRealTemp(void);
    void slUpdateRegsRelay(void);
    void slUpdateRegsPID(void);
    void slUpdateRegsEnable(void);
    void slUpdateRegsSetpoint(void);

signals:
    void siSendNewParameters(uint8_t address, uint16_t tempaddr, uint16_t relayaddr, uint16_t pidaddr, uint16_t enanbleaddr, uint16_t setpointaddr);
};

#endif // MODBUS_SETTINGS_H
