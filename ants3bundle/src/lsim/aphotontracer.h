#ifndef APHOTONTRACER_H
#define APHOTONTRACER_H

#include "aphoton.h"
#include "aphotontrackrecord.h"
#include "aphotonhistorylog.h"

#include "TString.h"

#include <QVector>
#include <vector>

class APhotonSimSettings;
class AMaterialHub;
class AInterfaceRuleHub;
class ASensorHub;
class ARandomHub;
class APhotonStatistics;
class TGeoManager;
class AMaterial;
class AOneEvent;
class TGeoNavigator;
class TGeoVolume;
class AGridElementRecord;
class QTextStream;

class APhotonTracer
{
public:
    APhotonTracer(AOneEvent & event, QTextStream* & streamTracks);

    void init();

    void tracePhoton(const APhoton & Photon);

    APhotonTrackRecord         Track;
    QVector<APhotonHistoryLog> PhLog;

private:
    const AMaterialHub       & MatHub;
    const AInterfaceRuleHub  & RuleHub;
    const ASensorHub         & SensorHub;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    APhotonStatistics        & SimStat;

    // external resources
    AOneEvent                & Event;
    QTextStream            * & StreamTracks;
    TGeoManager              * GeoManager   = nullptr;
    TGeoNavigator            * Navigator    = nullptr;

//    const QVector<AGridElementRecord*>* grids;

    APhoton  p; //the photon which is traced
    int      AddedTracks = 0;
    int      Counter = 0; //number of photon transitions
    double   rnd; //pre-generated random number for accelerated mode
    double   Step;
    double * N = nullptr; //normal vector to the surface
    bool     fHaveNormal = false;
    int      MatIndexFrom; //material index of the current medium or medium before the interface
    int      MatIndexTo;   //material index of the medium after interface
    double   RefrIndexFrom;
    double   RefrIndexTo;
    bool     fDoFresnel; //flag - to perform or not the fresnel calculation on the interface
    TString  nameFrom;
    TString  nameTo;

    const TGeoVolume * VolumeFrom   = nullptr;
    const TGeoVolume * VolumeTo     = nullptr;
    const AMaterial  * MaterialFrom = nullptr; //material before the interface
    const AMaterial  * MaterialTo   = nullptr; //material after the interface

    bool         fGridShiftOn = false;
    double       FromGridCorrection[3]; //add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double       FromGridElementToGridBulk[3]; //add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    TGeoVolume * GridVolume = nullptr; // the grid bulk

    static constexpr double c_in_vac = 299.7925; //speed of light in mm/ns

    enum AbsRayEnum {AbsRayNotTriggered=0, AbsTriggered, RayTriggered, WaveShifted};
    AbsRayEnum AbsorptionAndRayleigh();   // !!!*** TOD: fix Rayleigh
    double CalculateReflectionCoefficient();
    void processSensorHit(int iSensor);
    bool PerformRefraction(double nn);
    void PerformReflection();
    void RandomDir();   // !!!*** APhoton already has this method!
    bool GridWasHit(int GridNumber); // !!!***
    void ReturnFromGridShift();      // !!!***
    void AppendHistoryRecord();  // !!!*** why save photon tracks only those which are not filtered by the log?

    void savePhotonLogRecord(){} // !!!***
    void saveTrack();
};
#endif // APHOTONTRACER_H
