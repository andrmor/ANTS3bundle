#ifndef APHOTONGENERATOR_H
#define APHOTONGENERATOR_H

class APhoton;

class APhotonGenerator
{
public:
    explicit APhotonGenerator();

    static void generateWave(APhoton & Photon, int iMaterial);  // !!!*** TH1.random to custom?
    static void generateTime(APhoton & Photon, int iMaterial);
};

#endif // APHOTONGENERATOR_H
