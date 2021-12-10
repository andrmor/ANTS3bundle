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

bool ASensorHub::updateRuntimeProperties()
{
    for (ASensorData & sd : SensorData)
    {
        const int & index = sd.ModelIndex;
        if (index < 0 || index >= Models.size())
        {
            // !!!*** error reporting
            qCritical() << "Bad sensor model index:" << index;
            exit(222);
        }
        ASensorModel & model = Models[index];
        model.updateRuntimeProperties();
    }
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
    QJsonObject mainJs;
        QJsonArray ar;
        for (const ASensorModel & m : Models)
        {
            QJsonObject js;
            m.writeToJson(js);
            ar.push_back(js);
        }
        mainJs["Models"] = ar;
    json["Sensors"] = mainJs;
}

QString ASensorHub::readFromJson(const QJsonObject & json)
{
    Models.clear();

    QJsonObject mainJs;
    bool ok = jstools::parseJson(json, "Sensors", mainJs);
    if (!ok)
    {
        qWarning() << "No sensor data, adding a dummy sensor";
        Models.push_back(ASensorModel());
        return "";
    }

    QJsonArray ar;
    jstools::parseJson(mainJs, "Models", ar);
    Models.reserve(ar.size());

    QString err;
    for (int i = 0; i < ar.size(); i++)
    {
        const QJsonObject js = ar[i].toObject();
        ASensorModel model;
        bool ok = model.readFromJson(js);
        if (!ok) err = "Bad format of sensor model json";
        Models.push_back(model);
    }
    return err;
}

ASensorHub::ASensorHub()
{
    Models.resize(1);
    Models.front().Name = "Ideal";
}
