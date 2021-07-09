
#CONFIG += ants2_Python      #enable Python scripting
#CONFIG += ants2_RootServer  #enable cern CERN ROOT html server
#CONFIG += ants2_jsroot       #enables JSROOT visualisation at GeometryWindow. Automatically enables ants2_RootServer

CONFIG += ants2_GUI         #if disabled, GUI is not compiled

QT += core
ants2_GUI {
    QT += gui
    QT += widgets
    DEFINES += GUI
} else {
    QT -= gui
}

QT += qml   #this is for jsengine
QT += websockets

CONFIG += c++11

QMAKE_CXXFLAGS += -march=native

TARGET = ants3
TEMPLATE = app

# CERN ROOT
     INCLUDEPATH += $$system(root-config --incdir)
     LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb
     ants2_RootServer {LIBS += -lRHTTP  -lXMLIO}

DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs. In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += js
INCLUDEPATH += gui
INCLUDEPATH += tools
INCLUDEPATH += particlesim
INCLUDEPATH += dispatch
INCLUDEPATH += farm
INCLUDEPATH += config
INCLUDEPATH += ../dispatcher

DESTDIR = ../../bin #ignored in the meta project

SOURCES += \
    ../dispatcher/a3dispatcher.cpp \
    ../dispatcher/awebsocketsessionserver.cpp \
    ../dispatcher/awebsocketsession.cpp \
    ../dispatcher/a3processhandler.cpp \
    ../dispatcher/a3remotehandler.cpp \
    ../dispatcher/a3wsclient.cpp \
    main.cpp \
    js/a3scriptworker.cpp \
    js/a3scriptmanager.cpp \
    js/a3scriptres.cpp \
    particlesim/a3particlesimmanager.cpp \
    dispatch/a3dispinterface.cpp \
    config/a3config.cpp \
    config/a3global.cpp \
    config/a3workdistrconfig.cpp \
    tools/ajsontools.cpp \
    tools/afiletools.cpp \
    gui/mainwindow.cpp \
    gui/guitools.cpp \
    js/a3farmsi.cpp

HEADERS += \
    ../dispatcher/a3dispatcher.h \
    ../dispatcher/awebsocketsessionserver.h \
    ../dispatcher/awebsocketsession.h \
    ../dispatcher/a3processhandler.h \
    ../dispatcher/a3remotehandler.h \
    ../dispatcher/a3wsclient.h \
    js/a3scriptworker.h \
    js/a3scriptmanager.h \
    js/a3scriptres.h \
    particlesim/a3particlesimmanager.h \
    dispatch/a3dispinterface.h \
    config/a3config.h \
    config/a3global.h \
    config/a3workdistrconfig.h \
    farm/a3farmnoderecord.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/mainwindow.h \
    gui/guitools.h \
    js/a3farmsi.h

FORMS += \
        gui/mainwindow.ui
