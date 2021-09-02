#ifndef APARTICLESOURCERECORD_H
#define APARTICLESOURCERECORD_H

#include "ahistogram.h"

#include <string>
#include <vector>

class QJsonObject;

struct GunParticleStruct
{
    std::string  Particle        = "geantino";
    double       StatWeight      = 1.0;
    bool         bUseFixedEnergy = true;
    double       energy          = 100.0; //in keV
    std::string  PreferredUnits  = "keV";
    bool         Individual      = true; // true = individual particle; false = linked
    int          LinkedTo        = 0; // index of the "parent" particle this one is following
    double       LinkedProb      = 0;  //probability to be emitted after the parent particle
    bool         LinkedOpposite  = false; // false = random direction; otherwise particle is emitted in the opposite direction in respect to the LinkedTo particle

    AHistogram1D EnergyDistr; //energy spectrum   !!!*** check initRandomGenerator is called

    double  generateEnergy() const;
    bool    loadSpectrum(const std::string & fileName); // !!!***

    void writeToJson(QJsonObject & json) const;   // !!!***
    bool readFromJson(const QJsonObject & json);  // !!!***
};

struct AParticleSourceRecord
{
    std::string name = "No_name";
    int         shape = 0;    // !!!*** change to enum
    //position
    double      X0 = 0;
    double      Y0 = 0;
    double      Z0 = 0;
    //orientation
    double      Phi = 0;
    double      Theta = 0;
    double      Psi = 0;
    //size
    double      size1 = 10.0;
    double      size2 = 10.0;
    double      size3 = 10.0;
    //collimation
    double      CollPhi = 0;
    double      CollTheta = 0;
    double      Spread = 45.0;

    //limit to material
    bool        DoMaterialLimited = false;
    std::string LimtedToMatName;

    //Relative activity
    double      Activity = 1.0;

    //time
    int         TimeAverageMode = 0;
    double      TimeAverage = 0;
    double      TimeAverageStart = 0;
    double      TimeAveragePeriod = 10.0;
    int         TimeSpreadMode = 0;
    double      TimeSpreadSigma = 50.0;
    double      TimeSpreadWidth = 100.0;

    //particles
    std::vector<GunParticleStruct> GunParticles;

    void writeToJson(QJsonObject & json) const;
    bool readFromJson(const QJsonObject & json);

    std::string getShapeString() const;

    std::string check() const;

    // next 3: !!!*** to generator?
    void updateLimitedToMat();
    //runtime properties
    int  LimitedToMat; //automatically calculated if LimtedToMatName matches a material
    bool bLimitToMat = false;
};


#endif // APARTICLESOURCERECORD_H
