#include "asimulationstatistics.h"
#include "aphotonsimhub.h"
#include "ageoobject.h"
#include "amonitor.h"
#include "aroothistappenders.h"
#include "ajsontools.h"

#include <QDebug>

#include "TH1D.h"

ASimulationStatistics::~ASimulationStatistics()
{
    clearAll();
}

void ASimulationStatistics::clearAll()
{
    delete WaveDistr;       WaveDistr       = nullptr;
    delete TimeDistr;       TimeDistr       = nullptr;
    delete AngularDistr;    AngularDistr    = nullptr;
    delete TransitionDistr; TransitionDistr = nullptr;

    clearMonitors();
}

#include "ageometryhub.h"
void ASimulationStatistics::init()
{
    const APhotonSimSettings & SimSet = APhotonSimHub::getConstInstance().Settings;

    const int WaveNodes = SimSet.WaveSet.countNodes();
    delete WaveDistr;
    if (SimSet.WaveSet.Enabled) WaveDistr       = new TH1D("", "", WaveNodes, 0, WaveNodes);
    else                        WaveDistr       = new TH1D("", "", 1, -1, 0);

    delete TimeDistr;           TimeDistr       = new TH1D("", "", 100, 0, SimSet.RunSet.UpperTimeLimit);

    delete AngularDistr;        AngularDistr    = new TH1D("", "", 90, 0, 90.0);

    //delete TransitionDistr;     TransitionDistr = new TH1D("", "", 100, 0, 0);
    delete TransitionDistr;     TransitionDistr = new TH1D("", "", 100, 0, SimSet.OptSet.MaxPhotonTransitions+1);

    Absorbed = InterfaceRuleLoss = HitSensor = Escaped = LossOnGrid = TracingSkipped = MaxTransitions = GeneratedOutside = MonitorKill = 0;

    FresnelTransmitted = FresnelReflected = BulkAbsorption = Rayleigh = Reemission = 0;
    InterfaceRuleForward = InterfaceRuleBack = 0;

//    PhotonHistoryLog.clear();  !!!***
//    PhotonHistoryLog.squeeze(); !!!***

    clearMonitors();
    for (const AGeoObject * obj : AGeometryHub::getInstance().MonitorsRecords)
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

void ASimulationStatistics::append(ASimulationStatistics * from)
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

void ASimulationStatistics::writeToJson(QJsonObject & json) const
{
    json["Absorbed"]             = (double)Absorbed;
    json["InterfaceRuleLoss"]    = (double)InterfaceRuleLoss;
    json["HitSensor"]            = (double)HitSensor;
    json["Escaped"]              = (double)Escaped;
    json["LossOnGrid"]           = (double)LossOnGrid;
    json["TracingSkipped"]       = (double)TracingSkipped;
    json["MaxTransitions"]       = (double)MaxTransitions;
    json["GeneratedOutside"]     = (double)GeneratedOutside;
    json["MonitorKill"]          = (double)MonitorKill;

    json["FresnelTransmitted"]   = (double)FresnelTransmitted;
    json["FresnelReflected"]     = (double)FresnelReflected;
    json["BulkAbsorption"]       = (double)BulkAbsorption;
    json["Rayleigh"]             = (double)Rayleigh;
    json["Reemission"]           = (double)Reemission;

    json["InterfaceRuleBack"]    = (double)InterfaceRuleBack;
    json["InterfaceRuleForward"] = (double)InterfaceRuleForward;

    json["WaveDistr"]            = jstools::regularTh1dToJson(WaveDistr);
    json["TimeDistr"]            = jstools::regularTh1dToJson(TimeDistr);
    json["AngularDistr"]         = jstools::regularTh1dToJson(AngularDistr);
    json["TransitionDistr"]      = jstools::regularTh1dToJson(TransitionDistr);
}

void toDistr(const QJsonObject & json, const QString & name, TH1D* & Distr)
{
    QJsonObject js;
    bool ok = jstools::parseJson(json, name, js);
    if (!ok) Distr = nullptr;
    else     Distr = jstools::jsonToRegularTh1D(js);
}

void ASimulationStatistics::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Absorbed"            , Absorbed);
    jstools::parseJson(json, "InterfaceRuleLoss"   , InterfaceRuleLoss);
    jstools::parseJson(json, "HitSensor"           , HitSensor);
    jstools::parseJson(json, "Escaped"             , Escaped);
    jstools::parseJson(json, "LossOnGrid"          , LossOnGrid);
    jstools::parseJson(json, "TracingSkipped"      , TracingSkipped);
    jstools::parseJson(json, "MaxTransitions"      , MaxTransitions);
    jstools::parseJson(json, "GeneratedOutside"    , GeneratedOutside);
    jstools::parseJson(json, "MonitorKill"         , MonitorKill);

    jstools::parseJson(json, "FresnelTransmitted"  , FresnelTransmitted);
    jstools::parseJson(json, "FresnelReflected"    , FresnelReflected);
    jstools::parseJson(json, "BulkAbsorption"      , BulkAbsorption);
    jstools::parseJson(json, "Rayleigh"            , Rayleigh);
    jstools::parseJson(json, "Reemission"          , Reemission);

    jstools::parseJson(json, "InterfaceRuleBack"   , InterfaceRuleBack);
    jstools::parseJson(json, "InterfaceRuleForward", InterfaceRuleForward);

    toDistr(json, "WaveDistr",       WaveDistr);
    toDistr(json, "TimeDistr",       TimeDistr);
    toDistr(json, "AngularDistr",    AngularDistr);
    toDistr(json, "TransitionDistr", TransitionDistr);
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

