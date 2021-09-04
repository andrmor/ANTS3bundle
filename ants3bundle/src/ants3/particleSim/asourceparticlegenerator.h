#ifndef ASOURCEPARTICLEGENERATOR_H
#define ASOURCEPARTICLEGENERATOR_H

#include "aparticlegun.h"

#include "avector.h"

#include <vector>

class  ASourceGenSettings;
struct AParticleSourceRecord;
class  ARandomHub;

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
    ASourceParticleGenerator(const ASourceGenSettings & settings);

    bool init() override; // !!! has to be called before the first use of GenerateEvent()!
    bool generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) override; // !!!*** inside

private:
    const ASourceGenSettings & Settings;
    ARandomHub               & RandomHub;

    //full recipe of emission builder (containes particles linked to particles etc up to the top level individual particle)
    std::vector< std::vector< std::vector<ALinkedParticle> > > LinkedPartiles; //[isource] [iparticle] []  (includes the record of the particle iteslf!!!)

    double TotalActivity = 0;

    std::vector<double>   TotalParticleWeight;
    std::vector<AVector3> CollimationDirection;   //[isource] collimation direction
    std::vector<double>   CollimationProbability; //[isource] collimation probability: solid angle inside cone / 4Pi

    std::vector<int>      LimitedToMat;

    void   updateLimitedToMat();

    int    selectNumberOfPrimaries() const;
    int    selectSource() const;   // !!!*** to size_t
    size_t selectParticle(int iSource) const;
    bool   selectPosition(int iSource, double * R) const;
    void   doGeneratePosition(const AParticleSourceRecord & rec, double * R) const;
    double selectTime(const AParticleSourceRecord & Source, int iEvent);
    void   addParticleInCone(int iSource, int iParticle, double * position, double time, std::vector<AParticleRecord> & generatedParticles) const;
};

#endif // ASOURCEPARTICLEGENERATOR_H
