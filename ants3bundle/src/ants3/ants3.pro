# optional features
CONFIG += ants3_GUI          #if commented away, GUI is not compiled
CONFIG += ants3_FARM         #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled

#CONFIG += ants2_Python      #enable Python scripting
#CONFIG += ants2_RootServer  #enable cern CERN ROOT html server
#CONFIG += ants2_jsroot       #enables JSROOT visualisation at GeometryWindow. Automatically enables ants2_RootServer

# CERN ROOT
INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb
#ants2_RootServer {LIBS += -lRHTTP  -lXMLIO}

QT += core
ants3_GUI {
    QT += gui
    QT += widgets
    DEFINES += GUI
} else {
    QT -= gui
}

QT += qml   #this is for qjsengine

#QT += core5compat

lessThan(QT_MAJOR_VERSION, 6) {
    CONFIG += c++11
} else {
    CONFIG += c++17
    # ROOT has to be compiled with c++17 too!!!!
}

QMAKE_CXXFLAGS += -march=native

TARGET = ants3
TEMPLATE = app

DEFINES += QT

DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs. In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += TARGET_DIR=\"\\\"$${OUT_PWD}\\\"\"

INCLUDEPATH += js
INCLUDEPATH += js/ScriptInterfaces
INCLUDEPATH += gui
INCLUDEPATH += gui/geom
INCLUDEPATH += gui/raster
INCLUDEPATH += gui/materials
INCLUDEPATH += gui/photsim
INCLUDEPATH += gui/graph
INCLUDEPATH += gui/farm
INCLUDEPATH += gui/particleSim
INCLUDEPATH += gui/script
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
INCLUDEPATH += ../lsim
INCLUDEPATH += /usr/include

DESTDIR = ../../bin

