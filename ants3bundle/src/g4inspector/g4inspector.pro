#CONFIG += ants3_NCrystal   # !!! Requires installation of NCrystal with NCrystal-Geant4, and need to adjust paths below !!!

QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

G4DIR = $$system(geant4-config --prefix)
#message($$G4DIR)
G4INCLUDE = $$join(G4DIR,,,/include/Geant4)
#message($$G4INCLUDE)
INCLUDEPATH += $$G4INCLUDE
LIBS += $$system(geant4-config --libs) -lxerces-c

DESTDIR = ../../bin

INCLUDEPATH += ../ants3/tools

DEFINES += GEANT4
DEFINES += JSON11

SOURCES += \
        ainspector.cpp \
        ../ants3/tools/ajsontools.cpp \
        main.cpp

HEADERS += \
        ainspector.h \
        ../ants3/tools/ajsontools.h

ants3_NCrystal {
  DEFINES += ANTS3_NCRYSTAL

  #NCrystal
  #INCLUDEPATH += /home/andr/NCrystal/ncrystal/ncrystal-main/ncrystal_install/include
  INCLUDEPATH += $$system(ncrystal-config --show includedir)

  #LIBS += -L$$system(ncrystal-config --show libdir) -lNCrystal
  #LIBS += -L/home/andr/NCrystal/ncrystal/ncrystal-main/ncrystal_build -lNCrystal
  #LIBS += -L/home/andr/NCrystal/ncrystal/ncrystal-main/ncrystal_install/lib -lNCrystal
  LIBS += -L$$system(ncrystal-config --show libdir) -lNCrystal

  #NCrystal-Geant4
  INCLUDEPATH += /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/include

  SOURCES += \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCBias.cc \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCManager.cc \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCMatHelper.cc \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCProcWrapper.hh \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCInstall.cc \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCManager.hh \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/src/G4NCProcWrapper.cc

  HEADERS += \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/include/G4NCrystal/G4NCBias.hh \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/include/G4NCrystal/G4NCInstall.hh \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/include/G4NCrystal/G4NCMatHelper.hh \
  /home/andr/NCrystal/ncrystal/ncrystal-geant4-main/src/ncrystal_geant4/cmake/include/G4NCrystal/G4NCrystal.hh
}
