QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport serialport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fastheat.cpp \
    main.cpp \
    modbus_settings.cpp \
    oven_heater.cpp \
    profileheat.cpp \
    qcustomplot.cpp \
    serial_settings.cpp

HEADERS += \
    fastheat.h \
    modbus_settings.h \
    oven_heater.h \
    profileheat.h \
    qcustomplot.h \
    serial_settings.h

FORMS += \
    fastheat.ui \
    modbus_settings.ui \
    oven_heater.ui \
    profileheat.ui \
    serial_settings.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rec_icons.qrc