SOURCES += \
    ../dispatcher/a3dispatcher.cpp \
    ../dispatcher/a3processhandler.cpp \
    ../lsim/anoderecord.cpp \
    geo/acalorimeter.cpp \
    geo/acalorimeterhub.cpp \
    gui/aglobsetwindow.cpp \
    gui/aguiwindow.cpp \
    gui/alineeditwithescape.cpp \
    gui/geom/ageoconstexpressiondialog.cpp \
    gui/geom/ashownumbersdialog.cpp \
    gui/materials/amatwin.cpp \
    gui/particleSim/aparticlesourceplotter.cpp \
    gui/particleSim/atrackdrawdialog.cpp \
    gui/photsim/asensordrawwidget.cpp \
    config/aconfig.cpp \
    dispatch/adispatcherinterface.cpp \
    farm/afarmhub.cpp \
    geo/ageospecial.cpp \
    geo/amonitorhub.cpp \
    gui/ademowindow.cpp \
    gui/aproxystyle.cpp \
    gui/arootmarkerconfigurator.cpp \
    gui/aroottextconfigurator.cpp \
    gui/farm/aremotewindow.cpp \
    gui/farm/aserverdelegate.cpp \
    gui/geom/ageometrytester.cpp \
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
    gui/particleSim/aparticlesimoutputdialog.cpp \
    gui/particleSim/aparticlesimwin.cpp \
    gui/particleSim/aparticlesourcedialog.cpp \
    gui/photsim/abombadvanceddialog.cpp \
    gui/photsim/ainterfaceruledialog.cpp \
    gui/photsim/ainterfacerulewin.cpp \
    gui/photsim/ainterfacewidgetfactory.cpp \
    gui/photsim/aphotonsimoutputdialog.cpp \
    ademomanager.cpp \
    gui/photsim/asensorgview.cpp \
    gui/photsim/asensorwindow.cpp \
    gui/script/ageoscriptmaker.cpp \
    gui/script/ahighlighters.cpp \
    gui/script/ascriptbook.cpp \
    gui/script/ascriptwindow.cpp \
    gui/script/atabrecord.cpp \
    gui/script/atextedit.cpp \
    gui/script/atextoutputwindow.cpp \
    js/ScriptInterfaces/ageo_si.cpp \
    js/ScriptInterfaces/ageowin_si.cpp \
    js/ScriptInterfaces/agraphwin_si.cpp \
    js/ScriptInterfaces/amsg_si.cpp \
    js/ScriptInterfaces/aparticlesim_si.cpp \
    js/ScriptInterfaces/asensor_si.cpp \
    js/ajscripthub.cpp \
    js/ajscriptmanager.cpp \
    js/ajscriptworker.cpp \
    js/apeakfinder.cpp \
    js/arootgraphrecord.cpp \
    js/aroothistrecord.cpp \
    js/arootobjbase.cpp \
    js/arootobjcollection.cpp \
    js/aroottreerecord.cpp \
    js/ascriptobjstore.cpp \
    js/avarrecordbase.cpp \
    js/ScriptInterfaces/aconfig_si.cpp \
    js/ScriptInterfaces/acore_si.cpp \
    js/ScriptInterfaces/ademo_si.cpp \
    js/ScriptInterfaces/afarm_si.cpp \
    js/ScriptInterfaces/ahist_si.cpp \
    js/ScriptInterfaces/amath_si.cpp \
    js/ScriptInterfaces/aminijs_si.cpp \
    js/ScriptInterfaces/apartanalysis_si.cpp \
    js/ScriptInterfaces/aphotonsim_si.cpp \
    js/ScriptInterfaces/ascriptinterface.cpp \
    js/ScriptInterfaces/ascriptminimizerbase.cpp \
    js/ScriptInterfaces/atrackrec_si.cpp \
    js/ScriptInterfaces/agraph_si.cpp \
    js/ScriptInterfaces/atree_si.cpp \
    js/ScriptInterfaces/awindowinterfacebase.cpp \
    particleSim/acalsettings.cpp \
    particleSim/aeventtrackingrecord.cpp \
    particleSim/afilegeneratorsettings.cpp \
    particleSim/afileparticlegenerator.cpp \
    particleSim/ag4simulationsettings.cpp \
    particleSim/amonitorsettings.cpp \
    particleSim/aparticlerecord.cpp \
    particleSim/aparticlerunsettings.cpp \
    particleSim/aparticlesimhub.cpp \
    particleSim/aparticlesimmanager.cpp \
    particleSim/aparticlesimsettings.cpp \
    particleSim/aparticlesourcerecord.cpp \
    particleSim/aparticletrackvisuals.cpp \
    particleSim/asourcegeneratorsettings.cpp \
    particleSim/asourceparticlegenerator.cpp \
    particleSim/atrackingdataimporter.cpp \
    particleSim/atrackinghistorycrawler.cpp \
    photonSim/adeporecord.cpp \
    photonSim/adepositionfilehandler.cpp \
    photonSim/afilehandlerbase.cpp \
    photonSim/afilesettingsbase.cpp \
    photonSim/aphotonbombfilehandler.cpp \
    photonSim/aphotonfilehandler.cpp \
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
    gui/materials/aelementandisotopedelegates.cpp \
    gui/photsim/a3photsimwin.cpp \
    gui/raster/acameracontroldialog.cpp \
    gui/raster/rasterwindowbaseclass.cpp \
    gui/raster/rasterwindowgraphclass.cpp \
    main.cpp \
    materials/achemicalelement.cpp \
    materials/aisotope.cpp \
    materials/aisotopeabundancehandler.cpp \
    materials/amaterial.cpp \
    materials/amaterialcomposition.cpp \
    config/a3global.cpp \
    dispatch/a3workdistrconfig.cpp \
    photonSim/acommonfunctions.cpp \
    photonSim/aphoton.cpp \
    photonSim/aphotonsimmanager.cpp \
    photonSim/aphotonsimsettings.cpp \
    photonSim/aphotonstatistics.cpp \
    photonSim/asensorsignalarray.cpp \
    tools/aerrorhub.cpp \
    tools/ageowriter.cpp \
    tools/ahistogram.cpp \
    tools/ajsontoolsroot.cpp \
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
    tools/arandomhub.cpp \
    tools/ath.cpp \
    tools/athreadpool.cpp \
    tools/avector.cpp

