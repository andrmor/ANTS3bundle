#ifndef AORTHOPOSITRONIUMGAMMAGENERATOR_H
#define AORTHOPOSITRONIUMGAMMAGENERATOR_H

#include "avector.h"

#include "array"

class AOrthoPositroniumGammaGenerator
{
public:
    static void   generate(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies);

private:
    static double getOrthoPsM(double w1, double w2, double w3);
    static void   makeCandidateVectors(std::array<AVector3, 3> & unitVectors, std::array<double, 3> & energies);
};

#endif // AORTHOPOSITRONIUMGAMMAGENERATOR_H
