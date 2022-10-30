#ifndef FASTHEAT_H
#define FASTHEAT_H

#include <QWidget>
#include <QMenu>
#include <QCursor>

namespace Ui {
class FastHeat;
}

class FastHeat : public QWidget
{
    Q_OBJECT

public:
    explicit FastHeat(QWidget *parent = nullptr);
    ~FastHeat();

private:
    QMenu * plot_menu = nullptr;
    Ui::FastHeat *ui;

    void SetRelayLabel(bool state);
    void SetRunningLabel(bool state);

public slots:
    void slRun(void);
    void slStop(void);
    void slSaveImage(void);
    void slSaveData(void);
    void slSaveAll(void);
    void slShowPlotMenu(const QPoint & pos);
};

#endif // FASTHEAT_H
