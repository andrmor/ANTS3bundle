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

    void clearAllRecords();

    bool isValidRecord(const APhotonFunctionalRecord & rec, QString & error) const;

    APhotonFunctionalModel * findModel(int trigger);          // returns nullptr if not found
    APhotonFunctionalRecord * findOverritenRecord(int index); // returns nullptr if not found
    QString modifyOrAddRecord(int index, int linkedTo, APhotonFunctionalModel * model); // transfers model! make clone if need to keep it!
    void removeRecord(int index); // !!!*** delete model?

    QString checkRecordsReadyForRun(); // returns error string
    bool updateRuntimeProperties(); // !!!***  // generate AErrorHub error if there are problems

    int overrideUnconnectedLinkFunctionals();

    std::vector<APhotonFunctionalRecord> OverritenRecords;

    std::vector<ATunnelRuntimeData> RuntimeData; // does not own the models: do not delete!
};

#endif // APHOTONFUNCTIONALHUB_H
