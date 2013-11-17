#-------------------------------------------------
#
# Project created by QtCreator 2013-05-03T21:55:11
#
#-------------------------------------------------

QT       += core network serialport

QT       -= gui

TARGET = Gateway
CONFIG   += console debug serialport
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcpgateway.cpp \
    clientoven.cpp \
    utils.cpp \
    abstractdevice.cpp \
    rs232device.cpp \
    rs232deviceprivate.cpp \
    handlermessage.cpp

HEADERS += \
    tcpgateway.h \
    clientoven.h \
    utils.h \
    abstractdevice.h \
    rs232device.h \
    rs232deviceprivate.h \
    handlermessage.h

LIBS += -lQtSerialPort

unix {
SOURCES += candevice.cpp
HEADERS += candevice.h
}
