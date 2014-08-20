TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += core gui opengl multimedia

CONFIG += link_pkgconfig
PKGCONFIG += opencv
PKGCONFIG += libusb # For HardwareThunder

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
    detector/detectorparameter.cpp \
    ui/detectorparameterwidget.cpp \
    util.cpp
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
    detector/detectorparameter.h \
    ui/detectorparameterwidget.h \
    util.h

FORMS += \
    sentryui.ui \
    ui/detectorparameterwidget.ui


SOURCES += \
    httpd/httpdaemon.cpp \
    httpd/httpreply.cpp \
    httpd/httprequest.cpp

HEADERS += \
    httpd/httpdaemon.h \
    httpd/httpreply.h \
    httpd/httprequest.h
