#include "asensor_si.h"
#include "asensorhub.h"

ASensor_SI::ASensor_SI() :
    SensHub(ASensorHub::getInstance())
{

}

int ASensor_SI::countSensors()
{
    return SensHub.countSensors();
}

int ASensor_SI::countModels()
{
    return SensHub.countSensorModels();
}

void ASensor_SI::clearAssignment()
{
    SensHub.clearAssignment();
}

void ASensor_SI::assignModel(int iSensor, int iModel)
{
    if (iSensor < 0 || iSensor >= SensHub.countSensors())
    {
        abort("Invalid sensor index");
        return;
    }
    if (iModel < 0 || iModel >= SensHub.countSensorModels())
    {
        abort("Invalid sensor model index");
        return;
    }

    SensHub.SensorData[iSensor].ModelIndex = iModel;
}

int ASensor_SI::newModel()
{
    return SensHub.addNewModel();
}

int ASensor_SI::cloneModel(int iModel)
{
    if (iModel < 0 || iModel >= SensHub.countSensorModels())
    {
        abort("Invalid sensor model index");
        return 0;
    }
    return SensHub.cloneModel(iModel);
}
