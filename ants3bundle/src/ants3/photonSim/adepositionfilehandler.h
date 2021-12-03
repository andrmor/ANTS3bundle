#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "aphotonsimsettings.h"
#include "afilehandlerbase.h"
#include "adeporecord.h"

class ADepositionFileHandler : public AFileHandlerBase
{
public:
    ADepositionFileHandler(APhotonDepoSettings & depoSettings);

    //bool checkFile(bool collectStatistics); // !!!*** add statistics!

    //bool readNextRecordOfSameEvent(ADepoRecord & record); // returns false if event ended

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName) {};

private:
    APhotonDepoSettings & Settings;

};

#endif // ADEPOSITIONFILEHANDLER_H
