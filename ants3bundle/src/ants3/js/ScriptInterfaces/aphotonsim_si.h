#ifndef APHOTONSIM_SI_H
#define APHOTONSIM_SI_H

#include "ascriptinterface.h"

class APhotonSimManager;

class APhotonSim_SI : public AScriptInterface
{
    Q_OBJECT

public:
    APhotonSim_SI();
    ~APhotonSim_SI();

public slots:
    QString simulate(bool updateGui);

    void setSeed(double seed);

private:
    APhotonSimManager & SimMan;

};

#endif // APHOTONSIM_SI_H
