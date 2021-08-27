#ifndef StackingAction_h
#define StackingAction_h

#include "G4UserStackingAction.hh"

class G4Track;

class StackingAction : public G4UserStackingAction
{
public:
    StackingAction();
    virtual ~StackingAction();

    virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track * track) override;
};

#endif // StackingAction_h
