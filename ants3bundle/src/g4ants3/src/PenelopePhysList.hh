#ifndef PenelopePhysList_h
#define PenelopePhysList_h 1

#include "G4VModularPhysicsList.hh"

class PenelopePhysList : public G4VModularPhysicsList
{
public:
    PenelopePhysList(G4int ver = 1);

    // delete copy constructor and assignment operator
    PenelopePhysList(const PenelopePhysList &)=delete;
    PenelopePhysList & operator=(const PenelopePhysList &right)=delete;
};

#endif //PenelopePhysList_h
