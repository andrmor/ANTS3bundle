#include "aphotonbombfilehandler.h"
#include "aerrorhub.h"
#include "anoderecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

APhotonBombFileHandler::APhotonBombFileHandler(ABombFileSettings & settings) :
    AFileHandlerBase(settings), Settings(settings)
{
    FileType = "photon bomb";
}
