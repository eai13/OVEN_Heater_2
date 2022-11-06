#ifndef OVEN_HEATER_H
#define OVEN_HEATER_H

#include <QMainWindow>
#include "ui_oven_heater.h"
#include "fastheat.h"
#include "profileheat.h"
#include <QIcon>
#include <QActionGroup>
#include <QAction>
#include <QGridLayout>
#include <QFileDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
    class OVEN_Heater;
}
QT_END_NAMESPACE

class OVEN_Heater : public QMainWindow
{
    Q_OBJECT

public:
    OVEN_Heater(QWidget *parent = nullptr);
    ~OVEN_Heater();

private:

    struct Pack{
        QByteArray data;
        uint16_t bytes_awaited;
        Pack(QByteArray d, uint16_t ba){
            this->data = d; this->bytes_awaited = ba;
        }
    };
    QTimer *            serial_timeout_timer = nullptr;
    QTimer *            serial_ping_timer = nullptr;
    QTimer *            serial_queue_timer = nullptr;
    QSerialPort *       serial = nullptr;
    QList<Pack>         serial_queue;
    uint16_t            bytes_awaited;


    uint8_t slave_addr;
    uint16_t temperature_addr;
    uint16_t relay_addr;
    uint16_t pid_addr;
    uint16_t pid = 0;
    uint16_t enable_addr;
    uint16_t enable = 0;
    uint16_t setpoint_addr;
    uint16_t setpoint = 0;

    int baudrate;

    bool ConnectionOK = false;
    QList<QSerialPortInfo> SerialPortsToScan;

    QString filename = "";

    QWidget * handle_widget = nullptr;
    FastHeat * fast_heat = nullptr;
    ProfileHeat * profile_heat = nullptr;
    Ui::OVEN_Heater *ui;

    uint16_t crc16(uint8_t * bytes, uint32_t length){
        uint16_t crc = 0xFFFF;
        for (uint32_t pos = 0; pos < length; pos++) {
            crc ^= (uint16_t)bytes[pos];
            for (int i = 8; i != 0; i--){
                if ((crc & 0x0001) != 0) {
                    crc >>= 1; crc ^= 0xA001;
                }
                else crc >>= 1;
            }
        }
        return crc;
    }
    void C_ReadTemperatureRelay(void);
    void C_WritePIDEnableSetPoint(void);

signals:
    void siSendTemperature(float temp);
    void siSendRelay(uint16_t relay);

public slots:
    void slReceiveEnable(uint16_t enb){
        this->enable = enb;
    }
    void slReceivePID(uint16_t PID){
        this->pid = PID;
    }
    void slReceiveSetPoint(uint16_t sp){
        this->setpoint = sp;
    }

private slots:
    void slSerialTimeout(void);
    void slSerialPing(void);
    void slSerialRxProcess(void);
    void slSerialQueueProcess(void);

    void slSaveExperiment(void);
    void slUploadExperiment(void);
    void slSwitchToFastHeat(bool state);
    void slSwitchToProfileHeat(bool state);

    void slShowMODBUSConfig(void);
    void slSaveMODBUSParameters(uint8_t address, uint16_t tempaddr, uint16_t relayaddr, uint16_t pidaddr, uint16_t enanbleaddr, uint16_t setpointaddr);
    void slShowSerialConfig(void);
    void slSaveSerialParameters(uint32_t baud);

    void slStarted(void);
    void slStopped(void);
};

#endif // OVEN_HEATER_H
