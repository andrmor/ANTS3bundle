#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "avector.h"
#include "aphotonsimsettings.h"

#include <QString>

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
    ADepositionFileHandler(APhotonDepoSettings & depoSettings);
    virtual ~ADepositionFileHandler();

    int  checkFile(bool collectStatistics); // returns number of events    !!!*** add statistics!

    bool init();
    bool gotoEvent(int iEvent);

    bool readNextRecordOfSameEvent(ADepoRecord & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    void determineFormat(); // very simplistic, better to make more strict !!!***

private:
    APhotonDepoSettings & Settings;

    int             CurrentEvent = -1;
    bool            EventEndReached = false;

    //resources for ascii input
    QFile         * inTextFile    = nullptr;
    QTextStream   * inTextStream  = nullptr;
    QString         LineText;
    //resources for binary input
    std::ifstream * inStream      = nullptr;
    char            Header        = 0x00;

    void clearResources();
    bool processEventHeader();
};

#endif // ADEPOSITIONFILEHANDLER_H
