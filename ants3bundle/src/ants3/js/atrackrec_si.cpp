#include "atrackrec_si.h"
#include "aeventtrackingrecord.h"
#include "atrackingdataimporter.h"

#include <cmath>

ATrackRec_SI::ATrackRec_SI() :
    AScriptInterface()
{
    Help["getTrackRecord"] = "returns [string_ParticleName, bool_IsSecondary, int_NumberOfSecondaries, int_NumberOfTrackingSteps]";
    Help["getStepRecord"]  = "returns [ [X,Y,Z], Time, [MatIndex, VolumeName, VolumeIndex], Energy, DepositedEnergy, ProcessName, indexesOfSecondaries[] ]\n"
                             "XYZ in mm, Time in ns, Energies in keV, [MatVolIndex] array is empty if node does not exist, indexesOfSec is array with ints";
}

ATrackRec_SI::~ATrackRec_SI()
{
    clearData();
}

void ATrackRec_SI::clearData()
{
    delete EventRecord; EventRecord = nullptr;
    ParticleRecord = nullptr;
    CurrentEvent = 0;
    CurrentStep = 0;
}

void ATrackRec_SI::configure(QString fileName, bool binary)
{
    FileName = fileName;
    bBinaryFile = binary;
}

int ATrackRec_SI::countEvents()
{
    if (FileName.isEmpty())
    {
        abort(RecordNotSet);
        return 0;
    }
    ATrackingDataImporter TDI(FileName, bBinaryFile);
    if (!TDI.ErrorString.isEmpty())
    {
        abort("Error accessing the tracking history file:\n" + TDI.ErrorString);
        return 0;
    }
    return TDI.countEvents();
}

void ATrackRec_SI::setEvent(int iEvent)
{
    if (FileName.isEmpty())
    {
        abort(RecordNotSet);
        return;
    }

    clearData();

    EventRecord = AEventTrackingRecord::create();

    ATrackingDataImporter TDI(FileName, bBinaryFile);
    bool ok = TDI.extractEvent(iEvent, EventRecord);
    if (!ok) abort(TDI.ErrorString);

    CurrentEvent = iEvent;

    if (countPrimaries() > 0) setPrimary(0);
}

int ATrackRec_SI::countPrimaries()
{
    return EventRecord->countPrimaries();
}

void ATrackRec_SI::setPrimary(int iPrimary)
{
    if (iPrimary < 0 || iPrimary >= EventRecord->countPrimaries())
    {
        abort(QString("Bad primary # (%1) for event # %2").arg(iPrimary).arg(CurrentEvent));
        return;
    }
    ParticleRecord = EventRecord->getPrimaryParticleRecords().at(iPrimary);
    CurrentStep = -1;
}

QString ATrackRec_SI::recordToString(bool includeSecondaries)
{
    QString s;
    if (ParticleRecord)
        ParticleRecord->logToString(s, 0, includeSecondaries);
    else abort(RecordNotSet);
    return s;
}

QVariantList ATrackRec_SI::getTrackRecord()
{
    QVariantList vl;

    if (ParticleRecord)
    {
        vl << ParticleRecord->ParticleName
           << (bool)ParticleRecord->getSecondaryOf()
           << (int)ParticleRecord->getSecondaries().size()
           << (int)ParticleRecord->getSteps().size();
    }
    else abort(RecordNotSet);

    return vl;
}

