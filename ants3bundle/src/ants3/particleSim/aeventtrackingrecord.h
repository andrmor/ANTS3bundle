#ifndef AEVENTTRACKINGRECORD_H
#define AEVENTTRACKINGRECORD_H

#include <QString>
#include <QStringList>
#include <vector>

class ATrackingStepData
{
public:
    ATrackingStepData(const double * position, double time, double energy, double depositedEnergy, const QString & process);
    ATrackingStepData(double x, double y, double z, double time, double energy, double depositedEnergy, const QString & process);

    virtual ~ATrackingStepData();

    void extractTargetIsotope();

    virtual void logToString(QString & str, int offset) const; // !!!*** obsolete?

public:
    double  Position[3];
    double  Time;
    double  Energy;
    double  DepositedEnergy;
    QString Process;              //step defining process
    QString TargetIsotope;        //defined only for hadronic procresses, otherwise empty
    std::vector<int> Secondaries; //secondaries created in this step - indexes in the parent record
};

// TODO consider adding "normal step" or even "hadronic step" to save memory? !!!***

class ATransportationStepData : public ATrackingStepData
{
public:
    ATransportationStepData(double x, double y, double z, double time, double energy, double depositedEnergy, const QString & process);
    ATransportationStepData(const double * position, double time, double energy, double depositedEnergy, const QString & process);

    void setVolumeInfo(const QString & volName, int volIndex, int matIndex);

public:
    // for "T" step it is for the next volume, for "C" step it is for the current
    QString VolName;
    int VolIndex;
    int iMaterial;

    virtual void logToString(QString & str, int offset) const override;
};

class AParticleTrackingRecord
{
public:
    static AParticleTrackingRecord* create(const QString & Particle);
    static AParticleTrackingRecord* create(); // try to avoid this

    void updatePromisedSecondary(const QString & particle, double startEnergy, double startX, double startY, double startZ, double startTime, const QString& volName, int volIndex, int matIndex);
    void addStep(ATrackingStepData * step);

    void addSecondary(AParticleTrackingRecord * sec);
    int  countSecondaries() const;

    bool isPrimary() const {return !SecondaryOf;}
    bool isSecondary() const {return (bool)SecondaryOf;}
    const std::vector<ATrackingStepData *> & getSteps() const {return Steps;}
    const AParticleTrackingRecord * getSecondaryOf() const {return SecondaryOf;}
    const std::vector<AParticleTrackingRecord *> & getSecondaries() const {return Secondaries;}

    bool isHaveProcesses(const QStringList & Proc, bool bOnlyPrimary);
    bool isTouchedVolumes(const QStringList & Vols, const QStringList &VolsStartsWith) const;
    bool isContainParticle(const QStringList & PartNames) const;

    bool isNoInteractions() const;

    void logToString(QString & str, int offset, bool bExpandSecondaries) const;

    void fillDepositionData(std::vector<std::pair<double, double>> & data) const;

    virtual ~AParticleTrackingRecord();

    // prevent creation on the stack and copy/move
private:
    AParticleTrackingRecord(const QString & particle) : ParticleName(particle) {}

    AParticleTrackingRecord(const AParticleTrackingRecord &) = delete;
    AParticleTrackingRecord & operator=(const AParticleTrackingRecord &) = delete;
    AParticleTrackingRecord(AParticleTrackingRecord &&) = delete;
    AParticleTrackingRecord & operator=(AParticleTrackingRecord &&) = delete;

public:
    //int     ParticleId;                       // ants ID of the particle
    QString ParticleName;

private:
    std::vector<ATrackingStepData *> Steps;   // tracking steps
    AParticleTrackingRecord * SecondaryOf = nullptr;    // 0 means primary
    std::vector<AParticleTrackingRecord *> Secondaries; // vector of secondaries

};

class AEventTrackingRecord
{
public:
    static AEventTrackingRecord* create();
    void   addPrimaryRecord(AParticleTrackingRecord * rec);

    void   clear();

    bool   isEmpty() const {return PrimaryParticleRecords.empty();}
    int    countPrimaries() const;

    bool   isHaveProcesses(const QStringList & Proc, bool bOnlyPrimary) const;
    bool   isTouchedVolumes(const QStringList & Vols, const QStringList & VolsStartsWith) const;
    bool   isContainParticle(const QStringList & PartNames) const;

    const std::vector<AParticleTrackingRecord *> & getPrimaryParticleRecords() const {return PrimaryParticleRecords;}

    virtual ~AEventTrackingRecord();

    // prevent creation on the stack and copy/move
private:
    AEventTrackingRecord();

    AEventTrackingRecord(const AEventTrackingRecord &) = delete;
    AEventTrackingRecord & operator=(const AEventTrackingRecord &) = delete;
    AEventTrackingRecord(AEventTrackingRecord &&) = delete;
    AEventTrackingRecord & operator=(AEventTrackingRecord &&) = delete;

private:
    std::vector<AParticleTrackingRecord *> PrimaryParticleRecords;

};


#endif // AEVENTTRACKINGRECORD_H
