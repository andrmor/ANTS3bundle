#ifndef ASOURCEPARTICLEGENERATOR_H
#define ASOURCEPARTICLEGENERATOR_H

#include "aparticlegun.h"
#include "aparticlerecord.h"

#include "avector.h"

#include <vector>

class  ASourceGeneratorSettings;
struct AParticleSourceRecord;
class  ARandomHub;
class  G4Navigator;
class  G4Material;

class ALinkedParticle
{
public:
    int iParticle; //indexed according to GunParticles index
    int LinkedTo;  //index of particle it is linked to

    ALinkedParticle() {}
    ALinkedParticle(int iparticle, int linkedto = -1) {iParticle = iparticle; LinkedTo = linkedto;}
};

class ASourceParticleGenerator : public AParticleGun
{
public:
    ASourceParticleGenerator(const ASourceGeneratorSettings & settings);

    bool init() override; // !!! has to be called before the first use of GenerateEvent()!
    bool generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) override; // !!!*** inside

private:
    const ASourceGeneratorSettings & Settings;
    ARandomHub & RandomHub;

    //full recipe of emission builder (containes particles linked to particles etc up to the top level individual particle)
    std::vector< std::vector< std::vector<ALinkedParticle> > > LinkedPartiles; //[isource] [iparticle] []  (includes the record of the particle iteslf!!!)

    double TotalActivity = 0;

    std::vector<double>   TotalParticleWeight;
    std::vector<AVector3> CollimationDirection;   //[isource] collimation direction
    std::vector<double>   CollimationProbability; //[isource] collimation probability: solid angle inside cone / 4Pi

#ifdef GEANT4
    G4Navigator * Navigator = nullptr;
    std::vector<G4Material*> LimitedToMat;
#else
    std::vector<int>         LimitedToMat;
#endif

    std::vector<AParticleRecord> GeneratedParticles;

    void   updateLimitedToMat();  // !!!*** implementation for Geant4

    int    selectNumberOfPrimaries() const;
    int    selectSource() const;   // !!!*** to size_t
    size_t selectParticle(int iSource) const;
    bool   selectPosition(int iSource, double * R) const;  // !!!*** implementation for Geant4
    void   doGeneratePosition(const AParticleSourceRecord & rec, double * R) const;
    double selectTime(const AParticleSourceRecord & Source, int iEvent);
    void   addParticleInCone(int iSource, int iParticle, double * position, double time);

};

#endif // ASOURCEPARTICLEGENERATOR_H
