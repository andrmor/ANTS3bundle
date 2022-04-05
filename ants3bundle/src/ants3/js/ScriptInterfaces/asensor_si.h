#ifndef ASENSOR_SI_H
#define ASENSOR_SI_H

#include "ascriptinterface.h"

class ASensorHub;

class ASensor_SI : public AScriptInterface
{
public:
    ASensor_SI();

public slots:
    int countSensors();
    int countModels();

    void assignModel(int iSensor, int iModel);

    int  newModel();
    int  cloneModel(int iModel);

private:
    ASensorHub & SensHub;
};

#endif // ASENSOR_SI_H
