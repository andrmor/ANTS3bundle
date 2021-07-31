QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# CERN ROOT
     INCLUDEPATH += $$system(root-config --incdir)
     LIBS += $$system(root-config --libs) -lGeom -lGeomPainter -lGeomBuilder -lMinuit2 -lSpectrum -ltbb

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../ants3/materials
INCLUDEPATH += ../ants3/photonSim
INCLUDEPATH += ../ants3/tools

SOURCES += \
#        ../ants3/materials/amaterial.cpp \
#        ../ants3/materials/amaterialcomposition.cpp \
#        ../ants3/materials/amaterialhub.cpp \
#        ../ants3/photonSim/aphotonsimsettings.cpp \
#        ../ants3/photonSim/asimulationstatistics.cpp \
#        aphotonhistorylog.cpp \
#        aphoton.cpp \
#        aphotontracer.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
#    ../ants3/materials/amaterial.h \
#    ../ants3/materials/amaterialcomposition.h \
#    ../ants3/materials/amaterialhub.h \
#    ../ants3/photonSim/aphotonsimsettings.h \
#    ../ants3/photonSim/asimulationstatistics.h \
#    aphotonhistorylog.h \
#    aphoton.h \
#    aphotontracer.h
