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
    return &SensorModels.at(iModel);
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
    SensorModels.resize(1);
    SensorModels.front().name = "Ideal";
}
