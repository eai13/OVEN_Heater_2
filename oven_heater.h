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

QT_BEGIN_NAMESPACE
namespace Ui { class OVEN_Heater; }
QT_END_NAMESPACE

class OVEN_Heater : public QMainWindow
{
    Q_OBJECT

public:
    OVEN_Heater(QWidget *parent = nullptr);
    ~OVEN_Heater();

private:
    QString filename = "";

    QWidget * handle_widget = nullptr;
    FastHeat * fast_heat = nullptr;
    ProfileHeat * profile_heat = nullptr;
    Ui::OVEN_Heater *ui;

private slots:
    void slSaveExperiment(void);
    void slUploadExperiment(void);
    void slSwitchToFastHeat(bool state);
    void slSwitchToProfileHeat(bool state);
};
#endif // OVEN_HEATER_H
