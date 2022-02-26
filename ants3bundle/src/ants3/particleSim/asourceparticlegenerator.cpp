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
            if (gp.Individual)
                TotalParticleWeight[isource] += gp.StatWeight;
    }

    updateLimitedToMat(); // !!!*** error handling?

    // !!!*** check energy histograms, init them

    //creating lists of linked particles
    LinkedPartiles.resize(NumSources);
    for (int isource=0; isource<NumSources; isource++)
    {
        int numParts = Settings.SourceData[isource].Particles.size();
        LinkedPartiles[isource].resize(numParts);
        for (int iparticle=0; iparticle<numParts; iparticle++)
        {
            LinkedPartiles[isource][iparticle].resize(0);
            if (!Settings.SourceData[isource].Particles[iparticle].Individual)
            {
                if (iparticle == 0)
                {
                    AErrorHub::addError("The first particle of the source is marked as linked!");
                    return false;
                }
                continue; //nothing to do for non-individual particles
            }

            //every individual particles defines an "event generation chain" containing the particle iteslf and all linked (and linked to linked to linked etc) particles
            LinkedPartiles[isource][iparticle].push_back(ALinkedParticle(iparticle)); //list always contains the particle itself - simplifies the generation algorithm
            //only particles with larger indexes can be linked to this particle
            for (int ip=iparticle+1; ip<numParts; ip++)
                if (!Settings.SourceData[isource].Particles[ip].Individual) //only looking for non-individuals
                {
                    //for iparticle, checking if it is linked to any particle in the list of the LinkedParticles
                    for (size_t idef=0; idef<LinkedPartiles[isource][iparticle].size(); idef++)
                    {
                        int compareWith = LinkedPartiles[isource][iparticle][idef].iParticle;
                        int linkedTo = Settings.SourceData[isource].Particles[ip].LinkedTo;
                        if ( linkedTo == compareWith)
                        {
                            LinkedPartiles[isource][iparticle].push_back(ALinkedParticle(ip, linkedTo));
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
        const double CollPhi   = Settings.SourceData[isource].CollPhi*3.1415926535/180.0;
        const double CollTheta = Settings.SourceData[isource].CollTheta*3.1415926535/180.0;
        const double Spread    = Settings.SourceData[isource].Spread*3.1415926535/180.0;

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

double ASourceParticleGenerator::selectTime(const AParticleSourceRecord & Source, int iEvent)
{
    double time;

    if (Source.TimeAverageMode == 0) time = Source.TimeAverage;
    else                             time = Source.TimeAverageStart + iEvent * Source.TimeAveragePeriod;

    switch (Source.TimeSpreadMode)
    {
    default:
    case 0:
        break;
    case 1:
        time = ARandomHub::getInstance().gauss(time, Source.TimeSpreadSigma);
        break;
    case 2:
        time += (-0.5 * Source.TimeSpreadWidth + ARandomHub::getInstance().uniform() * Source.TimeSpreadWidth);
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
        if (Source.Particles[iParticle].Individual)
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

        // first store generated particles as there could be linked particles and we need direction for the "opposite direction" case
        GeneratedParticles.clear();

        if (LinkedPartiles[iSource][iParticle].size() == 1)
        {
            // there are no linked particles (the first record is the particle itself)
            addGeneratedParticle(iSource, iParticle, position, time);
        }
        else
        {
            std::vector<ALinkedParticle> & ThisLP = LinkedPartiles[iSource][iParticle];

            for (size_t ip = 0; ip < ThisLP.size(); ip++)  //ip - index in the list of linked particles
            {
                ThisLP[ip].bWasGenerated = false;

                const int  thisParticle = ThisLP[ip].iParticle;
                const int  linkedTo     = ThisLP[ip].LinkedTo;
                const bool bOpposite    = Source.Particles[thisParticle].LinkedOpposite;

                if (ip != 0) // first is always generated, init checks that the first is not marked as opposite
                {
                    if (!ThisLP[linkedTo].bWasGenerated) continue; // parent was not generated

                    const double LinkingProbability = Source.Particles[thisParticle].LinkedProb;
                    if (ARandomHub::getInstance().uniform() > LinkingProbability) continue;

                    if (!bOpposite)
                    {
                        if (ARandomHub::getInstance().uniform() > CollimationProbability[iSource])
                            continue; // cone test fail
                    }
                    // else it will be generated in opposite direction and ignore collimation cone
                }

                ThisLP[ip].bWasGenerated = true;

                int index = -1;
                if (bOpposite)
                {
                    for (int i = 0; i < linkedTo + 1; i++)
                        if (ThisLP[i].bWasGenerated) index++;
                }
                addGeneratedParticle(iSource, thisParticle, position, time, index);
            }
        }

        // using casll-back to do the actual work with the particles
        for (const AParticleRecord & particle : GeneratedParticles)
            handler(particle);
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

void ASourceParticleGenerator::addGeneratedParticle(int iSource, int iParticle, double * position, double time, int oppositeToIndex)
{
    const AGunParticle & gp = Settings.SourceData[iSource].Particles[iParticle];

    const double energy = gp.generateEnergy();

#ifdef GEANT4
    if (gp.particleDefinition)
#else
    if (gp.Particle != "-")
#endif
    {
        AParticleRecord particle(
#ifdef GEANT4
                                 gp.particleDefinition,
#else
                                 gp.Particle,
#endif
                                 position, time, energy);

        if (oppositeToIndex == -1)
        {
            //generating random direction inside the collimation cone
            const double spread   = Settings.SourceData[iSource].Spread * 3.14159265358979323846 / 180.0; //max angle away from generation diretion
            const double cosTheta = cos(spread);
            const double z   = cosTheta + RandomHub.uniform() * (1.0 - cosTheta);
            const double tmp = sqrt(1.0 - z * z);
            const double phi = RandomHub.uniform() * 3.14159265358979323846 * 2.0;

            AVector3 K1( tmp * cos(phi), tmp * sin(phi), z);
            AVector3 Coll( CollimationDirection[iSource] );
            K1.rotateUz(Coll);
            for (int i = 0; i < 3; i++)  particle.v[i] = K1[i];
        }
        else
        {
            for (int i = 0; i < 3; i++) particle.v[i] = -GeneratedParticles[oppositeToIndex].v[i];
        }

        GeneratedParticles.push_back(particle);
    }
    else
    {
#ifdef GEANT4
        // direct deposition
        SessionManager & SM = SessionManager::getInstance();

        if (!Navigator) Navigator = new G4Navigator();
        Navigator->SetWorldVolume(SM.WorldPV);
        G4VPhysicalVolume * vol = Navigator->LocateGlobalPointAndSetup({position[0], position[1], position[2]});
        if (vol && vol->GetLogicalVolume())
        {
            const int iMat = SM.findMaterial(vol->GetLogicalVolume()->GetMaterial()->GetName());
            SM.saveDepoRecord("-", iMat, energy, position, time);
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
//            G4NistManager * man = G4NistManager::Instance();
//            SourceMat = man->FindMaterial(source.LimtedToMatName);
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
