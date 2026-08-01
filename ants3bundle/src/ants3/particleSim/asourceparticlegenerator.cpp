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
    clearSources();

    AbortRequested = false;

    size_t numSources = Settings.SourceData.size();
    if (numSources == 0)
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

    for (size_t iSource = 0; iSource < numSources; iSource++)
    {
        AParticleSourceRecord_EcoMug * muSource = dynamic_cast<AParticleSourceRecord_EcoMug*>(Settings.SourceData[iSource]);
        if (muSource)
        {
            Sources.push_back( new ASource_EcoMug(muSource) );
            continue;
        }

        AParticleSourceRecord_Standard * stSource = dynamic_cast<AParticleSourceRecord_Standard*>(Settings.SourceData[iSource]);
        if (stSource)
        {
            Sources.push_back( new ASource_Standard(stSource) );
            continue;
        }

        AErrorHub::addError("Unknow type of particle source");
        clearSources();
        return false;
    }

    for (ASource_Base * source : Sources)
    {
        bool ok = source->init();
        if (!ok)
        {
            AErrorHub::addError("Failed to init a particle source");
            clearSources();
            return false;
        }
    }

    return true;
}

size_t ASourceParticleGenerator::selectSource() const
{
    size_t iSource = 0;

    const size_t numSources = Settings.getNumSources();
    if (numSources > 1)
    {
        double rnd = ARandomHub::getInstance().uniform() * TotalActivity;
        for (; iSource < numSources - 1; iSource++)
        {
            if (Settings.SourceData[iSource]->Activity >= rnd) break;
            rnd -= Settings.SourceData[iSource]->Activity;
        }
    }

    return iSource;
}

//after any operation with sources (add, remove), init should be called before the first use!
bool ASourceParticleGenerator::generateEvent(std::function<void(const AParticleRecord&)> handler, int iEvent)
{
    const int numPrimaries = selectNumberOfPrimaries();

    for (int iPrimary = 0; iPrimary < numPrimaries; iPrimary++)
    {
        const size_t iSource = selectSource();
        bool ok = Sources[iSource]->generatePrimary(handler, iEvent);
        if (!ok) return false;

        #ifndef GEANT4
        if (AbortRequested) return false;
        #endif
    }

    return true;
}

bool ASourceParticleGenerator::getFirstSourceCollimationDirection(AVector3 & vec)
{
    init();
    if (Sources.size() != 1) return false;

    ASource_Standard * ss = dynamic_cast<ASource_Standard*>(Sources.front());
    if (!ss) return false;

    if (!ss->Settings->isDirectional()) return false;

    vec = ss->CollimationDirection;
    return true;
}

void ASourceParticleGenerator::doRequestAbort()
{
    for (ASource_Base * s : Sources)
        s->AbortRequested = true;
}

void ASourceParticleGenerator::clearSources()
{
    for (ASource_Base * source : Sources)
        delete source;
    Sources.clear();
}

#ifdef GEANT4
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4SystemOfUnits.hh"
#endif

#ifdef GEANT4
#include "G4Gamma.hh"
#endif


int ASourceParticleGenerator::selectNumberOfPrimaries() const
{
    if (!Settings.MultiEnabled) return 1;

    if (Settings.MultiMode == ASourceGeneratorSettings::Constant)
        return std::round(Settings.MultiNumber);

    int num = RandomHub.poisson(Settings.MultiNumber);
    return std::max(1, num);
}

// ---------------------

ASource_Base::ASource_Base() :
    RandomHub(ARandomHub::getInstance()) {}

// ---------------------

ASource_Standard::ASource_Standard(const AParticleSourceRecord_Standard * settings) :
    ASource_Base(), Settings(settings) {}

