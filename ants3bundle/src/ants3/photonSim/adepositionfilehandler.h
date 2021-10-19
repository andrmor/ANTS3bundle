#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "avector.h"

#include <QString>

// TRANSFER FROM ANTS2 !!!***    to be refactored to a universal class

// WORK IN PROGRESS

class QFile;
class QTextStream;

class ADepoRecord
{
public:
    ADepoRecord(double energy, const AVector3 & pos, double time, const QString & particle, int matIndex) :
        Energy(energy), Pos(pos), Time(time), Particle(particle), MatIndex(matIndex) {}

    double   Energy; // in keV
    AVector3 Pos;    // in mmm
    double   Time;
    QString  Particle;
    int      MatIndex;
};

class ADepositionFileHandler
{
public:
    enum EFileFormat {Unknown, Invalid, G4ascii, G4binary};

    ADepositionFileHandler(const QString & fileName, bool binary) :
        FileName(fileName), bBinary(binary) {}



    bool readNextRecordOfSameEvent(ADepoRecord & record) {} // returns false if event ended
    bool acknowledgeNextEventStarted() {}


    QString FileName;
    bool bBinary = false;

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         G4DepoLine;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    int             G4NextEventId = -1;

    int eventCurrent = 0;
    int eventBegin = 0;
    int eventEnd = 0;

    bool processG4DepositionData();
    bool readG4DepoEventFromTextFile();
    bool readG4DepoEventFromBinFile(bool expectNewEvent);
};

#endif // ADEPOSITIONFILEHANDLER_H
