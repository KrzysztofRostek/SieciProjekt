QT += core gui charts
QT += core network
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += headers

SOURCES += \
    PakietLicz.cpp \
    mytcpclient.cpp \
    mytcpserwer.cpp \
    sieciowka.cpp \
    src/arx.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/ModelARX.cpp \
    src/ProstyUAR.cpp \
    src/RegulatorPID.cpp \
    src/ConfigManager.cpp \
    src/Generator.cpp \
    src/Kontroler.cpp


HEADERS += \
    Network.h \
    PakietLicz.h \
    headers/arx.h \
    headers/mainwindow.h \
    headers/ModelARX.h \
    headers/ProstyUAR.h \
    headers/Regulator.h \
    headers/RegulatorPID.h \
    headers/ConfigManager.h \
    headers/Generator.h \
    headers/Kontroler.h \
    mytcpclient.h \
    mytcpserwer.h \
    sieciowka.h
FORMS += \
    forms/arx.ui \
    forms/mainwindow.ui \
    sieciowka.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
