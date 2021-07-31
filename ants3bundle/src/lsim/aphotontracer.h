#ifndef APHOTONTRACER_H
#define APHOTONTRACER_H

#include "aphotonhistorylog.h"

#include <QVector>
#include <vector>

//#include "TMathBase.h"

class APhotonSimSettings;
class AMaterialHub;
class APhoton;
class TGeoManager;
class AMaterial;
class APmHub;
class AMaterialParticleCollection;
class AOneEvent;
class TGeoNavigator;
class TrackHolderClass;
class TRandom2;
class TGeoVolume;
class AGridElementRecord;
//class ATracerStateful;

class APhotonTracer
{
public:
    explicit APhotonTracer(APhotonSimSettings & simSet,
                           TGeoManager* geoManager,
                           TRandom2* RandomGenerator,
                           APmHub* Pms,
                           const QVector<AGridElementRecord*>* Grids);
    ~APhotonTracer();

    void configure(AOneEvent* oneEvent, std::vector<TrackHolderClass*> * tracks);

    void TracePhoton(const APhoton* Photon);

    AOneEvent* getEvent() {return OneEvent;}  //only used in LRF-based sim

    void setMaxTracks(int maxTracks) {MaxTracks = maxTracks;}

    void hardAbort(); //before using it, give a chance to finish normally using abort at higher levels

private:
    const APhotonSimSettings & SimSet;
    const AMaterialHub & MatHub;

    TGeoManager * GeoManager = nullptr;
    TRandom2 * RandGen;
    APmHub* PMs;

    TGeoNavigator * Navigator = nullptr;

    const QVector<AGridElementRecord*>* grids;
    AOneEvent* OneEvent; //PM signals for this event are collected here
    std::vector<TrackHolderClass *> * Tracks;
    TrackHolderClass* track;
    QVector<APhotonHistoryLog> PhLog;

    //ATracerStateful* ResourcesForOverrides;

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
    bool bBuildTracks;
    bool fGridShiftOn;
    double FromGridCorrection[3]; //add to xyz of the current point in glob coordinates of the grid element to obtain true global point coordinates
    double FromGridElementToGridBulk[3]; //add to xyz of current point for gridnavigator to obtain normal navigator current point coordinates
    TGeoVolume* GridVolume; // the grid bulk

    QString nameFrom;
    QString nameTo;

    bool bAbort = false;

    double c_in_vac = 2.997925e2; //speed of light in mm/ns

    enum AbsRayEnum {AbsRayNotTriggered=0, AbsTriggered, RayTriggered, WaveShifted};
    inline AbsRayEnum AbsorptionAndRayleigh();
    inline double CalculateReflectionCoefficient();
    inline void PMwasHit(int PMnumber);
    inline bool PerformRefraction(double nn);
    inline void PerformReflection();
    inline void RandomDir();
    inline bool GridWasHit(int GridNumber);
    inline void ReturnFromGridShift();
    inline void AppendTrack();
    inline void AppendHistoryRecord();
};
#endif // APHOTONTRACER_H