bool ASource_Standard::init()
{
    AbortRequested = false;

    TotalParticleWeight = 0;
    for (const AGunParticle & gp : Settings->Particles)
        if (gp.GenerationType == AGunParticle::Independent)
            TotalParticleWeight += gp.StatWeight;

    const int numParts = Settings->Particles.size();  // !!!*** to size_t
    LinkedPartiles.resize(numParts);
    for (int iParticle = 0; iParticle < numParts; iParticle++)
    {
        LinkedPartiles[iParticle].clear();
        if (Settings->Particles[iParticle].GenerationType != AGunParticle::Independent)
            continue; //nothing to do for dependent particles

        //every independent particle defines an "event generation chain" containing the particle iteslf and all linked (and linked to linked to linked etc) particles
        LinkedPartiles[iParticle].push_back(ALinkedParticle(iParticle)); //list always contains the particle itself - simplifies the generation algorithm
        //only particles with larger indexes can be linked to this particle
        for (int ip = iParticle + 1; ip < numParts; ip++)
            if (Settings->Particles[ip].GenerationType != AGunParticle::Independent) //only looking for dependent
            {
                //for iparticle, checking if it is linked to any particle in the list of the LinkedParticles
                for (size_t idef = 0; idef < LinkedPartiles[iParticle].size(); idef++)
                {
                    int compareWith = LinkedPartiles[iParticle][idef].iParticle;
                    int linkedTo = Settings->Particles[ip].LinkedTo;
                    if ( linkedTo == compareWith)
                    {
                        LinkedPartiles[iParticle].push_back(ALinkedParticle(ip, linkedTo));
                        break;
                    }
                }
            }
    }

    const double collPhi   = Settings->DirectionPhi * M_PI / 180.0;
    const double collTheta = Settings->DirectionTheta * M_PI / 180.0;
    const double spread    = Settings->CutOff * M_PI / 180.0;
    if (Settings->DirectionBySphericalAngles)
        CollimationDirection = AVector3(sin(collTheta)*sin(collPhi), sin(collTheta)*cos(collPhi), cos(collTheta));
    else
    {
        CollimationDirection = AVector3(Settings->DirectionVectorX, Settings->DirectionVectorY, Settings->DirectionVectorZ);
        CollimationDirection.toUnitVector();
    }

    CollimationProbability = 0.5 * (1.0 - cos(spread));

    updateLimitedToMat();

    return true;
}

bool ASource_Standard::generatePrimary(std::function<void (const AParticleRecord &)> handler, int iEvent)
{
    const size_t iParticle = selectParticle();

    double position[3];
    bool ok = selectPosition(position);
    if (!ok) return false; // !!!*** error handling!

    // Time
    const double time = selectTime(iEvent);

    // generating the selected particle itself
    addGeneratedParticle(iParticle, position, time, false, handler);

    // generating linked particles
    std::vector<ALinkedParticle> & ThisLP = LinkedPartiles[iParticle];
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
            if (Settings->Particles[thisParticle].GenerationType == AGunParticle::Linked_IfNotGenerated)
            {
                ThisLP[ip].bWasGenerated = false;
                continue;
            }
        }
        else // parent was NOT generated
        {
            if (Settings->Particles[thisParticle].GenerationType == AGunParticle::Linked_IfGenerated)
            {
                ThisLP[ip].bWasGenerated = false;
                continue;
            }
        }

        const double LinkingProbability = Settings->Particles[thisParticle].LinkedProb;
        if (ARandomHub::getInstance().uniform() > LinkingProbability)
        {
            ThisLP[ip].bWasGenerated = false;
            continue;
        }

        double halfLife = Settings->Particles[thisParticle].HalfLife;
        if (halfLife != 0) ThisLP[ip].TimeStamp += RandomHub.getInstance().exp(halfLife/log(2));

        if (Settings->UseCutOff && Settings->Particles[thisParticle].Particle != "-") //direct deposition ("-") ignores direction and cut-off
        {
            double inCutoffProbability = CollimationProbability;
            if (Settings->Particles[thisParticle].BtBPair) inCutoffProbability *= 2.0; // if a pair, chance to get in cutoff is doubled
            if (ARandomHub::getInstance().uniform() > inCutoffProbability)
            {
                // did not pass cut-off
                ThisLP[ip].bWasGenerated = true;  // marked as generated, just do not add to tracking since outside of cut-off
                continue;
            }
        }

        ThisLP[ip].bWasGenerated = true;

        addGeneratedParticle(thisParticle, position, ThisLP[ip].TimeStamp, true, handler);
    }
    return true;
}

void ASource_Standard::updateLimitedToMat()
{
    if (Settings->AngularMode == AParticleSourceRecord_Standard::HomogeneousIsotropicField)
    {
#ifdef GEANT4
        LimitedToMat = nullptr;
#else
        LimitedToMat = -1;
#endif
        return;
    }

#ifdef GEANT4
    Navigator = new G4Navigator();
    SessionManager & SM = SessionManager::getInstance();
    Navigator->SetWorldVolume(SM.WorldPV);

    G4Material * mat = nullptr;
    if (Settings->MaterialLimited)
    {
        G4NistManager * man = G4NistManager::Instance();
        mat = man->FindMaterial(Settings->LimtedToMatName);
    }
    LimitedToMat = mat;
#else
    const QStringList mats = AMaterialHub::getConstInstance().getListOfMaterialNames();

    int matIndex = -1; // not limited

    if (Settings->MaterialLimited)
    {
        bool bFound = false;
        int iMat = 0;
        const QString LimitTo = Settings->LimtedToMatName.data();
        for (; iMat < mats.size(); iMat++)
            if (LimitTo == mats[iMat])
            {
                bFound = true;
                break;
            }

        if (bFound) matIndex = iMat;
    }

    LimitedToMat = matIndex;
#endif
}

