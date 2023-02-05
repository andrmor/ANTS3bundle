#include "ainterfacerulehub.h"
#include "ainterfacerule.h"
#include "amaterialhub.h"
#include "ajsontools.h"

AInterfaceRuleHub::AInterfaceRuleHub() :
    MatHub(AMaterialHub::getConstInstance())
{
    clearRules();
}

AInterfaceRuleHub &AInterfaceRuleHub::getInstance()
{
    static AInterfaceRuleHub instance;
    return instance;
}

const AInterfaceRuleHub &AInterfaceRuleHub::getConstInstance()
{
    return getInstance();
}

AInterfaceRule * AInterfaceRuleHub::getVolumeRule(const TString & from, const TString & to) const
{
    auto it = VolumeRules.find({from, to});
    if (it != VolumeRules.end())
        return it->second;
    else return nullptr;
}

void AInterfaceRuleHub::setVolumeRule(const TString & from, const TString & to, AInterfaceRule * rule)
{
    VolumeRules[{from, to}] = rule;
}

bool AInterfaceRuleHub::isFromVolume(const TString & name) const
{
    return (VolumesFrom.find(name) != VolumesFrom.end());
}

bool AInterfaceRuleHub::isToVolume(const TString & name) const
{
    return (VolumesTo.find(name) != VolumesTo.end());
}

void AInterfaceRuleHub::removeVolumeRule(const TString & from, const TString & to)
{
    AInterfaceRule * old = getVolumeRule(from, to);
    delete old;
    VolumeRules.erase({from, to});
}

void AInterfaceRuleHub::moveVolumeRule(const TString & oldFrom, const TString & oldTo, const TString & newFrom, const TString & newTo)
{
    AInterfaceRule * rule = getVolumeRule(oldFrom, oldTo);
    VolumeRules.erase({oldFrom, oldTo});
    setVolumeRule(newFrom, newTo, rule);
}

void AInterfaceRuleHub::updateRuntimeProperties()
{
    for (auto & rv : MaterialRules)
        for (auto & r : rv)
            if (r) r->initializeWaveResolved();
}

void AInterfaceRuleHub::onMaterialRemoved(int iMat)
{
    const int size = MaterialRules.size();

    //delete rules from this material to other ones
    for (int iOther = 0; iOther < size; iOther++)
    {
        delete MaterialRules[iMat][iOther]; MaterialRules[iMat][iOther] = nullptr;
    }

    //delete rules from other materials to this one
    for (int iOther = 0; iOther < size; iOther++)
    {
        if (iOther == iMat) continue;

        delete MaterialRules[iOther][iMat]; MaterialRules[iOther][iMat] = nullptr;
        MaterialRules[iOther].erase(MaterialRules[iOther].begin() + iMat);
    }

    MaterialRules.erase(MaterialRules.begin() + iMat);
}

void AInterfaceRuleHub::clearRules()
{
    for (auto & rv : MaterialRules)
        for (auto & r : rv)
            delete r;
    const int numMats = MatHub.countMaterials();
    MaterialRules.resize(numMats);
    for (auto & rv : MaterialRules)
        rv = std::vector<AInterfaceRule*>(numMats, nullptr);

    for (auto const & r : VolumeRules) delete r.second;
    VolumeRules.clear();
    VolumesFrom.clear();
    VolumesTo.clear();
}

void AInterfaceRuleHub::writeToJson(QJsonObject & json) const
{
    QJsonObject js;

    // Material to material
    {
        QJsonArray ar;
        for (auto & rv : MaterialRules)
            for (auto & r : rv)
                if (r)
                {
                    QJsonObject js;
                    r->writeToJson(js);
                    ar.append(js);
                }
        js["MaterialRules"] = ar;
    }

    // Volume to volume
    {
        QJsonArray ar;
        for (auto const & r : VolumeRules)
        {
            QJsonObject jj;
            const TString from = r.first.first;  jj["VolumeFrom"] = from.Data();
            const TString to   = r.first.second; jj["VolumeTo"]   = to  .Data();
            r.second->writeToJson(jj);
            ar.append(jj);
        }
        js["VolumeRules"] = ar;
    }

    json["InterfaceRules"] = js;
}

