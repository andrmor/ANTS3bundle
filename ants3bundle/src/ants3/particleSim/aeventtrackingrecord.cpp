#include "aeventtrackingrecord.h"

#include <QDebug>

#include <cmath>

// ============= Step ==============

ATrackingStepData::ATrackingStepData(float *position, float time, float energy, float depositedEnergy, const QString & process) :
    Time(time), Energy(energy), DepositedEnergy(depositedEnergy), Process(process)
{
    for (int i=0; i<3; i++) Position[i] = position[i];
}

ATrackingStepData::ATrackingStepData(const double *position, double time, double energy, double depositedEnergy, const QString &process) :
    Time(time), Energy(energy), DepositedEnergy(depositedEnergy), Process(process)
{
    for (int i=0; i<3; i++) Position[i] = position[i];
}

ATrackingStepData::ATrackingStepData(float x, float y, float z, float time, float energy, float depositedEnergy, const QString &process) :
    Time(time), Energy(energy), DepositedEnergy(depositedEnergy), Process(process)
{
    Position[0] = x;
    Position[1] = y;
    Position[2] = z;
}

ATrackingStepData::~ATrackingStepData(){}

void ATrackingStepData::extractTargetIsotope()
{
    if (Process.endsWith('#'))
    {
        Process.chop(1);
        const int pos = Process.indexOf('#');

        TargetIsotope = Process.mid(pos + 1);
        Process       = Process.left(pos);
    }
    else TargetIsotope.clear();
}

void ATrackingStepData::logToString(QString & str, int offset) const
{
    str += QString(' ').repeated(offset);
    str += QString("%2 at [%4, %5, %6]mm t=%7ns depo=%3keV E=%1keV").arg(Energy).arg(Process).arg(DepositedEnergy).arg(Position[0]).arg(Position[1]).arg(Position[2]).arg(Time);
    if (Secondaries.size() > 0)
        str += QString("  #sec:%1").arg(Secondaries.size());
    str += '\n';
}

ATransportationStepData::ATransportationStepData(float x, float y, float z, float time, float energy, float depositedEnergy, const QString &process) :
    ATrackingStepData(x,y,z, time, energy, depositedEnergy, process) {}

ATransportationStepData::ATransportationStepData(const double *position, double time, double energy, double depositedEnergy, const QString &process) :
    ATrackingStepData(position, time, energy, depositedEnergy, process) {}

void ATransportationStepData::setVolumeInfo(const QString & volName, int volIndex, int matIndex)
{
    VolName  = volName;
    VolIndex = volIndex;
    iMaterial = matIndex;
}

void ATransportationStepData::logToString(QString &str, int offset) const
{
    if (Process == "C")
    {
        str += QString(' ').repeated(offset);
        str += QString("C at %1 %2 (mat %3)").arg(VolName).arg(VolIndex).arg(iMaterial);
        str += QString(" [%1, %2, %3]mm t=%4ns E=%5keV").arg(Position[0]).arg(Position[1]).arg(Position[2]).arg(Time).arg(Energy);
        str += '\n';
    }
    else if (Process == "T")
    {
        str += QString(' ').repeated(offset);
        str += QString("T to %1 %2 (mat %3)").arg(VolName).arg(VolIndex).arg(iMaterial);
        str += QString(" [%1, %2, %3]mm t=%4ns depo=%5keV E=%6keV").arg(Position[0]).arg(Position[1]).arg(Position[2]).arg(Time).arg(DepositedEnergy).arg(Energy);
        if (Secondaries.size() > 0)  str += QString("  #sec:%1").arg(Secondaries.size());
        str += '\n';
    }
    else ATrackingStepData::logToString(str, offset);
}

// ============= Track ==============

AParticleTrackingRecord::~AParticleTrackingRecord()
{
    for (ATrackingStepData * step : Steps) delete step;
    Steps.clear();

    for (AParticleTrackingRecord * sec  : Secondaries) delete sec;
    Secondaries.clear();
}

AParticleTrackingRecord *AParticleTrackingRecord::create(const QString & Particle)
{
    return new AParticleTrackingRecord(Particle);
}

AParticleTrackingRecord *AParticleTrackingRecord::create()
{
    return new AParticleTrackingRecord("undefined");
}

void AParticleTrackingRecord::updatePromisedSecondary(const QString & particle, float startEnergy, float startX, float startY, float startZ, float startTime, const QString& volName, int volIndex, int matIndex)
{
    ParticleName = particle;
    ATransportationStepData * st = new ATransportationStepData(startX, startY, startZ, startTime, startEnergy, 0, "C");
    st->setVolumeInfo(volName, volIndex, matIndex);
    Steps.push_back(st);
}

void AParticleTrackingRecord::addStep(ATrackingStepData * step)
{
    Steps.push_back( step );
}

