#ifndef ABASICINTERFACERULE_H
#define ABASICINTERFACERULE_H

#include "ainterfacerule.h"

class ABasicInterfaceRule : public AInterfaceRule
{
public:
    ABasicInterfaceRule(int MatFrom, int MatTo);

    OpticalOverrideResultEnum calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "Simplistic";}
    QString getAbbreviation() const override {return "Simp";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;
    QString getDescription() const override;

    bool    canHaveRoughSurface() const override {return true;}

    double  Abs          = 0; //probability of absorption
    double  Spec         = 0; //probability of specular reflection
    double  Scat         = 0; //probability of scattering
    int     ScatterModel = 1; //0 - 4Pi, 1 - 2Pi back, 2 - 2Pi forward

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override;

    QString doCheckOverrideData() override;
};

#endif // ABASICINTERFACERULE_H
