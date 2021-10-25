#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "adeporecord.h"
#include "aphotonsimsettings.h"

#include <QString>

class QFile;
class QTextStream;

class ADepositionFileHandler
{
public:
    ADepositionFileHandler(APhotonDepoSettings & depoSettings); // reformat to add rundir? !!!***
    virtual ~ADepositionFileHandler();

    void determineFormat(); // very simplistic, better to make more strict !!!***
    bool checkFile(bool collectStatistics); // !!!*** add statistics!

    bool init();
    bool gotoEvent(int iEvent);

    bool readNextRecordOfSameEvent(ADepoRecord & record); // returns false if event ended
    void acknowledgeNextEvent() {EventEndReached = false;}

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName);

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
