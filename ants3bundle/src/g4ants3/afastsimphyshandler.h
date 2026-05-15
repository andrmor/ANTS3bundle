#ifndef AFASTSIMPHYSHANDLER_H
#define AFASTSIMPHYSHANDLER_H

class G4VModularPhysicsList;
class G4Region;

class AFastSimPhysHandler
{
public:
    static void createFastSimulationPhysics(G4VModularPhysicsList * physicsList);
};

#endif // AFASTSIMPHYSHANDLER_H
