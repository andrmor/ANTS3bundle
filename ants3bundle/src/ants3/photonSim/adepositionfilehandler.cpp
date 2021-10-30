#include "adepositionfilehandler.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

// WORK IN PROGRESS

ADepositionFileHandler::ADepositionFileHandler(APhotonDepoSettings & depoSettings) :
    Settings(depoSettings) {}

ADepositionFileHandler::~ADepositionFileHandler()
{
    clearResources();
}

bool ADepositionFileHandler::checkFile(bool collectStatistics)
{
    bool ok = init();
    if (!ok) return false; // error already added

    Settings.FileLastModified = QFileInfo(Settings.FileName).lastModified();

    Settings.NumEvents = 1;
    while (gotoEvent(CurrentEvent + 1))
    {
        if (AErrorHub::isError())
        {
            Settings.FileFormat = APhotonDepoSettings::Invalid;
            return false;
        }
        Settings.NumEvents++;
    }
    return true;
}

void ADepositionFileHandler::clearResources()
{
    delete inStream;     inStream     = nullptr;

    delete inTextStream; inTextStream = nullptr;

    if (inTextFile) inTextFile->close();
    delete inTextFile;   inTextFile   = nullptr;

    CurrentEvent = -1;
    EventEndReached = false;
}

bool ADepositionFileHandler::init()
{
    clearResources();

    if (Settings.FileFormat == APhotonDepoSettings::G4Binary)
    {
        inStream = new std::ifstream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            Settings.FileFormat = APhotonDepoSettings::Undefined;
            AErrorHub::addQError( QString("Cannot open input file: " + Settings.FileName) );
            return false;
        }

        char header;
        *inStream >> header;
        if (inStream->bad() || header != (char)0xEE)
        {
            Settings.FileFormat = APhotonDepoSettings::Invalid;
            AErrorHub::addError("Bad format in binary energy depo file");
            return false;
        }
    }
    else
    {
        inTextFile = new QFile(Settings.FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            Settings.FileFormat = APhotonDepoSettings::Undefined;
            AErrorHub::addQError( QString("Cannot open input file: " + Settings.FileName) );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);

        LineText = inTextStream->readLine();
        if (!LineText.startsWith('#'))
        {
            Settings.FileFormat = APhotonDepoSettings::Invalid;
            AErrorHub::addError("Format error for #event field in ascii energy deposition file");
            return false;
        }
    }
    return processEventHeader();
}

bool ADepositionFileHandler::processEventHeader()
{
    if (Settings.FileFormat == APhotonDepoSettings::G4Binary)
    {
        inStream->read((char*)&CurrentEvent, sizeof(int));
        if (inStream->bad())
        {
            Settings.FileFormat = APhotonDepoSettings::Invalid;
            AErrorHub::addError("Bad format in binary energy depo file (event record)");
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
            Settings.FileFormat = APhotonDepoSettings::Invalid;
            AErrorHub::addError("Bad format in ascii energy depo file (event record)");
            return false;
        }
    }
    return true;
}

bool ADepositionFileHandler::gotoEvent(int iEvent)
{
    if (CurrentEvent == iEvent) return true;
    if (CurrentEvent > iEvent)
    {
        init();
        if (CurrentEvent == iEvent) return true;
    }

    // iEvent is larger than CurrentEvent
    if (Settings.FileFormat == APhotonDepoSettings::G4Binary)
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

bool ADepositionFileHandler::readNextRecordOfSameEvent(ADepoRecord & record)
{
    if (EventEndReached) return false;

    if (Settings.FileFormat == APhotonDepoSettings::G4Binary)
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

        const QStringList fields = LineText.split(' ', Qt::SkipEmptyParts);
        //qDebug() << "------------>" << fields;
//        if (fields.isEmpty()) break; //last event had no depo - end of file reached
        if (fields.size() < 7)
        {
            AErrorHub::addError("Format error in ascii depo file (deposition record)");
            return false;
        }
        //particle mId dE x y z t
        // 0        1   2 3 4 5 6
        record.Particle =  fields[0];
        record.MatIndex =  fields[1].toInt();
        record.Energy   =  fields[2].toDouble();
        record.Pos      = {fields[3].toDouble(),
                           fields[4].toDouble(),
                           fields[5].toDouble()};
        record.Time     =  fields[6].toDouble();
    }
    return true;
}

