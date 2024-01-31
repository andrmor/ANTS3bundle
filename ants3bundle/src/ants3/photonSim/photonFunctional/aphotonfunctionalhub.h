#ifndef APHOTONFUNCTIONALHUB_H
#define APHOTONFUNCTIONALHUB_H

#include <QString>

#include <vector>

class APhotonFunctionalModel;
class QJsonObject;

class APhotonFunctionalRecord
{
public:
    int Index = 0;
    int LinkedTo = 0;
    APhotonFunctionalModel * Model = nullptr;

    void writeToJson(QJsonObject & json) const;
    void readFromJson(const QJsonObject & json);
};

class ATunnelRuntimeData
{
public:
    bool   isTrigger = false;
    size_t LinkedIndex;
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

    bool isValidRecord(const APhotonFunctionalRecord & rec, QString & error) const;

    APhotonFunctionalModel * findModel(int trigger, int target); // returns nullptr if not found
    QString addOrModifyRecord(int trigger, int target, APhotonFunctionalModel * model);
    void removeRecord(int trigger, int target);

    QString checkRecordsReadyForRun(); // returns error string
    bool updateRuntimeProperties(); // !!!***  // generate AErrorHub error if there are problems

    //std::vector<ATunnelModelBase*> Models;
    std::vector<APhotonFunctionalRecord> FunctionalRecords;

    std::vector<ATunnelRuntimeData> RuntimeData;
};

#endif // APHOTONFUNCTIONALHUB_H