HEADERS += \
    ../dispatcher/a3dispatcher.h \
    ../dispatcher/a3processhandler.h \
    ../lsim/anoderecord.h \
    ademomanager.h \
    geo/acalorimeter.h \
    geo/acalorimeterhub.h \
    gui/aglobsetwindow.h \
    gui/aguiwindow.h \
    gui/alineeditwithescape.h \
    gui/geom/ageoconstexpressiondialog.h \
    gui/geom/ashownumbersdialog.h \
    gui/materials/amatwin.h \
    gui/particleSim/aparticlesourceplotter.h \
    gui/particleSim/atrackdrawdialog.h \
    gui/photsim/asensordrawwidget.h \
    config/aconfig.h \
    dispatch/adispatcherinterface.h \
    farm/afarmhub.h \
    geo/ageospecial.h \
    geo/amonitorhub.h \
    gui/ademowindow.h \
    gui/aproxystyle.h \
    gui/arootmarkerconfigurator.h \
    gui/aroottextconfigurator.h \
    gui/farm/aremotewindow.h \
    gui/farm/aserverdelegate.h \
    gui/geom/ageometrytester.h \
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
    gui/particleSim/aparticlesimoutputdialog.h \
    gui/particleSim/aparticlesimwin.h \
    gui/particleSim/aparticlesourcedialog.h \
    gui/photsim/abombadvanceddialog.h \
    gui/photsim/ainterfaceruledialog.h \
    gui/photsim/ainterfacerulewin.h \
    gui/photsim/ainterfacewidgetfactory.h \
    gui/photsim/aphotonsimoutputdialog.h \
    gui/photsim/asensorgview.h \
    gui/photsim/asensorwindow.h \
    gui/script/ageoscriptmaker.h \
    gui/script/ahighlighters.h \
    gui/script/ascriptbook.h \
    gui/script/ascriptlanguageenum.h \
    gui/script/ascriptwindow.h \
    gui/script/atabrecord.h \
    gui/script/atextedit.h \
    gui/script/atextoutputwindow.h \
    js/ScriptInterfaces/ageo_si.h \
    js/ScriptInterfaces/ageowin_si.h \
    js/ScriptInterfaces/agraphwin_si.h \
    js/ScriptInterfaces/amsg_si.h \
    js/ScriptInterfaces/aparticlesim_si.h \
    js/ScriptInterfaces/asensor_si.h \
    js/arootgraphrecord.h \
    js/aroothistrecord.h \
    js/arootobjbase.h \
    js/arootobjcollection.h \
    js/aroottreerecord.h \
    js/ajscripthub.h \
    js/ajscriptmanager.h \
    js/ajscriptworker.h \
    js/apeakfinder.h \
    js/ascriptobjstore.h \
    js/avarrecordbase.h \
    js/ScriptInterfaces/aconfig_si.h \
    js/ScriptInterfaces/acore_si.h \
    js/ScriptInterfaces/ademo_si.h \
    js/ScriptInterfaces/afarm_si.h \
    js/ScriptInterfaces/ahist_si.h \
    js/ScriptInterfaces/amath_si.h \
    js/ScriptInterfaces/aminijs_si.h \
    js/ScriptInterfaces/apartanalysis_si.h \
    js/ScriptInterfaces/aphotonsim_si.h \
    js/ScriptInterfaces/ascriptinterface.h \
    js/ScriptInterfaces/ascriptminimizerbase.h \
    js/ScriptInterfaces/atrackrec_si.h \
    js/ScriptInterfaces/agraph_si.h \
    js/ScriptInterfaces/atree_si.h \
    js/ScriptInterfaces/awindowinterfacebase.h \
    particleSim/acalsettings.h \
    particleSim/aeventtrackingrecord.h \
    particleSim/afilegeneratorsettings.h \
    particleSim/afileparticlegenerator.h \
    particleSim/ag4simulationsettings.h \
    particleSim/amonitorsettings.h \
    particleSim/aparticlegun.h \
    particleSim/aparticlerecord.h \
    particleSim/aparticlerunsettings.h \
    particleSim/aparticlesimhub.h \
    particleSim/aparticlesimmanager.h \
    particleSim/aparticlesimsettings.h \
    particleSim/aparticlesourcerecord.h \
    particleSim/aparticletrackvisuals.h \
    particleSim/asourcegeneratorsettings.h \
    particleSim/asourceparticlegenerator.h \
    particleSim/atrackingdataimporter.h \
    particleSim/atrackinghistorycrawler.h \
    photonSim/adataiobase.h \
    photonSim/adeporecord.h \
    photonSim/adepositionfilehandler.h \
    photonSim/afilehandlerbase.h \
    photonSim/afilesettingsbase.h \
    photonSim/aphotonbombfilehandler.h \
    photonSim/aphotonfilehandler.h \
    photonSim/aphotonsimhub.h \
    photonSim/amonitorconfig.h \
    photonSim/acommonfunctions.h \
    photonSim/aphoton.h \
    photonSim/aphotonsimmanager.h \
    photonSim/aphotonsimsettings.h \
    photonSim/aphotonstatistics.h \
    photonSim/aphotontrackrecord.h \
    photonSim/asensorsignalarray.h \
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
    geo/ageometryhub.h \
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
    gui/materials/aelementandisotopedelegates.h \
    gui/photsim/a3photsimwin.h \
    gui/raster/acameracontroldialog.h \
    gui/raster/rasterwindowbaseclass.h \
    gui/raster/rasterwindowgraphclass.h \
    materials/amaterialhub.h \
    materials/achemicalelement.h \
    materials/aisotope.h \
    materials/aisotopeabundancehandler.h \
    materials/amaterial.h \
    materials/amaterialcomposition.h \
    config/a3global.h \
    dispatch/a3workdistrconfig.h \
    farm/a3farmnoderecord.h \
    tools/aerrorhub.h \
    tools/ageowriter.h \
    tools/ahistogram.h \
    tools/ajsontoolsroot.h \
    tools/aroothistappenders.h \
    tools/afilemerger.h \
    tools/agraphbuilder.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/mainwindow.h \
    gui/guitools.h \
    tools/arandomhub.h \
    tools/ath.h \
    tools/athreadpool.h \
    tools/avector.h

