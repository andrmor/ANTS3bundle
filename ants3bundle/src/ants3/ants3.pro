
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

QT += qml   #this is for qjsengine

#QT += core5compat

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
INCLUDEPATH += gui/geom
INCLUDEPATH += gui/raster
INCLUDEPATH += gui/materials
INCLUDEPATH += gui/photsim
INCLUDEPATH += geo
INCLUDEPATH += materials
INCLUDEPATH += tools
INCLUDEPATH += particlesim
INCLUDEPATH += photonSim
INCLUDEPATH += photonSim/interfaceRules
INCLUDEPATH += dispatch
INCLUDEPATH += farm
INCLUDEPATH += config
INCLUDEPATH += ../dispatcher
INCLUDEPATH += /usr/include

DESTDIR = ../../bin

SOURCES += \
    ../dispatcher/a3dispatcher.cpp \
    ../dispatcher/a3processhandler.cpp \
    anodesettingsdialog.cpp \
    photonSim/amonitorconfig.cpp \
    geo/a3geometry.cpp \
    geo/ageoobject.cpp \
    geo/ageoshape.cpp \
    geo/ageotype.cpp \
    geo/ageoconsts.cpp \
    geo/amonitor.cpp \
    gui/arootlineconfigurator.cpp \
    gui/geom/a3geoconwin.cpp \
    gui/geom/ageobasedelegate.cpp \
    gui/geom/ageobasetreewidget.cpp \
    gui/geom/ageodelegatewidget.cpp \
    gui/geom/ageometrywindow.cpp \
    gui/geom/ageoobjectdelegate.cpp \
    gui/geom/ageotree.cpp \
    gui/geom/agridelementdelegate.cpp \
    gui/geom/agridelementdialog.cpp \
    gui/geom/amonitordelegate.cpp \
    gui/geom/amonitordelegateform.cpp \
    gui/geom/aonelinetextedit.cpp \
    gui/materials/a3matwin.cpp \
    gui/materials/aelementandisotopedelegates.cpp \
    gui/photsim/a3photsimwin.cpp \
    gui/raster/acameracontroldialog.cpp \
    gui/raster/rasterwindowbaseclass.cpp \
    gui/raster/rasterwindowgraphclass.cpp \
    main.cpp \
    js/a3scriptworker.cpp \
    js/a3scriptmanager.cpp \
    js/a3scriptres.cpp \
    materials/a3mathub.cpp \
    materials/achemicalelement.cpp \
    materials/aisotope.cpp \
    materials/aisotopeabundancehandler.cpp \
    materials/amaterial.cpp \
    materials/amaterialcomposition.cpp \
    particlesim/a3particlesimmanager.cpp \
    dispatch/a3dispinterface.cpp \
    config/a3config.cpp \
    config/a3global.cpp \
    dispatch/a3workdistrconfig.cpp \
    photonSim/acommonfunctions.cpp \
    photonSim/aphoton.cpp \
    photonSim/aroothistappenders.cpp \
    photonSim/asimulationstatistics.cpp \
    photonSim/atracerstateful.cpp \
    photonSim/interfaceRules/a3optinthub.cpp \
    photonSim/interfaceRules/abasicopticaloverride.cpp \
    photonSim/interfaceRules/aopticaloverride.cpp \
    photonSim/interfaceRules/aopticaloverridescriptinterface.cpp \
    photonSim/interfaceRules/ascriptopticaloverride.cpp \
    photonSim/interfaceRules/awaveshifteroverride.cpp \
    photonSim/interfaceRules/fsnpopticaloverride.cpp \
    photonSim/interfaceRules/scatteronmetal.cpp \
    photonSim/interfaceRules/spectralbasicopticaloverride.cpp \
    config/aphotsimsettings.cpp \
    tools/ajsontools.cpp \
    tools/afiletools.cpp \
    gui/mainwindow.cpp \
    gui/guitools.cpp \
    js/a3farmsi.cpp

