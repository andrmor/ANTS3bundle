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

struct AGunParticle
{
    std::string  Particle        = "geantino";
    double       StatWeight      = 1.0;
    bool         UseFixedEnergy  = true;
    double       Energy          = 100.0;      //in keV
    std::string  PreferredUnits  = "keV";
    bool         Individual      = true;       // true = individual particle; false = linked
    int          LinkedTo        = 0;          // index of the "parent" particle this one is following
    double       LinkedProb      = 0;          //probability to be emitted after the parent particle
    bool         LinkedOpposite  = false;      // false = isotropic direction; else opposite direction in respect to the LinkedTo particle

    std::vector<std::pair<double, double>> EnergySpectrum;

    bool    buildEnergyHistogram();
    double  generateEnergy() const;

#ifdef JSON11
    bool    readFromJson(const json11::Json::object & json);  // !!!***
#else
    void    writeToJson(QJsonObject & json) const;   // !!!***
    bool    readFromJson(const QJsonObject & json);  // !!!***
#endif

    //run-time
    G4ParticleDefinition * particleDefinition = nullptr;
    AHistogram1D _EnergyHist; //energy spectrum   !!!*** check initRandomGenerator is called
};

struct AParticleSourceRecord
{
    enum EShape {Point, Line, Rectangle, Round, Box, Cylinder};

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

    // Collimation
    double      CollPhi   = 0;
    double      CollTheta = 0;
    double      Spread    = 45.0;

    // Limit to material
    bool        MaterialLimited = false;  // !!!*** remove? use empty LimtedToMatName
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
    void writeToJson(QJsonObject & json) const;
    bool readFromJson(const QJsonObject & json); // !!!*** error handling
#endif

    std::string getShapeString() const;

    std::string check() const;  // !!!*** check energy spectrum

};

#endif // APARTICLESOURCERECORD_H
