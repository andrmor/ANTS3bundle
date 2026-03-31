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

private:
    APhotonGenerator         & PhotonGenerator;
    APhotonTracer            & PhotonTracer;
    const APhotonSimSettings & SimSet;
    ARandomHub               & RandomHub;
    const AMaterialHub       & MatHub;
    ALightSensorEvent        & Event;    // only used in LRF mode
};

#endif // AS1GENERATOR_H
