#include "afilehandlerbase.h"
#include "aerrorhub.h"
#include "ajsontools.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

// --- settings

bool AFileSettingsBase::isValidated() const
{
    if (FileFormat == Undefined || FileFormat == Invalid) return false;

    QFileInfo fi(FileName);
    if (!fi.exists()) return false;
    if (LastModified != fi.lastModified()) return false;

    return true;
}

QString AFileSettingsBase::getFormatName() const
{
    switch (FileFormat)
    {
    case Binary:  return "Binary";
    case Ascii:   return "Ascii";
    case Invalid: return "Invalid";
    default:;
    }
    return "Undefined";
}

void AFileSettingsBase::writeToJson(QJsonObject & json) const
{
    json["FileName"]     = FileName;
    json["NumEvents"]    = NumEvents;
    json["FileFormat"]   = getFormatName();
    json["LastModified"] = LastModified.toMSecsSinceEpoch();

    doWriteToJson(json);
}

void AFileSettingsBase::readFromJson(const QJsonObject & json)
{
    clear();

    jstools::parseJson(json, "FileName",  FileName);
    jstools::parseJson(json, "NumEvents", NumEvents);

    QString fstr;
    jstools::parseJson(json, "FileFormat", fstr);
    if      (fstr == "G4Binary") FileFormat = Binary;
    else if (fstr == "G4Ascii")  FileFormat = Ascii;
    else if (fstr == "Invalid")  FileFormat = Invalid;
    else                         FileFormat = Undefined;

    qint64 lastMod;
    jstools::parseJson(json, "LastModified", lastMod);
    LastModified = QDateTime::fromMSecsSinceEpoch(lastMod);

    doReadFromJson(json);
}

void AFileSettingsBase::clear()
{
    FileName.clear();
    FileFormat   = Undefined;
    NumEvents    = 0;
    LastModified = QDateTime();

    clearStatistics();
}

// --- file handler

AFileHandlerBase::AFileHandlerBase(AFileSettingsBase & settings) : Settings(settings) {}

AFileHandlerBase::~AFileHandlerBase()
{
    clearResources();
}

void AFileHandlerBase::determineFormat()
{
    QFile TextFile(Settings.FileName);
    if (!TextFile.open(QIODevice::ReadOnly | QFile::Text))
    {
        Settings.FileFormat = AFileSettingsBase::Invalid;
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
            Settings.FileFormat = AFileSettingsBase::Ascii;
            return;
        }
    }

    std::ifstream inStream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        Settings.FileFormat = AFileSettingsBase::Invalid;
        return;
    }
    char header = 0x00;
    inStream >> header;
    if (inStream.good() && header == (char)0xEE)
    {
        Settings.FileFormat = AFileSettingsBase::Binary;
        return;
    }

    Settings.FileFormat = AFileSettingsBase::Undefined;
}

bool AFileHandlerBase::checkFile(bool collectStatistics)
{
    bool ok = init();
    if (!ok) return false; // error already added

    Settings.LastModified = QFileInfo(Settings.FileName).lastModified();

    Settings.NumEvents = 1;
    while (gotoEvent(CurrentEvent + 1))
    {
        if (AErrorHub::isError())
        {
            Settings.FileFormat = AFileSettingsBase::Invalid;
            return false;
        }
        Settings.NumEvents++;
    }
    return true;
}

bool AFileHandlerBase::init()
{
    clearResources();

    if (Settings.FileFormat == AFileSettingsBase::Binary)
    {
        inStream = new std::ifstream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            Settings.FileFormat = AFileSettingsBase::Undefined;
            AErrorHub::addQError( QString("Cannot open %0 input file: %1").arg(FileType, Settings.FileName) );
            return false;
        }

        char header;
        *inStream >> header;
        if (inStream->bad() || header != (char)0xEE)
        {
            Settings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of binary %0 input file: %1").arg(FileType, Settings.FileName) );
            return false;
        }
    }
    else
    {
        inTextFile = new QFile(Settings.FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            Settings.FileFormat = AFileSettingsBase::Undefined;
            AErrorHub::addQError( QString("Cannot open %0 input file: %1").arg(FileType, Settings.FileName) );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);

        LineText = inTextStream->readLine();
        if (!LineText.startsWith('#'))
        {
            Settings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of ascii %0 input file: %1").arg(FileType, Settings.FileName) );
            return false;
        }
    }
    return processEventHeader();
}

bool AFileHandlerBase::gotoEvent(int iEvent)
{
    if (CurrentEvent == iEvent) return true;
    if (CurrentEvent > iEvent)
    {
        init();
        if (CurrentEvent == iEvent) return true;
    }

    // iEvent is larger than CurrentEvent
    if (Settings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary format not yet implemented");
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

void AFileHandlerBase::clearResources()
{
    delete inStream;     inStream     = nullptr;

    delete inTextStream; inTextStream = nullptr;

    if (inTextFile) inTextFile->close();
    delete inTextFile;   inTextFile   = nullptr;

    CurrentEvent = -1;
    EventEndReached = false;
}

bool AFileHandlerBase::processEventHeader()
{
    if (Settings.FileFormat == AFileSettingsBase::Binary)
    {
        inStream->read((char*)&CurrentEvent, sizeof(int));
        if (inStream->bad())
        {
            Settings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of binary %0 input file: %1").arg(FileType, Settings.FileName) );
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
            Settings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of ascii %0 input file: %1").arg(FileType, Settings.FileName) );
            return false;
        }
    }
    return true;
}

bool AFileHandlerBase::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    AErrorHub::addQError( QString("There is no implementation for %0 file copy procedure!").arg(FileType) );
    return false;

    /*
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
    */
}
