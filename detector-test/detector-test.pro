#-------------------------------------------------
#
# Project created by QtCreator 2014-08-10T10:23:14
#
#-------------------------------------------------

QT       += testlib gui

TARGET = tst_hardwareemulator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += opencv

DEFINES += SRCDIR=\\\"$$PWD/\\\"
INCLUDEPATH += $$PWD/../detector/hardware/

SOURCES += tst_hardwareemulator.cpp

SOURCES += \
    ../detector/hardware/hardware.cpp \
    ../detector/hardware/hardwareemulator.cpp

HEADERS += \
    ../detector/hardware/hardware.h \
    ../detector/hardware/hardwareemulator.h
