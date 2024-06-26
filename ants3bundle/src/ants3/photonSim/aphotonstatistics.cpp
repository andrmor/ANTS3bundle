#include "aphotonstatistics.h"
#include "aphotonsimhub.h"
#include "aroothistappenders.h"
#include "ajsontools.h"
#include "ajsontoolsroot.h"

#include <QDebug>

#include "TH1D.h"

APhotonStatistics::~APhotonStatistics()
{
    clear();
}

void APhotonStatistics::clear()
{
    Absorbed = InterfaceRuleLoss = HitSensor = Escaped = LossOnGrid = TracingSkipped = MaxTransitions = GeneratedOutside = MonitorKill = 0;

    FresnelTransmitted = FresnelReflected = BulkAbsorption = Rayleigh = Reemission = CustomScatter = 0;
    InterfaceRuleForward = InterfaceRuleBack = 0;

    delete WaveDistr;       WaveDistr       = nullptr;
    delete TimeDistr;       TimeDistr       = nullptr;
    delete AngularDistr;    AngularDistr    = nullptr;
    delete TransitionDistr; TransitionDistr = nullptr;
}

void APhotonStatistics::init()
{
    clear();

    const APhotonSimSettings & SimSet = APhotonSimHub::getConstInstance().Settings;
    if (SimSet.WaveSet.Enabled)
    {
        const int WaveNodes = SimSet.WaveSet.countNodes();
        WaveDistr   = new TH1D("", "", WaveNodes, 0, WaveNodes);
    }
    else
    {
        WaveDistr   = new TH1D("", "", 1, -1, 0);
    }

    TimeDistr       = new TH1D("", "", 100, 0, SimSet.RunSet.UpperTimeLimit);

    AngularDistr    = new TH1D("", "", 90, 0, 90.0);

    TransitionDistr = new TH1D("", "", 100, 0, SimSet.OptSet.MaxPhotonTransitions+1); //new TH1D("", "", 100, 0, 0);
}

bool APhotonStatistics::isEmpty()
{    
    return (countPhotons() == 0);
}

void APhotonStatistics::registerWave(int iWave)
{
    WaveDistr->Fill(iWave);
}

void APhotonStatistics::registerTime(double Time)
{
    TimeDistr->Fill(Time);
}

void APhotonStatistics::registerAngle(double angle)
{
    AngularDistr->Fill(angle);
}

void APhotonStatistics::registerNumTrans(int NumTransitions)
{
    TransitionDistr->Fill(NumTransitions);
}

void APhotonStatistics::append(const APhotonStatistics & from)
{
    appendTH1D(AngularDistr,    from.AngularDistr);
    appendTH1D(TimeDistr,       from.TimeDistr);
    appendTH1D(WaveDistr,       from.WaveDistr);
    appendTH1D(TransitionDistr, from.TransitionDistr);

    Absorbed             += from.Absorbed;
    InterfaceRuleLoss    += from.InterfaceRuleLoss;
    HitSensor            += from.HitSensor;
    Escaped              += from.Escaped;
    LossOnGrid           += from.LossOnGrid;
    TracingSkipped       += from.TracingSkipped;
    MaxTransitions       += from.MaxTransitions;
    GeneratedOutside     += from.GeneratedOutside;
    MonitorKill          += from.MonitorKill;

    FresnelTransmitted   += from.FresnelTransmitted;
    FresnelReflected     += from.FresnelReflected;
    BulkAbsorption       += from.BulkAbsorption;
    Rayleigh             += from.Rayleigh;
    CustomScatter        += from.CustomScatter;
    Reemission           += from.Reemission;

    InterfaceRuleBack    += from.InterfaceRuleBack;
    InterfaceRuleForward += from.InterfaceRuleForward;
}

void APhotonStatistics::writeToJson(QJsonObject & json) const
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
    json["CustomScatter"]        = (double)CustomScatter;
    json["Reemission"]           = (double)Reemission;

    json["InterfaceRuleBack"]    = (double)InterfaceRuleBack;
    json["InterfaceRuleForward"] = (double)InterfaceRuleForward;

    json["WaveDistr"]            = jstools::regularTh1dToJson(WaveDistr);
    json["TimeDistr"]            = jstools::regularTh1dToJson(TimeDistr);
    json["AngularDistr"]         = jstools::regularTh1dToJson(AngularDistr);
    json["TransitionDistr"]      = jstools::regularTh1dToJson(TransitionDistr);
}

void APhotonStatistics::readFromJson(const QJsonObject & json)
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
    jstools::parseJson(json, "CustomScatter"       , CustomScatter);
    jstools::parseJson(json, "Reemission"          , Reemission);

    jstools::parseJson(json, "InterfaceRuleBack"   , InterfaceRuleBack);
    jstools::parseJson(json, "InterfaceRuleForward", InterfaceRuleForward);

    jstools::parseJson(json, "WaveDistr"           , WaveDistr);
    jstools::parseJson(json, "TimeDistr"           , TimeDistr);
    jstools::parseJson(json, "AngularDistr"        , AngularDistr);
    jstools::parseJson(json, "TransitionDistr"     , TransitionDistr);
}

long APhotonStatistics::countPhotons()
{
    return Absorbed + InterfaceRuleLoss + HitSensor + Escaped + LossOnGrid + TracingSkipped + MaxTransitions + GeneratedOutside + MonitorKill;
}
