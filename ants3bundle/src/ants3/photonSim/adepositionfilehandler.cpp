#include "adepositionfilehandler.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

// WORK IN PROGRESS

ADepositionFileHandler::ADepositionFileHandler(APhotonDepoSettings & depoSettings) :
    AFileHandlerBase(depoSettings), Settings(depoSettings) {}

bool ADepositionFileHandler::readNextRecordOfSameEvent(ADepoRecord & record)
{
    if (EventEndReached) return false;

    if (Settings.FileFormat == AFileSettingsBase::Binary)
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

bool ADepositionFileHandler::copyToFile(int fromEvent, int toEvent, const QString & fileName)
{
    bool ok = gotoEvent(fromEvent);
    if (!ok)
    {
        AErrorHub::addQError( QString("Bad start event index in depo file copy procedure: ").arg(fromEvent) );
        return false;
    }

    if (Settings.FileFormat == AFileSettingsBase::Binary)
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
