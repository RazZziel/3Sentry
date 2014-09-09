TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += core gui opengl multimedia serialport

CONFIG += link_pkgconfig
PKGCONFIG += opencv
PKGCONFIG += libusb # For HardwareThunder
PKGCONFIG += sdl2

SOURCES += \
    main.cpp \
    controller.cpp \
    hardware/hardware.cpp \
    hardware/hardwareemulator.cpp \
    hardware/hardwarearduino.cpp \
    hardware/hardwarethunder.cpp \
    detector/detector.cpp \
    detector/movementdetector.cpp \
    detector/colordetector.cpp \
    detector/cascadeclassifierdetector.cpp \
    detector/facedetector.cpp \
    detector/bodydetector.cpp \
    ui/cqtopencvviewergl.cpp \
    sentryui.cpp \
    audio.cpp \
    sentryweb.cpp \
    sentryinput.cpp \
    util.cpp \
    parameter.cpp \
    ui/parameterwidget.cpp \
    parametermanager.cpp
HEADERS += \
    controller.h \
    hardware/hardware.h \
    hardware/hardwareemulator.h \
    hardware/hardwarearduino.h \
    hardware/hardwarethunder.h \
    detector/detector.h \
    detector/movementdetector.h \
    detector/colordetector.h \
    detector/cascadeclassifierdetector.h \
    detector/facedetector.h \
    detector/bodydetector.h \
    ui/cqtopencvviewergl.h \
    sentryui.h \
    audio.h \
    sentryweb.h \
    sentryinput.h \
    util.h \
    parameter.h \
    ui/parameterwidget.h \
    parametermanager.h \
    profilingdata.h

FORMS += \
    sentryui.ui \
    ui/parameterwidget.ui


SOURCES += \
    httpd/httpdaemon.cpp \
    httpd/httpreply.cpp \
    httpd/httprequest.cpp

HEADERS += \
    httpd/httpdaemon.h \
    httpd/httpreply.h \
    httpd/httprequest.h
