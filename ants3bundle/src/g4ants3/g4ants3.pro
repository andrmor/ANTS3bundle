QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

G4DIR = $$system(geant4-config --prefix)
#message($$G4DIR)
G4INCLUDE = $$join(G4DIR,,,/include/Geant4)
#message($$G4INCLUDE)
INCLUDEPATH += $$G4INCLUDE
LIBS += $$system(geant4-config --libs) -lxerces-c

GEANTVERSTR = $$system(geant4-config --version)
GEANTVERLIST = $$split(GEANTVERSTR, ".")
G4MAJOR = $$member(GEANTVERLIST, 0)
G4MINOR = $$member(GEANTVERLIST, 1)
MAJORTXT = GEANT4_MAJOR=\"$$G4MAJOR\"
message($$MAJORTXT)
DEFINES += $$MAJORTXT
MINORTXT = GEANT4_MAJOR=\"$$G4MINOR\"
message($$MINORTXT)
DEFINES += $$MINORTXT

greaterThan(G4MAJOR, 10){
   message("Geant4 major version ($$G4MAJOR) is 11 or larger")
   DEFINES += GEANT_VERSION_FROM_11
} else {
  message("Geant4 major version ($$G4MAJOR) is smaller than 11")
}

DESTDIR = ../../bin

INCLUDEPATH += src
INCLUDEPATH += ../ants3/particleSim
INCLUDEPATH += ../ants3/tools

DEFINES += GEANT4
DEFINES += JSON11

SOURCES += \
        ../ants3/particleSim/acalsettings.cpp \
        ../ants3/particleSim/afilegeneratorsettings.cpp \
        ../ants3/particleSim/afileparticlegenerator.cpp \
        ../ants3/particleSim/ag4simulationsettings.cpp \
        ../ants3/particleSim/amonitorsettings.cpp \
        ../ants3/particleSim/aparticlerunsettings.cpp \
        ../ants3/particleSim/aparticlesimsettings.cpp \
        ../ants3/tools/aerrorhub.cpp \
        main.cpp \
        ../ants3/particleSim/aparticlesourcerecord.cpp \
        ../ants3/particleSim/aparticlerecord.cpp \
        ../ants3/particleSim/asourcegeneratorsettings.cpp \
        ../ants3/particleSim/asourceparticlegenerator.cpp \
        ../ants3/tools/ahistogram.cpp \
        ../ants3/tools/avector.cpp \
        src/ActionInitialization.cc \
        src/DetectorConstruction.cc \
        src/EventAction.cc \
        src/PrimaryGeneratorAction.cc \
        src/SensitiveDetector.cc \
        src/SessionManager.cc \
        src/StackingAction.cc \
        src/SteppingAction.cc \
        src/TrackingAction.cc \
        #src/ahistogram.cc \
        src/arandomg4hub.cpp \
        src/exceptionhandler.cpp \
        src/json11.cc \
        src/js11tools.cc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../ants3/particleSim/acalsettings.h \
    ../ants3/particleSim/afilegeneratorsettings.h \
    ../ants3/particleSim/afileparticlegenerator.h \
    ../ants3/particleSim/ag4simulationsettings.h \
    ../ants3/particleSim/amonitorsettings.h \
    ../ants3/particleSim/aparticlegun.h \
    ../ants3/particleSim/aparticlerunsettings.h \
    ../ants3/particleSim/aparticlesimsettings.h \
    ../ants3/particleSim/aparticlesourcerecord.h \
    ../ants3/particleSim/aparticlerecord.h \
    ../ants3/particleSim/asourcegeneratorsettings.h \
    ../ants3/particleSim/asourceparticlegenerator.h \
    ../ants3/tools/aerrorhub.h \
    ../ants3/tools/ahistogram.h \
    ../ants3/tools/avector.h \
    src/ActionInitialization.hh \
    src/DetectorConstruction.hh \
    src/EventAction.hh \
    src/PrimaryGeneratorAction.hh \
    src/SensitiveDetector.hh \
    src/SessionManager.hh \
    src/StackingAction.hh \
    src/SteppingAction.hh \
    src/TrackingAction.hh \
    #src/ahistogram.hh \
    src/arandomg4hub.h \
    src/exceptionhandler.h \
    src/json11.hh \
    src/js11tools.hh
