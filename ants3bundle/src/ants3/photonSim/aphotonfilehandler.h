#ifndef APHOTONFILEHANDLER_H
#define APHOTONFILEHANDLER_H

#include "afilehandlerbase.h"
#include "aphotonsimsettings.h"

class APhoton;

class APhotonFileHandler : public AFileHandlerBase
{
public:
    APhotonFileHandler(APhotonFileSettings & settings);

protected:
    APhotonFileSettings & Settings; // !!!*** maybe base is enough?
};

#endif // APHOTONFILEHANDLER_H
