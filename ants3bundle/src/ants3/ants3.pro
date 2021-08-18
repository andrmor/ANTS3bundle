
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
INCLUDEPATH += gui/graph
INCLUDEPATH += geo
INCLUDEPATH += materials
INCLUDEPATH += tools
INCLUDEPATH += particleSim
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
    geo/ageospecial.cpp \
    geo/amonitorhub.cpp \
    gui/arootmarkerconfigurator.cpp \
    gui/aroottextconfigurator.cpp \
    gui/graph/aaxesdialog.cpp \
    gui/graph/abasketitem.cpp \
    gui/graph/abasketlistwidget.cpp \
    gui/graph/abasketmanager.cpp \
    gui/graph/adrawexplorerwidget.cpp \
    gui/graph/adrawobject.cpp \
    gui/graph/adrawtemplate.cpp \
    gui/graph/alegenddialog.cpp \
    gui/graph/alinemarkerfilldialog.cpp \
    gui/graph/amultigraphdesigner.cpp \
    gui/graph/apadgeometry.cpp \
    gui/graph/apadproperties.cpp \
    gui/graph/arootcolorselectordialog.cpp \
    gui/graph/atemplateselectiondialog.cpp \
    gui/graph/atemplateselectionrecord.cpp \
    gui/graph/atextpavedialog.cpp \
    gui/graph/atoolboxscene.cpp \
    gui/graph/graphicsruler.cpp \
    gui/graph/graphwindowclass.cpp \
    gui/graph/shapeablerectitem.cpp \
    gui/photsim/abombadvanceddialog.cpp \
    gui/photsim/ainterfaceruledialog.cpp \
    gui/photsim/ainterfacerulewin.cpp \
    gui/photsim/ainterfacewidgetfactory.cpp \
    gui/photsim/aphotonsimoutputdialog.cpp \
    photonSim/aphotonsimhub.cpp \
    geo/ageometryhub.cpp \
    materials/amaterialhub.cpp \
    photonSim/amonitorconfig.cpp \
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
    materials/achemicalelement.cpp \
    materials/aisotope.cpp \
    materials/aisotopeabundancehandler.cpp \
    materials/amaterial.cpp \
    materials/amaterialcomposition.cpp \
    particleSim/a3particlesimmanager.cpp \
    dispatch/a3dispinterface.cpp \
    config/a3config.cpp \
    config/a3global.cpp \
    dispatch/a3workdistrconfig.cpp \
    photonSim/acommonfunctions.cpp \
    photonSim/aphoton.cpp \
    photonSim/aphotonsimmanager.cpp \
    photonSim/aphotonsimsettings.cpp \
    photonSim/aphotonstatistics.cpp \
    tools/ahistogram.cpp \
    tools/aroothistappenders.cpp \
    photonSim/asensorhub.cpp \
    photonSim/asensormodel.cpp \
    photonSim/astatisticshub.cpp \
    photonSim/interfaceRules/abasicinterfacerule.cpp \
    photonSim/interfaceRules/ainterfacerule.cpp \
    photonSim/interfaceRules/ainterfacerulehub.cpp \
    photonSim/interfaceRules/ametalinterfacerule.cpp \
    photonSim/interfaceRules/aspectralbasicinterfacerule.cpp \
    photonSim/interfaceRules/awaveshifterinterfacerule.cpp \
    photonSim/interfaceRules/fsnpinterfacerule.cpp \
    tools/afilemerger.cpp \
    tools/agraphbuilder.cpp \
    tools/ajsontools.cpp \
    tools/afiletools.cpp \
    gui/mainwindow.cpp \
    gui/guitools.cpp \
    js/a3farmsi.cpp \
    tools/arandomhub.cpp \
    tools/avector.cpp

