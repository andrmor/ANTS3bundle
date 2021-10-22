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
        FileName(fileName), Binary(binary) {}
    virtual ~ADepositionFileHandler();

    bool validate(bool collectStatistics) {return false;}

    bool init();
    bool gotoEvent(int iEvent);

    bool readNextRecordOfSameEvent(ADepoRecord & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    QString         FileName;
    bool            Binary       = false;

    int             CurrentEvent = -1;
    bool            EventEndReached = false;

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         LineText;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    char            Header        = 0x00;

    //bool readG4DepoEventFromBinFile(bool expectNewEvent);

private:
    void clearResources();
    bool processEventHeader();
};

#endif // ADEPOSITIONFILEHANDLER_H
