#include "aphotonfilehandler.h"
#include "aerrorhub.h"
#include "aphoton.h"

#include <QTextStream>
#include <QFile>

APhotonFileHandler::APhotonFileHandler(APhotonFileSettings & settings) :
    AFileHandlerBase(settings)//, Settings(settings)
{
    FileType = "photon record";
}

void APhotonFileHandler::dummyReadBinaryDataUntilNewEvent()
{
    while (readNextRecordSameEvent(TmpRecord)) ;
}

QString APhotonFileHandler::formReportString() const
{
    QString txt;
    txt += QString("Number of events: %0\n").arg(BaseSettings.NumEvents);
    txt += QString("Number of empty events: %0\n").arg(EmptyEvents);
    if (EmptyEvents != BaseSettings.NumEvents)
    {
        txt += QString("Total number of photons: %0\n").arg(Photons);
        if (MinMaxPhotonsPerEvent.first != OnStartLimit)
            txt += QString("Number of photons per event: from %0 to %1\n").arg(MinMaxPhotonsPerEvent.first).arg(MinMaxPhotonsPerEvent.second);
        txt += "\n";
        txt += QString("Timestamp: from %0 to %1 ns\n").arg(MinMaxTime.first).arg(MinMaxTime.second);
        txt += "Position range:\n";
        txt += QString("  X from %0 to %1 mm\n").arg(MinMaxPosition[0].first).arg(MinMaxPosition[0].second);
        txt += QString("  Y from %0 to %1 mm\n").arg(MinMaxPosition[1].first).arg(MinMaxPosition[1].second);
        txt += QString("  Z from %0 to %1 mm\n").arg(MinMaxPosition[2].first).arg(MinMaxPosition[2].second);
        txt += "Direction unit vector range:\n";
        txt += QString("  X from %0 to %1\n").arg(MinMaxUnitDirection[0].first).arg(MinMaxUnitDirection[0].second);
        txt += QString("  Y from %0 to %1\n").arg(MinMaxUnitDirection[1].first).arg(MinMaxUnitDirection[1].second);
        txt += QString("  Z from %0 to %1\n").arg(MinMaxUnitDirection[2].first).arg(MinMaxUnitDirection[2].second);
        txt += "\n";
        txt += QString("Photons with wave index of -1: %0\n").arg(NumberMinus1WaveIndex);
        if (MinMaxWaveIndex.first != OnStartLimit)
        txt += QString("Wave index: from %0 to %1\n").arg(MinMaxWaveIndex.first).arg(MinMaxWaveIndex.second);
    }
    return txt;
}

void APhotonFileHandler::clearStatistics()
{
    EmptyEvents = 0;
    Photons = 0;

    MinMaxPhotonsPerEvent.first  = OnStartLimit;
    MinMaxPhotonsPerEvent.second = 0;

    MinMaxTime.first  =  OnStartLimit;
    MinMaxTime.second = -OnStartLimit;

    for (auto & pair : MinMaxPosition)
    {
        pair.first  =  OnStartLimit;
        pair.second = -OnStartLimit;
    }

    for (auto & pair : MinMaxUnitDirection)
    {
        pair.first  =  OnStartLimit;
        pair.second = -OnStartLimit;
    }

    NumberMinus1WaveIndex = 0;
    MinMaxWaveIndex.first  =  OnStartLimit;
    MinMaxWaveIndex.second = -OnStartLimit;
}

void APhotonFileHandler::fillStatisticsForCurrentEvent()
{
    size_t photonsThisEvent = 0;

    while (readNextRecordSameEvent(TmpRecord))
    {
        Photons++;
        photonsThisEvent++;

        if (TmpRecord.time < MinMaxTime.first)  MinMaxTime.first  = TmpRecord.time;
        if (TmpRecord.time > MinMaxTime.second) MinMaxTime.second = TmpRecord.time;

        for (size_t i = 0; i < 3; i++)
        {
            if (TmpRecord.r[i] < MinMaxPosition[i].first)  MinMaxPosition[i].first  = TmpRecord.r[i];
            if (TmpRecord.r[i] > MinMaxPosition[i].second) MinMaxPosition[i].second = TmpRecord.r[i];

            if (TmpRecord.v[i] < MinMaxUnitDirection[i].first)  MinMaxUnitDirection[i].first  = TmpRecord.v[i];
            if (TmpRecord.v[i] > MinMaxUnitDirection[i].second) MinMaxUnitDirection[i].second = TmpRecord.v[i];
        }

        if (TmpRecord.waveIndex == -1) NumberMinus1WaveIndex++;
        else
        {
            if (TmpRecord.waveIndex < MinMaxWaveIndex.first)  MinMaxWaveIndex.first  = TmpRecord.waveIndex;
            if (TmpRecord.waveIndex > MinMaxWaveIndex.second) MinMaxWaveIndex.second = TmpRecord.waveIndex;
        }
    }

    if (photonsThisEvent == 0) EmptyEvents++;

    if (photonsThisEvent < MinMaxPhotonsPerEvent.first)  MinMaxPhotonsPerEvent.first  = photonsThisEvent;
    if (photonsThisEvent > MinMaxPhotonsPerEvent.second) MinMaxPhotonsPerEvent.second = photonsThisEvent;
}
