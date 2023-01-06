#include "asourceparticlegenerator.h"
#include "aparticlesimsettings.h"
#include "aparticlerecord.h"
#include "aparticlesourcerecord.h"
#include "aerrorhub.h"

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
    #include "TGeoManager.h"
    #include "amaterialhub.h"
    #include "arandomhub.h"
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
        for (const AGunParticle & gp : Settings.SourceData[isource].Particles)
            if (gp.GenerationType == AGunParticle::Independent)
                TotalParticleWeight[isource] += gp.StatWeight;
    }

    updateLimitedToMat(); // !!!*** error handling?

    //creating lists of linked particles
    LinkedPartiles.resize(NumSources);
    for (int iSource = 0; iSource < NumSources; iSource++)
    {
        const int numParts = Settings.SourceData[iSource].Particles.size();
        LinkedPartiles[iSource].resize(numParts);
        for (int iParticle = 0; iParticle < numParts; iParticle++)
        {
            LinkedPartiles[iSource][iParticle].clear();
            if (Settings.SourceData[iSource].Particles[iParticle].GenerationType != AGunParticle::Independent)
                continue; //nothing to do for dependent particles

            //every independent particle defines an "event generation chain" containing the particle iteslf and all linked (and linked to linked to linked etc) particles
            LinkedPartiles[iSource][iParticle].push_back(ALinkedParticle(iParticle)); //list always contains the particle itself - simplifies the generation algorithm
            //only particles with larger indexes can be linked to this particle
            for (int ip = iParticle + 1; ip < numParts; ip++)
                if (Settings.SourceData[iSource].Particles[ip].GenerationType != AGunParticle::Independent) //only looking for dependent
                {
                    //for iparticle, checking if it is linked to any particle in the list of the LinkedParticles
                    for (size_t idef=0; idef<LinkedPartiles[iSource][iParticle].size(); idef++)
                    {
                        int compareWith = LinkedPartiles[iSource][iParticle][idef].iParticle;
                        int linkedTo = Settings.SourceData[iSource].Particles[ip].LinkedTo;
                        if ( linkedTo == compareWith)
                        {
                            LinkedPartiles[iSource][iParticle].push_back(ALinkedParticle(ip, linkedTo));
                            break;
                        }
                    }
                }
        }
    }

    //vectors related to collmation direction
    CollimationDirection.resize(NumSources);
    CollimationProbability.resize(NumSources);
    for (int isource=0; isource<NumSources; isource++)
    {
        const double CollPhi   = Settings.SourceData[isource].DirectionPhi*3.1415926535/180.0;
        const double CollTheta = Settings.SourceData[isource].DirectionTheta*3.1415926535/180.0;
        const double Spread    = Settings.SourceData[isource].CutOff*3.1415926535/180.0;

        CollimationDirection[isource] = AVector3(sin(CollTheta)*sin(CollPhi), sin(CollTheta)*cos(CollPhi), cos(CollTheta));
        CollimationProbability[isource] = 0.5 * (1.0 - cos(Spread));
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
            if (Settings.SourceData[iSource].Activity >= rnd) break;
            rnd -= Settings.SourceData[iSource].Activity;
        }
    }

    return iSource;
}

bool ASourceParticleGenerator::selectPosition(int iSource, double * R) const
{
    const AParticleSourceRecord & Source = Settings.SourceData[iSource];
    int attempts = 10000;

#ifdef GEANT4
    if (!LimitedToMat[iSource]) doGeneratePosition(Source, R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(Source, R);

            G4VPhysicalVolume * vol = Navigator->LocateGlobalPointAndSetup({R[0], R[1], R[2]});
            if (vol && vol->GetLogicalVolume())
                if (vol->GetLogicalVolume()->GetMaterial() == LimitedToMat[iSource])
                    break;
        }
        while (attempts-- != 0);
    }
#else
    if (LimitedToMat[iSource] < 0) doGeneratePosition(Source, R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(Source, R);

            TGeoNode * node = gGeoManager->FindNode(R[0], R[1], R[2]);
            if (node && node->GetVolume() && node->GetVolume()->GetMaterial()->GetIndex() == LimitedToMat[iSource]) break;
        }
        while (attempts-- != 0);
    }
#endif

    return true;
}

