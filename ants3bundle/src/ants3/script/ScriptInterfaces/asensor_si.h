#ifndef ASENSOR_SI_H
#define ASENSOR_SI_H

#include "ascriptinterface.h"

#include <QVariantList>

class ASensorHub;

class ASensor_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ASensor_SI();

    AScriptInterface * cloneBase() const {return new ASensor_SI();}

public slots:
    int countSensors();
    int countModels();

    void clearAssignment();
    void assignModel(int iSensor, int iModel);

    int  newModel();
    int  cloneModel(int iModel);

    void setPDE(int iModel, double effective_PDE);
    void setPDE_spectral(int iModel, QVariantList arWaveAndPDE);

    void setAngularFactors(int iModel, QVariantList arAngleAndFactor);
    void setArearFactors(int iModel, QVariantList arFactorMatrix, double stepX, double stepY);

    QVariantList getSensorPositions();

private:
    ASensorHub & SensHub;
};

#endif // ASENSOR_SI_H
