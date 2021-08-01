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

const ASensorModel * ASensorHub::getModelFast(int iModel) const
{
    return &Models.at(iModel);
}

const QStringList ASensorHub::getListOfModelNames() const
{
    QStringList list;
    list.reserve(Models.size());
    for (const auto & m : Models) list << m.Name;
    return list;
}

void ASensorHub::writeToJson(QJsonObject & json) const
{

}

bool ASensorHub::readFromJson(const QJsonObject & json)
{
    return true;
}

ASensorHub::ASensorHub()
{
    Models.resize(1);
    Models.front().Name = "Ideal";
}
