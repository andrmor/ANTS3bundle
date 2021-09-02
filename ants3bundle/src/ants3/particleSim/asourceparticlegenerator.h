#ifndef ASOURCEPARTICLEGENERATOR_H
#define ASOURCEPARTICLEGENERATOR_H

#include "aparticlegun.h"

#include "TVector3.h"

#include <vector>

class  ASourceGenSettings;
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
    ASourceParticleGenerator();

    bool init() override; // !!! has to be called before the first use of GenerateEvent()!
    bool generateEvent(std::vector<AParticleRecord> & GeneratedParticles, int iEvent) override; // !!!*** inside

private:
    const ASourceGenSettings & Settings;
    ARandomHub & RandomHub;

    //full recipe of emission builder (containes particles linked to particles etc up to the top level individual particle)
    std::vector< std::vector< std::vector<ALinkedParticle> > > LinkedPartiles; //[isource] [iparticle] []  (includes the record of the particle iteslf!!!)

    double TotalActivity = 0;

    std::vector<double>   TotalParticleWeight;
    std::vector<TVector3> CollimationDirection;   //[isource] collimation direction
    std::vector<double>   CollimationProbability; //[isource] collimation probability: solid angle inside cone / 4Pi

    std::vector<int>      LimitedToMat;

    void generatePosition(int isource, double * R) const;
    void addParticleInCone(int isource, int iparticle, std::vector<AParticleRecord> & GeneratedParticles) const;

    void updateLimitedToMat();
};

#endif // ASOURCEPARTICLEGENERATOR_H
