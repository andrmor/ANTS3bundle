#ifndef as2generator_H
#define as2generator_H

#include <vector>

class APhotonTracer;
class APhotonSimSettings;
class ARandomHub;
class AMaterialHub;
class TGeoManager;
class ADepoRecord;

struct DiffSigmas
{
    double sigmaTime; //ns
    double sigmaX;    //mm

    DiffSigmas(double sigmaTime, double sigmaX) : sigmaTime(sigmaTime), sigmaX(sigmaX) {}
    DiffSigmas() {}
};

class AS2Generator
{
public:
    AS2Generator(APhotonTracer & photonTracer);

    void generate(ADepoRecord & rec);
    void clearRemainer() {PhotonRemainer = 0; ElectronRemainer = 0;}

private:
    APhotonTracer            & PhotonTracer;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    const AMaterialHub       & MatHub;

    TGeoManager              * GeoManager = nullptr;

    int NumElectrons;
    int NumPhotons;

    double PhotonRemainer   = 0;
    double ElectronRemainer = 0;

    //double BaseTime;
    std::vector<DiffSigmas> DiffusionRecords;

private:
    bool doDrift(double & time);
    void generateLight(double * xyPosition, double time);
    void generateAndTracePhotons(double * Position, double Time, int NumPhotonsToGenerate, int MatIndexSecScint, double Zstart, double Zspan);
};

#endif // as2generator_H
