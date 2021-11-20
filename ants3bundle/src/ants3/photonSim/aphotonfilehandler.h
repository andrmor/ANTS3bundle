#ifndef APHOTONFILEHANDLER_H
#define APHOTONFILEHANDLER_H

#include "afilehandlerbase.h"
#include "aphotonsimsettings.h" // !!!*** to cpp?

class APhoton;

class APhotonFileHandler : public AFileHandlerBase
{
public:
    APhotonFileHandler(APhotonFileSettings & settings);

    bool readNextPhotonOfSameEvent(APhoton & photon); // returns false if event ended

    bool copyToFile(int fromEvent, int toEvent, const QString &fileName);

protected:
    APhotonFileSettings & Settings; // !!!*** maybe base is enough? just for the future
};

#endif // APHOTONFILEHANDLER_H
