#ifndef PROFILEHEAT_H
#define PROFILEHEAT_H

#include <QWidget>
#include <QMenu>
#include <QPointF>
#include <qmath.h>

namespace Ui {
class ProfileHeat;
}

class ProfileHeat : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileHeat(QString from_file = "", QWidget * parent = nullptr);
    ~ProfileHeat();

private:
    enum GUI_ENABLE_STATE{
        ENB_STATE_FULL_BLOCK    = 0x00,
        ENB_START_TEMP_NOT_SET  = 0x01,
        ENB_START_TEMP_SET      = 0x02,
        ENB_RUN                 = 0x03
    };
    enum PID_NUMBER{
        PID_RAMP        = 0x01,
        PID_P_PLAIN     = 0x02,
        PID_PI_PLAIN    = 0x03
    };

    void GUISetEnabled(GUI_ENABLE_STATE state);

    QList<QPointF> profile_lookup_table;
    QList<QPointF> profile_discrete;

    bool        is_connected            = false;
    bool        is_at_plain             = false;
    bool        is_prepared             = false;
    float       prev_set_temperature    = 0;
    float       real_temperature        = 0;
    QTime *     global_time             = nullptr;
    QTimer *    run_timer               = nullptr;

    QMenu * profile_plot_menu           = nullptr;
    QMenu * real_plot_menu              = nullptr;
    Ui::ProfileHeat *ui;

    void SetRelayLabel(bool state);
    void SetRunningLabel(bool state);

private slots:
    void slRunProcessCallback(void);
    void slEditFinishedSpeed(void);
    void slEditFinishedTemp(void);

public slots:
    void slReceiveTemp(float temp);
    void slReceiveRelay(uint16_t relay);
    void slRun(void);
    void slStop(void);
    void slDisconnectionStop(void);
    void slConnected(void);
    void slSaveRealImage(void);
    void slSaveRealData(void);
    void slSaveProfileImage(void);
    void slSaveProfileData(void);
    void slSaveAll(void);
    void slShowRealPlotMenu(const QPoint & pos);
    void slShowProfilePlotMenu(const QPoint & pos);

    void slSetStartingTemperature(void);
    void slAddProfilePoint(void);
    void slRemoveProfilePoint(void);
    void slClearProfile(void);

public:
    void SaveExperiment(QString filename);

signals:
    void siStarted(void);
    void siStopped(void);
    void siSendEnable(uint16_t enb);
    void siSendPID(uint16_t PID);
    void siSendSetPoint(float sp);
};

#endif // PROFILEHEAT_H
