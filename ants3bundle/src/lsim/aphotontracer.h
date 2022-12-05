#ifndef APHOTONTRACER_H
#define APHOTONTRACER_H

#include "aphoton.h"
#include "aphotontrackrecord.h"
#include "aphotonhistorylog.h"

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
class AOneEvent;
class TGeoNavigator;
class TGeoVolume;
class QTextStream;
class AInterfaceRule;
class TGeoNode;

enum class EBulkProcessResult {NotTriggered, Absorbed, Scattered, WaveShifted};
enum class EFresnelResult     {Reflected, Transmitted};
enum class EInterRuleResult   {NotTriggered, DelegateLocalNormal, Absorbed, Reflected, Transmitted};

class APhotonTracer
{
public:
    APhotonTracer(AOneEvent & event, QTextStream* & streamTracks, QTextStream* & StreamSensorLog);

    void configureTracer();

    void tracePhoton(const APhoton & phot);

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
    AOneEvent                & Event;
    QTextStream*             & StreamTracks;
    QTextStream*             & StreamSensorLog;
    TGeoManager              * GeoManager   = nullptr;
    TGeoNavigator            * Navigator    = nullptr;

    APhoton  Photon;                // the photon which is traced
    int      AddedTracks = 0;
    int      TransitionCounter = 0; // number of photon transitions
    double   Rnd;                   // pre-generated random number for accelerated mode
    double   Step;
    double * N = nullptr;           //normal vector to the surface - returned by TGeo
    double   LocalN[3];             //local normal vector to the surface - only if the interface rule defines it!
    bool     bHaveNormal = false;
    int      MatIndexFrom;          // material index of the current medium or medium before the interface
    int      MatIndexTo;            // material index of the medium after interface
    bool     bDoFresnel;            // flag - to perform or not the fresnel calculation on the interface
    TString  NameFrom;
    TString  NameTo;
    double   SpeedOfLight;          // photon speed in current material (MatIndexFrom) in mm/ns

    const TGeoVolume * VolumeFrom   = nullptr;
    const TGeoVolume * VolumeTo     = nullptr;
    const AMaterial  * MaterialFrom = nullptr; // material before the interface
    const AMaterial  * MaterialTo   = nullptr; // material after the interface
    //TGeoNode         * NodeAfter    = nullptr; // node after interface

    bool               bGridShiftOn = false;
    double             R_afterStep[3];
    double             FromGridToGlobal[3];          // add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double             FromGridElementToGridBulk[3]; // add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    const TGeoVolume * GridVolume = nullptr;         // the grid bulk

    AInterfaceRule * InterfaceRule = nullptr;
    bool    bUseLocalNormal = false;

    double _MaxQE = 1.0;

    bool initBeforeTracing(const APhoton & phot);
    void initTracks();
    void initPhotonLog();
    void endTracing();
    double calculateReflectionProbability();
    void processSensorHit(int iSensor);
    bool performRefraction();
    void performReflection();
    bool enterGrid(int GridNumber);
    bool isOutsideGridBulk();
    void exitGrid();
    void appendHistoryRecord();    // !!!*** why save photon tracks only those which are not filtered by the log?
    void savePhotonLogRecord(){}   // !!!***
    void saveTrack();
    AInterfaceRule * getInterfaceRule() const; // can return nullptr
    EFresnelResult tryReflection();
    EInterRuleResult tryInterfaceRule();
    EBulkProcessResult checkBulkProcesses();
    void checkSpecialVolume(TGeoNode * NodeAfterInterface, bool & returnEndTracingFlag);
    bool isPhotonEscaped();
    void appendToSensorLog(int ipm, double time, double x, double y, double angle, int waveIndex);
};
#endif // APHOTONTRACER_H
