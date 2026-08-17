#include "asensor_si.h"
#include "asensorhub.h"
#include "ascripthub.h"

ASensor_SI::ASensor_SI() :
    SensHub(ASensorHub::getInstance())
{

}

int ASensor_SI::countSensors()
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return 0;
    return SensHub.countSensors();
}

int ASensor_SI::countModels()
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return 0;
    return SensHub.countModels();
}

QVariantList ASensor_SI::getGains()
{
    QVariantList vl;

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return vl;

    for (double gain : SensHub.SensorGains)
        vl.push_back(gain);

    return vl;
}

void ASensor_SI::setGains(QVariantList gains)
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

    if (gains.size() != SensHub.countSensors())
    {
        abort("Invalid size of the gain array in sens.setGains");
        return;
    }

    SensHub.SensorGains.clear();

    for (int i = 0; i < gains.size(); i++)
        SensHub.SensorGains.push_back(gains[i].toDouble());

    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

void ASensor_SI::disableCustomModelAssignment()
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

    SensHub.CustomModelAssignmentEnabled = false;
    SensHub.CustomModelAssignmentArray.clear();
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

void ASensor_SI::enableCustomModelAssignment(QVariantList sensorModels)
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

    std::vector<int> models;
    for (int i = 0; i < sensorModels.size(); i++)
    {
        int iModel = sensorModels[i].toInt();
        if (iModel < 0 || iModel >= SensHub.countModels())
        {
            abort("Invalid sensor model index");
            SensHub.CustomModelAssignmentEnabled = false;
            SensHub.CustomModelAssignmentArray.clear();
            return;
        }
    }

    SensHub.CustomModelAssignmentEnabled = true;
    SensHub.CustomModelAssignmentArray = models;
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

int ASensor_SI::newModel()
{
    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return 0;

    int iModel = SensHub.addNewModel();
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
    return iModel;
}

int ASensor_SI::cloneModel(int iModel)
{
    if (iModel < 0 || iModel >= SensHub.countModels())
    {
        abort("Invalid sensor model index");
        return 0;
    }

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return 0;

    iModel = SensHub.cloneModel(iModel);
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
    return iModel;
}

void ASensor_SI::setPDE(int iModel, double effective_PDE)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;
    model->PDE_effective = effective_PDE;
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

void ASensor_SI::setPDE_spectral(int iModel, QVariantList arWaveAndPDE)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

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
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

void ASensor_SI::setAngularFactors(int iModel, QVariantList arAngleAndFactor)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

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
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

void ASensor_SI::setArearFactors(int iModel, QVariantList arFactorMatrix, double stepX, double stepY)
{
    ASensorModel * model = SensHub.model(iModel);
    if (!model)
    {
        abort("Invalid sensor model index");
        return;
    }

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return;

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
    AScriptHub::getInstance().registerHubsModified_JsonNotYetUpdated(true);
}

QVariantList ASensor_SI::getSensorPositions()
{
    QVariantList vl;

    if (AScriptHub::getInstance().abortIfHubAccessBlocked(Lang)) return vl;

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
