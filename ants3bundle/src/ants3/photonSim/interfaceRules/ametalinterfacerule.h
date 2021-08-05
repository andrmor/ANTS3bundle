#ifndef SCATTERONMETAL_H
#define SCATTERONMETAL_H

#include "ainterfacerule.h"

class AMetalInterfaceRule : public AInterfaceRule
{
public:    
    AMetalInterfaceRule(int MatFrom, int MatTo)
        : AInterfaceRule(MatFrom, MatTo) {}

    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "DielectricToMetal";}
    QString getAbbreviation() const override {return "Met";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject &json) const override;
    bool readFromJson(const QJsonObject &json) override;

    QString checkOverrideData() override;

    double RealN      = 1.07;
    double ImaginaryN = 0.6;

private:
    double calculateReflectivity(double CosTheta, double RealN, double ImaginaryN, int waveIndex);

};

#endif // SCATTERONMETAL_H
