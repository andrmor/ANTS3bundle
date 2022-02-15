#include "atrack_si.h"
#include "aeventtrackingrecord.h"
#include "atrackingdataimporter.h"

#include <QDebug>

#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"

ATrack_SI::ATrack_SI() :
    AScriptInterface()
{
    Help["getTrackRecord"] = "returns [string_ParticleName, bool_IsSecondary, int_NumberOfSecondaries, int_NumberOfTrackingSteps]";
    Help["getStepRecord"] = "returns [ [X,Y,Z], Time, [MatIndex, VolumeName, VolumeIndex], Energy, DepositedEnergy, ProcessName, indexesOfSecondaries[] ]\n"
                            "XYZ in mm, Time in ns, Energies in keV, [MatVolIndex] array is empty if node does not exist, indexesOfSec is array with ints";
}

void ATrack_SI::configure(QString fileName, bool binary)
{
    FileName = fileName;
    bBinaryFile = binary;
}

void ATrack_SI::setEvent(int iEvent)
{
    ATrackingDataImporter TDI(FileName, bBinaryFile);
    bool ok = TDI.extractEvent(iEvent, EventRecord);
}

int ATrack_SI::countPrimaries(int iEvent)
{
    if (iEvent < 0 || iEvent >= (int)TH.size())
    {
        abort("Bad event number");
        return 0;
    }
    return TH.at(iEvent)->countPrimaries();
}

QString ATrack_SI::recordToString(int iEvent, int iPrimary, bool includeSecondaries)
{
    if (iEvent < 0 || iEvent >= (int)TH.size())
    {
        abort("Bad event number");
        return "";
    }
    if (iPrimary < 0 || iPrimary >= TH.at(iEvent)->countPrimaries())
    {
        abort(QString("Bad primary number (%1) for event #%2").arg(iPrimary).arg(iEvent));
        return "";
    }
    QString s;
    TH.at(iEvent)->getPrimaryParticleRecords().at(iPrimary)->logToString(s, 0, includeSecondaries);
    return s;
}

void ATrack_SI::set(int iEvent, int iPrimary)
{
    if (iEvent < 0 || iEvent >= (int)TH.size())
    {
        abort("Bad event number");
        return;
    }
    if (iPrimary < 0 || iPrimary >= TH.at(iEvent)->countPrimaries())
    {
        abort(QString("Bad primary number (%1) for event #%2").arg(iPrimary).arg(iEvent));
        return;
    }
    Rec = TH[iEvent]->getPrimaryParticleRecords().at(iPrimary);
    CurrentStep = -1;
}

QVariantList ATrack_SI::getTrackRecord()
{
    QVariantList vl;
    if (Rec)
    {
        vl << Rec->ParticleName
           << (bool)Rec->getSecondaryOf()
           << (int)Rec->getSecondaries().size()
           << (int)Rec->getSteps().size();
    }
    else abort("Record not set: use cd_set command");

    return vl;
}

QString ATrack_SI::getProductionProcess()
{
    if (Rec)
    {
        const AParticleTrackingRecord * parent = Rec->getSecondaryOf();
        if (!parent) return "";

        const std::vector<AParticleTrackingRecord *> & vecSec = parent->getSecondaries();
        const int size = (int)vecSec.size();
        int index = 0;
        for (;index < size; index++)
            if (vecSec[index] == Rec) break;
        if (index == size)
        {
            abort("Corruption in secondary record detected: this record not found in parent record");
            return "";
        }

        const std::vector<ATrackingStepData *> & vecSteps = parent->getSteps();
        const int numSteps = (int)vecSteps.size();
        for (int iS = numSteps - 1; iS > -1; iS--)
        {
            const std::vector<int> & thisStepSecs = vecSteps.at(iS)->Secondaries;
            for (const int & iSec : thisStepSecs)
                if (iSec == index)
                    return vecSteps.at(iS)->Process;
        }
        abort("Corruption in secondary record detected: secondary not found in the parent record");
    }
    else abort("Record not set: use set command");
    return "";
}

bool ATrack_SI::step()
{
    if (Rec)
    {
        if (CurrentStep < (int)Rec->getSteps().size() - 1)
        {
            CurrentStep++;
            return true;
        }
        else return false;
    }

    abort("Record not set: use cd_set command");
    return false;
}

bool ATrack_SI::step(int iStep)
{
    if (Rec)
    {
        if (iStep<0 || iStep>=(int)Rec->getSteps().size())
            abort("bad step number");
        else
        {
            CurrentStep = iStep;
            return true;
        }
    }
    else
        abort("Record not set: use cd_set command");

    return false;
}

bool ATrack_SI::stepToProcess(QString processName)
{
    if (Rec)
    {
        while ( CurrentStep < (int)Rec->getSteps().size() - 1)
        {
            CurrentStep++;
            if (Rec->getSteps().at(CurrentStep)->Process == processName) return true;
        }
    }
    else abort("Record not set: use cd_set command");

    return false;
}

int ATrack_SI::getCurrentStep()
{
    if (Rec) return CurrentStep;

    abort("Record not set: use cd_set command");
    return 0;
}

void ATrack_SI::firstStep()
{
    if (Rec)
    {
        if (Rec->getSteps().empty())
        {
            abort("Error: container with steps is empty!");
            return;
        }
        CurrentStep = 0;
    }
    else abort("Record not set: use cd_set command");
}

