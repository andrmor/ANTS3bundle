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

bool APhotonFileHandler::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in %0 copy procedure for file:\n%1").arg(FileType, fromEvent) );
        return false;
    }

    if (Settings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary format not yet implemented");
        return false;
    }
    else
    {
        QFile OutFile(fileName);
        if (!OutFile.open(QIODevice::WriteOnly | QFile::Text))
        {
            AErrorHub::addQError( QString("Cannot open %0 output file:\n%1").arg(FileType, fileName) );
            return false;
        }
        QTextStream OutStream(&OutFile);

        APhoton Photon;
        do
        {
            OutStream << '#' << CurrentEvent << '\n';
            while (readNextPhotonOfSameEvent(Photon))
            {
                //X Y Z dX dY dZ Time iWave
                //0 1 2 3  4  5    6    7
                OutStream << Photon.r[0]      << ' '
                          << Photon.r[1]      << ' '
                          << Photon.r[2]      << ' '
                          << Photon.v[0]      << ' '
                          << Photon.v[1]      << ' '
                          << Photon.v[2]      << ' '
                          << Photon.time      << ' '
                          << Photon.waveIndex
                          << '\n';
            }
            CurrentEvent++;
            acknowledgeNextEvent();
        }
        while (CurrentEvent != toEvent);
    }

    return true;
}
