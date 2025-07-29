#include "asourceparticlegenerator.h"
#include "aparticlesimsettings.h"
#include "aparticlerecord.h"
#include "aparticlesourcerecord.h"
#include "aerrorhub.h"

#include "EcoMug/EcoMug.h"

#include <string>
#include <cmath>

#ifdef GEANT4
    #include "G4VPhysicalVolume.hh"
    #include "G4LogicalVolume.hh"
    #include "G4Navigator.hh"
    #include "arandomg4hub.h"
    //#include "G4ParticleDefinition.hh"
    #include "SessionManager.hh"
    #include "G4Material.hh"
    #include "G4NistManager.hh"
#else
    #include <QApplication>
    #include <QDebug>
    #include "amaterialhub.h"
    #include "arandomhub.h"
    #include "TGeoManager.h"
#endif

ASourceParticleGenerator::ASourceParticleGenerator(const ASourceGeneratorSettings & settings) :
    Settings(settings),
    RandomHub(ARandomHub::getInstance()){}

bool ASourceParticleGenerator::init()
{
    AbortRequested = false;

    int NumSources = Settings.SourceData.size();
    if (NumSources == 0)
    {
        AErrorHub::addError("No sources are defined");
        return false;
    }

    TotalActivity = Settings.calculateTotalActivity();
    if (TotalActivity == 0)
    {
        AErrorHub::addError("Total activity is zero");
        return false;
    }

    if (!Settings.check()) return false;

    TotalParticleWeight = std::vector<double>(NumSources, 0);
    for (int isource = 0; isource<NumSources; isource++)
    {
        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(Settings.SourceData[isource]);
        if (!stSource) continue;

        for (const AGunParticle & gp : stSource->Particles)
            if (gp.GenerationType == AGunParticle::Independent)
                TotalParticleWeight[isource] += gp.StatWeight;
    }

    updateLimitedToMat(); // !!!*** error handling?

    //creating lists of linked particles
    LinkedPartiles.resize(NumSources);
    for (int iSource = 0; iSource < NumSources; iSource++)
    {
        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(Settings.SourceData[iSource]);
        if (!stSource) continue;

        const int numParts = stSource->Particles.size();
        LinkedPartiles[iSource].resize(numParts);
        for (int iParticle = 0; iParticle < numParts; iParticle++)
        {
            LinkedPartiles[iSource][iParticle].clear();
            if (stSource->Particles[iParticle].GenerationType != AGunParticle::Independent)
                continue; //nothing to do for dependent particles

            //every independent particle defines an "event generation chain" containing the particle iteslf and all linked (and linked to linked to linked etc) particles
            LinkedPartiles[iSource][iParticle].push_back(ALinkedParticle(iParticle)); //list always contains the particle itself - simplifies the generation algorithm
            //only particles with larger indexes can be linked to this particle
            for (int ip = iParticle + 1; ip < numParts; ip++)
                if (stSource->Particles[ip].GenerationType != AGunParticle::Independent) //only looking for dependent
                {
                    //for iparticle, checking if it is linked to any particle in the list of the LinkedParticles
                    for (size_t idef=0; idef<LinkedPartiles[iSource][iParticle].size(); idef++)
                    {
                        int compareWith = LinkedPartiles[iSource][iParticle][idef].iParticle;
                        int linkedTo = stSource->Particles[ip].LinkedTo;
                        if ( linkedTo == compareWith)
                        {
                            LinkedPartiles[iSource][iParticle].push_back(ALinkedParticle(ip, linkedTo));
                            break;
                        }
                    }
                }
        }
    }

    //vectors related to collimation direction
    CollimationDirection.resize(NumSources);
    CollimationProbability.resize(NumSources);
    for (int isource = 0; isource < NumSources; isource++)
    {
        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(Settings.SourceData[isource]);
        if (!stSource) continue;

        const double CollPhi   = stSource->DirectionPhi * M_PI / 180.0;
        const double CollTheta = stSource->DirectionTheta * M_PI / 180.0;
        const double Spread    = stSource->CutOff * M_PI / 180.0;

        if (stSource->DirectionBySphericalAngles)
            CollimationDirection[isource] = AVector3(sin(CollTheta)*sin(CollPhi), sin(CollTheta)*cos(CollPhi), cos(CollTheta));
        else
        {
            CollimationDirection[isource] = AVector3(stSource->DirectionVectorX, stSource->DirectionVectorY, stSource->DirectionVectorZ);
            CollimationDirection[isource].toUnitVector();
        }

        CollimationProbability[isource] = 0.5 * (1.0 - cos(Spread));
    }

    // Init for EcoMug sources
    EcoMugGenerators.resize(NumSources);
    for (int iSource = 0; iSource < NumSources; iSource++)
    {
        delete EcoMugGenerators[iSource]; EcoMugGenerators[iSource] = nullptr;

        AParticleSourceRecord_EcoMug * muSource = dynamic_cast<AParticleSourceRecord_EcoMug*>(Settings.SourceData[iSource]);
        if (!muSource) continue;

        EcoMug * gen = new EcoMug();
        gen->SetUseHSphere();
        gen->SetHSphereRadius(50.0);
        gen->SetHSphereCenterPosition({0,0,0});

        EcoMugGenerators[iSource] = gen;
    }

    return true;
}

