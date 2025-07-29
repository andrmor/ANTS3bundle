#ifndef APARTICLESOURCERECORD_H
#define APARTICLESOURCERECORD_H

#include "ahistogram.h"

#include <string>
#include <vector>

#ifdef JSON11
    #include "js11tools.hh"
#else
    class QJsonObject;
#endif

struct AParticleSourceRecordBase
{
    virtual ~AParticleSourceRecordBase(){}

    virtual std::string getType() const = 0; //{return "Base";}

    std::string Name     = "No_name";
    double      Activity = 1.0;

    void clear();
    virtual void doClear(){}

    virtual std::string check() const = 0;

#ifdef JSON11
    bool readFromJson(const json11::Json::object & json);
    virtual bool doReadFromJson(const json11::Json::object & /*json*/) {return true;}
#else
    bool readFromJson(const QJsonObject & json);
    virtual bool doReadFromJson(const QJsonObject & /*json*/) {return true;}

    void writeToJson(QJsonObject & json) const;
    virtual void doWriteToJson(QJsonObject & /*json*/) const {}
#endif

static AParticleSourceRecordBase * factory(std::string sourceType);
#ifdef JSON11
static AParticleSourceRecordBase * factory(const json11::Json::object & json);
#else
static AParticleSourceRecordBase * factory(const QJsonObject & json);
#endif
};

class G4ParticleDefinition;

// !!!*** add check method (check energy spectrum, values are positive, add: LinkedBtBPair cannot be for "-")
struct AGunParticle
{
    enum EType {Independent, Linked_IfGenerated, Linked_IfNotGenerated};
    enum EEneryUnits {meV, eV, keV, MeV, Angstrom};

    std::string  Particle        = "geantino";

    EType        GenerationType  = Independent;

    double       StatWeight      = 1.0;
    int          LinkedTo        = 0;          // index of the "parent" particle
    double       LinkedProb      = 0;          // probability to be emitted if the parent particle is generated / not_generated
    double       HalfLife        = 0;          // half-life for the time delay in the generated particle

    bool         BtBPair         = false;      // false = normal case (single particle), true = back-to-back pair of identical particles

    bool         UseFixedEnergy  = true;
    double       FixedEnergy     = 100.0;      // in keV
    EEneryUnits  PreferredUnits  = keV;
    std::vector<std::pair<double, double>> EnergySpectrum;
    bool         UseGaussBlur    = false;
    double       EnergySigma     = 50.0;
    EEneryUnits  PreferredSigmaUnits = keV;

    std::string  configureEnergySampler();
    double       generateEnergy() const;

    bool         isDirectDeposition() const;    // will return true on "special" particles too!

#ifdef JSON11
    bool         readFromJson(const json11::Json::object & json);  // !!!***
#else
    bool         readFromJson(const QJsonObject & json);  // !!!***
    void         writeToJson(QJsonObject & json) const;   // !!!***
#endif

    //run-time
    G4ParticleDefinition * particleDefinition = nullptr;
    ARandomSampler _EnergySampler;
};

struct AParticleSourceRecord_Standard : public AParticleSourceRecordBase
{
    virtual std::string getType() const override {return "Standard";}

    enum EShape {Point, Line, Rectangle, Round, Box, Cylinder};
    enum EAngularMode {Isotropic, FixedDirection, GaussDispersion, CustomAngular};
    enum EAxialMode {GaussAxial, CustomAxial};
    enum EOffsetMode {FixedOffset, ByEventIndexOffset, CustomDistributionOffset};
    enum ESpreadMode {NoSpread, GaussianSpread, UniformSpread, ExponentialSpread};
    enum ETimeUnits {ns, us, ms, s, min, h};

    EShape      Shape    = Point;

    // Position
    double      X0    = 0;
    double      Y0    = 0;
    double      Z0    = 0;

    // Orientation
    double      Phi   = 0;
    double      Theta = 0;
    double      Psi   = 0;

    // Size
    double      Size1 = 10.0;
    double      Size2 = 10.0;
    double      Size3 = 10.0;

    // Axial distribution for round
    bool        UseAxialDistribution = false;
    EAxialMode  AxialDistributionType = GaussAxial;
    double      AxialDistributionSigma = 10.0;
    std::vector<std::pair<double, double>> AxialDistribution;

    // Limit to material
    bool        MaterialLimited = false;
    std::string LimtedToMatName;

    // Angular properties
    EAngularMode AngularMode     = Isotropic;
    bool         DirectionBySphericalAngles = false;
    double       DirectionVectorX = 1.0;
    double       DirectionVectorY = 0.0;
    double       DirectionVectorZ = 0.0;
    double       DirectionPhi     = 0;
    double       DirectionTheta   = 0;
    bool         UseCutOff        = false;
    double       CutOff           = 45.0;
    double       DispersionSigma  = 1.0;
    std::vector<std::pair<double, double>> AngularDistribution;

    // Time
    EOffsetMode TimeOffsetMode = FixedOffset;
    double      TimeFixedOffset = 0;
    ETimeUnits  TimeFixedOffsetPrefUnit = ns;   // only for GUI!
    double      TimeByEventStart = 0;
    ETimeUnits  TimeByEventStartPrefUnit = ns;  // only for GUI!
    double      TimeByEventPeriod = 10.0;
    ETimeUnits  TimeByEventPeriodPrefUnit = ns; // only for GUI!
    ESpreadMode TimeSpreadMode = NoSpread;
    double      TimeSpreadSigma = 50.0;
    ETimeUnits  TimeSpreadSigmaPrefUnit = ns;  // only for GUI!
    double      TimeSpreadWidth = 100.0;       // always in ns
    ETimeUnits  TimeSpreadWidthPrefUnit = ns;  // only for GUI!
    double      TimeSpreadHalfLife = 100.0;    // alsways in ns
    ETimeUnits  TimeHalfLifePrefUnit = ns;     // only for GUI!
    std::vector<std::pair<double, double>> TimeDistribution;

    // Particles
    std::vector<AGunParticle> Particles;

    void doClear() override;

#ifdef JSON11
    bool doReadFromJson(const json11::Json::object & json) override; // !!!*** error handling?
#else
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override; // !!!*** error handling
#endif

    std::string getShapeString() const;
    bool        isDirectional() const;

    std::string check() const override;  // !!!*** check energy spectrum

    std::string configureAngularSampler();
    std::string configureTimeSampler();
    std::string configureAxialSampler();

    // run-time
    ARandomSampler      _AngularSampler;
    ARandomSampler      _TimeSampler;
    RandomRadialSampler _AxialSampler;

private:
    AParticleSourceRecord_Standard::ETimeUnits strToTimeUnits(const std::string & str) const;  // !!!*** error reporting
    std::string timeUnitsToString(AParticleSourceRecord_Standard::ETimeUnits timeUnits) const; // !!!*** error reporting
};

struct AParticleSourceRecord_EcoMug : public AParticleSourceRecordBase
{
    std::string getType() const override {return "EcoMug";}

    enum EShape {Rectangle, Cylinder, HalfSphere};

    EShape Shape = Rectangle;

    double Size1 = 50.0; // SizeX or Radius
    double Size2 = 50.0; // SizeY or Height

    // position
    double X0 = 0;
    double Y0 = 0;
    double Z0 = 0;

    void doClear() override;
    std::string check() const override;

#ifdef JSON11
    bool doReadFromJson(const json11::Json::object & /*json*/) override;
#else
    bool doReadFromJson(const QJsonObject & /*json*/) override;
    void doWriteToJson(QJsonObject & /*json*/) const override;
#endif

};

#endif // APARTICLESOURCERECORD_H