size_t ASource_Standard::selectParticle() const
{
    size_t iParticle = 0;

    double rnd = ARandomHub::getInstance().uniform() * TotalParticleWeight;
    for ( ; iParticle < Settings->Particles.size() - 1; iParticle++)
    {
        if (Settings->Particles[iParticle].GenerationType == AGunParticle::Independent)
        {
            if (Settings->Particles[iParticle].StatWeight >= rnd) break; //this one
            rnd -= Settings->Particles[iParticle].StatWeight;
        }
    }

    return iParticle;
}

#ifndef GEANT4
#include "ageometryhub.h"
#endif
bool ASource_Standard::selectPosition(double * R) const
{
    int attempts = 10000;

#ifdef GEANT4
    if (!LimitedToMat) doGeneratePosition(R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(R);

            G4VPhysicalVolume * vol = Navigator->LocateGlobalPointAndSetup({R[0], R[1], R[2]});
            if (vol && vol->GetLogicalVolume())
                if (vol->GetLogicalVolume()->GetMaterial() == LimitedToMat)
                    break;

            attempts--;
            if (attempts == 0)
                SessionManager::getInstance().terminateSession("Failed to generate position for material limited source");
        }
        while (attempts != 0);
    }
#else
    if (LimitedToMat < 0) doGeneratePosition(R);
    else
    {
        do
        {
            if (AbortRequested) return false;
            doGeneratePosition(R);

            TGeoNode * node = AGeometryHub::getInstance().GeoManager->FindNode(R[0], R[1], R[2]);
            if (node && node->GetVolume() && node->GetVolume()->GetMaterial()->GetIndex() == LimitedToMat) break;
            QApplication::processEvents();
        }
        while (attempts-- != 0);
    }
#endif

    return true;
}

void ASource_Standard::doGeneratePosition(double * R) const
{
    const double & X0     = Settings->X0;
    const double & Y0     = Settings->Y0;
    const double & Z0     = Settings->Z0;
    const double   Phi    = Settings->Phi   * 3.14159265358979323846 / 180.0;
    const double   Theta  = Settings->Theta * 3.14159265358979323846 / 180.0;
    const double   Psi    = Settings->Psi   * 3.14159265358979323846 / 180.0;
    const double & size1  = Settings->Size1;
    const double & size2  = Settings->Size2;
    const double & size3  = Settings->Size3;

    // special case first:
    if (Settings->AngularMode == AParticleSourceRecord_Standard::HomogeneousIsotropicField)
    {
        double u1 = RandomHub.uniform();
        double u2 = RandomHub.uniform();

        double theta = 2.0 * M_PI * u1;
        double z = size1 * (2.0 * u2 - 1.0);            // uniform in [-R, R]
        double r_xy = std::sqrt(size1 * size1 - z * z); // radius of circle at that z

        R[0] = X0 + r_xy * cos(theta);
        R[1] = Y0 + r_xy * sin(theta);
        R[2] = Z0 + z;
        return;
    }

    switch (Settings->Shape) //source geometry type
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
        if (!Settings->UseAxialDistribution)
        {
            double r = RandomHub.uniform() + RandomHub.uniform();
            if (r > 1.0) r = (2.0 - r) * size1;
            else r *=  size1;
            const double angle = RandomHub.uniform() * 3.14159265358979323846 * 2.0;
            circ[0] = r * cos(angle);
            circ[1] = r * sin(angle);
        }
        else if (Settings->AxialDistributionType == AParticleSourceRecord_Standard::GaussAxial)
        {
            // !!!*** can be faster if radial and angle, but take care of the radial distortion
            do
            {
                circ[0] = RandomHub.gauss(0, Settings->AxialDistributionSigma);
                circ[1] = RandomHub.gauss(0, Settings->AxialDistributionSigma);
            }
            while (circ[0]*circ[0] + circ[1]*circ[1] > size1*size1);
        }
        else
        {
            do Settings->_AxialSampler.generatePosition(circ);
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
    case AParticleSourceRecord_Standard::Sphere :
    {
        double x, y, z;
        do
        {
            x = -1.0 + 2.0 * RandomHub.uniform();
            y = -1.0 + 2.0 * RandomHub.uniform();
            z = -1.0 + 2.0 * RandomHub.uniform();
        }
        while (x*x + y*y + z*z > 1.0);

        R[0] = X0 + size1 * x;
        R[1] = Y0 + size1 * y;
        R[2] = Z0 + size1 * z;
        return;
    }
    }
    return;
}

