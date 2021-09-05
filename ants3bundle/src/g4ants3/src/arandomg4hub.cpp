#include "arandomg4hub.h"

#include "Randomize.hh"

ARandomHub &ARandomHub::getInstance()
{
    static ARandomHub instance;
    return instance;
}

ARandomHub::ARandomHub()
{

}

void ARandomHub::init(int seed)
{
    CLHEP::RanecuEngine * randGen = new CLHEP::RanecuEngine();
    randGen->setSeed(seed);
    G4Random::setTheEngine(randGen);
}

double ARandomHub::uniform()
{
    return G4UniformRand();
}

double ARandomHub::exp(double tau)
{
    return G4RandExponential::shoot(tau);
}

double ARandomHub::gauss(double mean, double sigma)
{
    return G4RandGauss::shoot(mean, sigma);
}

#include "CLHEP/Random/RandPoisson.h"
double ARandomHub::poisson(double mean)
{
    return CLHEP::RandPoissonQ::shoot(mean);
}
