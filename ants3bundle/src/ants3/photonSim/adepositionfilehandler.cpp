#include "adepositionfilehandler.h"
#include "aerrorhub.h"
#include "adeporecord.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>

#include <fstream>

ADepositionFileHandler::ADepositionFileHandler(APhotonDepoSettings & depoSettings) :
    AFileHandlerBase(depoSettings)//, Settings(depoSettings)
{
    FileType = "deposition";
}

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

QString ADepositionFileHandler::formReportString() const
{
    QString txt;
    txt += QString("Number of events: %0\n").arg(BaseSettings.NumEvents);
    txt += QString("Number of empty events: %0\n").arg(EmptyEvents);
    if (EmptyEvents != BaseSettings.NumEvents)
    {
        std::vector<std::pair<QString,ADepoStatRecord>> vec;
        for (const auto & p : SeenParticles) vec.push_back({p.first, p.second});
        std::sort(vec.begin(), vec.end(), [](const auto & lh, const auto & rh){return lh.second.TotalDepo > rh.second.TotalDepo;});

        txt += "Depositions by particle:\n";
        for (const auto & p : vec)
            txt += QString("   %0\t\t%1 keV in %2 deposition%3 (%4 event%5)\n")
                       .arg(p.first)
                       .arg(QString::number(p.second.TotalDepo, 'g', 4))
                       .arg(p.second.NumberOfTimes)
                       .arg(p.second.NumberOfTimes == 1 ? "" : "s")
                       .arg(p.second.InNumEvents)
                       .arg(p.second.InNumEvents == 1 ? "" : "s");
        txt += QString("Deposition energy per event: from %0 to %1 keV\n").arg(MinMaxEnergyPerEvent.first).arg(MinMaxEnergyPerEvent.second);
        txt += QString("Deposition energy: from %0 to %1 keV\n").arg(MinMaxDepoEnergy.first).arg(MinMaxDepoEnergy.second);
        txt += QString("Timestamp: from %0 to %1 ns\n").arg(MinMaxTime.first).arg(MinMaxTime.second);
        txt += "Position range:\n";
        txt += QString("  X from %0 to %1 mm\n").arg(MinMaxPosition[0].first).arg(MinMaxPosition[0].second);
        txt += QString("  Y from %0 to %1 mm\n").arg(MinMaxPosition[1].first).arg(MinMaxPosition[1].second);
        txt += QString("  Z from %0 to %1 mm\n").arg(MinMaxPosition[2].first).arg(MinMaxPosition[2].second);
    }
    return txt;
}

void ADepositionFileHandler::dummyReadBinaryDataUntilNewEvent()
{
    while (readNextRecordSameEvent(TmpRecord)) ;
}

void ADepositionFileHandler::fillStatisticsForCurrentEvent()
{
    double energyPerEvent = 0;
    QSet<QString> particlesSeenThisEvent;

    while (readNextRecordSameEvent(TmpRecord))
    {
        QString pname = TmpRecord.Particle;
        int index = pname.indexOf('[');
        if (index != -1) pname.resize(index);

        particlesSeenThisEvent << pname;

        SeenParticles[pname].NumberOfTimes++;
        SeenParticles[pname].TotalDepo += TmpRecord.Energy;

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

        for (const QString & pName : particlesSeenThisEvent)
            SeenParticles[pName].InNumEvents++;
    }
}
