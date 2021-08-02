QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# CERN ROOT
     INCLUDEPATH += $$system(root-config --incdir)
     LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../bin

INCLUDEPATH += ../ants3/config
INCLUDEPATH += ../ants3/materials
INCLUDEPATH += ../ants3/photonSim
INCLUDEPATH += ../ants3/photonSim/interfaceRules
INCLUDEPATH += ../ants3/tools

SOURCES += \
#        ../ants3/config/a3config.cpp \
#        ../ants3/materials/amaterial.cpp \
#        ../ants3/materials/amaterialcomposition.cpp \
#        ../ants3/materials/amaterialhub.cpp \
#        ../ants3/photonSim/aphoton.cpp \
#        ../ants3/photonSim/aphotonsimsettings.cpp \
#        ../ants3/photonSim/asimulationstatistics.cpp \
#        ../ants3/photonSim/interfaceRules/abasicinterfacerule.cpp \
#        ../ants3/photonSim/interfaceRules/ainterfacerule.cpp \
#        ../ants3/photonSim/interfaceRules/ainterfacerulehub.cpp \
#        ../ants3/photonSim/interfaceRules/ametalinterfacerule.cpp \
#        ../ants3/photonSim/interfaceRules/aspectralbasicinterfacerule.cpp \
#        ../ants3/photonSim/interfaceRules/awaveshifterinterfacerule.cpp \
#        ../ants3/photonSim/interfaceRules/fsnpinterfacerule.cpp \
#        aoneevent.cpp \
#        aphotonhistorylog.cpp \
#        aphotontracer.cpp \
        alogger.cpp \
        aphotonsimulator.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
#    ../ants3/config/a3config.h \
#    ../ants3/materials/amaterial.h \
#    ../ants3/materials/amaterialcomposition.h \
#    ../ants3/materials/amaterialhub.h \
#    ../ants3/photonSim/aphotonsimsettings.h \
#    ../ants3/photonSim/aphoton.h \
#    ../ants3/photonSim/asimulationstatistics.h \
#    ../ants3/photonSim/interfaceRules/abasicinterfacerule.h \
#    ../ants3/photonSim/interfaceRules/ainterfacerule.h \
#    ../ants3/photonSim/interfaceRules/ainterfacerulehub.h \
#    ../ants3/photonSim/interfaceRules/ametalinterfacerule.h \
#    ../ants3/photonSim/interfaceRules/aspectralbasicinterfacerule.h \
#    ../ants3/photonSim/interfaceRules/awaveshifterinterfacerule.h \
#    ../ants3/photonSim/interfaceRules/fsnpinterfacerule.h \
#    aoneevent.h \
#    aphotonhistorylog.h \
#    aphotontracer.h
    alogger.h \
    aphotonsimulator.h
