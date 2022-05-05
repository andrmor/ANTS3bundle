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

void ACalorimeterHub::writeDataToJson(QJsonArray & ar) const
{
    int index = 0;
    for (const ACalorimeterData & md : Calorimeters)
    {
        QJsonObject js;
        md.Calorimeter->writeDataToJson(js, index++);
        ar.push_back(js);
    }
}

QString ACalorimeterHub::appendDataFromJson(const QJsonArray & ar)
{
    if (ar.size() != (int)Calorimeters.size()) return "JsonArray contains data for wrong number of calorimeters";

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        Calorimeters[i].Calorimeter->appendDataFromJson(js);
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

int ACalorimeterHub::countCalorimetersWithData() const
{
    int counter = 0;
    for (const ACalorimeterData & md : Calorimeters)
        if (md.Calorimeter->getTotalEnergy() > 0) counter++;
    return counter;
}

QStringList ACalorimeterHub::getCalorimeterNames() const
{
    QStringList sl;
    for (const ACalorimeterData & d : Calorimeters) sl << d.Name;
    return sl;
}

std::vector<const ACalorimeterData *> ACalorimeterHub::getCalorimeters(const AGeoObject * obj) const
{
    std::vector<const ACalorimeterData *> vec;
    for (const ACalorimeterData & md : Calorimeters)
        if (md.GeoObj == obj) vec.push_back(&md);
    return vec;
}

#include "aerrorhub.h"
bool ACalorimeterHub::mergeCalorimeterFiles(const std::vector<QString> & inFiles, const QString & outFile)
{
    clearData();

    const size_t numCalorimeters = Calorimeters.size();

    for (const QString & FN : inFiles)
    {
        QJsonArray ar;
        bool ok = jstools::loadJsonArrayFromFile(ar, FN);
        if (!ok)
        {
            QString err = "Failed to read calorimeter file: " + FN;
            AErrorHub::addQError(err);
            qWarning() << err;
            return false;
        }

        for (int i = 0; i < ar.size(); i++)
        {
            QJsonObject json = ar[i].toObject();
            int iCalo;
            bool bOK = jstools::parseJson(json, "CalorimeterIndex", iCalo);
            if (!bOK)
            {
                QString err = "Failed to read calorimeter data: invalid format of Calorimeter index in file\n" + FN;
                AErrorHub::addQError(err);
                qWarning() << err;
                continue;
            }
            if (iCalo < 0 || iCalo >= (int)numCalorimeters)
            {
                QString err = "Failed to read calorimter data: Bad calorimeter index in file:\n" + FN;
                AErrorHub::addQError(err);
                qWarning() << err;
                return false;
            }

            Calorimeters[iCalo].Calorimeter->appendDataFromJson(json);
        }
    }

    QJsonArray jsCAr;
    writeDataToJson(jsCAr);
    return jstools::saveJsonArrayToFile(jsCAr, outFile);
}
