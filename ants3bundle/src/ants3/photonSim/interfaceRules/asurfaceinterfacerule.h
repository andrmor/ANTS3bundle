#ifndef ASURFACEINTERFACERULE_H
#define ASURFACEINTERFACERULE_H

#include "ainterfacerule.h"

class ASurfaceInterfaceRule : public AInterfaceRule
{
public:
    ASurfaceInterfaceRule(int MatFrom, int MatTo)
        : AInterfaceRule(MatFrom, MatTo) {}

    OpticalOverrideResultEnum calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "Surface";}
    QString getAbbreviation() const override {return "Surf";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject & json) const override;
    bool readFromJson(const QJsonObject & json) override;

    QString checkOverrideData() override;

};

#endif // ASURFACEINTERFACERULE_H
