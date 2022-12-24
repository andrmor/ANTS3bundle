# optional features
CONFIG += ants3_GUI          #if commented away, GUI is not compiled
CONFIG += ants3_FARM         #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled

CONFIG += ants3_Python      #enable Python scripting
# not yet!  #CONFIG += ants3_RootServer  #enable cern CERN ROOT html server
# not yet!  #CONFIG += ants3_jsroot      #enables JSROOT visualisation at GeometryWindow. Automatically enables ants2_RootServer

# CERN ROOT
INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb
#ants2_RootServer {LIBS += -lRHTTP  -lXMLIO}

# PYTHON
ants3_Python {
    DEFINES += ANTS3_PYTHON
    #LIBS = -L/usr/lib/python3.10/config-3.10-x86_64-linux-gnu -lcrypt -lpthread -ldl -lutil -lm -lpython3.10
    #LIBS = -L/usr/lib/python3.9/config-3.9-x86_64-linux-gnu -lcrypt -lpthread -ldl -lutil -lm -lpython3.9

    LIBS += $$system(python3-config --libs --embed)

    QMAKE_CXXFLAGS += $$system(python3-config --includes)

    SOURCES += \
        script/Python/apythoninterface.cpp \
        script/Python/apythonscriptmanager.cpp \
        script/Python/apythonworker.cpp \
        script/Python/aminipython_si.cpp

    HEADERS += \
        script/Python/apythoninterface.h \
        script/Python/apythonscriptmanager.h \
        script/Python/apythonworker.h \
        script/Python/aminipython_si.h

    INCLUDEPATH += script/Python
}

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

INCLUDEPATH += script
INCLUDEPATH += script/ScriptInterfaces
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
    geo/agridhub.cpp \
    gui/aglobsetwindow.cpp \
    gui/aguiwindow.cpp \
    gui/alineeditwithescape.cpp \
    gui/geom/ageoconstexpressiondialog.cpp \
    gui/geom/ageotreewin.cpp \
    gui/geom/ashownumbersdialog.cpp \
    gui/materials/amatwin.cpp \
    gui/particleSim/aparticlesourceplotter.cpp \
    gui/particleSim/atrackdrawdialog.cpp \
    gui/particleSim/aworldsizewarningdialog.cpp \
    gui/photsim/ainterfaceruletester.cpp \
    gui/photsim/aphotsimwin.cpp \
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
    photonSim/interfaceRules/asurfaceinterfacerule.cpp \
    photonSim/interfaceRules/asurfacesettings.cpp \
    photonSim/interfaceRules/aunifiedrule.cpp \
    script/ScriptInterfaces/ageo_si.cpp \
    script/ScriptInterfaces/ageowin_si.cpp \
    script/ScriptInterfaces/agraphwin_si.cpp \
    script/ScriptInterfaces/amsg_si.cpp \
    script/ScriptInterfaces/aparticlesim_si.cpp \
    script/ScriptInterfaces/asensor_si.cpp \
    script/ajscriptmanager.cpp \
    script/ajscriptworker.cpp \
    script/apeakfinder.cpp \
    script/arootgraphrecord.cpp \
    script/aroothistrecord.cpp \
    script/arootobjbase.cpp \
    script/arootobjcollection.cpp \
    script/aroottreerecord.cpp \
    script/ascripthub.cpp \
    script/ascriptobjstore.cpp \
    script/avarrecordbase.cpp \
    script/ScriptInterfaces/aconfig_si.cpp \
    script/ScriptInterfaces/acore_si.cpp \
    script/ScriptInterfaces/ademo_si.cpp \
    script/ScriptInterfaces/afarm_si.cpp \
    script/ScriptInterfaces/ahist_si.cpp \
    script/ScriptInterfaces/amath_si.cpp \
    script/ScriptInterfaces/aminijs_si.cpp \
    script/ScriptInterfaces/apartanalysis_si.cpp \
    script/ScriptInterfaces/aphotonsim_si.cpp \
    script/ScriptInterfaces/ascriptinterface.cpp \
    script/ScriptInterfaces/ascriptminimizerbase.cpp \
    script/ScriptInterfaces/atrackrec_si.cpp \
    script/ScriptInterfaces/agraph_si.cpp \
    script/ScriptInterfaces/atree_si.cpp \
    script/ScriptInterfaces/awindowinterfacebase.cpp \
    script/avirtualscriptmanager.cpp \
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
    geo/agridelementrecord.h \
    geo/agridhub.h \
    gui/aglobsetwindow.h \
    gui/aguiwindow.h \
    gui/alineeditwithescape.h \
    gui/geom/ageoconstexpressiondialog.h \
    gui/geom/ageotreewin.h \
    gui/geom/ashownumbersdialog.h \
    gui/materials/amatwin.h \
    gui/particleSim/aparticlesourceplotter.h \
    gui/particleSim/atrackdrawdialog.h \
    gui/particleSim/aworldsizewarningdialog.h \
    gui/photsim/ainterfaceruletester.h \
    gui/photsim/aphotsimwin.h \
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
    gui/script/ascriptwindow.h \
    gui/script/atabrecord.h \
    gui/script/atextedit.h \
    gui/script/atextoutputwindow.h \
    gui/script/escriptlanguage.h \
    photonSim/interfaceRules/asurfaceinterfacerule.h \
    photonSim/interfaceRules/asurfacesettings.h \
    photonSim/interfaceRules/aunifiedrule.h \
    script/ScriptInterfaces/ageo_si.h \
    script/ScriptInterfaces/ageowin_si.h \
    script/ScriptInterfaces/agraphwin_si.h \
    script/ScriptInterfaces/amsg_si.h \
    script/ScriptInterfaces/aparticlesim_si.h \
    script/ScriptInterfaces/asensor_si.h \
    script/arootgraphrecord.h \
    script/aroothistrecord.h \
    script/arootobjbase.h \
    script/arootobjcollection.h \
    script/aroottreerecord.h \
    script/ajscriptmanager.h \
    script/ajscriptworker.h \
    script/apeakfinder.h \
    script/ascripthub.h \
    script/ascriptobjstore.h \
    script/avarrecordbase.h \
    script/ScriptInterfaces/aconfig_si.h \
    script/ScriptInterfaces/acore_si.h \
    script/ScriptInterfaces/ademo_si.h \
    script/ScriptInterfaces/afarm_si.h \
    script/ScriptInterfaces/ahist_si.h \
    script/ScriptInterfaces/amath_si.h \
    script/ScriptInterfaces/aminijs_si.h \
    script/ScriptInterfaces/apartanalysis_si.h \
    script/ScriptInterfaces/aphotonsim_si.h \
    script/ScriptInterfaces/ascriptinterface.h \
    script/ScriptInterfaces/ascriptminimizerbase.h \
    script/ScriptInterfaces/atrackrec_si.h \
    script/ScriptInterfaces/agraph_si.h \
    script/ScriptInterfaces/atree_si.h \
    script/ScriptInterfaces/awindowinterfacebase.h \
    script/avirtualscriptmanager.h \
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
        gui/particleSim/aworldsizewarningdialog.ui \
        gui/photsim/ainterfaceruletester.ui \
        gui/photsim/aphotsimwin.ui \
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
        gui/geom/ageotreewin.ui \
        gui/geom/agridelementdialog.ui \
        gui/geom/amonitordelegateform.ui \
        gui/geom/ageometrywindow.ui \
        gui/mainwindow.ui \
        gui/materials/amatwin.ui \
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
