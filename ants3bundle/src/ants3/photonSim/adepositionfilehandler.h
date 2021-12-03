#ifndef ADEPOSITIONFILEHANDLER_H
#define ADEPOSITIONFILEHANDLER_H

#include "aphotonsimsettings.h"
#include "afilehandlerbase.h"

class ADepositionFileHandler : public AFileHandlerBase
{
public:
    ADepositionFileHandler(APhotonDepoSettings & depoSettings);

private:
    APhotonDepoSettings & Settings;
};

#endif // ADEPOSITIONFILEHANDLER_H
