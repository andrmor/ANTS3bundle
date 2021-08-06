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
class APhoton;
class TGeoManager;
class AMaterial;
class AOneEvent;
class TGeoNavigator;
class TGeoVolume;
class AGridElementRecord;

class AVector3
{
public:
    AVector3(const double * pos) {for (int i=0; i<3; i++) r[i] = pos[i];}
    AVector3() {}

    double r[3];
};

class ATrackRecord
{
public:
    bool HitSensor;
    int  ScintType;
    std::vector<AVector3> Positions;
};

class APhotonTracer
{
public:
    explicit APhotonTracer();
    ~APhotonTracer();

    void configure(AOneEvent* oneEvent);

    void TracePhoton(const APhoton* Photon);

    void setMaxTracks(int maxTracks) {MaxTracks = maxTracks;}

    void hardAbort(); //before using it, give a chance to finish normally using abort at higher levels

private:
    const AMaterialHub       & MatHub;
    const AInterfaceRuleHub  & RuleHub;
    const ASensorHub         & SensorHub;
    const APhotonSimSettings & SimSet;
          ARandomHub         & RandomHub;

    TGeoManager   * GeoManager = nullptr;
    TGeoNavigator * Navigator  = nullptr;

    AOneEvent     * OneEvent   = nullptr;

//    const QVector<AGridElementRecord*>* grids;

    ATrackRecord Track;
    QVector<APhotonHistoryLog> PhLog;

    int MaxTracks = 10;
    int PhotonTracksAdded = 0;

    int Counter; //number of photon transitions - there is a limit on this set by user
    APhoton * p; //the photon which is traced
    double rnd; //pre-generated random number for accelerated mode
    double Step;
    double * N; //normal vector to the surface
    bool fHaveNormal;
    bool fMissPM;
    int MatIndexFrom; //material index of the current medium or medium before the interface
    int MatIndexTo;   //material index of the medium after interface
    const AMaterial * MaterialFrom; //material before the interface
    const AMaterial * MaterialTo;   //material after the interface
    double RefrIndexFrom, RefrIndexTo; //refractive indexes n1 and n2
    bool fDoFresnel; //flag - to perform or not the fresnel calculation on the interface
    bool bBuildTracks = false;
    bool fGridShiftOn = false;
    double FromGridCorrection[3]; //add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double FromGridElementToGridBulk[3]; //add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    TGeoVolume* GridVolume; // the grid bulk

    QString nameFrom;
    QString nameTo;

    bool bAbort = false;

    const double c_in_vac = 2.997925e2; //speed of light in mm/ns

    enum AbsRayEnum {AbsRayNotTriggered=0, AbsTriggered, RayTriggered, WaveShifted};
    AbsRayEnum AbsorptionAndRayleigh();   // !!!*** TOD: fix Rayleigh
    double CalculateReflectionCoefficient();
    void PMwasHit(int PMnumber);
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
