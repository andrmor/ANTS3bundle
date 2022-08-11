#ifndef ASURFACEINTERFACERULE_H
#define ASURFACEINTERFACERULE_H

#include "ainterfacerule.h"

class ASurfaceInterfaceRule : public AInterfaceRule
{
public:
    ASurfaceInterfaceRule(int MatFrom, int MatTo);

    OpticalOverrideResultEnum calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "RoughSurface";}
    QString getAbbreviation() const override {return "Rough";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    bool canHaveRoughSurface() const override {return true;}

    QString checkOverrideData() override;

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override;

};

#endif // ASURFACEINTERFACERULE_H