void ADepositionFileHandler::determineFormat()
{
    QFile TextFile(Settings.FileName);
    if (!TextFile.open(QIODevice::ReadOnly | QFile::Text))
    {
        Settings.FileFormat = APhotonDepoSettings::Invalid;
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
            Settings.FileFormat = APhotonDepoSettings::G4Ascii;
            return;
        }
    }

    std::ifstream inStream(Settings.FileName.toLatin1().data(), std::ios::in | std::ios::binary);
    if (!inStream.is_open())
    {
        Settings.FileFormat = APhotonDepoSettings::Invalid;
        return;
    }
    char header = 0x00;
    inStream >> header;
    if (inStream.good() && header == (char)0xEE)
    {
        Settings.FileFormat = APhotonDepoSettings::G4Binary;
        return;
    }

    Settings.FileFormat = APhotonDepoSettings::Undefined;
}

bool ADepositionFileHandler::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in depo file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (Settings.FileFormat == APhotonDepoSettings::G4Binary)
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
            ADepoRecord r;
            while (readNextRecordOfSameEvent(r))
            {
                //particle mId dE x y z t
                // 0        1   2 3 4 5 6
                OutStream << r.Particle << ' '
                          << r.MatIndex << ' '
                          << r.Energy   << ' '
                          << r.Pos[0]   << ' '
                          << r.Pos[1]   << ' '
                          << r.Pos[2]   << ' '
                          << r.Time     << '\n';
            }
            CurrentEvent++;
            acknowledgeNextEvent();
        }
        while (CurrentEvent != toEvent);
    }

    return true;
}


//while (!inTextStream->atEnd());
/*
bool ADepositionFileHandler::readG4DepoEventFromBinFile(bool expectNewEvent)
{
    char header = 0;
    while (*inStream >> header)
    {
        if (header == char(0xee))
        {
            inStream->read((char*)&G4NextEventId, sizeof(int));
            //qDebug() << G4NextEventId << "expecting:" << eventCurrent+1;
            if (G4NextEventId != CurrentEvent + 1)
            {
                AErrorHub::addError( QString("Bad event number in G4ants energy depo file: expected %1 and got %2").arg(CurrentEvent, G4NextEventId).toLatin1().data() );
                return false;
            }
            return true; //ready to read this event
        }
        else if (expectNewEvent)
        {
            AErrorHub::addError( "New event tag not found in G4ants energy deposition file" );
            return false;
        }
        else if (header == char(0xff))
        {
            AEnergyDepositionCell * cell = new AEnergyDepositionCell();

            // format:
            // partId(0-terminated string) matId(int) DepoE(double) X(double) Y(double) Z(double) Time(double)
            inStream->read((char*)&cell->ParticleId, sizeof(int));
            inStream->read((char*)&cell->MaterialId, sizeof(int));
            inStream->read((char*)&cell->dE,         sizeof(double));
            inStream->read((char*)&cell->r[0],       sizeof(double));
            inStream->read((char*)&cell->r[1],       sizeof(double));
            inStream->read((char*)&cell->r[2],       sizeof(double));
            inStream->read((char*)&cell->time,       sizeof(double));

            EnergyVector << cell;
        }
        else
        {
            AErrorHub::addError("Unexpected format of record header for binary input in G4ants deposition file");
            return false;
        }

        if (inStream->eof() ) return true;
    }

    if (inStream->eof() ) return true;
    else if (inStream->fail())
    {
        AErrorHub::addError("Unexpected format of binary input in G4ants deposition file");
        return false;
    }

    qWarning() << "Error - 'should not be here' reached";
    return true; //should not be here
}
*/
