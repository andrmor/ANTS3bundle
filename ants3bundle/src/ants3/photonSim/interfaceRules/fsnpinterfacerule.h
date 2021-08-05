#ifndef FSNPOPTICALOVERRIDE_H
#define FSNPOPTICALOVERRIDE_H

#include "ainterfacerule.h"

class FsnpInterfaceRule : public AInterfaceRule
{
public:
    FsnpInterfaceRule(int MatFrom, int MatTo)
        : AInterfaceRule(MatFrom, MatTo) {}

    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "FSNP";}
    QString getAbbreviation() const override {return "FSNP";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject & json) const override;
    bool readFromJson(const QJsonObject & json) override;

    QString checkOverrideData() override;

    double Albedo = 0.95;
};

#endif // FSNPOPTICALOVERRIDE_H
