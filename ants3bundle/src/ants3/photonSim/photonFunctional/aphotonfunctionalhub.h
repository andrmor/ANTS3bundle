#ifndef APHOTONFUNCTIONALHUB_H
#define APHOTONFUNCTIONALHUB_H

#include <QString>

#include <vector>

class APhotonFunctionalModel;
class QJsonObject;

class APhotonFunctionalRecord
{
public:
    int     Trigger;
    int     Target;
    QString Model;
    QString Settings; // QJsonObject ?

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class ATunnelRuntimeData
{
public:
    bool   isTrigger = false;
    size_t TargetIndex;
    APhotonFunctionalModel * Model = nullptr;
};

class APhotonFunctionalHub
{
    APhotonFunctionalHub();
    ~APhotonFunctionalHub(){}

    APhotonFunctionalHub(const APhotonFunctionalHub&)            = delete;
    APhotonFunctionalHub(APhotonFunctionalHub&&)                 = delete;
    APhotonFunctionalHub& operator=(const APhotonFunctionalHub&) = delete;
    APhotonFunctionalHub& operator=(APhotonFunctionalHub&&)      = delete;

public:
    static       APhotonFunctionalHub & getInstance();
    static const APhotonFunctionalHub & getConstInstance();

    void writeToJson(QJsonObject & json) const;
    QString readFromJson(const QJsonObject & json);

    void clearAllRecords() {FunctionalRecords.clear();}

    bool isValidRecord(const APhotonFunctionalRecord & rec, bool registerError) const;

    QString addOrModifyRecord(int trigger, int target, QString model, const QString & settings);
    void removeRecord(int trigger, int target);

    bool updateRuntimeProperties(); // !!!***  // generate AErrorHub error if there are problems

    //std::vector<ATunnelModelBase*> Models;
    std::vector<APhotonFunctionalRecord> FunctionalRecords;

    std::vector<ATunnelRuntimeData> RuntimeData;
};

#endif // APHOTONFUNCTIONALHUB_H
