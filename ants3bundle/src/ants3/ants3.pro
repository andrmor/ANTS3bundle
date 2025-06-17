# --- Optional features to be configured by the user ---
#
CONFIG += ants3_GUI         #if commented away, GUI is not compiled
CONFIG += ants3_FARM        #if commented away, WebSockets are not compiled and distributed (farm) functionality is disabled
#
CONFIG += ants3_Python      #enables Python scripting
CONFIG += ants3_RootServer  #enables CERN ROOT html server
CONFIG += ants3_jsroot      #enables JSROOT visualisation of the geometry. Requires Qt WebEngine library installed and ants3_RootServer enabled
#
# --- end of user-configure area ---

# CERN ROOT
INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb
ants3_RootServer {LIBS += -lRHTTP  -lXMLIO}

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

# ROOT HTML server
ants3_RootServer{
  DEFINES += USE_ROOT_HTML

    SOURCES += net/aroothttpserver.cpp
    HEADERS += net/aroothttpserver.h

    # JSROOT viewer
    ants3_jsroot{
        DEFINES += __USE_ANTS_JSROOT__
        QT      += webenginewidgets
    }

}
#----------

# Permission to script to start external processes
# DEFINES += _ALLOW_LAUNCH_EXTERNAL_PROCESS_
#----------

QT += core
ants3_GUI {
    QT += gui
    QT += widgets
    DEFINES += GUI
} else {
    QT -= gui
}

# ANTS3 version
DEFINES += ANTS3_MAJOR=1
DEFINES += ANTS3_MINOR=5

QT += qml   #this is for qjsengine

CONFIG += c++17 #c++11

QMAKE_CXXFLAGS += -march=native

TARGET = ants3
TEMPLATE = app

DEFINES += QT

DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs. In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

//DEFINES += TARGET_DIR=\"\\\"$${OUT_PWD}\\\"\"
DEFINES += TARGET_DIR=\"\\\"$${OUT_PWD}/../../bin\\\"\"

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
INCLUDEPATH += photonSim/photonFunctional
INCLUDEPATH += dispatch
INCLUDEPATH += farm
INCLUDEPATH += config
INCLUDEPATH += net
INCLUDEPATH += ../dispatcher
INCLUDEPATH += ../lsim
INCLUDEPATH += rec
INCLUDEPATH += rec/PET
INCLUDEPATH += /usr/include

DESTDIR = ../../bin