bool ATrack_SI::firstNonTransportationStep()
{
    if (Rec)
    {
        if (Rec->getSteps().empty())
        {
            abort("Error: container with steps is empty!");
            return false;
        }
        CurrentStep = 0;

        const int last = (int)Rec->getSteps().size() - 1;
        while (CurrentStep < last)
        {
            CurrentStep++;
            if (CurrentStep == last) return false;
            if (Rec->getSteps().at(CurrentStep)->Process != "T") return true;
        }
    }
    else abort("Record not set: use cd_set command");
    return false;
}

void ATrack_SI::lastStep()
{
    if (Rec)
    {
        if (Rec->getSteps().empty())
        {
            abort("Error: container with steps is empty!");
            return;
        }
        CurrentStep = (int)Rec->getSteps().size() - 1;
    }
    else abort("Record not set: use cd_set command");
}

QVariantList ATrack_SI::getStepRecord()
{
    QVariantList vl;

    if (Rec)
    {
        if (CurrentStep < 0) CurrentStep = 0; //forced first step
        if (CurrentStep < (int)Rec->getSteps().size())
        {
            ATrackingStepData * s = Rec->getSteps().at(CurrentStep);
            vl.push_back( QVariantList() << s->Position[0] << s->Position[1] << s->Position[2] );
            vl << s->Time;
            QVariantList vnode;
            for (int iStep = CurrentStep; iStep > -2; iStep--)
            {
                if (iStep < 0)
                {
                    abort("Corrupted tracking history!");
                    return vl;
                }
                ATransportationStepData * trans = dynamic_cast<ATransportationStepData*>(Rec->getSteps().at(iStep));
                if (!trans) continue;

                vnode << trans->iMaterial;
                vnode << trans->VolName;
                vnode << trans->VolIndex;
                break;
            }
            vl.push_back(vnode);
            vl << s->Energy;
            vl << s->DepositedEnergy;
            vl << s->Process;
            QVariantList svl;
            for (int & iSec : s->Secondaries) svl << iSec;
            vl.push_back(svl);
        }
        else abort("Error: bad current step!");
    }
    else abort("Record not set: use cd_set command");

    return vl;
}

bool normVector(double * arr)
{
    double Norm = 0;
    for (int i=0 ;i<3; i++) Norm += arr[i] * arr[i];
    Norm = sqrt(Norm);
    if (Norm != 0)
    {
        for (int i=0; i<3; i++) arr[i] = arr[i]/Norm;
        return true;
    }
    return false;
}

QVariantList ATrack_SI::getDirections()
{
    QVariantList vl;
    if (Rec)
    {
        if (CurrentStep < 0) CurrentStep = 0; //forced first step

        QVariantList inDir;
        QVariantList outDir;
        double delta[3];
        const ATrackingStepData * thisStep = Rec->getSteps().at(CurrentStep);
        if (CurrentStep != 0)
        {
            const ATrackingStepData * lastStep = Rec->getSteps().at(CurrentStep-1);
            for (int i=0; i<3; i++) delta[i] = thisStep->Position[i] - lastStep->Position[i];
            bool ok = normVector(delta);
            if (ok) inDir << delta[0] << delta[1] << delta[2];
        }
        if (CurrentStep != (int)Rec->getSteps().size())
        {
            const ATrackingStepData * nextStep = Rec->getSteps().at(CurrentStep+1);
            for (int i=0; i<3; i++) delta[i] = nextStep->Position[i] - thisStep->Position[i];
            bool ok = normVector(delta);
            if (ok) outDir << delta[0] << delta[1] << delta[2];
        }
        vl.push_back(inDir);
        vl.push_back(outDir);
    }
    else abort("Record not set: use set command");

    return vl;
}

int ATrack_SI::countSteps()
{
    if (Rec) return Rec->getSteps().size();
    else     abort("Record not set: use cd_set command");
    return 0;
}

int ATrack_SI::countSecondaries()
{
    if (Rec)
    {
        return Rec->getSecondaries().size();
    }
    else
    {
        abort("Record not set: use cd_set command");
        return 0;
    }
}

bool ATrack_SI::hadPriorInteraction()
{
    if (Rec)
    {
        while ( CurrentStep > 2 && CurrentStep < (int)Rec->getSteps().size() ) // note Step-- below: no need to test Step=0 and the last step
        {
            CurrentStep--;
            const QString & Proc = Rec->getSteps().at(CurrentStep)->Process;
            if (Proc != "T") return true;
        }
    }
    else abort("Record not set: use cd_set command");
    return false;
}

void ATrack_SI::in(int indexOfSecondary)
{
    if (Rec)
    {
        if (indexOfSecondary > -1 && indexOfSecondary < (int)Rec->getSecondaries().size())
        {
            Rec = Rec->getSecondaries().at(indexOfSecondary);
            CurrentStep = 0;
        }
        else abort("bad index of secondary");
    }
    else abort("Record not set: use cd_set command");
}

bool ATrack_SI::out()
{
    if (Rec)
    {
        if (Rec->getSecondaryOf() == nullptr) return false;
        Rec = Rec->getSecondaryOf();
        CurrentStep = 0;
        return true;
    }
    abort("Record not set: use cd_set command");
    return false;
}
