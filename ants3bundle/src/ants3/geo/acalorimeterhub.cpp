#include "acalorimeterhub.h"
#include "acalorimeter.h"
#include "ageometryhub.h"
#include "ageoobject.h"
#include "ajsontools.h"

ACalorimeterHub & ACalorimeterHub::getInstance()
{
    static ACalorimeterHub instance;
    return instance;
}

const ACalorimeterHub & ACalorimeterHub::getConstInstance()
{
    return getInstance();
}

ACalorimeterHub::~ACalorimeterHub()
{
    clear();
}

void ACalorimeterHub::writeDataToJson(QJsonObject & json) const
{
    QJsonArray ar;
    for (const ACalorimeterData & md : Calorimeters)
    {
        QJsonObject js;
        md.Calorimeter->writeDataToJson(js);
        ar.push_back(js);
    }
    json["CalorimeterData"] = ar;
}

QString ACalorimeterHub::appendDataFromJson(const QJsonObject & json)
{
    QJsonArray ar;
    bool ok = jstools::parseJson(json, "CalorimeterData", ar);
    if (!ok) return "json does not contain calorimeter data";

    if (ar.size() != (int)Calorimeters.size()) return "json contain data for wrong number of calorimeters";

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        ACalorimeter tmp;
        tmp.readDataFromJson(js);

        Calorimeters[i].Calorimeter->append(tmp);
    }

    return "";
}

void ACalorimeterHub::clear()
{
    for (ACalorimeterData & md : Calorimeters) delete md.Calorimeter;
    Calorimeters.clear();
}

void ACalorimeterHub::clearData()
{
    for (ACalorimeterData & md : Calorimeters) md.Calorimeter->clearData();
}

int ACalorimeterHub::countCalorimeters() const
{
    return Calorimeters.size();
}

int ACalorimeterHub::countCalorimetersWithHits() const
{
    int counter = 0;
    for (const ACalorimeterData & md : Calorimeters)
        if (md.Calorimeter->getTotalEnergy() > 0) counter++;
    return counter;
}

std::vector<const ACalorimeterData *> ACalorimeterHub::getCalorimeters(const AGeoObject * obj) const
{
    std::vector<const ACalorimeterData *> vec;
    for (const ACalorimeterData & md : Calorimeters)
        if (md.GeoObj == obj) vec.push_back(&md);
    return vec;
}

void ACalorimeterHub::mergeCalorimeterFiles(const std::vector<QString> & inFiles, const QString & outFile)
{
    clearData();

    const int numCalorimeters = Calorimeters.size();
    for (const QString & FN : inFiles)
    {
        QJsonArray ar;
        bool ok = jstools::loadJsonArrayFromFile(ar, FN);
        if (!ok)
        {
            // !!!*** errorhub!
            qWarning() << "failed to read particle monitor file:" << FN;
            return;
        }

        for (int i=0; i<ar.size(); i++)
        {
            QJsonObject json = ar[i].toObject();
            int iCalo;
            bool bOK = jstools::parseJson(json, "CalorimeterIndex", iCalo);
            if (!bOK)
            {
                // !!!*** errorhub!
                qWarning() << "Failed to read calorimeter data: Calorimeter index not found";
                return;
            }
            if (iCalo < 0 || iCalo >= numCalorimeters)
            {
                // !!!*** errorhub!
                qWarning() << "Failed to read calorimter data: Bad calorimeter index";
                return;
            }

            ACalorimeter tmp;
            tmp.overrideDataFromJson(json);
            Calorimeters[iCalo].Calorimeter->append(tmp);
        }
    }

    QJsonObject json;
        writeDataToJson(json);
    jstools::saveJsonToFile(json, outFile);
}
