# optional features
CONFIG += ants3_FARM         #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled

QT -= gui
CONFIG += console
CONFIG -= app_bundle

lessThan(QT_MAJOR_VERSION, 6) {
    CONFIG += c++11
} else {
    CONFIG += c++17
    #Do we really need ROOT here? !!!***
    # ROOT has to be compiled with c++17 too!!!!
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# CERN ROOT  - needed for jstools only? !!!***
     INCLUDEPATH += $$system(root-config --incdir)
     LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb

DESTDIR = ../../bin

INCLUDEPATH += ../ants3/dispatch
INCLUDEPATH += ../ants3/config
INCLUDEPATH += ../ants3/tools

SOURCES += main.cpp \
    a3dispatcher.cpp \
    a3processhandler.cpp \
    ../ants3/dispatch/a3workdistrconfig.cpp \
    ../ants3/tools/ajsontools.cpp \
    ../ants3/tools/afiletools.cpp

HEADERS += \
    a3dispatcher.h \
    a3processhandler.h \
    ../ants3/dispatch/a3workdistrconfig.h \
    ../ants3/farm/a3farmnoderecord.h \
    ../ants3/tools/ajsontools.h \
    ../ants3/tools/afiletools.h

ants3_FARM {
    QT += websockets
    DEFINES += WEBSOCKETS

    SOURCES += \
        awebsocketsessionserver.cpp \
        awebsocketsession.cpp \
        a3remotehandler.cpp \
        a3wsclient.cpp

    HEADERS += \
        awebsocketsessionserver.h \
        awebsocketsession.h \
        a3remotehandler.h \
        a3wsclient.h

} else {
    QT -= websockets
}
