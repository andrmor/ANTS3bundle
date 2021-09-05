#ifndef SteppingAction_h
#define SteppingAction_h

#include "G4UserSteppingAction.hh"

#include <vector>

class G4Step;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction();
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step * step) override;

private:
    std::vector<int> TmpSecondaries; // to avoid repetitive re-allocations
};

#endif // SteppingAction_h
