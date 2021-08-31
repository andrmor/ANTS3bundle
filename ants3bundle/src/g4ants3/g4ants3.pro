QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

G4DIR = $$system(geant4-config --prefix)
#message($$G4DIR)
G4INCLUDE = $$join(G4DIR,,,/include/Geant4)
#message($$G4INCLUDE)
INCLUDEPATH += $$G4INCLUDE
LIBS += $$system(geant4-config --libs) -lxerces-c

DESTDIR = ../../bin

INCLUDEPATH += include

SOURCES += \
        main.cpp \
        src/ActionInitialization.cc \
        src/DetectorConstruction.cc \
        src/EventAction.cc \
        src/PrimaryGeneratorAction.cc \
        src/RunAction.cc \
        src/SensitiveDetector.cc \
        src/SessionManager.cc \
        src/StackingAction.cc \
        src/SteppingAction.cc \
        src/TrackingAction.cc \
        src/ahistogram.cc \
        src/json11.cc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    include/ActionInitialization.hh \
    include/DetectorConstruction.hh \
    include/EventAction.hh \
    include/PrimaryGeneratorAction.hh \
    include/RunAction.hh \
    include/SensitiveDetector.hh \
    include/SessionManager.hh \
    include/StackingAction.hh \
    include/SteppingAction.hh \
    include/TrackingAction.hh \
    include/ahistogram.hh \
    include/json11.hh
