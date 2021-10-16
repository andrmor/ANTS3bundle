#include "SteppingAction.hh"
#include "SessionManager.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4VProcess.hh"
#include "G4ProcessType.hh"
#include "G4SystemOfUnits.hh"
#include "G4VUserTrackInformation.hh"

#include <iostream>
#include <iomanip>
#include <vector>
#include <QDebug>
SteppingAction::SteppingAction(){}

SteppingAction::~SteppingAction(){}

void SteppingAction::UserSteppingAction(const G4Step *step)
{
    SessionManager & SM = SessionManager::getInstance();

    if (SM.Settings.RunSet.SaveSettings.Enabled)
    {
        const G4VProcess * proc = step->GetPostStepPoint()->GetProcessDefinedStep();
        if (proc && proc->GetProcessType() == fTransportation)
        {
            G4LogicalVolume * volFrom = step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume();
            if (volFrom == SM.ExitVolume)
            {
                const G4StepPoint * postP  = step->GetPostStepPoint();
                const double time = postP->GetGlobalTime()/ns;
                if ( !SM.Settings.RunSet.SaveSettings.TimeWindow ||
                     (time > SM.Settings.RunSet.SaveSettings.TimeFrom && time < SM.Settings.RunSet.SaveSettings.TimeTo) )
                {
                    double buf[6];
                    const G4ThreeVector & pos = postP->GetPosition();
                    buf[0] = pos[0]/mm;
                    buf[1] = pos[1]/mm;
                    buf[2] = pos[2]/mm;
                    const G4ThreeVector & dir = postP->GetMomentumDirection();
                    buf[3] = dir[0];
                    buf[4] = dir[1];
                    buf[5] = dir[2];

                    SM.saveParticle(step->GetTrack()->GetParticleDefinition()->GetParticleName(),
                                    postP->GetKineticEnergy()/keV,
                                    time,
                                    buf);

                    if (SM.Settings.RunSet.SaveSettings.StopTrack)
                        step->GetTrack()->SetTrackStatus(fStopAndKill);

                    if (SM.Settings.RunSet.SaveTrackingHistory)
                    {
                        const double kinE = step->GetPostStepPoint()->GetKineticEnergy()/keV;
                        const double depoE = step->GetTotalEnergyDeposit()/keV;
                        SM.saveTrackRecord("ExitStop",
                                           pos, time,
                                           kinE, depoE);
                    }
                }
            }
        }
    }

    if (SM.bMonitorsRequireSteppingAction)
        if (!step->GetTrack()->GetUserInformation()) //if exists, already marked as "indirect"
        {
            const G4VProcess * proc = step->GetPostStepPoint()->GetProcessDefinedStep();
            if (proc && proc->GetProcessType() != fTransportation) // on first non-transportation, the particle is marked as "indirect"
                step->GetTrack()->SetUserInformation(new G4VUserTrackInformation()); // owned by track!
        }

    if (!SM.Settings.RunSet.SaveTrackingHistory) return; // the rest is only to record telemetry!

    if (SM.bStoppedOnMonitor) // bug fix for Geant4 - have to be removed when it is fixed! Currently track has one more step after kill
    {
        SM.bStoppedOnMonitor = false;
        return;
    }

    const G4VProcess * proc = step->GetPostStepPoint()->GetProcessDefinedStep();

    bool bTransport = false;
    std::string procName;
    if (proc)
    {
        if (proc->GetProcessType() == fTransportation)
        {
            if (step->GetPostStepPoint()->GetStepStatus() != fWorldBoundary)
            {
                procName = 'T';
                bTransport = true;
            }
            else procName = 'O';
        }
        else procName = proc->GetProcessName();
    }
    else procName = '?';

    const G4ThreeVector & pos = step->GetPostStepPoint()->GetPosition();
    const double time = step->GetPostStepPoint()->GetGlobalTime()/ns;
    const double kinE = step->GetPostStepPoint()->GetKineticEnergy()/keV;
    const double depo = step->GetTotalEnergyDeposit()/keV;

    const std::vector<int> * secondaries = nullptr;
    const int numSec = step->GetNumberOfSecondariesInCurrentStep();
    if (numSec > 0)
    {
        TmpSecondaries.resize(numSec);
        for (int iSec = 0; iSec < numSec; iSec++)
        {
            TmpSecondaries[iSec] = SM.getPredictedTrackID();
            SM.incrementPredictedTrackID();
        }
        secondaries = &TmpSecondaries;
    }

    if (bTransport)
    {
        const int iMat = SM.findMaterial( step->GetPostStepPoint()->GetMaterial()->GetName() ); //will terminate session if not found!
        const std::string & VolNameTo = step->GetPostStepPoint()->GetPhysicalVolume()->GetLogicalVolume()->GetName();
        const int VolIndexTo = step->GetPostStepPoint()->GetPhysicalVolume()->GetCopyNo();

        SM.saveTrackRecord(procName, pos, time, kinE, depo, secondaries, iMat, VolNameTo, VolIndexTo);
    }
    else
        SM.saveTrackRecord(procName, pos, time, kinE, depo, secondaries);
}
