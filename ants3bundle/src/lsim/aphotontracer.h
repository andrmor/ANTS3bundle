#ifndef APHOTONTRACER_H
#define APHOTONTRACER_H

#include "aphoton.h"
#include "aphotontrackrecord.h"
#include "aphotonhistorylog.h"
#include "ainterfacerule.h"

#include "TString.h"

#include <vector>

class APhotonSimSettings;
class AMaterialHub;
class AInterfaceRuleHub;
class ASensorHub;
class AGridHub;
class ARandomHub;
class APhotonStatistics;
class TGeoManager;
class AMaterial;
class ALightSensorEvent;
class TGeoNavigator;
class TGeoVolume;
class QTextStream;
class TGeoNode;

enum class EBulkProcessResult {NotTriggered, Absorbed, Scattered, WaveShifted};

class APhotonTracer
{
public:
    APhotonTracer(ALightSensorEvent & event, QTextStream* & streamTracks, QTextStream* & streamSensorLog, QTextStream* & streamPhotonLog);

    void configureTracer();

    void tracePhoton(const APhoton & phot);

    // the next tree methods are used by AInterfaceRuleTester
    void             configureForInterfaceRuleTester(int fromMat, int toMat, AInterfaceRule * interfaceRule, double * globalNormal, APhoton & photon);
    EInterfaceResult processInterface();
    void             readBackPhoton(APhoton & photonToUpdate);

    APhotonTrackRecord             Track;
    std::vector<APhotonHistoryLog> PhLog;

private:
    // hubs & settings
    const AMaterialHub       & MatHub;
    const AInterfaceRuleHub  & RuleHub;
    const ASensorHub         & SensorHub;
    const AGridHub           & GridHub;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    APhotonStatistics        & SimStat;

    // external resources
    ALightSensorEvent        & Event;
    QTextStream*             & StreamTracks;
    QTextStream*             & StreamSensorLog;
    QTextStream*             & StreamPhotonLog;
    TGeoManager              * GeoManager   = nullptr;
    TGeoNavigator            * Navigator    = nullptr;

    APhoton  Photon;                // the photon which is traced
    int      AddedTracks = 0;
    int      AddedLogs = 0;
    int      TransitionCounter = 0; // number of photon transitions
    double   Rnd;                   // pre-generated random number for accelerated mode
    double   Step;
    double * N = nullptr;           //normal vector to the surface - returned by TGeo
    bool     bHaveNormal = false;
    int      MatIndexFrom;          // material index of the current medium or medium before the interface
    int      MatIndexTo;            // material index of the medium after interface
    TString  NameFrom;
    int      VolumeIndexFrom;
    TString  NameTo;
    int      VolumeIndexTo;
    double   SpeedOfLight;          // photon speed in current material (MatIndexFrom) in mm/ns

    const TGeoVolume * VolumeFrom   = nullptr;
    const TGeoVolume * VolumeTo     = nullptr;
    const AMaterial  * MaterialFrom = nullptr; // material before the interface
    const AMaterial  * MaterialTo   = nullptr; // material after the interface

    bool               bGridShiftOn = false;
    double             R_afterStep[3];
    double             FromGridToGlobal[3];          // add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double             FromGridElementToGridBulk[3]; // add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    const TGeoVolume * GridVolume = nullptr;         // the grid bulk

    AInterfaceRule * InterfaceRule = nullptr;
    bool bUseLocalNormal = false;

    double _MaxQE = 1.0;

    const int MaxNumberAttemptsGenerateReemissionWavelength = 10; // if failed within this number, photon is forced to absorption

    bool SaveLog = false;

    bool initBeforeTracing(const APhoton & phot);
    void initTracks();
    void initPhotonLog();
    void endTracing();
    void processSensorHit(int iSensor);
    bool enterGrid(int GridNumber);
    bool isOutsideGridBulk();
    void exitGrid();
    void appendHistoryRecord();
    void savePhotonLogRecord();
    void saveTrack();
    AInterfaceRule * getInterfaceRule() const; // can return nullptr
    AInterfaceRule::EInterfaceRuleResult tryInterfaceRule();
    EBulkProcessResult checkBulkProcesses();
    void checkSpecialVolume(TGeoNode * NodeAfterInterface, bool & returnEndTracingFlag);
    bool isPhotonEscaped();
    void appendToSensorLog(int ipm, double time, double x, double y, double angle, int waveIndex);
    double calculateReflectionProbability();
    void performReflection();
    bool performRefraction();
    void fillSimStatAndLog(EInterfaceResult status);
};
#endif // APHOTONTRACER_H
