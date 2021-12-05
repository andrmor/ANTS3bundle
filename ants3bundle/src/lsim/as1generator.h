#ifndef AS1GENERATOR_H
#define AS1GENERATOR_H

class APhotonTracer;
class APhotonSimSettings;
class ARandomHub;
class AMaterialHub;
class ADepoRecord;

class AS1Generator
{
public:
    AS1Generator(APhotonTracer & photonTracer);

    void generate(ADepoRecord & rec);
    void clearRemainer() {Remainer = 0;}

private:
    APhotonTracer            & PhotonTracer;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    const AMaterialHub       & MatHub;

    double Remainer = 0;  // is it still a good concept? !!!***
};

#endif // AS1GENERATOR_H
