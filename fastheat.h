#ifndef FASTHEAT_H
#define FASTHEAT_H

#include <QWidget>
#include <QMenu>
#include <QCursor>
#include <QTimer>

namespace Ui {
class FastHeat;
}

class FastHeat : public QWidget
{
    Q_OBJECT

public:
    explicit FastHeat(QWidget *parent = nullptr);
    ~FastHeat(void);

private:
    enum GUI_ENABLE_STATE{
        ENB_RUN     = 0x01,
        ENB_STOP    = 0x02
    };

    void GUISetEnabled(GUI_ENABLE_STATE state);

    QMenu * plot_menu = nullptr;
    Ui::FastHeat *ui;

    void SetRelayLabel(bool state);
    void SetRunningLabel(bool state);

//    QTimer * UpdateCurrentTempTimer = nullptr;
//    QTimer * SendSetpointTimer = nullptr;

public slots:
    void slReceiveTemp(float temp);
    void slReceiveRelay(uint16_t relay);
    void slRun(void);
    void slStop(void);
    void slSaveImage(void);
    void slSaveData(void);
    void slSaveAll(void);
    void slShowPlotMenu(const QPoint & pos);

//    void slUpdateCurrentTemp(void);
//    void slSendSetpoint(void);

signals:
    void siSendEnable(uint16_t enb);
    void siSendPID(uint16_t PID);
    void siSendSetPoint(uint16_t sp);
    void siStarted(void);
    void siStopped(void);
};

#endif // FASTHEAT_H
