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
    const size_t size = arWaveAndPDE.size();
    data.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        const QVariantList el = arWaveAndPDE[i].toList();
        if (el.size() != 2)
        {
            abort("arWaveAndPDE should contain arrays of two values: wavelength[nm] and the corresponding pde");
            return;
        }
        bool ok1, ok2;
        const double wave = el[0].toDouble(&ok1);
        const double pde  = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("Convertion to number error");
            return;
        }
        data[i] = {wave, pde};
    }

    ASensorModel tmpModel; tmpModel.PDE_spectral = data;
    const QString err = tmpModel.checkPDE_spectral();
    if (!err.isEmpty())
    {
        abort(err);
        return;
    }

    model->PDE_spectral = data;
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
    const size_t size = arAngleAndFactor.size();
    data.resize(size);
    for (size_t i = 0; i < size; i++)
    {
        const QVariantList el = arAngleAndFactor[i].toList();
        if (el.size() != 2)
        {
            abort("arAngleAndFactor should contain arrays of two values: angle[deg] and the corresponding angular factor");
            return;
        }
        bool ok1, ok2;
        const double angle  = el[0].toDouble(&ok1);
        const double factor = el[1].toDouble(&ok2);
        if (!ok1 || !ok2)
        {
            abort("Convertion to number error");
            return;
        }
        data[i] = {angle, factor};
    }

    ASensorModel tmpModel; tmpModel.AngularFactors = data;
    const QString err = tmpModel.checkAngularFactors();
    if (!err.isEmpty())
    {
        abort(err);
        return;
    }

    model->AngularFactors = data;
}

void ASensor_SI::setArearFactors(int iModel, QVariantList arFactorMatrix, double stepX, double stepY)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    std::vector<std::vector<double>> data;
    const size_t size = arFactorMatrix.size();
    data.resize(size);
    for (size_t iR = 0; iR < size; iR++)
    {
        const QVariantList el = arFactorMatrix[iR].toList();
        data[iR].resize(el.size());
        for (size_t iC = 0; iC < el.size(); iC++)
        {
            bool ok;
            data[iR][iC] = el[iC].toDouble(&ok);
            if (!ok)
            {
                abort("arFactorMatrix should contain arrays of numeric values");
                return;
            }
        }
    }

    ASensorModel tmpModel; tmpModel.AreaFactors = data; tmpModel.StepX = stepX; tmpModel.StepY = stepY;
    const QString err = tmpModel.checkAreaFactors();
    if (!err.isEmpty())
    {
        abort(err);
        return;
    }

    model->AreaFactors = data;
    model->StepX = stepX;
    model->StepY = stepY;
}

QVariantList ASensor_SI::getSensorPositions()
{
    QVariantList vl;
    const ASensorHub & hub = ASensorHub::getConstInstance();
    const int num = hub.countSensors();
    for (int iSens = 0; iSens < num; iSens++)
    {
        const AVector3 pos = hub.getPositionFast(iSens);
        QVariantList el{pos[0], pos[1], pos[2]};
        vl.push_back(el);
    }
    return vl;
}
