CONFIG += ants3_mercury     #enables Mercury library for position reconstructio

QT -= gui

CONFIG += console
CONFIG -= app_bundle

CONFIG += c++17 #c++11

QMAKE_CXXFLAGS += -O2

# CERN ROOT
     INCLUDEPATH += $$system(root-config --incdir)
     #LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb
     LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lMinuit2 -lSpectrum -ltbb

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../bin

INCLUDEPATH += ../ants3/config
INCLUDEPATH += ../ants3/materials
INCLUDEPATH += ../ants3/geo
INCLUDEPATH += ../ants3/photonSim
INCLUDEPATH += ../ants3/photonSim/interfaceRules
INCLUDEPATH += ../ants3/photonSim/photonFunctional
INCLUDEPATH += ../ants3/tools
INCLUDEPATH += ../ants3/particleSim # see comments below, needed only for calorimeters

DEFINES += QT

DEFINES += LSIM
DEFINES += NOT_NEED_MAT_COMPOSITION

# Mercury
ants3_mercury{
  DEFINES += USE_MERCURY

    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
    LIBS += -fopenmp

    INCLUDEPATH += /usr/include/eigen3
    INCLUDEPATH += ../ants3/rec/Mercury
    INCLUDEPATH += ../ants3/rec/Mercury/mercuryLib
    INCLUDEPATH += ../ants3/rec/Mercury/mercuryLib/lib
    INCLUDEPATH += ../ants3/rec/Mercury/mercuryLib/LRModel
    INCLUDEPATH += ../ants3/rec/Mercury/mercuryLib/spline123
    INCLUDEPATH += ../ants3/rec/Mercury/script
    INCLUDEPATH += ../ants3/gui/rec  # tmp!

    SOURCES += \
        ../ants3/rec/Mercury/mercuryLib/LRModel/compress.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrf.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfaxial.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfaxial3d.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfcomp.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfio.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfxy.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfxyz.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformula1.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrmodel.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/transform.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformulav.cpp \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformulaxy.cpp \
        ../ants3/rec/Mercury/mercuryLib/lib/json11.cpp \
        ../ants3/rec/Mercury/mercuryLib/lib/wformula.cpp \
        ../ants3/rec/Mercury/mercuryLib/reconstructor.cpp \
        ../ants3/rec/Mercury/mercuryLib/reconstructor_mp.cpp \
        ../ants3/rec/Mercury/mercuryLib/spline123/bsfit123.cpp \
        ../ants3/rec/Mercury/mercuryLib/spline123/bspline123d.cpp \
        ../ants3/rec/Mercury/mercuryLib/spline123/profileHist.cpp \
        ../ants3/rec/Mercury/alightresponsehub.cpp \
        ../ants3/gui/rec/alrfplotter.cpp # tmp!

    HEADERS += \
        ../ants3/rec/Mercury/mercuryLib/LRModel/compress.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrf.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfaxial.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfaxial3d.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfcomp.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfio.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfxy.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrfxyz.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformula1.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrmodel.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/transform.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformulav.h \
        ../ants3/rec/Mercury/mercuryLib/LRModel/lrformulaxy.h \
        ../ants3/rec/Mercury/mercuryLib/lib/eiquadprog.hpp \
        ../ants3/rec/Mercury/mercuryLib/lib/json11.hpp \
        ../ants3/rec/Mercury/mercuryLib/lib/functor.h \
        ../ants3/rec/Mercury/mercuryLib/lib/wformula.h \
        ../ants3/rec/Mercury/mercuryLib/reconstructor.h \
        ../ants3/rec/Mercury/mercuryLib/reconstructor_mp.h \
        ../ants3/rec/Mercury/mercuryLib/spline123/bsfit123.h \
        ../ants3/rec/Mercury/mercuryLib/spline123/bspline123d.h \
        ../ants3/rec/Mercury/mercuryLib/spline123/profileHist.h \
        ../ants3/rec/Mercury/alightresponsehub.h \
        ../ants3/gui/rec/alrfplotter.h  # tmp!
}

