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
        ENB_STATE_FULL_BLOCK    = 0x00,
        ENB_RUN                 = 0x01,
        ENB_STOP                = 0x02
    };
    enum PID_NUMBER{
        PID_RAMP        = 0x01,
        PID_P_PLAIN     = 0x02,
        PID_PI_PLAIN    = 0x03
    };

    float temperature = 0;
    bool is_at_plane = false;

    bool is_connected = false;
    void GUISetEnabled(GUI_ENABLE_STATE state);

    QTime *     global_time = nullptr;
    QTimer *    run_timer = nullptr;

    QMenu * plot_menu = nullptr;
    Ui::FastHeat *ui;

    void SetRelayLabel(bool state);
    void SetRunningLabel(bool state);

private slots:
    void slRunTimerProcess(void);

public slots:
    void slReceiveTemp(float temp);
    void slReceiveRelay(uint16_t relay);
    void slRun(void);
    void slStop(void);
    void slDisconnectionStop(void);
    void slConnected(void);
    void slSaveImage(void);
    void slSaveData(void);
    void slSaveAll(void);
    void slShowPlotMenu(const QPoint & pos);

signals:
    void siSendEnable(uint16_t enb);
    void siSendPID(uint16_t PID);
    void siSendSetPoint(uint16_t sp);
    void siStarted(void);
    void siStopped(void);
};

#endif // FASTHEAT_H