void ASourceParticleGenerator::generateDirection(size_t iSource, bool forceIsotropic, double * direction) const
{
    const AParticleSourceRecord & src = Settings.SourceData[iSource];

    if (src.AngularMode == AParticleSourceRecord::Isotropic || forceIsotropic)
    {
        //generating random direction inside the collimation cone
        const double spread   = (src.UseCutOff ? src.CutOff*3.14159265358979323846/180.0 : 3.14159265358979323846); //max angle away from generation diretion
        const double cosTheta = cos(spread);
        const double z   = cosTheta + RandomHub.uniform() * (1.0 - cosTheta);
        const double tmp = sqrt(1.0 - z * z);
        const double phi = RandomHub.uniform() * 3.14159265358979323846 * 2.0;

        AVector3 K1( tmp * cos(phi), tmp * sin(phi), z);
        AVector3 Coll( CollimationDirection[iSource] );
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    else if (src.AngularMode == AParticleSourceRecord::FixedDirection)
    {
        for (int i = 0; i < 3; i++) direction[i] = CollimationDirection[iSource][i];
    }
    //*** add error if CutOff is zero in this mode!
    else if (src.AngularMode == AParticleSourceRecord::GaussDispersion)
    {
        double angle = std::fabs(RandomHub.gauss(0, src.DispersionSigma));
        if (src.UseCutOff)
        {
            while (angle > src.CutOff)
                angle = std::fabs(RandomHub.gauss(0, src.DispersionSigma));
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll(CollimationDirection[iSource]);
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    // !!!*** add error if AngularDistribution does not have presence within CutOff
    else if (src.AngularMode == AParticleSourceRecord::CustomAngular)
    {
        double angle = src._AngularSampler.getRandom();
        if (src.UseCutOff)
        {
            while (angle > src.CutOff)
                angle = src._AngularSampler.getRandom();
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll( CollimationDirection[iSource] );
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
}

double ASourceParticleGenerator::selectTime(const AParticleSourceRecord & Source, int iEvent)
{
    double time;
    switch (Source.TimeOffsetMode)
    {
    default:
    case AParticleSourceRecord::FixedOffset              : time = Source.TimeFixedOffset;                                      break;
    case AParticleSourceRecord::ByEventIndexOffset       : time = Source.TimeByEventStart + iEvent * Source.TimeByEventPeriod; break;
    case AParticleSourceRecord::CustomDistributionOffset : time = Source._TimeSampler.getRandom();                             break;
    }

    switch (Source.TimeSpreadMode)
    {
    default:
    case AParticleSourceRecord::NoSpread :
        break;
    case AParticleSourceRecord::GaussianSpread :
        time += ARandomHub::getInstance().gauss(0, Source.TimeSpreadSigma);
        break;
    case AParticleSourceRecord::UniformSpread :
        time += Source.TimeSpreadWidth * (ARandomHub::getInstance().uniform() - 0.5);
        break;
    case AParticleSourceRecord::ExponentialSpread :
        constexpr double ln2 = std::log(2.0);
        time += ARandomHub::getInstance().exp(Source.TimeSpreadHalfLife / ln2);
        break;
    }

    return time;
}

size_t ASourceParticleGenerator::selectParticle(int iSource) const
{
    size_t iParticle = 0;

    const AParticleSourceRecord & Source = Settings.SourceData[iSource];
    double rnd = ARandomHub::getInstance().uniform() * TotalParticleWeight.at(iSource);
    for ( ; iParticle < Source.Particles.size() - 1; iParticle++)
    {
        if (Source.Particles[iParticle].GenerationType == AGunParticle::Independent)
        {
            if (Source.Particles[iParticle].StatWeight >= rnd) break; //this one
            rnd -= Source.Particles[iParticle].StatWeight;
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
        const AParticleSourceRecord & Source = Settings.SourceData[iSource];

        // Particle
        const size_t iParticle = selectParticle(iSource); // cannot avoid using index

        // Position
        double position[3];
        bool ok = selectPosition(iSource, position);  // cannot avoid using index
        if (!ok) return false; // !!!*** error handling!

        // Time
        const double time = selectTime(Source, iEvent);

        // generating the selected particle itself
        addGeneratedParticle(iSource, iParticle, position, time, false, handler);

        // generating linked particles
        std::vector<ALinkedParticle> & ThisLP = LinkedPartiles[iSource][iParticle];
        ThisLP.front().bWasGenerated = true;
        for (size_t ip = 1; ip < ThisLP.size(); ip++) // ThisLP starts from the particle itself, so skip the first record
        {
            const int thisParticle = ThisLP[ip].iParticle;
            const int linkedTo     = ThisLP[ip].LinkedTo;

            if (ThisLP[linkedTo].bWasGenerated) // parent was generated
            {
                if (Source.Particles[thisParticle].GenerationType == AGunParticle::Linked_IfNotGenerated)
                {
                    ThisLP[ip].bWasGenerated = false;
                    continue;
                }
            }
            else // parent was NOT generated
            {
                if (Source.Particles[thisParticle].GenerationType == AGunParticle::Linked_IfGenerated)
                {
                    ThisLP[ip].bWasGenerated = false;
                    continue;
                }
            }

            const double LinkingProbability = Source.Particles[thisParticle].LinkedProb;
            if (ARandomHub::getInstance().uniform() > LinkingProbability)
            {
                ThisLP[ip].bWasGenerated = false;
                continue;
            }

            if (Source.UseCutOff && Source.Particles[thisParticle].Particle != "-") //direct deposition ("-") ignores direction and cut-off
            {
                double inCutoffProbability = CollimationProbability[iSource];
                if (Source.Particles[thisParticle].BtBPair) inCutoffProbability *= 2.0; // if a pair, chance to get in cutoff is doubled
                if (ARandomHub::getInstance().uniform() > inCutoffProbability)
                {
                    // did not pass cut-off
                    ThisLP[ip].bWasGenerated = true;  // marked as generated, just do not add to tracking since outside of cut-off
                    continue;
                }
            }

            ThisLP[ip].bWasGenerated = true;

            addGeneratedParticle(iSource, thisParticle, position, time, true, handler);
        }
    }

    return true;
}

void ASourceParticleGenerator::doGeneratePosition(const AParticleSourceRecord & rec, double * R) const
{
    const double & X0     = rec.X0;
    const double & Y0     = rec.Y0;
    const double & Z0     = rec.Z0;
    const double   Phi    = rec.Phi   * 3.14159265358979323846 / 180.0;
    const double   Theta  = rec.Theta * 3.14159265358979323846 / 180.0;
    const double   Psi    = rec.Psi   * 3.14159265358979323846 / 180.0;
    const double & size1  = rec.Size1;
    const double & size2  = rec.Size2;
    const double & size3  = rec.Size3;

    switch (rec.Shape) //source geometry type
    {
    case AParticleSourceRecord::Point :
    {
        R[0] = X0; R[1] = Y0; R[2] = Z0;
        return;
    }
    case AParticleSourceRecord::Line :
    {
        AVector3 VV(sin(Theta)*sin(Phi), sin(Theta)*cos(Phi), cos(Theta));
        double off = -1.0 + 2.0 * RandomHub.uniform();
        off *= size1;
        R[0] = X0 + VV[0]*off;
        R[1] = Y0 + VV[1]*off;
        R[2] = Z0 + VV[2]*off;
        return;
    }
    case AParticleSourceRecord::Rectangle :
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
    case AParticleSourceRecord::Round :
    {
        const double angle = RandomHub.uniform() * 3.14159265358979323846 * 2.0;
        double r = RandomHub.uniform() + RandomHub.uniform();  //  !!!*** why?
        if (r > 1.0) r = (2.0 - r) * size1;
        else r *=  size1;
        double x = r * cos(angle);
        double y = r * sin(angle);

        AVector3 Circ(x, y, 0);
        Circ.rotateX(Phi);
        Circ.rotateY(Theta);
        Circ.rotateZ(Psi);

        R[0] = X0 + Circ[0];
        R[1] = Y0 + Circ[1];
        R[2] = Z0 + Circ[2];
        return;
    }
    case AParticleSourceRecord::Box :
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
    case AParticleSourceRecord::Cylinder :
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
void ASourceParticleGenerator::addGeneratedParticle(int iSource, int iParticle, double * position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler)
{
    const AParticleSourceRecord & src = Settings.SourceData[iSource];
    const AGunParticle & gp = src.Particles[iParticle];

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

        generateDirection(iSource, forceIsotropic, particle.v);
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
                SM.saveDepoRecord("-", iMat, energy, position, time);
            }
        }
#endif
    }
}

void ASourceParticleGenerator::updateLimitedToMat()
{
    LimitedToMat.clear();

#ifdef GEANT4
    Navigator = new G4Navigator();
    SessionManager & SM = SessionManager::getInstance();
    Navigator->SetWorldVolume(SM.WorldPV);

    for (const AParticleSourceRecord & source : Settings.SourceData)
    {
        G4Material * mat = nullptr;

        if (source.MaterialLimited)
        {
            G4NistManager * man = G4NistManager::Instance();
            mat = man->FindMaterial(source.LimtedToMatName);
        }

        LimitedToMat.push_back(mat);
    }
#else
    const QStringList mats = AMaterialHub::getConstInstance().getListOfMaterialNames();

    for (const AParticleSourceRecord & source : Settings.SourceData)
    {
        int matIndex = -1; // not limited

        if (source.MaterialLimited)
        {
            bool bFound = false;
            int iMat = 0;
            const QString LimitTo = source.LimtedToMatName.data();
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
