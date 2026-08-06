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

int ASensorHub::addModel(const ASensorModel & model)
{
    Models.push_back(model);
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

QString ASensorHub::removeModel(int iModel)
{
    if (iModel < 0 || iModel >= (int)Models.size())
        return "Invalid model index";
    if (Models.size() < 2)
        return "Cannot remove the last remaining model";
    if (countSensorsOfModel(iModel) > 0)
        return "Cannot remove: there are sensors of this model";

    Models.erase(Models.begin() + iModel);
    shiftModelAssignemnt(iModel);

    return "";
}

#include "ageometryhub.h"
void ASensorHub::shiftModelAssignemnt(int iModel)
{
    for (int & iAssigned : CustomModelAssignmentArray)
        if (iAssigned > iModel) iAssigned--;

    AGeometryHub::getInstance().shiftSensorModelsOnRemoveModel(iModel);
    AGeometryHub::getInstance().populateGeoManager();
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
    return SensorData[iSensor].GeoObj->Shape->minSize();
}

AGeoObject * ASensorHub::getGeoObject(int iSensor) const
{
    if (iSensor < 0 || iSensor >= (int)SensorData.size()) return nullptr;
    return SensorData[iSensor].GeoObj;
}

QString ASensorHub::updateRuntimeProperties()
{
    for (ASensorData & sd : SensorData)
    {
        const int & index = sd.ModelIndex;
        if (index < 0 || index >= (int)Models.size())
            return QString("Light sensor is assigned an invalid model index (%0)").arg(index);
    }

    for (int iMod = 0; iMod < Models.size(); iMod++)
    {
        ASensorModel & model = Models[iMod];

        std::vector<int> seenSensorMats;
        for (ASensorData & sd : SensorData)
        {
            if (sd.ModelIndex != iMod) continue;
            int iMat = sd.GeoObj->Material;
            if (std::find(seenSensorMats.begin(), seenSensorMats.end(), iMat) == seenSensorMats.end()) seenSensorMats.push_back(iMat);
        }

        QString err = model.updateRuntimeProperties(seenSensorMats);
        if (!err.isEmpty()) return err;
    }

    if (UseSensorGains)
        if (SensorGains.size() != SensorData.size()) return "Sensor gain vector has invalid size";

    return "";
}

void ASensorHub::writeToJson(QJsonObject & json) const
{
    QJsonObject mainJs;

    // Models
    {
        QJsonArray ar;
        for (const ASensorModel & m : Models)
        {
            QJsonObject js;
            m.writeToJson(js);
            ar.push_back(js);
        }
        mainJs["Models"] = ar;
    }

    // Custom model assignment
    {
        QJsonObject js;
            js["Enabled"] = CustomModelAssignmentEnabled;
            QJsonArray ar;
                for (int iModel : CustomModelAssignmentArray) ar << iModel;
            js["ModelAssignment"] = ar;
        mainJs["CustomModelAssignment"] = js;
    }

    // Gains
    {
        QJsonObject js;
            js["Enabled"] = UseSensorGains;
            QJsonArray ar;
            for (double gain : SensorGains)
                ar.push_back(gain);
            js["Gains"] = ar;
        mainJs["SensorGains"] = js;
    }

    json["Sensors"] = mainJs;
}

QString ASensorHub::readFromJson(const QJsonObject & json)
{
    Models.clear();
    CustomModelAssignmentEnabled = false;
    CustomModelAssignmentArray.clear();

    QJsonObject mainJs;
    bool ok = jstools::parseJson(json, "Sensors", mainJs);
    if (!ok)
    {
        qWarning() << "No sensor models are defined, adding an ideal PMT"; // runtime check will catch
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
            QString err = model.readFromJson(js);
            if (!err.isEmpty()) return err;
            Models.push_back(model);
        }
    }

    // Custom model assignment
    {
        const int maxModel = Models.size() - 1;
        QJsonObject js;
        bool ok = jstools::parseJson(mainJs, "CustomModelAssignment", js);
        if (ok)
        {
            jstools::parseJson(js, "Enabled", CustomModelAssignmentEnabled);
            QJsonArray ar;
            jstools::parseJson(js, "ModelAssignment", ar);
            for (int i = 0; i < ar.size(); i++)
            {
                int iMod = ar[i].toInt();
                if (iMod < 0 || iMod > maxModel)
                {
                    CustomModelAssignmentEnabled = false;
                    CustomModelAssignmentArray.clear();
                    return "Bad model index(es) in loaded custom sensor model assignment";
                }
                CustomModelAssignmentArray.push_back(iMod);
            }
        }
    }

    // Gains
    {
        QJsonObject js;
        ok = jstools::parseJson(mainJs, "SensorGains", js);
        if (ok)
        {
            jstools::parseJson(js, "Enabled", UseSensorGains);
            QJsonArray ar;
            jstools::parseJson(js, "Gains", ar);
            SensorGains.resize(ar.size());
            for (size_t i = 0; i < ar.size(); i++)
                SensorGains[i] = ar[i].toDouble();
        }
        else UseSensorGains = false; // compatibility
    }

    return "";
}

void ASensorHub::clear()
{
    Models.clear();
    Models.resize(1);
    Models.front().Name = "Ideal";

    UseSensorGains = false;
    SensorGains.clear();

    CustomModelAssignmentEnabled = false;
    CustomModelAssignmentArray.clear();

    clearSensors();
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
    //qDebug() << "----- Max QE:" << maxQE;
    return maxQE;
}

const ASensorData *ASensorHub::getSensorData(int iSensor) const
{
    if (iSensor > -1 && iSensor < SensorData.size()) return &(SensorData[iSensor]);
    return nullptr;
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
    if (CustomModelAssignmentEnabled)
    {
        const size_t index = SensorData.size();
        sr.ModelIndex = ( index < CustomModelAssignmentArray.size() ? CustomModelAssignmentArray[index]
                                                                    : 0 );
    }

    SensorData.push_back(sr);
}
