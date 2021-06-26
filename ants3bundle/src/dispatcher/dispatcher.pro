QT -= gui
QT += websockets

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../bin

INCLUDEPATH += ../ants3/dispatch
INCLUDEPATH += ../ants3/config
INCLUDEPATH += ../ants3/tools

SOURCES += main.cpp \
    a3dispatcher.cpp \
    awebsocketsessionserver.cpp \
    awebsocketsession.cpp \
    ../ants3/dispatch/a3processhandler.cpp \
    ../ants3/config/a3workdistrconfig.cpp \
    ../ants3/tools/ajsontools.cpp \
    ../ants3/tools/afiletools.cpp \
    a3remotehandler.cpp \
    a3wsclient.cpp

HEADERS += \
    a3dispatcher.h \
    awebsocketsessionserver.h \
    awebsocketsession.h \
    ../ants3/dispatch/a3processhandler.h \
    ../ants3/config/a3workdistrconfig.h \
    ../ants3/tools/ajsontools.h \
    ../ants3/tools/afiletools.h \
    a3remotehandler.h \
    a3wsclient.h
