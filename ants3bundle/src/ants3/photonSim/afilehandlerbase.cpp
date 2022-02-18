#include "afilehandlerbase.h"
#include "aerrorhub.h"
#include "adataiobase.h"
#include "ajsontools.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

AFileHandlerBase::AFileHandlerBase(AFileSettingsBase & settings) : BaseSettings(settings) {}

AFileHandlerBase::~AFileHandlerBase()
{
    clearResources();
}

void AFileHandlerBase::determineFormat()
{
    QFile TextFile(BaseSettings.FileName);
    if (!TextFile.open(QIODevice::ReadOnly | QFile::Text))
    {
        BaseSettings.FileFormat = AFileSettingsBase::Invalid;
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
            BaseSettings.FileFormat = AFileSettingsBase::Ascii;
            return;
        }
    }

    std::ifstream inStream(BaseSettings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        BaseSettings.FileFormat = AFileSettingsBase::Invalid;
        return;
    }
    char header = 0x00;
    inStream >> header;
    if (inStream.good() && header == (char)0xEE)
    {
        BaseSettings.FileFormat = AFileSettingsBase::Binary;
        return;
    }

    BaseSettings.FileFormat = AFileSettingsBase::Undefined;
}

bool AFileHandlerBase::checkFile(bool collectStatistics)
{
    AErrorHub::clear();

    bool ok = init();
    if (!ok) return false; // error already added

    BaseSettings.LastModified = QFileInfo(BaseSettings.FileName).lastModified();

    BaseSettings.NumEvents = 1;
    while (gotoEvent(CurrentEvent + 1))
    {
        if (AErrorHub::isError())
        {
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            return false;
        }
        BaseSettings.NumEvents++;
    }
    return true;
}

bool AFileHandlerBase::init()
{
    clearResources();

    if (BaseSettings.FileName.isEmpty())
    {
        AErrorHub::addQError("File name is empty!");
        return false;
    }

    if (!BaseSettings.isValidated())
    {
        determineFormat();

        if (BaseSettings.FileFormat == AFileSettingsBase::Invalid)
        {
            AErrorHub::addQError("Cannot open file: " + BaseSettings.FileName);
            return false;
        }
        if (BaseSettings.FileFormat == AFileSettingsBase::Undefined)
        {
            AErrorHub::addError("Invalid format of the file!");
            return false;
        }
    }

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        inStream = new std::ifstream(BaseSettings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            BaseSettings.FileFormat = AFileSettingsBase::Undefined;
            AErrorHub::addQError( QString("Cannot open %0 input file: %1").arg(FileType, BaseSettings.FileName) );
            return false;
        }

        char header;
        *inStream >> header;
        if (inStream->bad() || header != (char)0xEE)
        {
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of binary %0 input file: %1").arg(FileType, BaseSettings.FileName) );
            return false;
        }
    }
    else
    {
        inTextFile = new QFile(BaseSettings.FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            BaseSettings.FileFormat = AFileSettingsBase::Undefined;
            AErrorHub::addQError( QString("Cannot open %0 input file: %1").arg(FileType, BaseSettings.FileName) );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);

        LineText = inTextStream->readLine();
        if (!LineText.startsWith('#'))
        {
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of ascii %0 input file: %1").arg(FileType, BaseSettings.FileName) );
            return false;
        }
    }
    return processEventHeader();
}

bool AFileHandlerBase::isInitialized() const
{
    if (!BaseSettings.isValidated()) return false;

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        if (!inStream) return false;
        return inStream->is_open();
    }
    else
    {
        if (!inTextFile || !inTextStream) return false;
        return true;
    }
}

bool AFileHandlerBase::gotoEvent(int iEvent)
{
    if (CurrentEvent == iEvent && !ReadingEvent) return true;

    if (CurrentEvent >= iEvent)
    {
        init();
        if (CurrentEvent == iEvent) return true;
    }

    // iEvent is larger than CurrentEvent
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary format not yet implemented"); // !!!***
        return false;
    }
    else
    {
        do
        {
            LineText = inTextStream->readLine();
            if (!LineText.startsWith('#')) continue;
            bool ok = processEventHeader();
            if (!ok)
            {
                AErrorHub::addError("Error processing event header in gotoEvent method");
                return false;
            }

            if (CurrentEvent == iEvent) return true;
        }
        while (!inTextStream->atEnd());
    }

    AErrorHub::addError("Cannot reach requested event number in the file");
    return false;
}

bool AFileHandlerBase::atEnd() const
{
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
        return true; // !!!***
    else if (BaseSettings.FileFormat == AFileSettingsBase::Ascii)
        return inTextStream->atEnd();
    return true;
}

bool AFileHandlerBase::readNextRecordSameEvent(ADataIOBase & record)
{
    if (EventEndReached) return false;

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary depo not yet implemented");
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
        else ReadingEvent = true;

        return record.readAscii(LineText);
    }
}

bool AFileHandlerBase::copyToFileBuffered(int fromEvent, int toEvent, const QString & fileName, ADataIOBase & buffer)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in depo file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        AErrorHub::addError("Binary depo not yet implemented");
        return false;
    }
    else
    {
        QFile OutFile(fileName);
        if (!OutFile.open(QIODevice::WriteOnly | QFile::Text))
        {
            AErrorHub::addQError( QString("Cannot open depo output file: " + fileName) );
            return false;
        }
        QTextStream OutStream(&OutFile);

        do
        {
            OutStream << '#' << CurrentEvent << '\n';
            while (readNextRecordSameEvent(buffer))
                buffer.writeAscii(OutStream);
            CurrentEvent++;
            acknowledgeNextEvent();
        }
        while (CurrentEvent != toEvent);
    }

    return true;
}

QString AFileHandlerBase::preview(ADataIOBase & buffer, int numLines)
{
    bool ok = init();
    if (!ok) return AErrorHub::getQError();

    QString text;

    while (numLines > 0)
    {
        text += "#" + QString::number(CurrentEvent) + "\n";
        numLines--;

        while (readNextRecordSameEvent(buffer))
        {
            buffer.print(text);
            numLines--;
            if (numLines < 0) break;
        }
        acknowledgeNextEvent();
        CurrentEvent++;

        if (atEnd()) break;
    }

    return text;
}

void AFileHandlerBase::clearResources()
{
    delete inStream;     inStream     = nullptr;

    delete inTextStream; inTextStream = nullptr;

    if (inTextFile) inTextFile->close();
    delete inTextFile;   inTextFile   = nullptr;

    CurrentEvent    = -1;
    EventEndReached = false;
    ReadingEvent    = false;
}

bool AFileHandlerBase::processEventHeader()
{
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        inStream->read((char*)&CurrentEvent, sizeof(int));
        if (inStream->bad())
        {
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of binary %0 input file: %1").arg(FileType, BaseSettings.FileName) );
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
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            AErrorHub::addQError( QString("Bad format of ascii %0 input file: %1").arg(FileType, BaseSettings.FileName) );
            return false;
        }
    }

    ReadingEvent = false;
    return true;
}
