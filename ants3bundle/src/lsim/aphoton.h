#ifndef APHOTON
#define APHOTON

class ASimulationStatistics;
class TRandom2;

class APhoton
{
public:
    APhoton();
    APhoton(double* pos, double* dir, int waveIndex = -1, double time = 0);

    double r[3]; //position
    double v[3]; //direction (must be already normalized to unit vector!!!)
    double time; //time stamp
    int    waveIndex; //wavelength is always binned during simulations
    int    scint_type; //1 - primary //2 - secondary // 0 - undefined

    ASimulationStatistics * SimStat = nullptr;

    void copyFrom(const APhoton * Another);
    void ensureUnitaryLength();
    void randomDir(TRandom2 * RandGen);
};

#endif // APHOTON

