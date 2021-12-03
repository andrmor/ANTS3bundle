#include "aphotonfilehandler.h"
#include "aerrorhub.h"
#include "aphoton.h"

#include <QTextStream>
#include <QFile>

APhotonFileHandler::APhotonFileHandler(APhotonFileSettings & settings) :
    AFileHandlerBase(settings), Settings(settings)
{
    FileType = "photon record";
}
