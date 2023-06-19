#include "asensorhub.h"
#include "ageoobject.h"
#include "ageoshape.h"
#include "ajsontools.h"

ASensorHub & ASensorHub::getInstance()
{
    static ASensorHub instance;
    return instance;
}

const ASensorHub & ASensorHub::getConstInstance()
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

int ASensorHub::getModelIndex(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return -1;
    return SensorData[iSensor].ModelIndex;
}

int ASensorHub::addNewModel()
{
    Models.push_back(ASensorModel());
    return Models.size()-1;
}

int ASensorHub::cloneModel(int iModel)
{
    if (iModel < 0 || iModel >= (int)Models.size()) return -1;
    ASensorModel newModel = Models[iModel];
    newModel.Name += "_Clone";
    Models.push_back(newModel);
    return Models.size()-1;
}

void ASensorHub::clearAssignment()
{
    for (ASensorData & sd : SensorData)
        sd.ModelIndex = 0;
}

void ASensorHub::setSensorModel(int iSensor, int iModel)
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return;
    if (iModel  < 0 || iModel  >= (int)Models.size()) return;
    SensorData[iSensor].ModelIndex = iModel;
    PersistentModelAssignment = true;
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

AVector3 ASensorHub::getPosition(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return {0,0,0};
    return getPositionFast(iSensor);
}

AVector3 ASensorHub::getPositionFast(int iSensor) const
{
    return SensorData[iSensor].Position;
}

double ASensorHub::getMinSize(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size())
    {
        qWarning() << "Invalid sensor index in ASensorHub::getMinSize()";
        return 1000;
    }
    return getMinSizeFast(iSensor);
}

double ASensorHub::getMinSizeFast(int iSensor) const
{
    return SensorData[iSensor].GeoObj->Shape->minSize();    // !!!*** add minSize for all shapes!!!
}

AGeoObject * ASensorHub::getGeoObject(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return nullptr;
    return SensorData[iSensor].GeoObj;
}

bool ASensorHub::updateRuntimeProperties()
{
    for (ASensorData & sd : SensorData)
    {
        const int & index = sd.ModelIndex;
        if (index < 0 || index >= (int)Models.size())
        {
            // !!!*** error reporting
            qCritical() << "Bad sensor model index:" << index;
            exit(222);
        }
        ASensorModel & model = Models[index];
        model.updateRuntimeProperties();
    }
    return true;
}

void ASensorHub::exitPersistentMode()
{
    PersistentModelAssignment = false;
    LoadedModelAssignment.clear();
}

double ASensorHub::getMaxQE(bool waveResolved, int iWave) const
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

        QJsonArray arMA;
        if (PersistentModelAssignment)
            for (const ASensorData & sd : SensorData)
                arMA << sd.ModelIndex;
        mainJs["ModelAssignment"] = arMA;
    json["Sensors"] = mainJs;
}

QString ASensorHub::readFromJson(const QJsonObject & json)
{
    Models.clear();
    PersistentModelAssignment = false;
    LoadedModelAssignment.clear();

    QString err;

    QJsonObject mainJs;
    bool ok = jstools::parseJson(json, "Sensors", mainJs);
    if (!ok)
    {
        qWarning() << "No sensor data, adding a dummy sensor";
        Models.push_back(ASensorModel());
    }
    else
    {
        QJsonArray ar;
        jstools::parseJson(mainJs, "Models", ar);
        Models.reserve(ar.size());

        for (int i = 0; i < ar.size(); i++)
        {
            const QJsonObject js = ar[i].toObject();
            ASensorModel model;
            bool ok = model.readFromJson(js);
            if (!ok)
            {
                if (!err.isEmpty()) err += "\n";
                err += QString("Bad format of sensor model %0 json").arg(i);
            }
            Models.push_back(model);
        }
    }

    QJsonArray arMA;
    ok = jstools::parseJson(mainJs, "ModelAssignment", arMA);
    if (ok && !arMA.empty())
    {
        bool bFoundInvalidModelIndex = false;
        PersistentModelAssignment = true;
        const int maxModel = Models.size() - 1;
        LoadedModelAssignment.reserve(arMA.size());
        for (int i = 0; i < arMA.size(); i++)
        {
            int iMod = arMA[i].toInt();
            if (iMod < 0 || iMod > maxModel)
            {
                iMod = 0;
                bFoundInvalidModelIndex = true;
            }
            LoadedModelAssignment.push_back(iMod);
        }
        if (bFoundInvalidModelIndex)
        {
            if (!err.isEmpty()) err += "\n";
            err += "Bad model index(es) in loaded ModelAssignment, replaced with 0";
        }
    }

    return err;
}

void ASensorHub::clear()
{
    LoadedModelAssignment.clear();
    clearSensors();

    Models.clear();
    Models.resize(1);
    Models.front().Name = "Ideal";
}

double ASensorHub::getMaxQE(bool bWaveRes) const
{
    double maxQE = 0;
    for (const ASensorData & SM : SensorData)
    {
        const int & iModel = SM.ModelIndex;
        if (iModel < 0 || iModel >= (int)Models.size()) continue;
        double modelMaxQE = Models[iModel].getMaxQE(bWaveRes);
        if (modelMaxQE > maxQE) maxQE = modelMaxQE;
    }
    qDebug() << "----- Max QE:" << maxQE;
    return maxQE;
}

ASensorHub::ASensorHub()
{
    Models.resize(1);
    Models.front().Name = "Ideal";
}

void ASensorHub::clearSensors()
{
    SensorData.clear();
}

void ASensorHub::registerNextSensor(ASensorData & sr)
{
    if (PersistentModelAssignment)
    {
        const size_t index = SensorData.size();
        sr.ModelIndex = ( index < LoadedModelAssignment.size() ? LoadedModelAssignment[index]
                                                               : 0 );
    }

    SensorData.push_back(sr);
}
