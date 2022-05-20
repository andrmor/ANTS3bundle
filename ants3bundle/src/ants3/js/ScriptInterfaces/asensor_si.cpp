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
    return SensHub.countModels();
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
    if (iModel < 0 || iModel >= SensHub.countModels())
    {
        abort("Invalid sensor model index");
        return;
    }

    SensHub.setSensorModel(iSensor, iModel);
}

int ASensor_SI::newModel()
{
    return SensHub.addNewModel();
}

int ASensor_SI::cloneModel(int iModel)
{
    if (iModel < 0 || iModel >= SensHub.countModels())
    {
        abort("Invalid sensor model index");
        return 0;
    }
    return SensHub.cloneModel(iModel);
}
