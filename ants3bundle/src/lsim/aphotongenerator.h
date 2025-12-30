#ifndef APHOTONGENERATOR_H
#define APHOTONGENERATOR_H

#include "TVector3.h"

class APhoton;

class APhotonGenerator
{
public:
    void init();

    void generateDirection(APhoton & photon);
    void generateWave(APhoton & Photon, int iMaterial);  // !!!*** TH1.random to custom?
    void generateTime(APhoton & Photon, int iMaterial);

protected:
    TVector3 ColDirUnitary{0, 0, 1.0};
    double   CosConeAngle = 0;

    int      FixedWaveIndex = -1;
};

#endif // APHOTONGENERATOR_H
