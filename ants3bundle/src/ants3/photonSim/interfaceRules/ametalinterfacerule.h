#ifndef SCATTERONMETAL_H
#define SCATTERONMETAL_H

#include "ainterfacerule.h"

class AMetalInterfaceRule : public AInterfaceRule
{
public:    
    AMetalInterfaceRule(int MatFrom, int MatTo)
        : AInterfaceRule(MatFrom, MatTo) {}

    OpticalOverrideResultEnum calculate(APhoton * photon, const double * globalNormal) override;

    QString getType() const override {return "DielectricToMetal";}
    QString getAbbreviation() const override {return "Met";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    QString checkOverrideData() override;

    double RealN      = 1.07;
    double ImaginaryN = 0.6;

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override;

private:
    double calculateReflectivity(double CosTheta, double RealN, double ImaginaryN, int waveIndex);

};

#endif // SCATTERONMETAL_H
