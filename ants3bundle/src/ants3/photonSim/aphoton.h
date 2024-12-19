#ifndef APHOTON
#define APHOTON

#include "adataiobase.h"

class APhotonStatistics;
class QString;

enum class EInterfaceResult   {Undefined, Absorbed, Reflected, Transmitted};

class APhoton : public ADataIOBase
{
public:
    APhoton();
    APhoton(double * pos, double * dir, int waveIndex = -1, double time = 0);

    double r[3]; //position
    double v[3]; //direction (must be already normalized to unit vector!!!)
    double time           = 0;
    int    waveIndex      = -1;     //wavelength is always binned during simulations
    bool   SecondaryScint = false;

    void copyFrom(const APhoton & CopyFrom);
    void ensureUnitaryLength();
    void generateRandomDir(); // isotropic

    void writeAscii(QTextStream & stream) const override;
    bool readAscii(QString & line) override;

    //void writeBinary() const override;
    bool readBinary(std::ifstream & stream) override;

    void print(QString & text) override;
};

#endif // APHOTON

