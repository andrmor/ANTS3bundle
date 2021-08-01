#ifndef ABASICINTERFACERULE_H
#define ABASICINTERFACERULE_H

#include "ainterfacerule.h"

class ABasicInterfaceRule : public AInterfaceRule
{
public:
    ABasicInterfaceRule(int MatFrom, int MatTo);

    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "Simplistic";}
    QString getAbbreviation() const override {return "Simp";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject &json) const override;
    bool readFromJson(const QJsonObject &json) override;

#ifdef GUI
    QWidget * getEditWidget(QWidget *caller, GraphWindowClass* GraphWindow) override;
#endif
    QString checkOverrideData() override;

    double Abs          = 0; //probability of absorption
    double Spec         = 0; //probability of specular reflection
    double Scat         = 0; //probability of scattering
    int    ScatterModel = 1; //0 - 4Pi, 1 - 2Pi back, 2 - 2Pi forward
};

#endif // ABASICINTERFACERULE_H