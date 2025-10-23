#ifndef AUNIFIEDRULE_H
#define AUNIFIEDRULE_H

#include "ainterfacerule.h"

class AUnifiedRule : public AInterfaceRule
{
public:
    AUnifiedRule(int MatFrom, int MatTo);

    EInterfaceRuleResult calculate(APhoton * Photon, const double * NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "Unified";}
    QString getAbbreviation() const override {return "Unified";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;
    QString getDescription() const override;

    bool    canHaveRoughSurface() const override {return true;}

    double Cspec     = 0;   // specular spike
    double Cspeclobe = 1.0; // specular lobe (reflection on microfacet)
    double Cdiflobe  = 0;   // diffuse lobe (Lambertian)
    double Cback     = 0;   // backscatter spike

    bool   SkipFresnel = false;
    double AbsorptionOverride = 0.05;
    double ReflectionOverride = 0.5;

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override;

    QString doCheckOverrideData() override;

    double computeReflectionProbability(const APhoton *Photon, const double * NormalVector) const; // !!!*** code duplication! Not easy to remove...
};

#endif // AUNIFIEDRULE_H
