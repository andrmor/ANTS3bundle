#include "aphotonfilehandler.h"
#include "aerrorhub.h"
#include "aphoton.h"

#include <QTextStream>

APhotonFileHandler::APhotonFileHandler(APhotonFileSettings & settings) :
    AFileHandlerBase(settings), Settings(settings)
{
    FileType = "photon record";
}

bool APhotonFileHandler::readNextPhotonOfSameEvent(APhoton & photon)
{
    if (EventEndReached) return false;

    if (Settings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary photon bomb format not yet implemented");
        return false;
    }
    else
    {
        if (inTextStream->atEnd())
        {
            EventEndReached = true;
            return false;
        }
        LineText = inTextStream->readLine();
        if (LineText.startsWith('#'))
        {
            EventEndReached = true;
            return false;
        }

        const QStringList fields = LineText.split(' ', Qt::SkipEmptyParts);
        //qDebug() << "------------>" << fields;
//        if (fields.isEmpty()) break; //last event had no depo - end of file reached
        if (fields.size() < 8)
        {
            AErrorHub::addError("Format error in ascii photon record file (photon record)");
            return false;
        }
        //X Y Z dX dY dZ Time iWave
        //0 1 2 3  4  5    6    7
        photon.r[0]      =  fields[0].toDouble();
        photon.r[1]      =  fields[1].toDouble();
        photon.r[2]      =  fields[2].toDouble();
        photon.v[0]      =  fields[3].toDouble();
        photon.v[1]      =  fields[4].toDouble();
        photon.v[2]      =  fields[5].toDouble();
        photon.time      =  fields[6].toDouble();
        photon.waveIndex =  fields[7].toInt();
    }
    return true;
}
