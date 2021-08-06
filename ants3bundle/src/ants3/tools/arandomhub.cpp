#include "arandomhub.h"

#include "TRandom2.h"

ARandomHub &ARandomHub::getInstance()
{
    static ARandomHub instance;
    return instance;
}

ARandomHub::ARandomHub()
{
    RandGen = new TRandom2();
}

double ARandomHub::uniform()
{
    return RandGen->Rndm();
}

double ARandomHub::exp(double tau)
{
    return RandGen->Exp(tau);
}