HEADERS += \
    ../dispatcher/a3dispatcher.h \
    ../dispatcher/a3processhandler.h \
    anodesettingsdialog.h \
    photonSim/amonitorconfig.h \
    geo/a3geometry.h \
    geo/ageoobject.h \
    geo/ageoshape.h \
    geo/ageotype.h \
    geo/ageoconsts.h \
    geo/amonitor.h \
    gui/arootlineconfigurator.h \
    gui/geom/a3geoconwin.h \
    gui/geom/ageobasedelegate.h \
    gui/geom/ageobasetreewidget.h \
    gui/geom/ageodelegatewidget.h \
    gui/geom/ageomarkerclass.h \
    gui/geom/ageometrywindow.h \
    gui/geom/ageoobjectdelegate.h \
    gui/geom/ageotree.h \
    gui/geom/agridelementdelegate.h \
    gui/geom/agridelementdialog.h \
    gui/geom/amonitordelegate.h \
    gui/geom/amonitordelegateform.h \
    gui/geom/aonelinetextedit.h \
    gui/materials/a3matwin.h \
    gui/materials/aelementandisotopedelegates.h \
    gui/photsim/a3photsimwin.h \
    gui/raster/acameracontroldialog.h \
    gui/raster/rasterwindowbaseclass.h \
    gui/raster/rasterwindowgraphclass.h \
    js/a3scriptworker.h \
    js/a3scriptmanager.h \
    js/a3scriptres.h \
    materials/a3mathub.h \
    materials/achemicalelement.h \
    materials/aisotope.h \
    materials/aisotopeabundancehandler.h \
    materials/amaterial.h \
    materials/amaterialcomposition.h \
    particlesim/a3particlesimmanager.h \
    dispatch/a3dispinterface.h \
    config/a3config.h \
    config/a3global.h \
    dispatch/a3workdistrconfig.h \
    farm/a3farmnoderecord.h \
    photonSim/acommonfunctions.h \
    photonSim/aphoton.h \
    photonSim/aroothistappenders.h \
    photonSim/asimulationstatistics.h \
    photonSim/atracerstateful.h \
    photonSim/interfaceRules/a3optinthub.h \
    photonSim/interfaceRules/abasicopticaloverride.h \
    photonSim/interfaceRules/aopticaloverride.h \
    photonSim/interfaceRules/aopticaloverridescriptinterface.h \
    photonSim/interfaceRules/ascriptopticaloverride.h \
    photonSim/interfaceRules/awaveshifteroverride.h \
    photonSim/interfaceRules/fsnpopticaloverride.h \
    photonSim/interfaceRules/scatteronmetal.h \
    photonSim/interfaceRules/spectralbasicopticaloverride.h \
    config/aphotsimsettings.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/mainwindow.h \
    gui/guitools.h \
    js/a3farmsi.h

FORMS += \
        anodesettingsdialog.ui \
        gui/geom/a3geoconwin.ui \
        gui/geom/agridelementdialog.ui \
        gui/geom/amonitordelegateform.ui \
        gui/geom/ageometrywindow.ui \
        gui/mainwindow.ui \
        gui/materials/a3matwin.ui \
        gui/photsim/a3photsimwin.ui \
        gui/raster/acameracontroldialog.ui

RESOURCES += \
    resources.qrc

ants2_WS {
    QT += websockets
    DEFINES += WEBSOCKETS

    SOURCES += \
    ../dispatcher/awebsocketsessionserver.cpp \
    ../dispatcher/awebsocketsession.cpp \
    ../dispatcher/a3remotehandler.cpp \
    ../dispatcher/a3wsclient.cpp

    HEADERS += \
    ../dispatcher/awebsocketsessionserver.h \
    ../dispatcher/awebsocketsession.h \
    ../dispatcher/a3remotehandler.h \
    ../dispatcher/a3wsclient.h \
} else {
    QT -= websockets
}
