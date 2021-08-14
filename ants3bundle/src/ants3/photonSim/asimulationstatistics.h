#ifndef ASIMULATIONSTATISTICS_H
#define ASIMULATIONSTATISTICS_H

//#include "aphotonhistorylog.h"    !!!***

#include <QVector>   // to std::vector !!!***
#include <QSet>

#include "TString.h"

#include <vector>

class TH1I;
class TH1D;
class AMonitor;
class AGeoObject;
class QJsonObject;

class ASimulationStatistics
{
public:
    ~ASimulationStatistics();

    // !!!***
    void initialize(std::vector<const AGeoObject *> monitorRecords = std::vector<const AGeoObject*>(), int nBins = 0, int waveNodes = 0); //0 - default (100) or previously set value will be used

    void clearAll();

    bool isEmpty();  //need update - particles added, so photons-only test is not valid

    void registerWave(int iWave);
    void registerTime(double Time);
    void registerAngle(double angle);
    void registerNumTrans(int NumTransitions);

    void appendSimulationStatistics(ASimulationStatistics * from);

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);

    //photon loss statistics
    long Absorbed, InterfaceRuleLoss, HitSensor, Escaped, LossOnGrid, TracingSkipped, MaxTransitions, GeneratedOutside, MonitorKill;

    //statistics for optical processes
    long FresnelTransmitted, FresnelReflected, BulkAbsorption, Rayleigh, Reemission;
    long InterfaceRuleBack, InterfaceRuleForward;

    // !!!*** to separate class hosted in AStatisticsHub
//    QVector< QVector <APhotonHistoryLog> > PhotonHistoryLog;    !!!***
    QSet<int>        MustNotInclude_Processes; // v.fast
    QVector<int>     MustInclude_Processes;    // slow
    QSet<QString>    MustNotInclude_Volumes;   // fast
    QVector<QString> MustInclude_Volumes;      // v.slow

    //only for optical override tester!
    long WaveChanged = 0;
    long TimeChanged = 0;

    std::vector<AMonitor*> Monitors;

    TH1D * WaveDistr       = nullptr;
    TH1D * TimeDistr       = nullptr;
    TH1D * AngularDistr    = nullptr;
    TH1D * TransitionDistr = nullptr;

private:
    int NumBins = 100;
    int WaveNodes = 0;

    long countPhotons();
    void clearMonitors();
};

#endif // ASIMULATIONSTATISTICS_H
