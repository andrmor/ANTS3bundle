#ifndef APHOTONBOMBFILEHANDLER_H
#define APHOTONBOMBFILEHANDLER_H

#include "aphotonsimsettings.h"
#include "afilehandlerbase.h"

class ANodeRecord;

class APhotonBombFileHandler : public AFileHandlerBase
{
public:
    APhotonBombFileHandler(ABombFileSettings & settings);

    //bool checkFile(bool collectStatistics); // !!!*** add statistics!

    bool readNextBombOfSameEvent(ANodeRecord & record); // returns false if event ended

    bool copyToFile(int fromEvent, int toEvent, const QString & fileName);

private:
    ABombFileSettings & Settings;
};

#endif // APHOTONBOMBFILEHANDLER_H
