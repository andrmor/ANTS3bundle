#include "adepositionfilehandler.h"
#include "aerrorhub.h"
#include "adeporecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

ADepositionFileHandler::ADepositionFileHandler(APhotonDepoSettings & depoSettings) :
    AFileHandlerBase(depoSettings), Settings(depoSettings) {}
