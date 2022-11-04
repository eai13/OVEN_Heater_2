#ifndef SERIAL_SETTINGS_H
#define SERIAL_SETTINGS_H

#include <QWidget>

namespace Ui {
class Serial_Settings;
}

class Serial_Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Serial_Settings(QWidget *parent = nullptr);
    ~Serial_Settings();

private:
    Ui::Serial_Settings *ui;

private slots:
    void slApply(void);
    void slCancel(void);

signals:
    void siSendNewParameters(uint32_t baud);
};

#endif // SERIAL_SETTINGS_H
