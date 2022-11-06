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

void ASensor_SI::setPDE(int iModel, double effective_PDE)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    model->PDE_effective = effective_PDE;
}

void ASensor_SI::setPDE_spectral(int iModel, QVariantList arWaveAndPDE)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    std::vector<std::pair<double,double>> data;
    const int size = arWaveAndPDE.size();
    data.resize(size);
    for (int i = 0; i < size; i++)
    {
        QVariantList el = arWaveAndPDE[i].toList();
        if (el.size() != 2)
        {
            abort("arWaveAndPDE should contain arrays of two values: wavelength[nm] and the corresponding pde");
            return;
        }
        bool ok1, ok2;
        double wave = el[0].toDouble(&ok1);
        double pde  = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("Convertion to number error");
            return;
        }
        data[i] = {wave, pde};
    }

    const std::vector<std::pair<double,double>> oldData = model->PDE_spectral;
    model->PDE_spectral = data;
    QString err = model->checkPDE_spectral();
    if (!err.isEmpty())
    {
        model->PDE_spectral = oldData;
        abort(err);
        return;
    }
}

void ASensor_SI::setAngularFactors(int iModel, QVariantList arAngleAndFactor)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    std::vector<std::pair<double,double>> data;
    const int size = arAngleAndFactor.size();
    data.resize(size);
    for (int i = 0; i < size; i++)
    {
        QVariantList el = arAngleAndFactor[i].toList();
        if (el.size() != 2)
        {
            abort("arAngleAndFactor should contain arrays of two values: angle[deg] and the corresponding angular factor");
            return;
        }
        bool ok1, ok2;
        double angle  = el[0].toDouble(&ok1);
        double factor = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("Convertion to number error");
            return;
        }
        data[i] = {angle, factor};
    }

    const std::vector<std::pair<double,double>> oldData = model->AngularFactors;
    model->AngularFactors = data;
    QString err = model->checkAngularFactors();
    if (!err.isEmpty())
    {
        model->AngularFactors = oldData;
        abort(err);
        return;
    }
}
