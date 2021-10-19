#include "adepositionfilehandler.h"
#include "aerrorhub.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <fstream>



// WORK IN PROGRESS



bool ADepositionFileHandler::processG4DepositionData()
{
    if (bBinary)
    {
        inStream = new std::ifstream(FileName.toLatin1().data(), std::ios::in | std::ios::binary);

        if (!inStream->is_open())
        {
            AErrorHub::addError( QString("Cannot open input file: " + FileName).toLatin1().data() );
            return false;
        }
        eventCurrent = eventBegin - 1;
        bool bOK = readG4DepoEventFromBinFile(true); //only reads the header of the first event
        if (!bOK) return false;
    }
    else
    {
        inTextFile = new QFile(FileName);
        if (!inTextFile->open(QIODevice::ReadOnly | QFile::Text))
        {
            AErrorHub::addError( QString("Cannot open input file: " + FileName).toLatin1().data() );
            return false;
        }
        inTextStream = new QTextStream(inTextFile);
        G4DepoLine = inTextStream->readLine();
    }

/*
    for (eventCurrent = eventBegin; eventCurrent < eventEnd; eventCurrent++)
    {
        if (fStopRequested) break;
        if (EnergyVector.size() > 0) clearEnergyVector();

        //Filling EnergyVector for this event
        //  qDebug() << "iEv="<<eventCurrent << "building energy vector...";
        bool bOK = (GenSimSettings.G4SimSet.BinaryOutput ? readG4DepoEventFromBinFile()
                                                      : readG4DepoEventFromTextFile() );
        if (!bOK) return false;
        //  qDebug() << "Energy vector contains" << EnergyVector.size() << "cells";

        bOK = generateAndTrackPhotons();
        if (!bOK) return false;

        if (!GenSimSettings.fLRFsim) OneEvent->HitsToSignal();

        dataHub->Events.append(OneEvent->PMsignals);
        if (timeRange != 0) dataHub->TimedEvents.append(OneEvent->TimedPMsignals);

        EnergyVectorToScan();

        progress = (eventCurrent - eventBegin + 1) * updateFactor;
    }
*/

    return true;
}

bool ADepositionFileHandler::readG4DepoEventFromTextFile()
{
    //  qDebug() << " CurEv:"<<eventCurrent <<" -> " << G4DepoLine;
    if (!G4DepoLine.startsWith('#') || G4DepoLine.size() < 2)
    {
        AErrorHub::addError("Format error for #event field in G4ants energy deposition file");
        return false;
    }
    G4DepoLine.remove(0, 1);
    //  qDebug() << G4DepoLine << eventCurrent;
    if (G4DepoLine.toInt() != eventCurrent)
    {
        AErrorHub::addError("Missmatch of event number in G4ants energy deposition file");
        return false;
    }

    do
    {
        G4DepoLine = inTextStream->readLine();
        if (G4DepoLine.startsWith('#'))
            break; //next event

        //  qDebug() << ID << "->"<<G4DepoLine;
        //pId mId dE x y z t
        // 0   1   2 3 4 5 6     // pId can be = -1 !
        //populating energy vector data
        QStringList fields = G4DepoLine.split(' ', Qt::SkipEmptyParts);
        if (fields.isEmpty()) break; //last event had no depo - end of file reached
        if (fields.size() < 7)
        {
            AErrorHub::addError("Format error in G4ants energy deposition file");
            return false;
        }
/*
        AEnergyDepositionCell* cell = new AEnergyDepositionCell(fields[3].toDouble(), fields[4].toDouble(), fields[5].toDouble(), //x y z
                                                                fields[6].toDouble(), fields[2].toDouble(),  //time dE
                                                                fields[0].toInt(), fields[1].toInt(), 0, eventCurrent); //part mat sernum event
        EnergyVector << cell;
*/
    }
    while (!inTextStream->atEnd());

    return true;
}

bool ADepositionFileHandler::readG4DepoEventFromBinFile(bool expectNewEvent)
{
    char header = 0;
    while (*inStream >> header)
    {
        if (header == char(0xee))
        {
            inStream->read((char*)&G4NextEventId, sizeof(int));
            //qDebug() << G4NextEventId << "expecting:" << eventCurrent+1;
            if (G4NextEventId != eventCurrent + 1)
            {
                AErrorHub::addError( QString("Bad event number in G4ants energy depo file: expected %1 and got %2").arg(eventCurrent, G4NextEventId).toLatin1().data() );
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
/*
            AEnergyDepositionCell * cell = new AEnergyDepositionCell();

            // format:
            // partId(int) matId(int) DepoE(double) X(double) Y(double) Z(double) Time(double)
            inStream->read((char*)&cell->ParticleId, sizeof(int));
            inStream->read((char*)&cell->MaterialId, sizeof(int));
            inStream->read((char*)&cell->dE,         sizeof(double));
            inStream->read((char*)&cell->r[0],       sizeof(double));
            inStream->read((char*)&cell->r[1],       sizeof(double));
            inStream->read((char*)&cell->r[2],       sizeof(double));
            inStream->read((char*)&cell->time,       sizeof(double));

            EnergyVector << cell;
*/
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
