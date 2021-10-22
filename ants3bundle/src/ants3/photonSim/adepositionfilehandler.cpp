#include "adepositionfilehandler.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <fstream>



// WORK IN PROGRESS



ADepositionFileHandler::~ADepositionFileHandler()
{
    clearResources();
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

    if (Binary)
    {
        inStream = new std::ifstream(FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            AErrorHub::addQError( QString("Cannot open input file: " + FileName) );
            return false;
        }

        char header;
        *inStream >> header;
        if (inStream->bad() || header != (char)0xEE)
        {
            AErrorHub::addError("Bad format in binary energy depo file");
            return false;
        }
    }
    else
    {
        inTextFile = new QFile(FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            AErrorHub::addQError( QString("Cannot open input file: " + FileName) );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);

        QString G4DepoLine = inTextStream->readLine();
        if (!G4DepoLine.startsWith('#'))
        {
            AErrorHub::addError("Format error for #event field in ascii energy deposition file");
            return false;
        }
    }
    return processEventHeader();
}

bool ADepositionFileHandler::processEventHeader()
{
    if (Binary)
    {
        inStream->read((char*)&CurrentEvent, sizeof(int));
        if (inStream->bad())
        {
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
    if (Binary)
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

    AErrorHub::addQError( QString("Could not find event #%0 in depo file %1").arg(iEvent).arg(FileName) );
    return false;
}

bool ADepositionFileHandler::readNextRecordOfSameEvent(ADepoRecord & record)
{
    if (EventEndReached) return false;

    if (Binary)
    {
        AErrorHub::addError("Binary depo not yet implemented");
        return false;
    }
    else
    {
        LineText = inTextStream->readLine();
        if (LineText.startsWith('#'))
        {
            EventEndReached = true;
            return false;
        }
        const QStringList fields = LineText.split(' ', Qt::SkipEmptyParts);
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
