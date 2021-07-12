#ifndef ASCRIPTOPTICALOVERRIDE_H
#define ASCRIPTOPTICALOVERRIDE_H

#include "aopticaloverride.h"

#include <QString>

class AScriptOpticalOverride : public AOpticalOverride
{
public:
  AScriptOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo); // !!!***
  virtual ~AScriptOpticalOverride();

  OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

  const QString getType() const override {return "CustomScript";}
  const QString getAbbreviation() const override {return "JS";}
  const QString getReportLine() const override;
  const QString getLongReportLine() const override;

  void writeToJson(QJsonObject &json) const override;
  bool readFromJson(const QJsonObject &json) override;

#ifdef GUI
  QWidget* getEditWidget(QWidget *caller, GraphWindowClass* GraphWindow) override;
#endif
  const QString checkOverrideData() override;

private:
  QString Script = "if (math.random() < 0.25) photon.LambertForward()\n"
                   "else photon.SpecularReflection()";

#ifdef GUI
  void openScriptWindow(QWidget* caller);  // !!!***
#endif
};

#endif // ASCRIPTOPTICALOVERRIDE_H
