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
    QList<QPointF> profile_lookup_table;

    QMenu * profile_plot_menu = nullptr;
    QMenu * real_plot_menu = nullptr;
    Ui::ProfileHeat *ui;

    void SetRelayLabel(bool state);
    void SetRunningLabel(bool state);

public slots:
    void slRun(void);
    void slStop(void);
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
};

#endif // PROFILEHEAT_H
