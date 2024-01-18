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

bool AFileHandlerBase::checkFile()
{
    AErrorHub::clear();

    bool ok = init();
    if (!ok) return false; // error already added

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

    if (bFileEndReachedInGoto)
    {
        AErrorHub::clear();
        BaseSettings.LastModified = QFileInfo(BaseSettings.FileName).lastModified();
        return true;
    }
    else return false;
}

bool AFileHandlerBase::collectStatistics()
{
    AErrorHub::clear();

    clearStatistics();

    bool ok = init();
    if (!ok) return false; // error already added

    if (CurrentEvent != 0)
    {
        AErrorHub::addQError("File does not start from event number zero!");
        return false;
    }

    BaseSettings.NumEvents = 0;
    int expectedNextEvent = 1;
    while (true)
    {
        acknowledgeNextEvent();
        fillStatisticsForCurrentEvent();
        BaseSettings.NumEvents++;

        if (atEnd() && !LineText.startsWith('#')) break;  // file can end with an empty event, still need to register it

        bool ok = processEventHeader();
        if (!ok || CurrentEvent != expectedNextEvent)
        {
            AErrorHub::addQError("Bad format of the deposition file!");
            BaseSettings.FileFormat = AFileSettingsBase::Invalid;
            BaseSettings.NumEvents = -1;
            return false;
        }
        expectedNextEvent++;
    }

    BaseSettings.LastModified = QFileInfo(BaseSettings.FileName).lastModified();
    return true;
}

bool AFileHandlerBase::init()
{
    clearResources();

    if (BaseSettings.FileName.isEmpty())
    {
        AErrorHub::addQError("File name is empty!");
        BaseSettings.FileFormat = AFileSettingsBase::Undefined;
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
    bFileEndReachedInGoto = false;

    if (CurrentEvent == iEvent && !ReadingEvent) return true;

    if (CurrentEvent >= iEvent)
    {
        init();
        if (CurrentEvent == iEvent) return true;
    }

    // iEvent is larger than CurrentEvent
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        //AErrorHub::addError("Binary format not yet implemented"); return false;
        while (true)
        {
            acknowledgeNextEvent();
            dummyReadBinaryDataUntilNewEvent();
            if (AErrorHub::isError()) return false;
            if (atEnd()) break;

            bool ok = processEventHeader();
            if (!ok)
            {
                AErrorHub::addError("Error processing event header in gotoEvent method");
                return false;
            }
            if (CurrentEvent == iEvent) return true;
        }
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

    bFileEndReachedInGoto = true;
    AErrorHub::addError("Cannot reach requested event number in the file");
    return false;
}

bool AFileHandlerBase::atEnd() const
{
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
        return inStream->eof();
    else if (BaseSettings.FileFormat == AFileSettingsBase::Ascii)
        return inTextStream->atEnd();
    return true;
}

bool AFileHandlerBase::readNextRecordSameEvent(ADataIOBase & record)
{
    if (EventEndReached) return false;

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        char header;
        *inStream >> header;

        if (inStream->eof())
        {
            EventEndReached = true;
            return false;
        }
        if (inStream->bad())
        {
            AErrorHub::addError("Error reading input stream");
            return false;
        }

        if (header == (char)0xFF)
            return record.readBinary(*inStream);
        if (header == (char)0xEE)
        {
            EventEndReached = true;
            return false;
        }
        AErrorHub::addError("Unexpected format of a line in the binary file with the deposition data");
        return false;
    }
    else
    {
        if (inTextStream->atEnd())
        {
            EventEndReached = true;
            return false;
        }

        do
        {
            LineText = inTextStream->readLine();
            if (inTextStream->atEnd() && LineText.isEmpty()) // empty line at the end of the file
            {
                EventEndReached = true;
                return false;
            }
        }
        while (LineText.isEmpty());

        if (LineText.startsWith('#'))
        {
            EventEndReached = true;
            return false;
        }
        else ReadingEvent = true;

        return record.readAscii(LineText);
    }
}

static int tmpInt;
void AFileHandlerBase::skipToNextEventRecord()
{
    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
        inStream->read((char*)&tmpInt, sizeof(int)); // cannot use DepoHandler->processEventHeader() as it will override CurrentEvent
}

bool AFileHandlerBase::copyToFileBuffered(int fromEvent, int toEvent, const QString & fileName, ADataIOBase & buffer)
{
    qDebug() << "!!!----->" << fromEvent << toEvent;
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in depo file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (BaseSettings.FileFormat == AFileSettingsBase::Binary)
    {
        //AErrorHub::addError("Binary depo not yet implemented"); return false;
        std::ofstream OutStream(fileName.toLatin1().data(), std::ios::out | std::ios::binary);
        if (!OutStream.is_open())
        {
            AErrorHub::addQError( QString("Cannot open binary depo output file: " + fileName) );
            return false;
        }

        //int tmpInt;
        do
        {
            OutStream << char(0xEE);
            OutStream.write((char*)&CurrentEvent, sizeof(int));
            while (readNextRecordSameEvent(buffer))
                buffer.writeBinary(OutStream);
            //inStream->read((char*)&tmpInt, sizeof(int)); // cannot use processEventHeader() as it will override local CurrentEvent
            skipToNextEventRecord();
            CurrentEvent++;
            acknowledgeNextEvent();
        }
        while (CurrentEvent != toEvent && !atEnd());
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

    //int tmpInt;
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
        //if (BaseSettings.FileFormat == AFileSettingsBase::Binary) inStream->read((char*)&tmpInt, sizeof(int)); // cannot use processEventHeader() as it will override local CurrentEvent
        skipToNextEventRecord();
        CurrentEvent++;

        if (atEnd()) break;
    }

    return text;
}

void AFileHandlerBase::dummyReadBinaryDataUntilNewEvent()
{
    AErrorHub::addQError("Not yet implemented 'dummyReadBinaryDataUntilNewEvent()' for sub-class of AFileHandlerBase");
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