SOURCES += \
    ../dispatcher/a3dispatcher.cpp \
    ../dispatcher/a3processhandler.cpp \
    ../lsim/alightsensorevent.cpp \
    ../lsim/anoderecord.cpp \
    ../lsim/aphotonhistorylog.cpp \
    ../lsim/aphotontracer.cpp \
    gui/aviewer3dsettings.cpp \
    farm/afarmnoderecord.cpp \
    geo/acalorimeter.cpp \
    geo/acalorimeterhub.cpp \
    geo/agridhub.cpp \
    gui/aconfigexamplebrowser.cpp \
    gui/aglobsetwindow.cpp \
    gui/aguiwindow.cpp \
    gui/aitemselectiondialog.cpp \
    gui/alineedit.cpp \
    gui/alineeditwithescape.cpp \
    gui/amainwindow.cpp \
    gui/atreedatabaseselectordialog.cpp \
    gui/geom/ageoconstexpressiondialog.cpp \
    gui/geom/ageotreewin.cpp \
    gui/geom/aparticleanalyzerwidget.cpp \
    gui/geom/ashownumbersdialog.cpp \
    gui/graph/adrawmarginsrecord.cpp \
    gui/graph/agraphwindow.cpp \
    gui/graph/ahistoptstatdialog.cpp \
    gui/graph/amultidrawrecord.cpp \
    gui/graph/apaletteselectiondialog.cpp \
    gui/graph/asetmarginsdialog.cpp \
    gui/graph/aviewer3d.cpp \
    gui/graph/aviewer3dsettingsdialog.cpp \
    gui/graph/aviewer3dwidget.cpp \
    gui/materials/aabsorptiondataconverterdialog.cpp \
    gui/materials/amatwin.cpp \
    gui/materials/aopticaldataimportdialog.cpp \
    gui/materials/arefractiveindeximportdialog.cpp \
    gui/particleSim/aeventsdonedialog.cpp \
    gui/particleSim/aparticlesourceplotter.cpp \
    gui/particleSim/atrackdrawdialog.cpp \
    gui/particleSim/atrackingdataexplorer.cpp \
    gui/particleSim/aworldsizewarningdialog.cpp \
    gui/photsim/afunctionalmodelwidget.cpp \
    gui/photsim/ainterfaceruletester.cpp \
    gui/photsim/aphotfunctwindow.cpp \
    gui/photsim/aphotonlogsettingsform.cpp \
    gui/photsim/aphotsimwin.cpp \
    gui/photsim/asensordrawwidget.cpp \
    config/aconfig.cpp \
    dispatch/adispatcherinterface.cpp \
    farm/afarmhub.cpp \
    geo/ageospecial.cpp \
    geo/amonitorhub.cpp \
    gui/ademowindow.cpp \
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
    gui/graph/apadgeometry.cpp \
    gui/graph/apadproperties.cpp \
    gui/graph/arootcolorselectordialog.cpp \
    gui/graph/atemplateselectiondialog.cpp \
    gui/graph/atemplateselectionrecord.cpp \
    gui/graph/atextpavedialog.cpp \
    gui/graph/atoolboxscene.cpp \
    gui/graph/graphicsruler.cpp \
    gui/graph/shapeablerectitem.cpp \
    gui/particleSim/aparticlesimoutputdialog.cpp \
    gui/particleSim/aparticlesimwin.cpp \
    gui/particleSim/aparticlesourcedialog.cpp \
    gui/photsim/abombadvanceddialog.cpp \
    gui/photsim/ainterfaceruledialog.cpp \
    gui/photsim/ainterfacerulewin.cpp \
    gui/photsim/ainterfacewidgetfactory.cpp \
    gui/photsim/aphotonsimoutputdialog.cpp \
    farm/ademomanager.cpp \
    gui/photsim/asensorgview.cpp \
    gui/photsim/asensorwindow.cpp \
    gui/raster/agraphrasterwindow.cpp \
    gui/raster/arasterwindow.cpp \
    gui/script/aargumentcounter.cpp \
    gui/script/ageoscriptmaker.cpp \
    gui/script/aguifromscrwin.cpp \
    gui/script/ahighlighters.cpp \
    gui/script/ascriptbook.cpp \
    gui/script/ascriptexampleexplorer.cpp \
    gui/script/ascriptmessenger.cpp \
    gui/script/ascriptwindow.cpp \
    gui/script/astopwatch.cpp \
    gui/script/atabrecord.cpp \
    gui/script/atextedit.cpp \
    gui/script/atextoutputwindow.cpp \
    materials/amatcomposition.cpp \
    particleSim/ageant4inspectormanager.cpp \
    particleSim/aorthopositroniumgammagenerator.cpp \
    particleSim/aparticleanalyzerhub.cpp \
    particleSim/aparticleanalyzersettings.cpp \
    photonSim/aphotonloghandler.cpp \
    photonSim/interfaceRules/asurfaceinterfacerule.cpp \
    photonSim/interfaceRules/asurfacesettings.cpp \
    photonSim/interfaceRules/aunifiedrule.cpp \
    photonSim/photonFunctional/aphotonfunctionalhub.cpp \
    photonSim/photonFunctional/aphotonfunctionalmodel.cpp \
    rec/PET/acastorimageloader.cpp \
    script/ScriptInterfaces/ageo_si.cpp \
    script/ScriptInterfaces/ageowin_si.cpp \
    script/ScriptInterfaces/agraphwin_si.cpp \
    script/ScriptInterfaces/agui_si.cpp \
    script/ScriptInterfaces/amsg_si.cpp \
    script/ScriptInterfaces/aparticlesim_si.cpp \
    script/ScriptInterfaces/apet_si.cpp \
    script/ScriptInterfaces/arootstyle_si.cpp \
    script/ScriptInterfaces/asensor_si.cpp \
    script/ajscriptmanager.cpp \
    script/ajscriptworker.cpp \
    script/apeakfinder.cpp \
    rec/PET/apetcoincidencefinder.cpp \
    rec/PET/apeteventbuilder.cpp \
    script/arootgraphrecord.cpp \
    script/aroothistrecord.cpp \
    script/arootobjbase.cpp \
    script/arootobjcollection.cpp \
    script/aroottreerecord.cpp \
    script/ascriptexample.cpp \
    script/ascriptexampledatabase.cpp \
    script/ascripthelpentry.cpp \
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
    gui/raster/acameracontroldialog.cpp \
    main.cpp \
    materials/amaterial.cpp \
    config/a3global.cpp \
    dispatch/a3workdistrconfig.cpp \
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
    gui/guitools.cpp \
    tools/arandomhub.cpp \
    tools/ath.cpp \
    tools/athreadpool.cpp \
    tools/avector.cpp \
    tools/vformula.cpp

