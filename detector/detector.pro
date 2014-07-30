TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig
PKGCONFIG += opencv

SOURCES += camshifting.cpp \
    trackface.cpp
HEADERS += camshifting.h \
    trackface.h