double ASource_Standard::selectTime(int iEvent)
{
    double time;
    switch (Settings->TimeOffsetMode)
    {
    default:
    case AParticleSourceRecord_Standard::FixedOffset              : time = Settings->TimeFixedOffset;                                         break;
    case AParticleSourceRecord_Standard::ByEventIndexOffset       : time = Settings->TimeByEventStart + iEvent * Settings->TimeByEventPeriod; break;
    case AParticleSourceRecord_Standard::CustomDistributionOffset : time = Settings->_TimeSampler.getRandom();                                break;
    }

    switch (Settings->TimeSpreadMode)
    {
    default:
    case AParticleSourceRecord_Standard::NoSpread :
        break;
    case AParticleSourceRecord_Standard::GaussianSpread :
        time += ARandomHub::getInstance().gauss(0, Settings->TimeSpreadSigma);
        break;
    case AParticleSourceRecord_Standard::UniformSpread :
        time += Settings->TimeSpreadWidth * ARandomHub::getInstance().uniform();
        break;
    case AParticleSourceRecord_Standard::ExponentialSpread :
        const double ln2 = std::log(2.0);
        time += ARandomHub::getInstance().exp(Settings->TimeSpreadHalfLife / ln2);
        break;
    }

    return time;
}

void ASource_Standard::generateDirection(bool forceIsotropic, const double * position, double * direction) const
{
    if (Settings->AngularMode == AParticleSourceRecord_Standard::HomogeneousIsotropicField)
    {
        AVector3 P(position);
        AVector3 C(Settings->X0, Settings->Y0, Settings->Z0);

        // Inward-pointing normal (pole of the hemisphere)
        AVector3 w = (C - P) * (1.0 / Settings->Size1);
        w.toUnitVector();

        // Build an orthonormal basis (u, v, w) around w.
        // TVector3::Orthogonal() returns some vector orthogonal to w.
        AVector3 u = w.orthogonal();
        u.toUnitVector();
        AVector3 v = w.vectorProduct(u);
        v.toUnitVector();

        // Cosine-weighted sample on the hemisphere
        double xi1 = RandomHub.uniform();
        double xi2 = RandomHub.uniform();

        double phi = 2.0 * M_PI * xi1;
        double r   = std::sqrt(xi2);

        double xl = r * std::cos(phi);
        double yl = r * std::sin(phi);
        double zl = std::sqrt(1.0 - xi2);

        // Transform to world coordinates
        AVector3 D = u * xl + v * yl + w * zl;
        D.toUnitVector();

        for (size_t i = 0; i < 3; i++) direction[i] = D[i];
        return;
    }
    else if (Settings->AngularMode == AParticleSourceRecord_Standard::Isotropic || forceIsotropic)
    {
        //generating random direction inside the collimation cone
        const double spread   = (Settings->UseCutOff ? Settings->CutOff*3.14159265358979323846/180.0 : 3.14159265358979323846); //max angle away from generation diretion
        const double cosTheta = cos(spread);
        const double z   = cosTheta + RandomHub.uniform() * (1.0 - cosTheta);
        const double tmp = sqrt(1.0 - z * z);
        const double phi = RandomHub.uniform() * 3.14159265358979323846 * 2.0;

        AVector3 K1( tmp * cos(phi), tmp * sin(phi), z);
        AVector3 Coll(CollimationDirection);
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    else if (Settings->AngularMode == AParticleSourceRecord_Standard::FixedDirection)
    {
        for (int i = 0; i < 3; i++) direction[i] = CollimationDirection[i];
    }
    //*** add error if CutOff is zero in this mode!
    else if (Settings->AngularMode == AParticleSourceRecord_Standard::GaussDispersion)
    {
        double angle = std::fabs(RandomHub.gauss(0, Settings->DispersionSigma));
        if (Settings->UseCutOff)
        {
            while (angle > Settings->CutOff)
            {
                angle = std::fabs(RandomHub.gauss(0, Settings->DispersionSigma));
#ifndef GEANT4
                QApplication::processEvents();
                if (AbortRequested) break;
#endif
            }
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll(CollimationDirection);
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
    // !!!*** add error if AngularDistribution does not have presence within CutOff
    else if (Settings->AngularMode == AParticleSourceRecord_Standard::CustomAngular)
    {
        double angle = Settings->_AngularSampler.getRandom();
        if (Settings->UseCutOff)
        {
            while (angle > Settings->CutOff)
            {
                angle = Settings->_AngularSampler.getRandom();
#ifndef GEANT4
                QApplication::processEvents();
                if (AbortRequested) break;
#endif
            }
        }

        AVector3 K1(0, 0, 1.0);
        K1.rotateX(angle * 3.14159265358979323846 / 180.0);
        K1.rotateZ(RandomHub.uniform() * 3.14159265358979323846 * 2.0);
        AVector3 Coll(CollimationDirection);
        K1.rotateUz(Coll);
        for (int i = 0; i < 3; i++) direction[i] = K1[i];
    }
}

#include "aorthopositroniumgammagenerator.h"
void ASource_Standard::processSpecialParticle(const AGunParticle & particle, double *position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler)
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

void ASource_Standard::addGeneratedParticle(int iParticle, double *position, double time, bool forceIsotropic, std::function<void (const AParticleRecord &)> handler)
{
    const AGunParticle & gp = Settings->Particles[iParticle];

    if (gp.Particle[0] == '_')
    {
        processSpecialParticle(gp, position, time, forceIsotropic, handler);
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

        generateDirection(forceIsotropic, position, particle.v);
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

// ---------------------

ASource_EcoMug::ASource_EcoMug(const AParticleSourceRecord_EcoMug * settings) :
    ASource_Base(), Settings(settings) {}

ASource_EcoMug::~ASource_EcoMug()
{
    delete EcoMugGenerator;
}

bool ASource_EcoMug::init()
{
    AbortRequested = false;

    delete EcoMugGenerator; EcoMugGenerator = nullptr;
    EcoMugGenerator = new EcoMug();

    switch (Settings->Shape)
    {
    case AParticleSourceRecord_EcoMug::Rectangle :
        EcoMugGenerator->SetUseSky();
        EcoMugGenerator->SetSkySize({Settings->Size1, Settings->Size2});
        EcoMugGenerator->SetSkyCenterPosition({Settings->X0, Settings->Y0, Settings->Z0});
        break;
    case AParticleSourceRecord_EcoMug::Cylinder :
        EcoMugGenerator->SetUseCylinder();
        EcoMugGenerator->SetCylinderRadius(Settings->Size1);
        EcoMugGenerator->SetCylinderHeight(Settings->Size2);
        EcoMugGenerator->SetCylinderCenterPosition({Settings->X0, Settings->Y0, Settings->Z0});
        break;
    case AParticleSourceRecord_EcoMug::HalfSphere :
        EcoMugGenerator->SetUseHSphere();
        EcoMugGenerator->SetHSphereRadius(Settings->Size1);
        EcoMugGenerator->SetHSphereCenterPosition({Settings->X0, Settings->Y0, Settings->Z0});
        break;
    }

    return true;
}

bool ASource_EcoMug::generatePrimary(std::function<void (const AParticleRecord &)> handler, int)
{
    EcoMugGenerator->Generate();

    const double p = EcoMugGenerator->GetGenerationMomentum(); // [GeV/c]
    double mass = 0.10566; // GeV/c²
    // Ekin = sqrt(p²*c² + m²*c⁴) - mc²
    double energy_GeV = sqrt(p*p + mass*mass) - mass; // multiply by 1e6 to get in keV

    AParticleRecord particle(
#ifdef GEANT4
        (EcoMugGenerator->GetCharge() < 0 ? (G4ParticleDefinition*)G4MuonMinus::Definition() : (G4ParticleDefinition*)G4MuonPlus::Definition()),
        (double*)EcoMugGenerator->GetGenerationPosition().data(),
        0,
        energy_GeV * 1e6); // in keV
#else
        (EcoMugGenerator->GetCharge() < 0 ? "mu-" : "mu+"),
        (double*)EcoMugGenerator->GetGenerationPosition().data(),
        0,
        energy_GeV * 1e6); // in keV
#endif

    std::array<double,3> vec;
    EcoMugGenerator->GetGenerationMomentum(vec);
    particle.setDirection(vec.data());
    particle.ensureUnitaryLength();

    handler(particle);
    return true;
}
