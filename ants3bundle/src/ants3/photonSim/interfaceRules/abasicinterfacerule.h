#ifndef ABASICINTERFACERULE_H
#define ABASICINTERFACERULE_H

#include "ainterfacerule.h"

#include <QString>

class AMaterialHub;
class ATracerStateful;
class APhoton;
class QJsonObject;
class GraphWindowClass;

class ABasicInterfaceRule : public AInterfaceRule
{
public:
    ABasicInterfaceRule(int MatFrom, int MatTo);

    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    const QString getType() const override {return "Simplistic";}
    const QString getAbbreviation() const override {return "Simp";}
    const QString getReportLine() const override;
    const QString getLongReportLine() const override;

    // save/load config is not used for this type!
    void writeToJson(QJsonObject &json) const override;
    bool readFromJson(const QJsonObject &json) override;

#ifdef GUI
    QWidget * getEditWidget(QWidget *caller, GraphWindowClass* GraphWindow) override;
#endif
    const QString checkOverrideData() override;

    //--parameters--
    double probLoss     = 0; //probability of absorption
    double probRef      = 0; //probability of specular reflection
    double probDiff     = 0; //probability of scattering
    int    scatterModel = 1; //0 - 4Pi, 1 - 2Pi back, 2 - 2Pi forward
};

#endif // ABASICINTERFACERULE_H
