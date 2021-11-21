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

bool APhotonBombFileHandler::readNextBombOfSameEvent(ANodeRecord & record)
{
    if (EventEndReached) return false;

    if (Settings.FileFormat == ABombFileSettings::Binary)
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
        if (fields.size() < 5)
        {
            AErrorHub::addError("Format error in ascii depo file (deposition record)");
            return false;
        }
        //X Y Z Time Num
        //0 1 2  3    4
        record.R[0]    =  fields[0].toDouble();
        record.R[1]    =  fields[1].toDouble();
        record.R[2]    =  fields[2].toDouble();
        record.Time    =  fields[3].toDouble();
        record.NumPhot =  fields[4].toInt();
    }
    return true;
}

bool APhotonBombFileHandler::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in photon bomb file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (Settings.FileFormat == ABombFileSettings::Binary)
    {
        AErrorHub::addError("Binary photon bomb format not yet implemented");
        return false;
    }
    else
    {
        QFile OutFile(fileName);
        if (!OutFile.open(QIODevice::WriteOnly | QFile::Text))
        {
            AErrorHub::addQError( QString("Cannot open photon bomb output file: " + fileName) );
            return false;
        }
        QTextStream OutStream(&OutFile);

        ANodeRecord * Bomb = ANodeRecord::createS(0,0,0);
        do
        {
            OutStream << '#' << CurrentEvent << '\n';
            while (readNextBombOfSameEvent(*Bomb))
            {
                //X Y Z Time Num
                //0 1 2  3    4
                OutStream << Bomb->R[0]    << ' '
                          << Bomb->R[1]    << ' '
                          << Bomb->R[2]    << ' '
                          << Bomb->Time    << ' '
                          << Bomb->NumPhot << '\n';
            }
            CurrentEvent++;
            acknowledgeNextEvent();
        }
        while (CurrentEvent != toEvent);
    }

    return true;
}
