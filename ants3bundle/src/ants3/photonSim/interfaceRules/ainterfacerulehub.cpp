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

void AInterfaceRuleHub::updateRuntimeProperties()
{
    for (auto & rv : Rules)
        for (auto & r : rv)
            if (r) r->initializeWaveResolved();
}

void AInterfaceRuleHub::onMaterialRemoved(int iMat)
{
    const int size = Rules.size();

    //delete rules from this material to other ones
    for (int iOther = 0; iOther < size; iOther++)
    {
        delete Rules[iMat][iOther]; Rules[iMat][iOther] = nullptr;
    }

    //delete rules from other materials to this one
    for (int iOther = 0; iOther < size; iOther++)
    {
        if (iOther == iMat) continue;

        delete Rules[iOther][iMat]; Rules[iOther][iMat] = nullptr;
        Rules[iOther].erase(Rules[iOther].begin() + iMat);
    }

    Rules.erase(Rules.begin() + iMat);
}

void AInterfaceRuleHub::clearRules()
{
    for (auto & rv : Rules)
        for (auto & r : rv)
            delete r;

    const int numMats = MatHub.countMaterials();
    Rules.resize(numMats);
    for (auto & rv : Rules)
        rv = std::vector<AInterfaceRule*>(numMats, nullptr);
}

void AInterfaceRuleHub::writeToJson(QJsonObject & json) const
{
    QJsonArray ar;

    for (auto & rv : Rules)
        for (auto & r : rv)
            if (r)
            {
                QJsonObject js;
                r->writeToJson(js);
                ar.append(js);
            }

    json["InterfaceRules"] = ar;
}

QString AInterfaceRuleHub::readFromJson(const QJsonObject & json)
{
    clearRules();

    QJsonArray ar;
    bool ok = jstools::parseJson(json, "InterfaceRules", ar);
    if (!ok) return "Json does not contain settings for interface rules!";

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
            return QString("Failed to read rule (%0 %1 -> %2)\n").arg(Model, MatFrom, MatTo);
        }

        Rules[MatFrom][MatTo] = rule;
    }

    emit rulesLoaded();
    return "";
}

QString AInterfaceRuleHub::checkAll()
{
    QString err;

    for (auto & rv : Rules)
        for (auto & r : rv)
        {
            QString es = r->checkOverrideData();
            if (!es.isEmpty())
            {
                const QString matFrom = MatHub[r->getMaterialFrom()]->name;
                const QString matTo   = MatHub[r->getMaterialTo()]  ->name;
                err += QString("In interface rule from %1 to %2:\n").arg(matFrom, matTo) + err;
            }
        }

    return err;
}

void AInterfaceRuleHub::onMaterialAdded()
{
    const int size = Rules.size();
    for (auto & r : Rules) r.push_back(nullptr);
    Rules.push_back(std::vector<AInterfaceRule*>(size+1, nullptr));
}
