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

class G4ParticleDefinition;

// !!!*** add check method (check energy spectrum, values are positive, add: LinkedBtBPair cannot be for "-")
struct AGunParticle
{
    enum EType {Independent, Linked_IfGenerated, Linked_IfNotGenerated};

    std::string  Particle        = "geantino";

    EType        GenerationType  = Independent;

    double       StatWeight      = 1.0;
    int          LinkedTo        = 0;          // index of the "parent" particle
    double       LinkedProb      = 0;          // probability to be emitted if the parent particle is generated / not_generated

    bool         BtBPair         = false;      // false = normal case (single particle), true = back-to-back pair of identical particles

    bool         UseFixedEnergy  = true;
    double       FixedEnergy     = 100.0;   // in keV
    std::string  PreferredUnits  = "keV";
    std::vector<std::pair<double, double>> EnergySpectrum;
    bool         RangeBasedEnergies = false;

    bool    configureEnergySampler();
    double  generateEnergy() const;

    bool    isDirectDeposition() const;

#ifdef JSON11
    bool    readFromJson(const json11::Json::object & json);  // !!!***
#else
    bool    readFromJson(const QJsonObject & json);  // !!!***
    void    writeToJson(QJsonObject & json) const;   // !!!***
#endif

    //run-time
    G4ParticleDefinition * particleDefinition = nullptr;
    ARandomSampler _EnergySampler;
};

struct AParticleSourceRecord
{
    enum EShape {Point, Line, Rectangle, Round, Box, Cylinder};
    enum EAngularMode {UniformAngular, FixedDirection, GaussDispersion, CustomAngular};

    std::string Name  = "No_name";
    EShape      Shape = Point;

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

    // Angular properties
    EAngularMode AngularMode;
    double       DirectionPhi   = 0;
    double       DirectionTheta = 0;
    bool         UseCutOff = false;
    double       CutOff = 45.0;
    double       DispersionSigma = 1.0;
    std::vector<std::pair<double, double>> AngularDistribution;

    // Limit to material
    bool        MaterialLimited = false;
    std::string LimtedToMatName;

    // Relative activity
    double      Activity = 1.0;

    // Time
    int         TimeAverageMode = 0;  // !!!*** to enum
    double      TimeAverage = 0;
    double      TimeAverageStart = 0;
    double      TimeAveragePeriod = 10.0;
    int         TimeSpreadMode = 0;  // !!!*** to enum
    double      TimeSpreadSigma = 50.0;
    double      TimeSpreadWidth = 100.0;

    // Particles
    std::vector<AGunParticle> Particles;

    void clear();

#ifdef JSON11
    bool readFromJson(const json11::Json::object & json); // !!!*** error handling?
#else
    void writeToJson(QJsonObject & json) const;  // make subjson for time, angular etc !!!***
    bool readFromJson(const QJsonObject & json); // !!!*** error handling
#endif

    std::string getShapeString() const;

    std::string check() const;  // !!!*** check energy spectrum

    bool configureAngularSampler();

    // run-time
    ARandomSampler _AngularSampler;
};

#endif // APARTICLESOURCERECORD_H
