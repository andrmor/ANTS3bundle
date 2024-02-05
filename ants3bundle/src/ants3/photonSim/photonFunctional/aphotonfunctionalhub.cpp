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

        {
            QJsonArray ar;
            for (const APhotonFunctionalRecord & rec : OverritenRecords)
            {
                QJsonObject jse;
                    rec.writeToJson(jse);
                ar.push_back(jse);
            }
            js["OverritenRecords"] = ar;
        }

    json["PhotonFunctional"] = js;
}

QString APhotonFunctionalHub::readFromJson(const QJsonObject & json)
{
    clearAllRecords();

    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonFunctional", js);
    if (!ok) return ""; // cannot enforce, some old configs do not have this field

    {
        QJsonArray ar;
        jstools::parseJson(js, "OverritenRecords", ar);
        for (int i = 0; i < ar.size(); i++)
        {
            QJsonObject jse = ar[i].toObject();
            APhotonFunctionalRecord rec;
                rec.readFromJson(jse);
            OverritenRecords.push_back(rec);
        }
    }

    return "";
}

void APhotonFunctionalHub::clearAllRecords()
{
    for (APhotonFunctionalRecord & rec : OverritenRecords)
    {
        delete rec.Model;
        rec.Model = nullptr;
    }

    OverritenRecords.clear();
}

#include "ageoobject.h"
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

    const APhotonFunctionalModel * defaultModel = std::get<0>(GeoHub.PhotonFunctionals[rec.Index])->getDefaultPhotonFunctionalModel();
    if (!defaultModel)
    {
        error = "Not found default model!";
        return false;
    }
    const QString defaultModelType = defaultModel->getType();
    if (rec.Model->getType() != defaultModelType)
    {
        error = "Not matching model type: " + rec.Model->getType() + " overrides " + defaultModelType;
        return false;
    }

    // !!!*** clone model, run updateRuntimeProperties, report error if found

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
        // if (rec.Index == rec.LinkedTo)
        // {
        //     error = "Linked object should not have the same index: check records with index " + QString::number(rec.Index);
        //     return false;
        // }
    }

    if (!rec.Model->isValid())
    {
        error = "Photon functional record's model is not valid";
        return false;
    }

    return true;
}

/*
APhotonFunctionalModel * APhotonFunctionalHub::findModel(int trigger, int target)
{
    for (APhotonFunctionalRecord & rec : OverritenRecords)
    {
        if (rec.Index != trigger) continue;
        if (rec.LinkedTo != target) continue;
        return rec.Model;
    }
    return nullptr;
}
*/

APhotonFunctionalModel * APhotonFunctionalHub::findModel(int index)
{
    if (index < 0) return nullptr;

    for (APhotonFunctionalRecord & rec : OverritenRecords)
    {
        if (rec.Index != index) continue;
        return rec.Model;
    }

    AGeometryHub & GeoHub = AGeometryHub::getInstance();
    if (index >= GeoHub.PhotonFunctionals.size()) return nullptr;

    return std::get<0>(GeoHub.PhotonFunctionals[index])->getDefaultPhotonFunctionalModel();
}

APhotonFunctionalRecord * APhotonFunctionalHub::findOverritenRecord(int index)
{
    for (APhotonFunctionalRecord & rec : OverritenRecords)
    {
        if (rec.Index != index) continue;
        return &rec;
    }
    return nullptr;
}

QString APhotonFunctionalHub::modifyOrAddRecord(int index, int linkedTo, APhotonFunctionalModel * model)
{
    for (APhotonFunctionalRecord & rec : OverritenRecords)
    {
        if (rec.Index != index) continue;

        rec.LinkedTo = linkedTo;
        rec.Model    = model;
        return "";
    }

    OverritenRecords.push_back({index, linkedTo, model});
    return "";
}

void APhotonFunctionalHub::removeRecord(int index)
{
    for (auto it = OverritenRecords.begin(); it < OverritenRecords.end(); ++it)
    {
        if (it->Index != index) continue;

        OverritenRecords.erase(it);
        return;
    }
}

QString APhotonFunctionalHub::checkRecordsReadyForRun()
{
    QString error;
    for (const APhotonFunctionalRecord & rec : OverritenRecords)
    {
        bool ok = isValidRecord(rec, error);
        if (!ok) return error;
    }

    // !!!*** check there are tunnels without link (not in overriden records)

    QSet<int> seenIndexes;
    QSet<int> seenLinks;
    for (const APhotonFunctionalRecord & rec : OverritenRecords)
    {
        int index = rec.Index;
        if (seenIndexes.contains(index)) return QString("Multiple references to index %0").arg(index);
        seenIndexes << index;

        if (rec.Model->isLink())
        {
            int linked = rec.LinkedTo;
            if (seenLinks.contains(linked)) return QString("Multiple references to linked index %0").arg(linked);
            seenLinks << linked;
        }
    }

    return "";
}

bool APhotonFunctionalHub::updateRuntimeProperties()
{
    RuntimeData.clear();

    QString error = checkRecordsReadyForRun();
    if (!error.isEmpty())
    {
        AErrorHub::addQError("Error in assigned functional models:\n" + error);
        return false;
    }

    // updating

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    RuntimeData.resize(GeoHub.PhotonFunctionals.size());
    for (ATunnelRuntimeData & dr : RuntimeData) dr.isTrigger = false; // obsolete?

    for (size_t iDR = 0; iDR < RuntimeData.size(); iDR++)
    {
        ATunnelRuntimeData & runTimeRec = RuntimeData[iDR];

        APhotonFunctionalModel * defaultModel = std::get<0>(GeoHub.PhotonFunctionals[iDR])->getDefaultPhotonFunctionalModel();
        if (!defaultModel)
        {
            AErrorHub::addQError("Not found default functional model");
            return false;
        }

        APhotonFunctionalRecord * rec = findOverritenRecord(iDR);
        if (rec)
        {
            runTimeRec.isTrigger = true;   // obsolete
            runTimeRec.Model = rec->Model;
            if (runTimeRec.Model->isLink())
                runTimeRec.LinkedIndex = rec->LinkedTo;
            else
                runTimeRec.LinkedIndex = iDR;
        }
        else
        {
            runTimeRec.isTrigger = true;   // obsolete
            runTimeRec.Model = defaultModel;
            if (runTimeRec.Model->isLink()) // paranoic
            {
                AErrorHub::addQError("Photon functional model requires a link, which is not defined");
                return false;
            }
            runTimeRec.LinkedIndex = iDR;
        }

        QString err = runTimeRec.Model->updateRuntimeProperties();
        if (!err.isEmpty()) // paranoic
        {
            AErrorHub::addQError(QString("Functional model for index %0 error:\n").arg(iDR) + err);
            return false;
        }
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