void AParticleTrackingRecord::addSecondary(AParticleTrackingRecord *sec)
{
    sec->SecondaryOf = this;
    Secondaries.push_back(sec);
}

int AParticleTrackingRecord::countSecondaries() const
{
    return static_cast<int>(Secondaries.size());
}

bool AParticleTrackingRecord::isHaveProcesses(const QStringList & Proc, bool bOnlyPrimary)
{
    for (ATrackingStepData * s : Steps)
        for (const QString & p : Proc)
            if (p == s->Process) return true;

    if (!bOnlyPrimary)
    {
        for (AParticleTrackingRecord * sec : Secondaries)
            if (sec->isHaveProcesses(Proc, bOnlyPrimary))
                return true;
    }

    return false;
}

bool AParticleTrackingRecord::isTouchedVolumes(const QStringList & Vols, const QStringList & VolsStartsWith) const
{
    for (ATrackingStepData * s : Steps)
    {
        ATransportationStepData * tr = dynamic_cast<ATransportationStepData*>(s);
        if (tr)
        {
            const QString & volName = tr->VolName;

            if (Vols.contains(volName)) return true;
            for (const QString & s : VolsStartsWith)
                if (volName.startsWith(s)) return true;
        }
    }

    for (AParticleTrackingRecord * sec : Secondaries)
        if (sec->isTouchedVolumes(Vols, VolsStartsWith))
            return true;

    return false;
}

bool AParticleTrackingRecord::isContainParticle(const QStringList & PartNames) const
{
    for (const QString & name : PartNames)
    {
        if (name.endsWith('*'))
        {
            QString wild = name;
            wild.chop(1);
            if (ParticleName.startsWith(wild)) return true;
        }
        else if (ParticleName == name) return true;
    }

    for (AParticleTrackingRecord * sec : Secondaries)
        if (sec->isContainParticle(PartNames)) return true;

    return false;
}

bool AParticleTrackingRecord::isNoInteractions() const
{
    for (const ATrackingStepData * s : Steps)
    {
        if (s->Process == "C") continue;
        if (s->Process == "T") continue;
        if (s->Process == "O") return true;
        return false;
    }
    return true;
}

void AParticleTrackingRecord::logToString(QString & str, int offset, bool bExpandSecondaries) const
{
    str += QString(' ').repeated(offset) + '>';
    //str += (ParticleId > -1 && ParticleId < ParticleNames.size() ? ParticleNames.at(ParticleId) : "unknown");
    str += ParticleName + "\n";

    for (auto* st : Steps)
    {
        st->logToString(str, offset);
        if (bExpandSecondaries)
        {
            for (int & iSec : st->Secondaries)
                Secondaries.at(iSec)->logToString(str, offset + 4, bExpandSecondaries);
        }
    }
}

void AParticleTrackingRecord::fillDepositionData(std::vector<std::pair<double,double>> & data) const
{
    data.clear();
    if (Steps.size() < 2) return;

    const float * start = Steps[0]->Position;
    for (const ATrackingStepData * ts : Steps)
    {
        double delta = 0;
        for (int i = 0; i < 3; i++)
        {
            const double dd = ts->Position[i] - start[i];
            delta += dd * dd;
        }
        data.push_back( {sqrt(delta), ts->DepositedEnergy} );
    }
}

// ============= Event ==============

AEventTrackingRecord * AEventTrackingRecord::create()
{
    return new AEventTrackingRecord();
}

AEventTrackingRecord::AEventTrackingRecord(){}

AEventTrackingRecord::~AEventTrackingRecord()
{
    clear();
}

void AEventTrackingRecord::addPrimaryRecord(AParticleTrackingRecord *rec)
{
    PrimaryParticleRecords.push_back(rec);
}

void AEventTrackingRecord::clear()
{
    for (AParticleTrackingRecord * pr : PrimaryParticleRecords) delete pr;
    PrimaryParticleRecords.clear();
}

int AEventTrackingRecord::countPrimaries() const
{
    return static_cast<int>(PrimaryParticleRecords.size());
}

bool AEventTrackingRecord::isHaveProcesses(const QStringList & Proc, bool bOnlyPrimary) const
{
    for (AParticleTrackingRecord * pr : PrimaryParticleRecords)
        if (pr->isHaveProcesses(Proc, bOnlyPrimary)) return true;

    return false;
}

bool AEventTrackingRecord::isTouchedVolumes(const QStringList & Vols, const QStringList & VolsStartsWith) const
{
    for (AParticleTrackingRecord * pr : PrimaryParticleRecords)
        if (pr->isTouchedVolumes(Vols, VolsStartsWith)) return true;

    return false;
}

bool AEventTrackingRecord::isContainParticle(const QStringList & PartNames) const
{
    for (AParticleTrackingRecord * pr : PrimaryParticleRecords)
        if (pr->isContainParticle(PartNames)) return true;

    return false;
}