HEADERS += \
    ../dispatcher/a3dispatcher.h \
    ../dispatcher/a3processhandler.h \
    geo/ageospecial.h \
    geo/amonitorhub.h \
    gui/arootmarkerconfigurator.h \
    gui/aroottextconfigurator.h \
    gui/graph/aaxesdialog.h \
    gui/graph/abasketitem.h \
    gui/graph/abasketlistwidget.h \
    gui/graph/abasketmanager.h \
    gui/graph/adrawexplorerwidget.h \
    gui/graph/adrawobject.h \
    gui/graph/adrawtemplate.h \
    gui/graph/alegenddialog.h \
    gui/graph/alinemarkerfilldialog.h \
    gui/graph/amultigraphdesigner.h \
    gui/graph/apadgeometry.h \
    gui/graph/apadproperties.h \
    gui/graph/arootcolorselectordialog.h \
    gui/graph/atemplateselectiondialog.h \
    gui/graph/atemplateselectionrecord.h \
    gui/graph/atextpavedialog.h \
    gui/graph/atlegend.h.autosave \
    gui/graph/atoolboxscene.h \
    gui/graph/graphicsruler.h \
    gui/graph/graphwindowclass.h \
    gui/graph/shapeablerectitem.h \
    gui/photsim/abombadvanceddialog.h \
    gui/photsim/ainterfaceruledialog.h \
    gui/photsim/ainterfacerulewin.h \
    gui/photsim/ainterfacewidgetfactory.h \
    gui/photsim/aphotonsimoutputdialog.h \
    photonSim/aphotonsimhub.h \
    geo/ageometryhub.h \
    materials/amaterialhub.h \
    photonSim/amonitorconfig.h \
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
    materials/achemicalelement.h \
    materials/aisotope.h \
    materials/aisotopeabundancehandler.h \
    materials/amaterial.h \
    materials/amaterialcomposition.h \
    particleSim/a3particlesimmanager.h \
    dispatch/a3dispinterface.h \
    config/a3config.h \
    config/a3global.h \
    dispatch/a3workdistrconfig.h \
    farm/a3farmnoderecord.h \
    photonSim/acommonfunctions.h \
    photonSim/aphoton.h \
    photonSim/aphotonsimmanager.h \
    photonSim/aphotonsimsettings.h \
    photonSim/aphotonstatistics.h \
    photonSim/aphotontrackrecord.h \
    tools/ahistogram.h \
    tools/aroothistappenders.h \
    photonSim/asensorhub.h \
    photonSim/asensormodel.h \
    photonSim/astatisticshub.h \
    photonSim/interfaceRules/abasicinterfacerule.h \
    photonSim/interfaceRules/ainterfacerule.h \
    photonSim/interfaceRules/ainterfacerulehub.h \
    photonSim/interfaceRules/ametalinterfacerule.h \
    photonSim/interfaceRules/aspectralbasicinterfacerule.h \
    photonSim/interfaceRules/awaveshifterinterfacerule.h \
    photonSim/interfaceRules/fsnpinterfacerule.h \
    tools/afilemerger.h \
    tools/agraphbuilder.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/mainwindow.h \
    gui/guitools.h \
    js/a3farmsi.h \
    tools/arandomhub.h \
    tools/avector.h

FORMS += \
        gui/aroottextconfigurator.ui \
        gui/graph/aaxesdialog.ui \
        gui/graph/alegenddialog.ui \
        gui/graph/alinemarkerfilldialog.ui \
        gui/graph/amultigraphdesigner.ui \
        gui/graph/arootcolorselectordialog.ui \
        gui/graph/atemplateselectiondialog.ui \
        gui/graph/atextpavedialog.ui \
        gui/graph/graphwindowclass.ui \
        gui/photsim/abombadvanceddialog.ui \
        gui/geom/a3geoconwin.ui \
        gui/geom/agridelementdialog.ui \
        gui/geom/amonitordelegateform.ui \
        gui/geom/ageometrywindow.ui \
        gui/mainwindow.ui \
        gui/materials/a3matwin.ui \
        gui/photsim/a3photsimwin.ui \
        gui/photsim/ainterfacerulewin.ui \
        gui/photsim/ainterfaceruledialog.ui \
        gui/photsim/aphotonsimoutputdialog.ui \
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
