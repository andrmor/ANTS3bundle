#include "aphotonbombfilehandler.h"
#include "aerrorhub.h"
#include "anoderecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>
#include <array>

APhotonBombFileHandler::APhotonBombFileHandler(ABombFileSettings & settings) :
    AFileHandlerBase(settings)//, Settings(settings)
{
    FileType = "photon bomb";
}

QString APhotonBombFileHandler::formReportString() const
{
    QString txt;
    txt += QString("Number of events: %0\n").arg(BaseSettings.NumEvents);
    txt += QString("Number of empty events: %0\n").arg(EmptyEvents);
    if (EmptyEvents != BaseSettings.NumEvents)
    {
        txt += QString("Total number of photon bombs: %0\n").arg(Bombs);
        txt += QString("Total number of photons: %0\n").arg(Photons);
        txt += QString("Total number of bombs with delegated number of photons: %0\n").arg(BombsWithAutoNumPhotons);
        txt += "\n";
        txt += QString("Number of bombs per event: from %0 to %1\n").arg(MinMaxBombsPerEvent.first).arg(MinMaxBombsPerEvent.second);
        if (MinMaxPhotonsPerEvent.first != OnStartLimit)
            txt += QString("Number of photons per event: from %0 to %1\n").arg(MinMaxPhotonsPerEvent.first).arg(MinMaxPhotonsPerEvent.second);
        if (MinMaxPhotonsPerBomb.first != OnStartLimit)
            txt += QString("Number of photons per bomb: from %0 to %1\n").arg(MinMaxPhotonsPerBomb.first).arg(MinMaxPhotonsPerBomb.second);
        txt += "\n";
        txt += QString("Timestamp: from %0 to %1 ns\n").arg(MinMaxTime.first).arg(MinMaxTime.second);
        txt += "Position range:\n";
        txt += QString("  X from %0 to %1 mm\n").arg(MinMaxPosition[0].first).arg(MinMaxPosition[0].second);
        txt += QString("  Y from %0 to %1 mm\n").arg(MinMaxPosition[1].first).arg(MinMaxPosition[1].second);
        txt += QString("  Z from %0 to %1 mm\n").arg(MinMaxPosition[2].first).arg(MinMaxPosition[2].second);
    }
    return txt;
}

void APhotonBombFileHandler::dummyReadBinaryDataUntilNewEvent()
{
    while (readNextRecordSameEvent(TmpRecord)) ;
}

void APhotonBombFileHandler::clearStatistics()
{
    EmptyEvents = 0;
    BombsWithAutoNumPhotons = 0;
    Bombs = 0;
    Photons = 0;

    MinMaxBombsPerEvent.first  = OnStartLimit;
    MinMaxBombsPerEvent.second = 0;

    MinMaxPhotonsPerBomb.first  = OnStartLimit;
    MinMaxPhotonsPerBomb.second = 0;

    MinMaxPhotonsPerEvent.first  = OnStartLimit;
    MinMaxPhotonsPerEvent.second = 0;

    MinMaxTime.first  =  OnStartLimit;
    MinMaxTime.second = -OnStartLimit;

    for (auto & pair : MinMaxPosition)
    {
        pair.first  =  OnStartLimit;
        pair.second = -OnStartLimit;
    }
}

void APhotonBombFileHandler::fillStatisticsForCurrentEvent()
{
    size_t bombsThisEvent = 0;
    size_t photonsThisEvent = 0;
    bool wasAutoPhotons = false;

    while (readNextRecordSameEvent(TmpRecord))
    {
        Bombs++;
        bombsThisEvent++;
        wasAutoPhotons = false;

        if (TmpRecord.NumPhot == -1)
        {
            wasAutoPhotons = true;
            BombsWithAutoNumPhotons++;
        }
        else
        {
            Photons += TmpRecord.NumPhot;
            photonsThisEvent += TmpRecord.NumPhot;

            if (TmpRecord.NumPhot < MinMaxPhotonsPerBomb.first)  MinMaxPhotonsPerBomb.first  = TmpRecord.NumPhot;
            if (TmpRecord.NumPhot > MinMaxPhotonsPerBomb.second) MinMaxPhotonsPerBomb.second = TmpRecord.NumPhot;
        }

        if (TmpRecord.Time < MinMaxTime.first)  MinMaxTime.first  = TmpRecord.Time;
        if (TmpRecord.Time > MinMaxTime.second) MinMaxTime.second = TmpRecord.Time;

        for (size_t i = 0; i < 3; i++)
        {
            if (TmpRecord.R[i] < MinMaxPosition[i].first)  MinMaxPosition[i].first  = TmpRecord.R[i];
            if (TmpRecord.R[i] > MinMaxPosition[i].second) MinMaxPosition[i].second = TmpRecord.R[i];
        }
    }

    if (bombsThisEvent == 0) EmptyEvents++;

    if (bombsThisEvent < MinMaxBombsPerEvent.first)  MinMaxBombsPerEvent.first  = bombsThisEvent;
    if (bombsThisEvent > MinMaxBombsPerEvent.second) MinMaxBombsPerEvent.second = bombsThisEvent;

    if (!wasAutoPhotons)
    {
        if (photonsThisEvent < MinMaxPhotonsPerEvent.first)  MinMaxPhotonsPerEvent.first  = photonsThisEvent;
        if (photonsThisEvent > MinMaxPhotonsPerEvent.second) MinMaxPhotonsPerEvent.second = photonsThisEvent;
    }
}
