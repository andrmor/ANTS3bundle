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

int ASensorHub::countSensorsOfModel(int iModel) const
{
    int num = 0;
    for (const ASensorData & dat : SensorData)
        if (dat.ModelIndex == iModel) num++;
    return num;
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



void ASensorHub::addNewModel()
{
    Models.push_back(ASensorModel());
}

void ASensorHub::cloneModel(int iModel)
{
    if (iModel < 0 || iModel >= (int)Models.size()) return;
    ASensorModel newModel = Models[iModel];
    newModel.Name += "_Clone";
    Models.push_back(newModel);
}

QString ASensorHub::removeModel(int iModel)
{
    if (iModel < 0 || iModel >= (int)Models.size())
        return "Invalid model index";
    if (Models.size() < 2)
        return "Cannot remove the last model";
    if (countSensorsOfModel(iModel) > 0)
        return "Cannot remove: there are sensors of this model";

    Models.erase(Models.begin() + iModel);
    return "";
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