HEADERS += \
    ../dispatcher/a3dispatcher.h \
    ../dispatcher/a3processhandler.h \
    ../lsim/alightsensorevent.h \
    ../lsim/anoderecord.h \
    ../lsim/aphotonhistorylog.h \
    ../lsim/aphotontracer.h \
    farm/ademomanager.h \
    gui/aviewer3dsettings.h \
    gui/aitemselectiondialog.h \
    gui/amainwindow.h \
    gui/atreedatabaseselectordialog.h \
    gui/geom/aparticleanalyzerwidget.h \
    gui/graph/adrawmarginsrecord.h \
    gui/graph/agraphwindow.h \
    gui/graph/ahistoptstatdialog.h \
    gui/graph/amultidrawrecord.h \
    gui/graph/apaletteselectiondialog.h \
    gui/graph/asetmarginsdialog.h \
    gui/graph/aviewer3dsettingsdialog.h \
    gui/materials/aabsorptiondataconverterdialog.h \
    gui/materials/aopticaldataimportdialog.h \
    gui/materials/arefractiveindeximportdialog.h \
    gui/photsim/afunctionalmodelwidget.h \
    gui/photsim/aphotfunctwindow.h \
    gui/photsim/aphotonlogsettingsform.h \
    gui/raster/agraphrasterwindow.h \
    gui/raster/arasterwindow.h \
    gui/script/aargumentcounter.h \
    gui/script/ascriptexampleexplorer.h \
    particleSim/aorthopositroniumgammagenerator.h \
    particleSim/aparticleanalyzerhub.h \
    particleSim/aparticleanalyzersettings.h \
    photonSim/aphotonloghandler.h \
    photonSim/photonFunctional/aphotonfunctionalhub.h \
    photonSim/photonFunctional/aphotonfunctionalmodel.h \
    rec/PET/acastorimageloader.h \
    rec/PET/apetcoincidencefinderconfig.h \
    farm/afarmnoderecord.h \
    geo/acalorimeter.h \
    geo/acalorimeterhub.h \
    geo/agridelementrecord.h \
    geo/agridhub.h \
    gui/aconfigexamplebrowser.h \
    gui/aglobsetwindow.h \
    gui/aguiwindow.h \
    gui/alineedit.h \
    gui/alineeditwithescape.h \
    gui/geom/ageoconstexpressiondialog.h \
    gui/geom/ageotreewin.h \
    gui/geom/ashownumbersdialog.h \
    gui/graph/aviewer3d.h \
    gui/graph/aviewer3dwidget.h \
    gui/materials/amatwin.h \
    gui/particleSim/aeventsdonedialog.h \
    gui/particleSim/aparticlesourceplotter.h \
    gui/particleSim/atrackdrawdialog.h \
    gui/particleSim/atrackingdataexplorer.h \
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
    gui/graph/apadgeometry.h \
    gui/graph/apadproperties.h \
    gui/graph/arootcolorselectordialog.h \
    gui/graph/atemplateselectiondialog.h \
    gui/graph/atemplateselectionrecord.h \
    gui/graph/atextpavedialog.h \
    gui/graph/atlegend.h.autosave \
    gui/graph/atoolboxscene.h \
    gui/graph/graphicsruler.h \
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
    gui/script/aguifromscrwin.h \
    gui/script/ahighlighters.h \
    gui/script/ascriptbook.h \
    gui/script/ascriptmessenger.h \
    gui/script/ascriptwindow.h \
    gui/script/astopwatch.h \
    gui/script/atabrecord.h \
    gui/script/atextedit.h \
    gui/script/atextoutputwindow.h \
    gui/script/escriptlanguage.h \
    materials/amatcomposition.h \
    particleSim/ageant4inspectormanager.h \
    photonSim/interfaceRules/asurfaceinterfacerule.h \
    photonSim/interfaceRules/asurfacesettings.h \
    photonSim/interfaceRules/aunifiedrule.h \
    rec/PET/apeteventbuilderconfig.h \
    script/ScriptInterfaces/ageo_si.h \
    script/ScriptInterfaces/ageowin_si.h \
    script/ScriptInterfaces/agraphwin_si.h \
    script/ScriptInterfaces/agui_si.h \
    script/ScriptInterfaces/amsg_si.h \
    script/ScriptInterfaces/aparticlesim_si.h \
    script/ScriptInterfaces/apet_si.h \
    script/ScriptInterfaces/arootstyle_si.h \
    script/ScriptInterfaces/asensor_si.h \
    rec/PET/apetcoincidencefinder.h \
    rec/PET/apeteventbuilder.h \
    script/arootgraphrecord.h \
    script/aroothistrecord.h \
    script/arootobjbase.h \
    script/arootobjcollection.h \
    script/aroottreerecord.h \
    script/ajscriptmanager.h \
    script/ajscriptworker.h \
    script/apeakfinder.h \
    script/ascriptexample.h \
    script/ascriptexampledatabase.h \
    script/ascripthelpentry.h \
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
    gui/raster/acameracontroldialog.h \
    materials/amaterialhub.h \
    materials/amaterial.h \
    config/a3global.h \
    dispatch/a3workdistrconfig.h \
    tools/aerrorhub.h \
    tools/ageowriter.h \
    tools/ahistogram.h \
    tools/ajsontoolsroot.h \
    tools/aroothistappenders.h \
    tools/afilemerger.h \
    tools/agraphbuilder.h \
    tools/ajsontools.h \
    tools/afiletools.h \
    gui/guitools.h \
    tools/arandomhub.h \
    tools/ath.h \
    tools/athreadpool.h \
    tools/avector.h \
    tools/vformula.h

