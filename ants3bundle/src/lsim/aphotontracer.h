#ifndef APHOTONTRACER_H
#define APHOTONTRACER_H

#include "aphotonhistorylog.h"

#include <QVector>
#include <vector>

//#include "TMathBase.h"

class APhotonSimSettings;
class AMaterialHub;
class AInterfaceRuleHub;
class ASensorHub;
class ARandomHub;
class APhotonStatistics;
class APhoton;
class TGeoManager;
class AMaterial;
class AOneEvent;
class TGeoNavigator;
class TGeoVolume;
class AGridElementRecord;

#include "aphotontrackrecord.h"

class APhotonTracer
{
public:
    explicit APhotonTracer(AOneEvent & event);
    ~APhotonTracer();

    void init();

    void tracePhoton(const APhoton * Photon);  // !!!*** to const reference

    void hardAbort(); //before using it, give a chance to finish normally using abort at higher levels

    APhotonTrackRecord         Track;
    QVector<APhotonHistoryLog> PhLog;

private:
    const AMaterialHub       & MatHub;
    const AInterfaceRuleHub  & RuleHub;
    const ASensorHub         & SensorHub;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    APhotonStatistics    & SimStat;

    AOneEvent     & Event;

    TGeoManager   * GeoManager = nullptr;
    TGeoNavigator * Navigator  = nullptr;

//    const QVector<AGridElementRecord*>* grids;

    int AddedTracks = 0;

    int Counter; //number of photon transitions - there is a limit on this set by user
    APhoton * p; //the photon which is traced
    double rnd; //pre-generated random number for accelerated mode
    double Step;
    double * N; //normal vector to the surface
    bool fHaveNormal;
    int MatIndexFrom; //material index of the current medium or medium before the interface
    int MatIndexTo;   //material index of the medium after interface
    const TGeoVolume * VolumeFrom;
    const TGeoVolume * VolumeTo;
    const AMaterial * MaterialFrom; //material before the interface
    const AMaterial * MaterialTo;   //material after the interface
    double RefrIndexFrom, RefrIndexTo; //refractive indexes n1 and n2
    bool fDoFresnel; //flag - to perform or not the fresnel calculation on the interface
    bool fGridShiftOn = false;
    double FromGridCorrection[3]; //add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double FromGridElementToGridBulk[3]; //add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    TGeoVolume* GridVolume; // the grid bulk

    QString nameFrom;  // !!!*** to TString
    QString nameTo;    // !!!*** to TString

    bool bAbort = false;

    const double c_in_vac = 2.997925e2; //speed of light in mm/ns

    enum AbsRayEnum {AbsRayNotTriggered=0, AbsTriggered, RayTriggered, WaveShifted};
    AbsRayEnum AbsorptionAndRayleigh();   // !!!*** TOD: fix Rayleigh
    double CalculateReflectionCoefficient();
    void processSensorHit(int PMnumber);
    bool PerformRefraction(double nn);
    void PerformReflection();
    void RandomDir();   // !!!*** APhoton already has this method!
    bool GridWasHit(int GridNumber); // !!!***
    void ReturnFromGridShift();      // !!!***
    void AppendTrack();              // !!!***
    void AppendHistoryRecord();  // !!!*** why save photon tracks only those which are not filtered by the log?

    void savePhotonLogRecord(){} // !!!***
    void saveTrack() {}  //  !!!***
};
#endif // APHOTONTRACER_H
