#include "adepositionfilehandler.h"
#include "aerrorhub.h"
#include "adeporecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

ADepositionFileHandler::ADepositionFileHandler(APhotonDepoSettings & depoSettings) :
    AFileHandlerBase(depoSettings), Settings(depoSettings) {}

void ADepositionFileHandler::clearStatistics()
{
    EmptyEvents = 0;

    SeenParticles.clear();

    MinMaxDepoEnergy.first  =  OnStartLimit;
    MinMaxDepoEnergy.second = -OnStartLimit;

    MinMaxEnergyPerEvent.first  =  OnStartLimit;
    MinMaxEnergyPerEvent.second = -OnStartLimit;

    MinMaxTime.first  =  OnStartLimit;
    MinMaxTime.second = -OnStartLimit;

    for (auto & pair : MinMaxPosition)
    {
        pair.first  =  OnStartLimit;
        pair.second = -OnStartLimit;
    }
}

bool ADepositionFileHandler::collectStatistics()
{
    AErrorHub::clear();
    clearStatistics();

    bool ok = init();
    if (!ok) return false; // error already added

    BaseSettings.LastModified = QFileInfo(BaseSettings.FileName).lastModified();

    BaseSettings.NumEvents = 1;
    int expectedNextEvent = 1;
    while (true)
    {
        acknowledgeNextEvent();
        fillStatisticsForCurrentEvent();
        BaseSettings.NumEvents++;
        if (atEnd()) return true;

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

    return true;
}

void ADepositionFileHandler::fillStatisticsForCurrentEvent()
{
    double energyPerEvent = 0;
    while (readNextRecordSameEvent(TmpRecord))
    {
        QString tmp = TmpRecord.Particle;
        int index = tmp.indexOf('[');
        if (index != -1) tmp.resize(index);
        ++SeenParticles[tmp];

        if (TmpRecord.Energy < MinMaxDepoEnergy.first)  MinMaxDepoEnergy.first  = TmpRecord.Energy;
        if (TmpRecord.Energy > MinMaxDepoEnergy.second) MinMaxDepoEnergy.second = TmpRecord.Energy;

        if (TmpRecord.Time < MinMaxTime.first)  MinMaxTime.first  = TmpRecord.Time;
        if (TmpRecord.Time > MinMaxTime.second) MinMaxTime.second = TmpRecord.Time;

        for (size_t i = 0; i < 3; i++)
        {
            if (TmpRecord.Pos[i] < MinMaxPosition[i].first)  MinMaxPosition[i].first  = TmpRecord.Pos[i];
            if (TmpRecord.Pos[i] > MinMaxPosition[i].second) MinMaxPosition[i].second = TmpRecord.Pos[i];
        }

        energyPerEvent += TmpRecord.Energy;
    }

    if (energyPerEvent == 0) EmptyEvents++;
    else
    {
        if (energyPerEvent < MinMaxEnergyPerEvent.first)  MinMaxEnergyPerEvent.first  = energyPerEvent;
        if (energyPerEvent > MinMaxEnergyPerEvent.second) MinMaxEnergyPerEvent.second = energyPerEvent;
    }
}
