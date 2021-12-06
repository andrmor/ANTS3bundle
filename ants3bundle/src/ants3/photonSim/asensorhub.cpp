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

QStringList ASensorHub::getListOfModelNames() const
{
    QStringList list;
    list.reserve(Models.size());
    for (const auto & m : Models) list << m.Name;
    return list;
}

ASensorModel * ASensorHub::model(int iModel)
{
    if (iModel < 0 || iModel >= (int)Models.size()) return nullptr;
    return &Models[iModel];
}

const ASensorModel * ASensorHub::model(int iModel) const
{
    if (iModel < 0 || iModel >= (int)Models.size()) return nullptr;
    return &Models[iModel];
}

const ASensorModel * ASensorHub::sensorModel(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return nullptr;
    const int & iModel = SensorData[iSensor].ModelIndex;
    if (iModel < 0 || iModel >= (int)Models.size()) return nullptr;
    return &Models[iModel];
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
