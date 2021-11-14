#include "aphotonbombfilehandler.h"
#include "aerrorhub.h"
#include "anoderecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

APhotonBombFileHandler::APhotonBombFileHandler(ANodeFileSettings & settings) :
    Settings(settings) {}

APhotonBombFileHandler::~APhotonBombFileHandler()
{
    clearResources();
}

bool APhotonBombFileHandler::checkFile(bool collectStatistics)
{
    bool ok = init();
    if (!ok) return false; // error already added

    Settings.LastModified = QFileInfo(Settings.FileName).lastModified();

    Settings.NumEvents = 1;
    while (gotoEvent(CurrentEvent + 1))
    {
        if (AErrorHub::isError())
        {
            Settings.FileFormat = ANodeFileSettings::Invalid;
            return false;
        }
        Settings.NumEvents++;
    }
    return true;
}

void APhotonBombFileHandler::clearResources()
{
    delete inStream;     inStream     = nullptr;

    delete inTextStream; inTextStream = nullptr;

    if (inTextFile) inTextFile->close();
    delete inTextFile;   inTextFile   = nullptr;

    CurrentEvent = -1;
    EventEndReached = false;
}

bool APhotonBombFileHandler::init()
{
    clearResources();

    if (Settings.FileFormat == ANodeFileSettings::Binary)
    {
        inStream = new std::ifstream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            Settings.FileFormat = ANodeFileSettings::Undefined;
            AErrorHub::addQError( QString("Cannot open input file: " + Settings.FileName) );
            return false;
        }

        char header;
        *inStream >> header;
        if (inStream->bad() || header != (char)0xEE)
        {
            Settings.FileFormat = ANodeFileSettings::Invalid;
            AErrorHub::addError("Bad format in binary photon bomb file");
            return false;
        }
    }
    else
    {
        inTextFile = new QFile(Settings.FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            Settings.FileFormat = ANodeFileSettings::Undefined;
            AErrorHub::addQError( QString("Cannot open input file: " + Settings.FileName) );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);

        LineText = inTextStream->readLine();
        if (!LineText.startsWith('#'))
        {
            Settings.FileFormat = ANodeFileSettings::Invalid;
            AErrorHub::addError("Format error for #event field in ascii photon bomb file");
            return false;
        }
    }
    return processEventHeader();
}

bool APhotonBombFileHandler::processEventHeader()
{
    if (Settings.FileFormat == ANodeFileSettings::Binary)
    {
        inStream->read((char*)&CurrentEvent, sizeof(int));
        if (inStream->bad())
        {
            Settings.FileFormat = ANodeFileSettings::Invalid;
            AErrorHub::addError("Bad format in binary photon bomb file (event record)");
            return false;
        }
    }
    else
    {
        LineText.remove(0, 1);
        bool ok;
        CurrentEvent = LineText.toInt(&ok);
        if (!ok)
        {
            Settings.FileFormat = ANodeFileSettings::Invalid;
            AErrorHub::addError("Bad format in ascii photon bomb file (event record)");
            return false;
        }
    }
    return true;
}

bool APhotonBombFileHandler::gotoEvent(int iEvent)
{
    if (CurrentEvent == iEvent) return true;
    if (CurrentEvent > iEvent)
    {
        init();
        if (CurrentEvent == iEvent) return true;
    }

    // iEvent is larger than CurrentEvent
    if (Settings.FileFormat == ANodeFileSettings::Binary)
    {
        AErrorHub::addError("Binary depo not yet implemented");
        return false;
    }
    else
    {
        do
        {
            LineText = inTextStream->readLine();
            if (!LineText.startsWith('#')) continue;
            bool ok = processEventHeader();
            if (!ok) return false;

            if (CurrentEvent == iEvent) return true;
        }
        while (!inTextStream->atEnd());
    }

    return false;
}

bool APhotonBombFileHandler::readNextBombOfSameEvent(ANodeRecord & record)
{
    if (EventEndReached) return false;

    if (Settings.FileFormat == ANodeFileSettings::Binary)
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

void APhotonBombFileHandler::determineFormat()
{
    QFile TextFile(Settings.FileName);
    if (!TextFile.open(QIODevice::ReadOnly | QFile::Text))
    {
        Settings.FileFormat = ANodeFileSettings::Invalid;
        return;
    }
    QTextStream TextStream(&TextFile);

    QString Line = TextStream.readLine();
    if (Line.startsWith('#'))
    {
        Line.remove(0, 1);
        bool ok;
        Line.toInt(&ok);
        if (ok)
        {
            Settings.FileFormat = ANodeFileSettings::Ascii;
            return;
        }
    }

    std::ifstream inStream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        Settings.FileFormat = ANodeFileSettings::Invalid;
        return;
    }
    char header = 0x00;
    inStream >> header;
    if (inStream.good() && header == (char)0xEE)
    {
        Settings.FileFormat = ANodeFileSettings::Binary;
        return;
    }

    Settings.FileFormat = ANodeFileSettings::Undefined;
}

bool APhotonBombFileHandler::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in photon bomb file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (Settings.FileFormat == ANodeFileSettings::Binary)
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
