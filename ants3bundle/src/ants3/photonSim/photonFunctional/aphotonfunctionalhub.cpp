#include "aphotonfunctionalhub.h"
#include "aphotonfunctionalmodel.h"
#include "ageometryhub.h"
#include "aerrorhub.h"
#include "ajsontools.h"

APhotonFunctionalHub::APhotonFunctionalHub() {}

APhotonFunctionalHub & APhotonFunctionalHub::getInstance()
{
    static APhotonFunctionalHub instance;
    return instance;
}

const APhotonFunctionalHub & APhotonFunctionalHub::getConstInstance()
{
    return getInstance();
}

void APhotonFunctionalHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;

        // Records
        {
            QJsonArray ar;
            for (const APhotonFunctionalRecord & rec : FunctionalRecords)
            {
                QJsonObject jse;
                    rec.writeToJson(jse);
                ar.push_back(jse);
            }
            js["Records"] = ar;
        }

    json["PhotonFunctional"] = js;
}

QString APhotonFunctionalHub::readFromJson(const QJsonObject & json)
{
    clearAllRecords();

    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonFunctional", js);
    if (!ok) return ""; // cannot enforce, some old configs do not have this field

    // Records
    {
        QJsonArray ar;
        jstools::parseJson(js, "Records", ar);
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonObject jse = ar[i].toObject();
            APhotonFunctionalRecord rec;
                rec.readFromJson(jse);
            FunctionalRecords.push_back(rec);
        }
    }

    return "";
}

bool APhotonFunctionalHub::isValidRecord(const APhotonFunctionalRecord & rec, QString & error) const
{
    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    if (!rec.Model)
    {
        error = "Empty model in photon functional record";
        return false;
    }

    if (rec.Index < 0)
    {
        error = "Found negative index";
        return false;
    }
    if (rec.Index >= GeoHub.PhotonFunctionals.size())
    {
        error = "Found out of range index: " + QString::number(rec.Index);
        return false;
    }

    if (rec.Model->isLink())
    {
        if (rec.LinkedTo < 0)
        {
            error = "Found negative linked object index";
            return false;
        }
        if (rec.LinkedTo >= GeoHub.PhotonFunctionals.size())
        {
            error = "Found out of range index of the linked object: " + QString::number(rec.LinkedTo);
            return false;
        }
        if (rec.Index == rec.LinkedTo)
        {
            error = "Linked object should not have the same index: check records with index " + QString::number(rec.Index);
            return false;
        }
    }

    if (!rec.Model->isValid())
    {
        error = "Photon functional record's model is not valid";
        return false;
    }

    return true;
}

APhotonFunctionalModel * APhotonFunctionalHub::findModel(int trigger, int target)
{
    for (APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        if (rec.Index != trigger) continue;
        if (rec.LinkedTo != target) continue;
        return rec.Model;
    }
    return nullptr;
}

QString APhotonFunctionalHub::addOrModifyRecord(int trigger, int target, APhotonFunctionalModel * model)
{
    for (APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        if (rec.LinkedTo != target) continue;

        rec.Index  = trigger;
        rec.Model    = model;
        return "";
    }

    FunctionalRecords.push_back({trigger, target, model});
    return "";
}

void APhotonFunctionalHub::removeRecord(int trigger, int target)
{
    for (auto it = FunctionalRecords.begin(); it < FunctionalRecords.end(); ++it)
    {
        if (it->Index != trigger) continue;
        if (it->LinkedTo  != target)   continue;

        FunctionalRecords.erase(it);
        return;
    }
}

QString APhotonFunctionalHub::checkRecordsReadyForRun()
{
    QString error;
    for (const APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        bool ok = isValidRecord(rec, error);
        if (!ok) return error;
    }

    QSet<int> seenIndexes;
    for (const APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        int index = rec.Index;
        if (seenIndexes.contains(index)) return QString("Multiple references to index %0").arg(index);
        seenIndexes << index;

        if (rec.Model->isLink())
        {
            int linked = rec.LinkedTo;
            if (seenIndexes.contains(linked)) return QString("Multiple references to index %0").arg(linked);
            seenIndexes << linked;
        }
    }

    if (seenIndexes.size() != AGeometryHub::getConstInstance().PhotonFunctionals.size())
        return "There are not assigned photon functionals";

    return "";
}

bool APhotonFunctionalHub::updateRuntimeProperties()
{
    RuntimeData.clear();

    QString error = checkRecordsReadyForRun();
    if (!error.isEmpty())
    {
        AErrorHub::addQError("Error during check of the records of the functional models:\n" + error);
        return false;
    }

    // updating

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    RuntimeData.resize(GeoHub.PhotonFunctionals.size());
    for (ATunnelRuntimeData & dr : RuntimeData) dr.isTrigger = false;

    for (const APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        ATunnelRuntimeData & rt = RuntimeData[rec.Index];
        rt.isTrigger = true;

        rt.Model = rec.Model;
        QString err = rt.Model->updateRuntimeProperties();
        if (!err.isEmpty())
        {
            AErrorHub::addQError(QString("Functional model for index %0 error:\n").arg(rec.Index) + err);
            return false;
        }

        if (rec.Model->isLink())
            rt.LinkedIndex = rec.LinkedTo;
        else
            rt.LinkedIndex = rec.Index;
    }
    return true;
}

void APhotonFunctionalRecord::writeToJson(QJsonObject & json) const
{
    json["Index"]    = Index;
    json["LinkedTo"] = LinkedTo;

    QJsonObject js;
    Model->writeToJson(js);
    json["Model"] = js;
}

void APhotonFunctionalRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Index",    Index);
    jstools::parseJson(json, "LinkedTo", LinkedTo);

    QJsonObject js;
    jstools::parseJson(json, "Model", js);
    Model = APhotonFunctionalModel::factory(js);
}