FORMS += \
        gui/geom/ashownumbersdialog.ui \
        gui/aglobsetwindow.ui \
        gui/particleSim/atrackdrawdialog.ui \
        gui/photsim/asensordrawwidget.ui \
        gui/ademowindow.ui \
        gui/photsim/asensorwindow.ui \
        gui/script/ascriptwindow.ui \
        gui/aroottextconfigurator.ui \
        gui/farm/aremotewindow.ui \
        gui/graph/aaxesdialog.ui \
        gui/graph/alegenddialog.ui \
        gui/graph/alinemarkerfilldialog.ui \
        gui/graph/amultigraphdesigner.ui \
        gui/graph/arootcolorselectordialog.ui \
        gui/graph/atemplateselectiondialog.ui \
        gui/graph/atextpavedialog.ui \
        gui/graph/graphwindowclass.ui \
        gui/particleSim/aparticlesimoutputdialog.ui \
        gui/particleSim/aparticlesimwin.ui \
        gui/particleSim/aparticlesourcedialog.ui \
        gui/photsim/abombadvanceddialog.ui \
        gui/geom/a3geoconwin.ui \
        gui/geom/agridelementdialog.ui \
        gui/geom/amonitordelegateform.ui \
        gui/geom/ageometrywindow.ui \
        gui/mainwindow.ui \
        gui/materials/amatwin.ui \
        gui/photsim/a3photsimwin.ui \
        gui/photsim/ainterfacerulewin.ui \
        gui/photsim/ainterfaceruledialog.ui \
        gui/photsim/aphotonsimoutputdialog.ui \
        gui/raster/acameracontroldialog.ui

RESOURCES += \
    resources.qrc

ants3_FARM {
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
