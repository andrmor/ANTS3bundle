#ifndef AS1GENERATOR_H
#define AS1GENERATOR_H

class APhotonTracer;
class APhotonSimSettings;
class ARandomHub;
class AMaterialHub;
class ADepoRecord;
class ALightSensorEvent;
class APhotonGenerator;

class AS1Generator
{
public:
    AS1Generator(APhotonGenerator & photonGenerator, APhotonTracer & photonTracer, ALightSensorEvent & event);

    void generate(ADepoRecord & rec);
    void clearRemainer() {Remainer = 0;}

private:
    APhotonGenerator         & PhotonGenerator;
    APhotonTracer            & PhotonTracer;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    const AMaterialHub       & MatHub;
    ALightSensorEvent        & Event;    // only used in LRF mode

    double Remainer = 0;  // is it still a good concept? !!!***
};

#endif // AS1GENERATOR_H