int ASourceParticleGenerator::selectSource() const
{
    int iSource = 0;

    const int NumSources = Settings.getNumSources();
    if (NumSources > 1)
    {
        double rnd = ARandomHub::getInstance().uniform() * TotalActivity;
        for (; iSource < NumSources - 1; iSource++)
        {
            if (Settings.SourceData[iSource]->Activity >= rnd) break;
            rnd -= Settings.SourceData[iSource]->Activity;
        }
    }

    return iSource;
}

bool ASourceParticleGenerator::selectPosition(int iSource, AParticleSourceRecord_Standard * source, double * R) const
{
    int attempts = 10000;

#ifdef GEANT4
    if (!LimitedToMat[iSource]) doGeneratePosition(source, R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(source, R);

            G4VPhysicalVolume * vol = Navigator->LocateGlobalPointAndSetup({R[0], R[1], R[2]});
            if (vol && vol->GetLogicalVolume())
                if (vol->GetLogicalVolume()->GetMaterial() == LimitedToMat[iSource])
                    break;

            attempts--;
            if (attempts == 0)
                SessionManager::getInstance().terminateSession("Failed to generate position for material limited source");
        }
        while (attempts != 0);
    }
#else
    if (LimitedToMat[iSource] < 0) doGeneratePosition(source, R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(source, R);

            TGeoNode * node = gGeoManager->FindNode(R[0], R[1], R[2]);
            if (node && node->GetVolume() && node->GetVolume()->GetMaterial()->GetIndex() == LimitedToMat[iSource]) break;
            QApplication::processEvents();
        }
        while (attempts-- != 0);
    }
#endif

    return true;
}

