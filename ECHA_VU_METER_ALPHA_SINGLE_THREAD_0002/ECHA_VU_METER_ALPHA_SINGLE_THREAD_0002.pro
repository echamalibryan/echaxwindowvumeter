# (copy of) Echa Corp and Echa Community
# Licensed under GPLv3 (GNU General Public License v3.0)

greaterThan(QT_MAJOR_VERSION, 4) : QT += widgets

QMAKE_CXXFLAGS += `pkg-config --cflags gtk+-2.0`

LIBS += `pkg-config --libs gtk+-2.0` -lpulse -lX11

SOURCES += \
    e.cpp

HEADERS += \
    e.h
