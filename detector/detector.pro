TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += core gui opengl multimedia

CONFIG += link_pkgconfig
PKGCONFIG += opencv

SOURCES += \
    main.cpp \
    trackface/camshifting.cpp \
    trackface/trackface.cpp \
    controller.cpp \
    hardware.cpp \
    detector/detector.cpp \
    detector/movementdetector.cpp \
    detector/colordetector.cpp \
    detector/facedetector.cpp \
    detector/bodydetector.cpp \
    ui/cqtopencvviewergl.cpp \
    sentryui.cpp \
    audio.cpp \
    webui.cpp
HEADERS += \
    trackface/camshifting.h \
    trackface/trackface.h \
    controller.h \
    hardware.h \
    detector/detector.h \
    detector/movementdetector.h \
    detector/colordetector.h \
    detector/facedetector.h \
    detector/bodydetector.h \
    ui/cqtopencvviewergl.h \
    sentryui.h \
    audio.h \
    webui.h

FORMS += \
    sentryui.ui


SOURCES += \
    httpd/httpdaemon.cpp \
    httpd/httpreply.cpp \
    httpd/httprequest.cpp

HEADERS += \
    httpd/httpdaemon.h \
    httpd/httpreply.h \
    httpd/httprequest.h
