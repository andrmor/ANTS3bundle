#include "asensorhub.h"
#include "ajsontools.h"

ASensorHub & ASensorHub::getInstance()
{
    static ASensorHub instance;
    return instance;
}

const ASensorHub &ASensorHub::getConstInstance()
{
    return getInstance();
}

const ASensorModel & ASensorHub::getModelFast(int iModel) const
{
    return Models.at(iModel);
}

const QStringList ASensorHub::getListOfModelNames() const
{
    QStringList list;
    list.reserve(Models.size());
    for (const auto & m : Models) list << m.Name;
    return list;
}

double ASensorHub::getMaxQEvsWave(int iWave) const
{
    return 1.0;
}

double ASensorHub::getMaxQE() const
{
    return 1.0;
}

void ASensorHub::writeToJson(QJsonObject & json) const
{

}

QString ASensorHub::readFromJson(const QJsonObject & json)
{
    return "";
}

ASensorHub::ASensorHub()
{
    Models.resize(1);
    Models.front().Name = "Ideal";
}
