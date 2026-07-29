#ifndef ASOURCEPARTICLEGENERATOR_H
#define ASOURCEPARTICLEGENERATOR_H

#include "aparticlegun.h"
#include "aparticlerecord.h"

#include "avector.h"

#include <vector>
#include <functional>

class  ASourceGeneratorSettings;
class  ARandomHub;
class  G4Navigator;
class  G4Material;
class  AGunParticle;
struct AParticleSourceRecord_Standard;
struct AParticleSourceRecord_EcoMug;
class  ASource_Base;

// ---

class ASourceParticleGenerator : public AParticleGun
{
public:
    ASourceParticleGenerator(const ASourceGeneratorSettings & settings);

    bool init() override; // !!! has to be called before the first use of GenerateEvent()!

    bool generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent) override;

    AVector3 getCollimationDirection(int iSource) const; // !!!*** to ASource_Standard

    const ASourceGeneratorSettings & Settings;

protected:
    void doRequestAbort() override;

private:
    ARandomHub & RandomHub;

    double TotalActivity = 0;

    std::vector<ASource_Base*> Sources;

    void   clearSources();

    int    selectNumberOfPrimaries() const; // !!!*** to size_t
    size_t selectSource() const;
};

// --- particular source implementations ---

class ASource_Base
{
public:
    ASource_Base();
    virtual ~ASource_Base(){};

    virtual bool init() = 0;
    virtual bool generatePrimary(std::function<void (const AParticleRecord &)> handler, int iEvent) = 0;

    ARandomHub & RandomHub;
    bool AbortRequested = false;
};

class ALinkedParticle
{
public:
    int iParticle; // indexed according to GunParticles index
    int LinkedTo;  // index of particle it is linked to

    ALinkedParticle() {}
    ALinkedParticle(int iparticle, int linkedto = -1) {iParticle = iparticle; LinkedTo = linkedto;}

    // Run-time
    bool bWasGenerated = false;
    double TimeStamp = 0;
};

class ASource_Standard : public ASource_Base
{
public:
    ASource_Standard(const AParticleSourceRecord_Standard * settings);

    bool init() override;
    bool generatePrimary(std::function<void (const AParticleRecord &)> handler, int iEvent) override;

    const AParticleSourceRecord_Standard * Settings;

    // Run-time
    double   TotalParticleWeight;
    AVector3 CollimationDirection;
    double   CollimationProbability; //collimation probability: solid angle inside cone / 4Pi

    //full recipe of emission builder (containes particles linked to particles etc up to the top level individual particle)
    std::vector<std::vector<ALinkedParticle>> LinkedPartiles; //[iparticle] []  (includes the record of the particle iteslf (first one)


#ifdef GEANT4
    G4Navigator * Navigator = nullptr;  // !!!*** one should be enough!
    G4Material  * LimitedToMat = nullptr;
#else
    int           LimitedToMat = 0;
#endif

private:
    void   updateLimitedToMat();  // !!!*** add error handling
    size_t selectParticle() const;
    bool   selectPosition(double * R) const;
    void   doGeneratePosition(double * R) const;
    double selectTime(int iEvent);
    void   generateDirection(bool forceIsotropic, double * direction) const;

    // !!!*** error handling:
    void   processSpecialParticle(const AGunParticle & particle, double * position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler);

    // old comment, cannot remember the idea --> "override for secondaries to uniform!"
    void addGeneratedParticle(int iParticle, double * position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler);
};

class EcoMug;
struct AParticleSourceRecord_EcoMug;
class ASource_EcoMug : public ASource_Base
{
public:
    ASource_EcoMug(const AParticleSourceRecord_EcoMug * settings);
    ~ASource_EcoMug();

    bool init() override;
    bool generatePrimary(std::function<void (const AParticleRecord&)> handler, int iEvent) override;

    const AParticleSourceRecord_EcoMug * Settings;

    // Run-time
    EcoMug * EcoMugGenerator = nullptr;
};

#endif // ASOURCEPARTICLEGENERATOR_H
