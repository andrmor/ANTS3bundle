#include "asimulationstatistics.h"
#include "ageoobject.h"
#include "amonitor.h"
#include "aroothistappenders.h"

#include <QDebug>

#include "TH1D.h"

ASimulationStatistics::~ASimulationStatistics()
{
    clearAll();
}

void ASimulationStatistics::clearAll()
{
    delete WaveDistr;    WaveDistr    = nullptr;
    delete TimeDistr;    TimeDistr    = nullptr;
    delete AngularDistr;    AngularDistr    = nullptr;
    delete TransitionDistr; TransitionDistr = nullptr;

    clearMonitors();
}

void ASimulationStatistics::initialize(std::vector<const AGeoObject*> monitorRecords, int nBins, int waveNodes)
{    
    if (nBins != 0)     NumBins = nBins;
    if (waveNodes != 0) WaveNodes = waveNodes;

    delete WaveDistr;
    if (WaveNodes != 0)     WaveDistr = new TH1D("iWaveSpectrum", "WaveIndex spectrum", WaveNodes, 0, WaveNodes);
    else                    WaveDistr = new TH1D("iWaveSpectrum", "WaveIndex spectrum", NumBins, 0, -1);
    delete TimeDistr;       TimeDistr = new TH1D("TimeSpectrum", "Time spectrum", NumBins, 0, -1);
    delete AngularDistr;    AngularDistr = new TH1D("AngularDistr", "cosAngle spectrum", NumBins, 0, 90.0);
    delete TransitionDistr; TransitionDistr = new TH1D("TransitionsSpectrum", "Transitions", NumBins, 0,-1);

    Absorbed = InterfaceRuleLoss = HitSensor = Escaped = LossOnGrid = TracingSkipped = MaxTransitions = GeneratedOutside = MonitorKill = 0;

    FresnelTransmitted = FresnelReflected = BulkAbsorption = Rayleigh = Reemission = 0;
    InterfaceRuleForward = InterfaceRuleBack = 0;

//    PhotonHistoryLog.clear();  !!!***
//    PhotonHistoryLog.squeeze(); !!!***

    clearMonitors();
    for (const AGeoObject * obj : monitorRecords)
        Monitors.push_back(new AMonitor(obj));
}

bool ASimulationStatistics::isEmpty()
{    
  return (countPhotons() == 0);
}

void ASimulationStatistics::registerWave(int iWave)
{
    WaveDistr->Fill(iWave);
}

void ASimulationStatistics::registerTime(double Time)
{
    TimeDistr->Fill(Time);
}

void ASimulationStatistics::registerAngle(double angle)
{
    AngularDistr->Fill(angle);
}

void ASimulationStatistics::registerNumTrans(int NumTransitions)
{
    TransitionDistr->Fill(NumTransitions);
}

void ASimulationStatistics::AppendSimulationStatistics(ASimulationStatistics * from)
{
    appendTH1D(AngularDistr,    from->AngularDistr);
    appendTH1D(TimeDistr,       from->TimeDistr);
    appendTH1D(WaveDistr,       from->WaveDistr);
    appendTH1D(TransitionDistr, from->TransitionDistr);

    Absorbed             += from->Absorbed;
    InterfaceRuleLoss    += from->InterfaceRuleLoss;
    HitSensor            += from->HitSensor;
    Escaped              += from->Escaped;
    LossOnGrid           += from->LossOnGrid;
    TracingSkipped       += from->TracingSkipped;
    MaxTransitions       += from->MaxTransitions;
    GeneratedOutside     += from->GeneratedOutside;
    MonitorKill          += from->MonitorKill;

    FresnelTransmitted   += from->FresnelTransmitted;
    FresnelReflected     += from->FresnelReflected;
    BulkAbsorption       += from->BulkAbsorption;
    Rayleigh             += from->Rayleigh;
    Reemission           += from->Reemission;

    InterfaceRuleBack    += from->InterfaceRuleBack;
    InterfaceRuleForward += from->InterfaceRuleForward;

    if (Monitors.size() != from->Monitors.size())
    {
        qWarning() << "Cannot append monitor data - size mismatch:\n" <<
                      "Monitors here and in 'from':" << Monitors.size() << from->Monitors.size();
    }
    else
    {
        for (size_t i = 0; i < Monitors.size(); i++)
            Monitors[i]->appendDataFromAnotherMonitor(from->Monitors[i]);
    }
}

long ASimulationStatistics::countPhotons()
{
    return Absorbed + InterfaceRuleLoss + HitSensor + Escaped + LossOnGrid + TracingSkipped + MaxTransitions + GeneratedOutside + MonitorKill;
}

void ASimulationStatistics::clearMonitors()
{
    for (AMonitor * mon : Monitors) delete mon;
    Monitors.clear();
}