SOURCES += \
        ../ants3/geo/acalorimeter.cpp \   # not needed for functionality, can be removed using a new DEFINE
        ../ants3/geo/acalorimeterhub.cpp \ # not needed for functionality, can be removed using a new DEFINE
        ../ants3/geo/agridhub.cpp \
        ../ants3/particleSim/acalsettings.cpp \ # not needed for functionality, can be removed using a new DEFINE
        ../ants3/geo/amonitorhub.cpp \
        ../ants3/particleSim/aparticleanalyzersettings.cpp \ # not needed for functionality, can be removed using a new DEFINE
        ../ants3/photonSim/adeporecord.cpp \
        ../ants3/photonSim/adepositionfilehandler.cpp \
        ../ants3/photonSim/afilehandlerbase.cpp \
        ../ants3/photonSim/afilesettingsbase.cpp \
        ../ants3/photonSim/ageomeshhandler.cpp \
        ../ants3/photonSim/aphotonbombfilehandler.cpp \
        ../ants3/photonSim/aphotonfilehandler.cpp \
        ../ants3/photonSim/asensormodel.cpp \
        ../ants3/photonSim/interfaceRules/alutinterfacerule.cpp \
        ../ants3/photonSim/photonFunctional/aphotonfunctionalhub.cpp \
        ../ants3/photonSim/photonFunctional/aphotonfunctionalmodel.cpp \
        ../ants3/tools/aerrorhub.cpp \
        ../ants3/tools/ahistogram.cpp \
        ../ants3/tools/ajsontools.cpp \
        ../ants3/tools/afiletools.cpp \
        ../ants3/tools/ajsontoolsroot.cpp \
        ../ants3/tools/arandomhub.cpp \
        ../ants3/geo/ageometryhub.cpp \
        ../ants3/geo/ageoconsts.cpp \
        ../ants3/geo/ageoobject.cpp \
        ../ants3/geo/ageoshape.cpp \
        ../ants3/geo/ageospecial.cpp \
        ../ants3/geo/ageotype.cpp \
        ../ants3/geo/amonitor.cpp \
        ../ants3/photonSim/amonitorconfig.cpp \
        ../ants3/photonSim/asensorhub.cpp \
        ../ants3/materials/amaterialhub.cpp \
        ../ants3/materials/amaterial.cpp \
        ../ants3/materials/amatcomposition.cpp \
        ../ants3/photonSim/aphotonsimhub.cpp \
        ../ants3/photonSim/aphotonsimsettings.cpp \
        ../ants3/photonSim/interfaceRules/ainterfacerulehub.cpp \
        ../ants3/photonSim/interfaceRules/ainterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/abasicinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/ametalinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/aspectralbasicinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/awaveshifterinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/fsnpinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/asurfaceinterfacerule.cpp \
        ../ants3/photonSim/interfaceRules/aunifiedrule.cpp \
        ../ants3/photonSim/aphoton.cpp \
        ../ants3/photonSim/aphotonstatistics.cpp \
        ../ants3/tools/aroothistappenders.cpp \
        ../ants3/photonSim/astatisticshub.cpp \
        ../ants3/tools/ath.cpp \
        ../ants3/tools/avector.cpp \
        alightsensorevent.cpp \
        anoderecord.cpp \
        aphotongenerator.cpp \
        aphotonsimulator.cpp \
        aphotontracer.cpp \
        aphotonhistorylog.cpp \
        alogger.cpp \
        as1generator.cpp \
        as2generator.cpp \
        ../ants3/photonSim/interfaceRules/asurfacesettings.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../ants3/geo/acalorimeter.h \
    ../ants3/geo/acalorimeterhub.h \
    ../ants3/geo/agridelementrecord.h \
    ../ants3/geo/agridhub.h \
    ../ants3/geo/amonitorhub.h \
    ../ants3/particleSim/acalsettings.h \
    ../ants3/particleSim/aparticleanalyzersettings.h \
    ../ants3/photonSim/adeporecord.h \
    ../ants3/photonSim/adepositionfilehandler.h \
    ../ants3/photonSim/afilehandlerbase.h \
    ../ants3/photonSim/afilesettingsbase.h \
    ../ants3/photonSim/ageomeshhandler.h \
    ../ants3/photonSim/aphotonbombfilehandler.h \
    ../ants3/photonSim/aphotonfilehandler.h \
    ../ants3/photonSim/asensormodel.h \
    ../ants3/photonSim/interfaceRules/alutinterfacerule.h \
    ../ants3/photonSim/photonFunctional/aphotonfunctionalhub.h \
    ../ants3/photonSim/photonFunctional/aphotonfunctionalmodel.h \
    ../ants3/tools/aerrorhub.h \
    ../ants3/tools/ahistogram.h \
    ../ants3/tools/ajsontools.h \
    ../ants3/tools/afiletools.h \
    ../ants3/tools/ajsontoolsroot.h \
    ../ants3/tools/arandomhub.h \
    ../ants3/geo/ageometryhub.h \
    ../ants3/geo/ageoconsts.h \
    ../ants3/geo/ageoobject.h \
    ../ants3/geo/ageoshape.h \
    ../ants3/geo/ageospecial.h \
    ../ants3/geo/ageotype.h \
    ../ants3/geo/amonitor.h \
    ../ants3/photonSim/amonitorconfig.h \
    ../ants3/photonSim/asensorhub.h \
    ../ants3/materials/amaterialhub.h \
    ../ants3/materials/amaterial.h \
    ../ants3/materials/amatcomposition.h \
    ../ants3/photonSim/aphotonsimhub.h \
    ../ants3/photonSim/aphotonsimsettings.h \
     ../ants3/photonSim/interfaceRules/ainterfacerulehub.h \
     ../ants3/photonSim/interfaceRules/ainterfacerule.h \
    ../ants3/photonSim/interfaceRules/abasicinterfacerule.h \
    ../ants3/photonSim/interfaceRules/ametalinterfacerule.h \
    ../ants3/photonSim/interfaceRules/aspectralbasicinterfacerule.h \
    ../ants3/photonSim/interfaceRules/awaveshifterinterfacerule.h \
    ../ants3/photonSim/interfaceRules/fsnpinterfacerule.h \
    ../ants3/photonSim/interfaceRules/asurfaceinterfacerule.h \
    ../ants3/photonSim/interfaceRules/aunifiedrule.h \
    ../ants3/photonSim/aphoton.h \
    ../ants3/photonSim/aphotonstatistics.h \
    ../ants3/tools/aroothistappenders.h \
    ../ants3/photonSim/astatisticshub.h \
    ../ants3/photonSim/aphotontrackrecord.h \
    ../ants3/tools/ath.h \
    ../ants3/tools/avector.h \
    alightsensorevent.h \
    aphotonsimulator.h \
    aphotontracer.h \
    aphotonhistorylog.h \
    anoderecord.h \
    aphotongenerator.h \
    alogger.h \
    as1generator.h \
    as2generator.h \
    ../ants3/photonSim/interfaceRules/asurfacesettings.h
