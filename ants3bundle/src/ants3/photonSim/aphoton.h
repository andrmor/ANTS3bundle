#ifndef APHOTON
#define APHOTON

class APhoton
{
public:
    APhoton();
    APhoton(double * pos, double * dir, int waveIndex = -1, double time = 0);

    double r[3]; //position
    double v[3]; //direction (must be already normalized to unit vector!!!)
    double time           = 0;
    int    waveIndex      = -1;     //wavelength is always binned during simulations
    bool   SecondaryScint = false;

    void copyFrom(const APhoton * CopyFrom);
    void ensureUnitaryLength();
    void generateRandomDir(); // !!!*** in APhotonGenerator now!
};

#endif // APHOTON

