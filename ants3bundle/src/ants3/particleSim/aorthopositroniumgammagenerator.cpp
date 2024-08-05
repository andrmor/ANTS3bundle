#include "aorthopositroniumgammagenerator.h"
#include "arandomhub.h"

#include <math.h>

void AOrthoPositroniumGammaGenerator::generate(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies)
{
    ARandomHub & RandHub = ARandomHub::getInstance();

    const double orthoPsMMax = 7.65928;
    double weight, random_weight;
    do
    {
        makeCandidateVectors(unitVectors, energies);

        weight = getOrthoPsM(energies[0], energies[1], energies[2]);
        random_weight = orthoPsMMax * RandHub.uniform();
    }
    while (random_weight > weight);
}

double AOrthoPositroniumGammaGenerator::getOrthoPsM(double w1, double w2, double w3)
{
    constexpr double electronMass = 2.0 * 0.510998910;
    return pow( ( electronMass - w1 ) / ( w2 * w3 ), 2 ) + pow( ( electronMass - w2 ) / ( w1 * w3 ), 2 ) + pow( ( electronMass - w3 ) / ( w1 * w2 ), 2 );
}

void AOrthoPositroniumGammaGenerator::makeCandidateVectors(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies)
{
    ARandomHub & RandHub = ARandomHub::getInstance();

    constexpr double parentmass = 2.0 * 0.510998910;

    // https://apc.u-paris.fr/~franco/g4doxy/html/classG4GeneralPhaseSpaceDecay.html

    double daughtermomentum[3];
    double momentummax = 0;
    double momentumsum = 0;
    do
    {
        double rd1 = RandHub.uniform();
        double rd2 = RandHub.uniform();
        if (rd2 > rd1)
        {
            double rd  = rd1;
            rd1 = rd2;
            rd2 = rd;
        }
        momentummax = 0;
        momentumsum = 0;

        // daughter 0
        double energy = rd2 * parentmass;
        daughtermomentum[0] = energy;
        if (daughtermomentum[0] > momentummax) momentummax = daughtermomentum[0];
        momentumsum += daughtermomentum[0];

        // daughter 1
        energy = (1.0 - rd1) * parentmass;
        daughtermomentum[1] = energy;
        if (daughtermomentum[1] > momentummax) momentummax = daughtermomentum[1];
        momentumsum += daughtermomentum[1];

        // daughter 2
        energy = (rd1 - rd2) * parentmass;
        daughtermomentum[2] = energy;
        if ( daughtermomentum[2] >momentummax )momentummax = daughtermomentum[2];
        momentumsum += daughtermomentum[2];
    }
    while (momentummax > momentumsum - momentummax);

    //qDebug() << "     daughter 0:" << daughtermomentum[0];// /GeV << "[GeV/c]" <<G4endl;
    //qDebug() << "     daughter 1:" << daughtermomentum[1];
    //qDebug() << "     daughter 2:" << daughtermomentum[2];
    //qDebug() << "   momentum sum:" << momentumsum;//  /GeV << "[GeV/c]" <<G4endl;

    // first
    double costheta = 2.0 * RandHub.uniform() - 1.0;
    double sintheta = std::sqrt((1.0 - costheta)*(1.0 + costheta));
    double phi = 2.0*3.1415926535 * RandHub.uniform(); // [rad]
    double sinphi = std::sin(phi);
    double cosphi = std::cos(phi);
    unitVectors[0] = AVector3(sintheta*cosphi, sintheta*sinphi, costheta);
    energies[0] = daughtermomentum[0];
    //AVector3 momentumVector = direction * daughtermomentum[0];

    // second
    double costhetan = (daughtermomentum[1]*daughtermomentum[1] - daughtermomentum[2]*daughtermomentum[2] - daughtermomentum[0]*daughtermomentum[0]) / (2.0*daughtermomentum[2]*daughtermomentum[0]);
    double sinthetan = std::sqrt( (1.0 - costhetan) * (1.0 + costhetan) );
    double phin  = 2.0 * 3.1415926535 * RandHub.uniform(); // [rad]
    double sinphin = std::sin(phin);
    double cosphin = std::cos(phin);
    unitVectors[1] = AVector3(sinthetan*cosphin*costheta*cosphi - sinthetan*sinphin*sinphi + costhetan*sintheta*cosphi,
                              sinthetan*cosphin*costheta*sinphi + sinthetan*sinphin*cosphi + costhetan*sintheta*sinphi,
                              -sinthetan*cosphin*sintheta + costhetan*costheta);
    energies[1] = std::sqrt(daughtermomentum[2]*daughtermomentum[2]/unitVectors[1].mag2());
    unitVectors[1].toUnitVector();

    // third
    AVector3 mom(unitVectors[0]*daughtermomentum[0] + unitVectors[1]*daughtermomentum[2]);
    mom *= -1.0;
    energies[2] = std::sqrt(mom.mag2());
    unitVectors[2] = mom.toUnitVector();
}