QString ATrackRec_SI::getProductionProcess()
{
    if (ParticleRecord)
    {
        const AParticleTrackingRecord * parent = ParticleRecord->getSecondaryOf();
        if (!parent) return "";

        const std::vector<AParticleTrackingRecord *> & vecSec = parent->getSecondaries();
        const int size = (int)vecSec.size();
        int index = 0;
        for (;index < size; index++)
            if (vecSec[index] == ParticleRecord) break;
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
    else abort(RecordNotSet);
    return "";
}

int ATrackRec_SI::countSteps()
{
    if (ParticleRecord) return ParticleRecord->getSteps().size();

    abort(RecordNotSet);
    return 0;
}

int ATrackRec_SI::countSecondaries()
{
    if (ParticleRecord) return ParticleRecord->getSecondaries().size();

    abort(RecordNotSet);
    return 0;
}

void ATrackRec_SI::firstStep()
{
    if (ParticleRecord)
    {
        if (ParticleRecord->getSteps().empty())
        {
            abort("Error: container with step records is empty!");
            return;
        }
        CurrentStep = 0;
    }
    else abort(RecordNotSet);
}

void ATrackRec_SI::lastStep()
{
    if (ParticleRecord)
    {
        if (ParticleRecord->getSteps().empty())
        {
            abort("Error: container with steps is empty!");
            return;
        }
        CurrentStep = (int)ParticleRecord->getSteps().size() - 1;
    }
    else abort(RecordNotSet);
}

bool ATrackRec_SI::makeStep()
{
    if (ParticleRecord)
    {
        if (CurrentStep < (int)ParticleRecord->getSteps().size() - 1)
        {
            CurrentStep++;
            return true;
        }
        else return false;
    }

    abort(RecordNotSet);
    return false;
}

void ATrackRec_SI::gotoStep(int iStep)
{
    if (ParticleRecord)
    {
        if (iStep < 0 || iStep >= (int)ParticleRecord->getSteps().size()) abort("Bad step number");
        else CurrentStep = iStep;
    }
    else abort(RecordNotSet);
}

bool ATrackRec_SI::gotoNextProcessStep(QString processName)
{
    if (ParticleRecord)
    {
        while ( CurrentStep < (int)ParticleRecord->getSteps().size() - 1)
        {
            CurrentStep++;
            if (ParticleRecord->getSteps().at(CurrentStep)->Process == processName) return true;
        }
    }
    else abort(RecordNotSet);

    return false;
}

bool ATrackRec_SI::gotoNextNonTransportationStep()
{
    if (ParticleRecord)
    {
        while ( makeStep() )
        {
            const QString & pr = ParticleRecord->getSteps().at(CurrentStep)->Process;
            if (pr == "T" || pr == "C" || pr == "O") continue;
            return true;
        }
        return false;
    }
    else abort(RecordNotSet);
    return false;
}

int ATrackRec_SI::getCurrentStep()
{
    if (ParticleRecord) return CurrentStep;

    abort(RecordNotSet);
    return 0;
}

QVariantList ATrackRec_SI::getStepRecord()
{
    QVariantList vl;

    if (ParticleRecord)
    {
        if (CurrentStep < (int)ParticleRecord->getSteps().size())
        {
            ATrackingStepData * s = ParticleRecord->getSteps().at(CurrentStep);
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
                ATransportationStepData * trans = dynamic_cast<ATransportationStepData*>(ParticleRecord->getSteps().at(iStep));
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
    else abort(RecordNotSet);

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

QVariantList ATrackRec_SI::getDirectionApproximate()
{
    QVariantList vl;

    if (ParticleRecord)
    {
        if (CurrentStep < 0) CurrentStep = 0; //forced first step if not initialized

        QVariantList inDir;
        QVariantList outDir;
        double delta[3];
        const ATrackingStepData * thisStep = ParticleRecord->getSteps().at(CurrentStep);
        if (CurrentStep != 0)
        {
            const ATrackingStepData * lastStep = ParticleRecord->getSteps().at(CurrentStep-1);
            for (int i=0; i<3; i++) delta[i] = thisStep->Position[i] - lastStep->Position[i];
            bool ok = normVector(delta);
            if (ok) inDir << delta[0] << delta[1] << delta[2];
        }
        if (CurrentStep != (int)ParticleRecord->getSteps().size())
        {
            const ATrackingStepData * nextStep = ParticleRecord->getSteps().at(CurrentStep+1);
            for (int i=0; i<3; i++) delta[i] = nextStep->Position[i] - thisStep->Position[i];
            bool ok = normVector(delta);
            if (ok) outDir << delta[0] << delta[1] << delta[2];
        }
        vl.push_back(inDir);
        vl.push_back(outDir);
    }
    else abort(RecordNotSet);

    return vl;
}

bool ATrackRec_SI::hadPriorInteraction()
{
    if (ParticleRecord)
    {
        int iStep = CurrentStep;
        while (iStep > 2) // note iStep-- below: no need to test Step=0 and the last step
        {
            iStep--;
            const QString & Proc = ParticleRecord->getSteps().at(iStep)->Process;
            if (Proc != "T") return true;
        }
        return false;
    }
    else abort(RecordNotSet);
    return false;
}

void ATrackRec_SI::enterSecondary(int indexOfSecondary)
{
    if (ParticleRecord)
    {
        if (indexOfSecondary > -1 && indexOfSecondary < (int)ParticleRecord->getSecondaries().size())
        {
            ParticleRecord = ParticleRecord->getSecondaries().at(indexOfSecondary);
            ReturnStepIndex.push_back(CurrentStep);
            CurrentStep = 0;
        }
        else abort("Bad index of secondary");
    }
    else abort(RecordNotSet);
}

bool ATrackRec_SI::exitSecondary()
{
    if (ParticleRecord)
    {
        if (ParticleRecord->getSecondaryOf() == nullptr) return false;
        ParticleRecord = ParticleRecord->getSecondaryOf();

        if (!ReturnStepIndex.empty())
        {
            CurrentStep = ReturnStepIndex.back();
            ReturnStepIndex.pop_back();
        }
        else CurrentStep = 0;

        return true;
    }
    abort(RecordNotSet);
    return false;
}