QString AInterfaceRuleHub::readFromJson(const QJsonObject & json)
{
    clearRules();

    QJsonObject js;
    bool ok = jstools::parseJson(json, "InterfaceRules", js);
    if (!ok) return "Config json does not contain settings for interface rules!";

    QString err = readMaterialRulesFromJson(js);
    if (!err.isEmpty()) return err;

    err = readVolumeRulesFromJson(js);
    if (!err.isEmpty()) return err;

    emit rulesLoaded();
    return "";
}

QString AInterfaceRuleHub::readMaterialRulesFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    jstools::parseJson(json, "MaterialRules", ar);
    for (int iAr = 0; iAr < ar.size(); iAr++)
    {
        QJsonObject js = ar[iAr].toObject();

        QString Model = "NotProvided";
        int MatFrom = -1;
        int MatTo   = -1;

        jstools::parseJson(js, "Model",   Model);
        jstools::parseJson(js, "MatFrom", MatFrom);
        jstools::parseJson(js, "MatTo",   MatTo);

        if (MatFrom < 0 || MatFrom >= MatHub.countMaterials())
            return QString("Bad material index %0\n").arg(MatFrom);
        if (MatTo < 0 || MatTo >= MatHub.countMaterials())
            return QString("Bad material index %0\n").arg(MatTo);
        AInterfaceRule * rule = AInterfaceRule::interfaceRuleFactory(Model, MatFrom, MatTo);
        if (!rule)
            return "Unknown rule model: " + Model + '\n';
        bool ok = rule->readFromJson(js);
        if (!ok)
        {
            delete rule;
            return QString("Failed to read rule (%0 %1 -> %2)\n").arg(Model).arg(MatFrom).arg(MatTo);
        }

        MaterialRules[MatFrom][MatTo] = rule;
    }

    return "";
}

QString AInterfaceRuleHub::readVolumeRulesFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "VolumeRules", ar);
    if (ok)
    {
        for (int i=0; i<ar.size(); i++)
        {
            QJsonObject js = ar[i].toObject();
            QString From = js["VolumeFrom"].toString();
            QString To   = js["VolumeTo"].toString();

            QString Model = "NotProvided";
            int MatFrom = 0;  // updated in runtime
            int MatTo   = 0;  // updated in runtime

            jstools::parseJson(js, "Model",   Model);
            jstools::parseJson(js, "MatFrom", MatFrom);
            jstools::parseJson(js, "MatTo",   MatTo);

            AInterfaceRule * rule = AInterfaceRule::interfaceRuleFactory(Model, MatFrom, MatTo);
            if (!rule) return "Unknown rule model: " + Model + '\n';
            bool ok = rule->readFromJson(js);
            if (!ok)
            {
                delete rule;
                return QString("Failed to read rule (%0 %1 -> %2)\n").arg(Model).arg(MatFrom).arg(MatTo);
            }
            setVolumeRule(From.toLatin1().data(), To.toLatin1().data(), rule);
        }
    }

    return "";
}

void AInterfaceRuleHub::updateVolumesFromTo()
{
    VolumesFrom.clear();
    VolumesTo.clear();
    for (auto const & r : VolumeRules)
    {
        VolumesFrom.insert(r.first.first);
        VolumesTo  .insert(r.first.second);
    }
}

QString AInterfaceRuleHub::checkAll()
{
    QString err;

    for (auto & rv : MaterialRules)
        for (auto & r : rv)
        {
            QString es = r->checkOverrideData();
            if (!es.isEmpty())
            {
                const QString matFrom = MatHub[r->getMaterialFrom()]->Name;
                const QString matTo   = MatHub[r->getMaterialTo()]  ->Name;
                err += QString("In interface rule from %1 to %2:\n").arg(matFrom, matTo) + err;
            }
        }

    return err;
}

void AInterfaceRuleHub::onMaterialAdded()
{
    const int size = MaterialRules.size();
    for (auto & r : MaterialRules) r.push_back(nullptr);
    MaterialRules.push_back(std::vector<AInterfaceRule*>(size+1, nullptr));
}
