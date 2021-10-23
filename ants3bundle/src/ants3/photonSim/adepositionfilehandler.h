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
    ADepositionFileHandler(const QString & fileName, APhotonDepoSettings::EFormat format);
    virtual ~ADepositionFileHandler();

    int  checkFile(bool collectStatistics); // returns number of events    !!!*** add statistics!

    bool init();
    bool gotoEvent(int iEvent);

    bool readNextRecordOfSameEvent(ADepoRecord & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    static APhotonDepoSettings::EFormat determineFormat(const QString & FileName); // very simplistic, better to make more strict !!!***

private:
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

    void clearResources();
    bool processEventHeader();
};

#endif // ADEPOSITIONFILEHANDLER_H
