#ifndef AS1GENERATOR_H
#define AS1GENERATOR_H

#include "adeporecord.h"

class APhotonTracer;
class APhotonSimSettings;
class ARandomHub;
class AMaterialHub;

class AS1Generator
{
public:
    AS1Generator(APhotonTracer & photonTracer);

    bool generate(ADepoRecord & rec);

private:
    APhotonTracer            & PhotonTracer;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    const AMaterialHub       & MatHub;

    double Remainer = 0; // !!!*** make it reset at event end?
};

#endif // AS1GENERATOR_H
