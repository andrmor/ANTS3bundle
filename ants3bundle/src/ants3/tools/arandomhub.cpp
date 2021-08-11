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

void ARandomHub::setSeed(int seed)
{
    RandGen->SetSeed(seed);
}

double ARandomHub::uniform()
{
    return RandGen->Rndm();
}

double ARandomHub::exp(double tau)
{
    return RandGen->Exp(tau);
}

double ARandomHub::gauss(double mean, double sigma)
{
    return RandGen->Gaus(mean, sigma);
}

double ARandomHub::poisson(double mean)
{
    return RandGen->Poisson(mean);
}
