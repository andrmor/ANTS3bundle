#ifndef ASIMULATIONSTATISTICS_H
#define ASIMULATIONSTATISTICS_H

#include <vector>

class QJsonObject;
class TH1D;
class QString;

class APhotonStatistics
{
public:
    ~APhotonStatistics();

    void init();

    void clear();

    bool isEmpty();  //need update - particles added, so photons-only test is not valid

    void registerWave(int iWave);
    void registerTime(double Time);
    void registerAngle(double angle);
    void registerNumTrans(int NumTransitions);

    void append(const APhotonStatistics & from);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    //photon loss statistics
    long Absorbed, InterfaceRuleLoss, HitSensor, Escaped, LossOnGrid, TracingSkipped, MaxTransitions, GeneratedOutside, MonitorKill;

    //statistics for optical processes
    long FresnelTransmitted, FresnelReflected, BulkAbsorption, Rayleigh, Reemission, CustomScatter;
    long InterfaceRuleBack, InterfaceRuleForward;

    //only for optical override tester!
    long WaveChanged = 0;  // !!!*** to optical processes group?
    long TimeChanged = 0;  // kill?

    TH1D * WaveDistr       = nullptr;
    TH1D * TimeDistr       = nullptr;
    TH1D * AngularDistr    = nullptr;
    TH1D * TransitionDistr = nullptr;

private:
    long countPhotons();
    void toDistr(const QJsonObject & json, const QString & name, TH1D* & distr);
};

#endif // ASIMULATIONSTATISTICS_H