void ASourceParticleGenerator::generateDirection(size_t iSource, AParticleSourceRecord_Standard * src, bool forceIsotropic, double * direction) const
{
    if (src->AngularMode == AParticleSourceRecord_Standard::Isotropic || forceIsotropic)
    {
        //generating random direction inside the collimation cone
        const double spread   = (src->UseCutOff ? src->CutOff*3.14159265358979323846/180.0 : 3.14159265358979323846); //max angle away from generation diretion
        const double cosTheta = cos(spread);
        const double z   = cosTheta + RandomHub.uniform() * (1.0 - cosTheta);
        const double tmp = sqrt(1.0 - z * z);
        const double phi = RandomHub.uniform() * 3.14159265358979323846 * 2.0;

        AVector3 K1( tmp * cos(phi), tmp * sin(phi), z);
        AVector3 Coll( CollimationDirection[iSource] );
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    else if (src->AngularMode == AParticleSourceRecord_Standard::FixedDirection)
    {
        for (int i = 0; i < 3; i++) direction[i] = CollimationDirection[iSource][i];
    }
    //*** add error if CutOff is zero in this mode!
    else if (src->AngularMode == AParticleSourceRecord_Standard::GaussDispersion)
    {
        double angle = std::fabs(RandomHub.gauss(0, src->DispersionSigma));
        if (src->UseCutOff)
        {
            while (angle > src->CutOff)
            {
                angle = std::fabs(RandomHub.gauss(0, src->DispersionSigma));
#ifndef GEANT4
                QApplication::processEvents();
                if (AbortRequested) break;
#endif
            }
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll(CollimationDirection[iSource]);
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    // !!!*** add error if AngularDistribution does not have presence within CutOff
    else if (src->AngularMode == AParticleSourceRecord_Standard::CustomAngular)
    {
        double angle = src->_AngularSampler.getRandom();
        if (src->UseCutOff)
        {
            while (angle > src->CutOff)
            {
                angle = src->_AngularSampler.getRandom();
#ifndef GEANT4
                QApplication::processEvents();
                if (AbortRequested) break;
#endif
            }
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll( CollimationDirection[iSource] );
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
}

double ASourceParticleGenerator::selectTime(AParticleSourceRecord_Standard * source, int iEvent)
{
    double time;
    switch (source->TimeOffsetMode)
    {
    default:
    case AParticleSourceRecord_Standard::FixedOffset              : time = source->TimeFixedOffset;                                       break;
    case AParticleSourceRecord_Standard::ByEventIndexOffset       : time = source->TimeByEventStart + iEvent * source->TimeByEventPeriod; break;
    case AParticleSourceRecord_Standard::CustomDistributionOffset : time = source->_TimeSampler.getRandom();                              break;
    }

    switch (source->TimeSpreadMode)
    {
    default:
    case AParticleSourceRecord_Standard::NoSpread :
        break;
    case AParticleSourceRecord_Standard::GaussianSpread :
        time += ARandomHub::getInstance().gauss(0, source->TimeSpreadSigma);
        break;
    case AParticleSourceRecord_Standard::UniformSpread :
        time += source->TimeSpreadWidth * ARandomHub::getInstance().uniform();
        break;
    case AParticleSourceRecord_Standard::ExponentialSpread :
        const double ln2 = std::log(2.0);
        time += ARandomHub::getInstance().exp(source->TimeSpreadHalfLife / ln2);
        break;
    }

    return time;
}

size_t ASourceParticleGenerator::selectParticle(int iSource, AParticleSourceRecord_Standard * source) const
{
    size_t iParticle = 0;

    double rnd = ARandomHub::getInstance().uniform() * TotalParticleWeight[iSource];
    for ( ; iParticle < source->Particles.size() - 1; iParticle++)
    {
        if (source->Particles[iParticle].GenerationType == AGunParticle::Independent)
        {
            if (source->Particles[iParticle].StatWeight >= rnd) break; //this one
            rnd -= source->Particles[iParticle].StatWeight;
        }
    }

    return iParticle;
}

//after any operation with sources (add, remove), init should be called before the first use!
bool ASourceParticleGenerator::generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent)
{
    const int numPrimaries = selectNumberOfPrimaries();

    for (int iPrimary = 0; iPrimary < numPrimaries; iPrimary++)
    {
        // Source
        int iSource = selectSource();
        AParticleSourceRecordBase * Source = Settings.SourceData[iSource];

        bool ok = true;
        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(Source);
        if (stSource) ok = generatePrimary_StandardSource(iSource, stSource, handler, iEvent);
        else
        {
            AParticleSourceRecord_EcoMug * muSource = dynamic_cast<AParticleSourceRecord_EcoMug*>(Source);
            if (muSource) ok = generatePrimary_EcoMugSource(iSource, muSource, handler);
        }

        if (!ok) return false;

        #ifndef GEANT4
        if (AbortRequested) return false;
        #endif
    }

    return true;
}

bool ASourceParticleGenerator::generatePrimary_StandardSource(int iSource, AParticleSourceRecord_Standard * source, std::function<void(const AParticleRecord&)> handler, int iEvent)
{
    // Particle
    const size_t iParticle = selectParticle(iSource, source); // cannot avoid using index

    // Position
    double position[3];
    bool ok = selectPosition(iSource, source, position);  // cannot avoid using index
    if (!ok) return false; // !!!*** error handling!

    // Time
    const double time = selectTime(source, iEvent);

    // generating the selected particle itself
    addGeneratedParticle(iSource, source, iParticle, position, time, false, handler);

    // generating linked particles
    std::vector<ALinkedParticle> & ThisLP = LinkedPartiles[iSource][iParticle];
    ThisLP.front().bWasGenerated = true;
    ThisLP.front().TimeStamp = time;
    for (size_t ip = 1; ip < ThisLP.size(); ip++) // ThisLP starts from the particle itself, so skip the first record
    {
        const int thisParticle = ThisLP[ip].iParticle;
        const int linkedTo     = ThisLP[ip].LinkedTo;

        bool parentWasGenerated = false;
        for (int index = ip-1; index > -1; index--)
        {
            if (ThisLP[index].iParticle == linkedTo)
            {
                parentWasGenerated = ThisLP[index].bWasGenerated;
                ThisLP[ip].TimeStamp = ThisLP[index].TimeStamp; // by default inherits parent's timestamp
                break;
            }
        }

        if (parentWasGenerated) // parent was generated
        {
            if (source->Particles[thisParticle].GenerationType == AGunParticle::Linked_IfNotGenerated)
            {
                ThisLP[ip].bWasGenerated = false;
                continue;
            }
        }
        else // parent was NOT generated
        {
            if (source->Particles[thisParticle].GenerationType == AGunParticle::Linked_IfGenerated)
            {
                ThisLP[ip].bWasGenerated = false;
                continue;
            }
        }

        const double LinkingProbability = source->Particles[thisParticle].LinkedProb;
        if (ARandomHub::getInstance().uniform() > LinkingProbability)
        {
            ThisLP[ip].bWasGenerated = false;
            continue;
        }

        double halfLife = source->Particles[thisParticle].HalfLife;
        if (halfLife != 0) ThisLP[ip].TimeStamp += RandomHub.getInstance().exp(halfLife/log(2));

        if (source->UseCutOff && source->Particles[thisParticle].Particle != "-") //direct deposition ("-") ignores direction and cut-off
        {
            double inCutoffProbability = CollimationProbability[iSource];
            if (source->Particles[thisParticle].BtBPair) inCutoffProbability *= 2.0; // if a pair, chance to get in cutoff is doubled
            if (ARandomHub::getInstance().uniform() > inCutoffProbability)
            {
                // did not pass cut-off
                ThisLP[ip].bWasGenerated = true;  // marked as generated, just do not add to tracking since outside of cut-off
                continue;
            }
        }

        ThisLP[ip].bWasGenerated = true;

        addGeneratedParticle(iSource, source, thisParticle, position, ThisLP[ip].TimeStamp, true, handler);
    }
    return true;
}

#ifdef GEANT4
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4SystemOfUnits.hh"
#endif

bool ASourceParticleGenerator::generatePrimary_EcoMugSource(int iSource, AParticleSourceRecord_EcoMug * source, std::function<void (const AParticleRecord &)> handler)
{
    EcoMug * gen = EcoMugGenerators[iSource];
    gen->Generate();

    const double p = gen->GetGenerationMomentum(); // [GeV/c]
    double mass = 0.10566; // GeV/c²
    // Ekin = sqrt(p²*c² + m²*c⁴) - mc²
    double energy_GeV = sqrt(p*p + mass*mass) - mass; // multiply by 1e6 to get in keV

    AParticleRecord particle(
#ifdef GEANT4
        (gen->GetCharge() < 0 ? (G4ParticleDefinition*)G4MuonMinus::Definition() : (G4ParticleDefinition*)G4MuonPlus::Definition()),
        (double*)gen->GetGenerationPosition().data(),
        0,
        energy_GeV * 1e6); // in keV
#else
        (gen->GetCharge() < 0 ? "mu-" : "mu+"),
        (double*)gen->GetGenerationPosition().data(),
        0,
        energy_GeV * 1e6); // in keV
#endif

    std::array<double, 3> vec;
    gen->GetGenerationMomentum(vec);
    particle.setDirection(vec.data());
    particle.ensureUnitaryLength();

    handler(particle);
    return true;
}

void ASourceParticleGenerator::doGeneratePosition(AParticleSourceRecord_Standard * source, double * R) const
{
    const double & X0     = source->X0;
    const double & Y0     = source->Y0;
    const double & Z0     = source->Z0;
    const double   Phi    = source->Phi   * 3.14159265358979323846 / 180.0;
    const double   Theta  = source->Theta * 3.14159265358979323846 / 180.0;
    const double   Psi    = source->Psi   * 3.14159265358979323846 / 180.0;
    const double & size1  = source->Size1;
    const double & size2  = source->Size2;
    const double & size3  = source->Size3;

    switch (source->Shape) //source geometry type
    {
    case AParticleSourceRecord_Standard::Point :
    {
        R[0] = X0; R[1] = Y0; R[2] = Z0;
        return;
    }
    case AParticleSourceRecord_Standard::Line :
    {
        AVector3 VV(sin(Theta)*sin(Phi), sin(Theta)*cos(Phi), cos(Theta));
        double off = -1.0 + 2.0 * RandomHub.uniform();
        off *= size1;
        R[0] = X0 + VV[0]*off;
        R[1] = Y0 + VV[1]*off;
        R[2] = Z0 + VV[2]*off;
        return;
    }
    case AParticleSourceRecord_Standard::Rectangle :
    {
        AVector3 V[3];
        V[0] = AVector3(size1, 0,     0);
        V[1] = AVector3(0,     size2, 0);
        V[2] = AVector3(0,     0,     size3);
        for (int i = 0; i < 3; i++)
        {
            V[i].rotateX(Phi);
            V[i].rotateY(Theta);
            V[i].rotateZ(Psi);
        }

        const double off1 = -1.0 + 2.0 * RandomHub.uniform();
        const double off2 = -1.0 + 2.0 * RandomHub.uniform();

        R[0] = X0 + V[0][0]*off1 + V[1][0]*off2;
        R[1] = Y0 + V[0][1]*off1 + V[1][1]*off2;
        R[2] = Z0 + V[0][2]*off1 + V[1][2]*off2;
        return;
    }
    case AParticleSourceRecord_Standard::Round :
    {
        AVector3 circ(0,0,0);
        if (!source->UseAxialDistribution)
        {
            double r = RandomHub.uniform() + RandomHub.uniform();
            if (r > 1.0) r = (2.0 - r) * size1;
            else r *=  size1;
            const double angle = RandomHub.uniform() * 3.14159265358979323846 * 2.0;
            circ[0] = r * cos(angle);
            circ[1] = r * sin(angle);
        }
        else if (source->AxialDistributionType == AParticleSourceRecord_Standard::GaussAxial)
        {
            // !!!*** can be faster if radial and angle, but take care of the radial distortion
            do
            {
                circ[0] = RandomHub.gauss(0, source->AxialDistributionSigma);
                circ[1] = RandomHub.gauss(0, source->AxialDistributionSigma);
            }
            while (circ[0]*circ[0] + circ[1]*circ[1] > size1*size1);
        }
        else
        {
            do source->_AxialSampler.generatePosition(circ);
            while (circ[0]*circ[0] + circ[1]*circ[1] > size1*size1);
        }

        circ.rotateX(Phi);
        circ.rotateY(Theta);
        circ.rotateZ(Psi);

        R[0] = X0 + circ[0];
        R[1] = Y0 + circ[1];
        R[2] = Z0 + circ[2];
        return;
    }
    case AParticleSourceRecord_Standard::Box :
    {
        AVector3 V[3];
        V[0] = AVector3(size1, 0,     0);
        V[1] = AVector3(0,     size2, 0);
        V[2] = AVector3(0,     0,     size3);
        for (int i = 0; i < 3; i++)
        {
            V[i].rotateX(Phi);
            V[i].rotateY(Theta);
            V[i].rotateZ(Psi);
        }

        const double off1 = -1.0 + 2.0 * RandomHub.uniform();
        const double off2 = -1.0 + 2.0 * RandomHub.uniform();
        const double off3 = -1.0 + 2.0 * RandomHub.uniform();

        R[0] = X0 + V[0][0]*off1 + V[1][0]*off2 + V[2][0]*off3;
        R[1] = Y0 + V[0][1]*off1 + V[1][1]*off2 + V[2][1]*off3;
        R[2] = Z0 + V[0][2]*off1 + V[1][2]*off2 + V[2][2]*off3;
        return;
    }
    case AParticleSourceRecord_Standard::Cylinder :
    {
        const double off = (-1.0 + 2.0 * RandomHub.uniform()) * size3;
        const double angle = RandomHub.uniform() * 3.14159265358979323846 * 2.0;
        double r = RandomHub.uniform() + RandomHub.uniform();
        if (r > 1.0) r = (2.0 - r) * size1;
        else r *=  size1;
        double x = r * cos(angle);
        double y = r * sin(angle);

        AVector3 Circ(x, y, off);
        Circ.rotateX(Phi);
        Circ.rotateY(Theta);
        Circ.rotateZ(Psi);
        R[0] = X0 + Circ[0];
        R[1] = Y0 + Circ[1];
        R[2] = Z0 + Circ[2];
        return;
    }
    }
    return;
}

// !!!*** override for secondaries to uniform!
void ASourceParticleGenerator::addGeneratedParticle(int iSource, AParticleSourceRecord_Standard * src, int iParticle, double * position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler)
{
    const AGunParticle & gp = src->Particles[iParticle];

    if (gp.Particle[0] == '_')
    {
        processSpecialParticle(gp, src, position, time, forceIsotropic, handler);
        return;
    }

    const double energy = gp.generateEnergy();

    if (!gp.isDirectDeposition())
    {
        AParticleRecord particle(
#ifdef GEANT4
                                 gp.particleDefinition,
#else
                                 gp.Particle,
#endif
                                 position, time, energy);

        generateDirection(iSource, src, forceIsotropic, particle.v);
        handler(particle);

        if (gp.BtBPair)
        {
            for (int i = 0; i < 3; i++) particle.v[i] = -particle.v[i];
            handler(particle);
        }
    }
    else
    {
#ifdef GEANT4
        // direct deposition
        SessionManager & SM = SessionManager::getInstance();

        if (!Navigator) Navigator = new G4Navigator();
        Navigator->SetWorldVolume(SM.WorldPV);
        G4VPhysicalVolume * vol = Navigator->LocateGlobalPointAndSetup({position[0], position[1], position[2]});
        if (vol)
        {
            G4LogicalVolume * logic = vol->GetLogicalVolume();
            if (logic && SM.isEnergyDepoLogger(logic))
            {
                const int iMat = SM.findMaterial(vol->GetLogicalVolume()->GetMaterial()->GetName());
                //SM.saveDepoRecord("-", iMat, energy, position, time, vol->GetCopyNo()); // cannot do it: event action not triggered yet!
                SM.DirectDepositionBuffer.push_back( {"-", iMat, energy, {position[0], position[1], position[2]}, time, vol->GetCopyNo()} );
            }
        }
#else
        AParticleRecord particle("-", position, time, energy);
        handler(particle);
#endif
    }
}

#ifdef GEANT4
#include "G4Gamma.hh"
#endif
#include "aorthopositroniumgammagenerator.h"
void ASourceParticleGenerator::processSpecialParticle(const AGunParticle & particle, AParticleSourceRecord_Standard * source, double * position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler)
{
    if (particle.Particle == "_oPs")
    {
        std::array<AVector3, 3> unitVectors;
        std::array<double, 3>   energies;
        AOrthoPositroniumGammaGenerator::generate(unitVectors, energies);

        AParticleRecord particle(
#ifdef GEANT4
            G4Gamma::Definition(),
#else
            "gamma",
#endif
            position, time, 1.234);

        for (size_t iGamma = 0; iGamma < 3; iGamma++)
        {
            particle.energy = energies[iGamma]*1000; // here energy has to be in ants3 units, which is keV
            for (size_t i = 0; i < 3; i++)
                particle.v[i] = unitVectors[iGamma][i];
            handler(particle);
        }
        return;
    }

#ifdef GEANT4
    SessionManager & SM = SessionManager::getInstance();
    SM.terminateSession("Unknown special particle: " + particle.Particle);
#else
    qWarning() << "Unknown special particle:" << particle.Particle.data() << "in ASourceParticleGenerator::processSpecialParticle";
#endif
}

void ASourceParticleGenerator::updateLimitedToMat()
{
    LimitedToMat.clear();

#ifdef GEANT4
    Navigator = new G4Navigator();
    SessionManager & SM = SessionManager::getInstance();
    Navigator->SetWorldVolume(SM.WorldPV);

    for (AParticleSourceRecordBase * source : Settings.SourceData)
    {
        G4Material * mat = nullptr;

        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(source);
        if (!stSource) continue;

        if (stSource->MaterialLimited)
        {
            G4NistManager * man = G4NistManager::Instance();
            mat = man->FindMaterial(stSource->LimtedToMatName);
        }

        LimitedToMat.push_back(mat);
    }
#else
    const QStringList mats = AMaterialHub::getConstInstance().getListOfMaterialNames();

    for (AParticleSourceRecordBase * source : Settings.SourceData)
    {
        int matIndex = -1; // not limited

        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(source);
        if (!stSource) continue;

        if (stSource->MaterialLimited)
        {
            bool bFound = false;
            int iMat = 0;
            const QString LimitTo = stSource->LimtedToMatName.data();
            for (; iMat < mats.size(); iMat++)
                if (LimitTo == mats[iMat])
                {
                    bFound = true;
                    break;
                }

            if (bFound) matIndex = iMat;
        }

        LimitedToMat.push_back(matIndex);
    }
#endif
}

int ASourceParticleGenerator::selectNumberOfPrimaries() const
{
    if (!Settings.MultiEnabled) return 1;

    if (Settings.MultiMode == ASourceGeneratorSettings::Constant)
        return std::round(Settings.MultiNumber);

    int num = RandomHub.poisson(Settings.MultiNumber);
    return std::max(1, num);
}
