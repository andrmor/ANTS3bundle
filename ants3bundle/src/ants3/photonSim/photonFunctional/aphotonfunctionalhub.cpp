#include "aphotonfunctionalhub.h"
#include "aphotonfunctionalmodel.h"
#include "ageometryhub.h"
#include "aerrorhub.h"
#include "ajsontools.h"

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

        // Models

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
    //clear models

    QJsonObject js;
    bool ok = jstools::parseJson(json, "PhotonFunctional", js);
    if (!ok) return ""; // cannot enforce, some old configs do not have this field

    // Models

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

bool APhotonFunctionalHub::isValidRecord(const APhotonFunctionalRecord & rec, bool registerError) const
{
    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    if (rec.Trigger < 0)
    {
        if (registerError) AErrorHub::addError("Bad 'From' index in photon functional record");
        return false;
    }
    if (rec.Trigger >= GeoHub.PhotonFunctionals.size())
    {
        if (registerError) AErrorHub::addError("Bad 'From' index in photon functional record");
        return false;
    }

    if (rec.Target < 0)
    {
        if (registerError) AErrorHub::addError("Bad 'To' index in photon functional record");
        return false;
    }
    if (rec.Target >= GeoHub.PhotonFunctionals.size())
    {
        if (registerError) AErrorHub::addError("Bad 'To' index in photon functional record");
        return false;
    }

    // !!!*** check model
    return true;
}

QString APhotonFunctionalHub::addOrModifyRecord(int trigger, int target, QString model, const QString & settings)
{
    for (APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        if (rec.Target != target) continue;

        // !!!*** check model

        rec.Trigger  = trigger;
        rec.Model    = model;
        rec.Settings = settings;
        return "";
    }

    FunctionalRecords.push_back({trigger, target, model, settings});
    return "";
}

void APhotonFunctionalHub::removeRecord(int trigger, int target)
{
    for (auto it = FunctionalRecords.begin(); it < FunctionalRecords.end(); ++it)
    {
        if (it->Trigger != trigger) continue;
        if (it->Target  != target)   continue;

        FunctionalRecords.erase(it);
        return;
    }
}

bool APhotonFunctionalHub::updateRuntimeProperties()
{
    RuntimeData.clear();

    for (const APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        bool ok = isValidRecord(rec, true);
        if (!ok) return false;
    }

    const AGeometryHub & GeoHub = AGeometryHub::getConstInstance();

    /*
    int numEntrances = GeoHub.PhotonFunctionals.size();
    int numExits = GeoHub.PhotonTunnelsOut.size();

    int numNotConnectedEntrances = 0;
    int numMultipleEntrances = 0;
    for (int i = 0; i < numEntrances; i++)
    {
        int seenTimes = 0;
        for (const APhotonFunctionalRecord & rec : FunctionalRecords)
            if (rec.Trigger == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedEntrances++;
        if (seenTimes > 1) numMultipleEntrances++;
    }

    int numNotConnectedExits = 0;
    int numMultipleExits = 0;
    for (int i = 0; i < numExits; i++)
    {
        int seenTimes = 0;
        for (const APhotonFunctionalRecord & rec : FunctionalRecords)
            if (rec.Target == i)
                seenTimes++;

        if (seenTimes == 0) numNotConnectedExits++;
        if (seenTimes > 1) numMultipleExits++;
    }

    if (numNotConnectedEntrances > 0)
    {
        AErrorHub::addError("There are not connected photon tunnel entrances!");
        return false;
    }

    if (numNotConnectedExits > 0)
    {
        AErrorHub::addError("There are not connected photon tunnel exits!");
        return false;
    }

    if (numMultipleExits > 0)
    {
        AErrorHub::addError("A photon tunnel leading to multiple exits was found: not yet implemented");
        return false;
    }
    */

    // updating

    RuntimeData.resize(GeoHub.PhotonFunctionals.size());
    for (ATunnelRuntimeData & dr : RuntimeData) dr.isTrigger = false;

    for (const APhotonFunctionalRecord & rec : FunctionalRecords)
    {
        ATunnelRuntimeData & rt = RuntimeData[rec.Trigger];
        rt.isTrigger = true;
        rt.TargetIndex = rec.Target;
        rt.Model = new APFM_OpticalFiber(); // !!!*** tmp
    }
    return true;
}

APhotonFunctionalHub::APhotonFunctionalHub() {}

void APhotonFunctionalRecord::writeToJson(QJsonObject & json) const
{
    json["Trigger"]  = Trigger;
    json["Target"]   = Target;
    json["Model"]    = Model;
    json["Settings"] = Settings;
}

void APhotonFunctionalRecord::readFromJson(const QJsonObject & json)
{
    jstools::parseJson(json, "Trigger",  Trigger);
    jstools::parseJson(json, "Target",   Target);
    jstools::parseJson(json, "Model",    Model);
    jstools::parseJson(json, "Settings", Settings);
}
