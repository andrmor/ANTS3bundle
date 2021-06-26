
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
# couldn't get of widgets yet due to funny compile erors - VS
QT += qml
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

DESTDIR = ../../bin

SOURCES += \
    main.cpp \
    js/a3scriptworker.cpp \
    js/a3scriptmanager.cpp \
    js/a3scriptres.cpp \
    particlesim/a3particlesimmanager.cpp \
    dispatch/a3dispinterface.cpp \
    dispatch/a3processhandler.cpp \
    config/a3config.cpp \
    tools/ajsontools.cpp \
    tools/afiletools.cpp \
    gui/mainwindow.cpp \
    gui/guitools.cpp \
    config/a3global.cpp \
    config/a3workdistrconfig.cpp \
    a3farmsi.cpp

HEADERS += \
    js/a3scriptworker.h \
    js/a3scriptmanager.h \
    js/a3scriptres.h \
    particlesim/a3particlesimmanager.h \
    dispatch/a3dispinterface.h \
    dispatch/a3processhandler.h \
    config/a3config.h \
    farm/a3farmnoderecord.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/mainwindow.h \
    gui/guitools.h \
    config/a3global.h \
    config/a3workdistrconfig.h \
    a3farmsi.h

FORMS += \
        gui/mainwindow.ui
