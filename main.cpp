#include "oven_heater.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OVEN_Heater w;
    w.show();
    return a.exec();
}