FORMS += \
        gui/aconfigexamplebrowser.ui \
        gui/aitemselectiondialog.ui \
        gui/amainwindow.ui \
        gui/geom/aparticleanalyzerwidget.ui \
        gui/geom/ashownumbersdialog.ui \
        gui/aglobsetwindow.ui \
        gui/graph/agraphwindow.ui \
        gui/graph/ahistoptstatdialog.ui \
        gui/graph/apaletteselectiondialog.ui \
        gui/graph/asetmarginsdialog.ui \
        gui/graph/aviewer3d.ui \
        gui/graph/aviewer3dsettingsdialog.ui \
        gui/graph/aviewer3dwidget.ui \
        gui/materials/aabsorptiondataconverterdialog.ui \
        gui/materials/aopticaldataimportdialog.ui \
        gui/materials/arefractiveindeximportdialog.ui \
        gui/particleSim/aeventsdonedialog.ui \
        gui/particleSim/atrackdrawdialog.ui \
        gui/particleSim/aworldsizewarningdialog.ui \
        gui/photsim/ainterfaceruletester.ui \
        gui/photsim/aphotfunctwindow.ui \
        gui/photsim/aphotonlogsettingsform.ui \
        gui/photsim/aphotsimwin.ui \
        gui/photsim/asensordrawwidget.ui \
        gui/ademowindow.ui \
        gui/photsim/asensorwindow.ui \
        gui/script/ascriptexampleexplorer.ui \
        gui/script/ascriptwindow.ui \
        gui/aroottextconfigurator.ui \
        gui/farm/aremotewindow.ui \
        gui/graph/aaxesdialog.ui \
        gui/graph/alegenddialog.ui \
        gui/graph/alinemarkerfilldialog.ui \
        gui/graph/arootcolorselectordialog.ui \
        gui/graph/atemplateselectiondialog.ui \
        gui/graph/atextpavedialog.ui \
        gui/particleSim/aparticlesimoutputdialog.ui \
        gui/particleSim/aparticlesimwin.ui \
        gui/particleSim/aparticlesourcedialog.ui \
        gui/photsim/abombadvanceddialog.ui \
        gui/geom/ageotreewin.ui \
        gui/geom/agridelementdialog.ui \
        gui/geom/amonitordelegateform.ui \
        gui/geom/ageometrywindow.ui \
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
    script/ScriptInterfaces/awebsocket_si.cpp \
    script/ScriptInterfaces/awebserver_si.cpp \
    net/awebsocketserver.cpp \
    ../dispatcher/awebsocketsessionserver.cpp \
    ../dispatcher/awebsocketsession.cpp \
    ../dispatcher/a3remotehandler.cpp \
    ../dispatcher/a3wsclient.cpp

    HEADERS += \
    script/ScriptInterfaces/awebsocket_si.h \
    script/ScriptInterfaces/awebserver_si.h \
    net/awebsocketserver.h \
    ../dispatcher/awebsocketsessionserver.h \
    ../dispatcher/awebsocketsession.h \
    ../dispatcher/a3remotehandler.h \
    ../dispatcher/a3wsclient.h \
} else {
    QT -= websockets
}
message("Copy resource files")
linux-g++ || unix{
   fromdir = $${PWD}/files
   message($$fromdir)
   todir = $${OUT_PWD}/../../bin
   message($$todir)
   QMAKE_POST_LINK = $$quote(cp -rf \"$${fromdir}\" \"$${todir}\"$$escape_expand(\n\t))
}
