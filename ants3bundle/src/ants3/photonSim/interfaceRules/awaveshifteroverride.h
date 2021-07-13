#ifndef AWAVESHIFTEROVERRIDE_H
#define AWAVESHIFTEROVERRIDE_H

#include "aopticaloverride.h"

#include <QString>
#include <QVector>

class A3MatHub;
class ATracerStateful;
class APhoton;
class QJsonObject;
class GraphWindowClass;
class TH1D;
class QPushButton;

class AWaveshifterOverride : public AOpticalOverride
{
public:
  AWaveshifterOverride(A3MatHub* MatCollection, int MatFrom, int MatTo);
  virtual ~AWaveshifterOverride();

  void initializeWaveResolved() override;
  OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

  virtual const QString getType() const override {return "SurfaceWLS";}
  virtual const QString getAbbreviation() const override {return "WLS";}
  virtual const QString getReportLine() const override;
  virtual const QString getLongReportLine() const override;

  // save/load config is not used for this type!
  void writeToJson(QJsonObject &json) const override;  // !!!***
  bool readFromJson(const QJsonObject &json) override; // !!!***

#ifdef GUI
  virtual QWidget* getEditWidget(QWidget *caller, GraphWindowClass* GraphWindow) override;
#endif

  virtual const QString checkOverrideData() override;

  //-- parameters --
  int ReemissionModel = 1; //0-isotropic (4Pi), 1-Lamb back (2Pi), 2-Lamb forward (2Pi)
  QVector<double> ReemissionProbability_lambda;
  QVector<double> ReemissionProbability;
  QVector<double> ReemissionProbabilityBinned;

  QVector<double> EmissionSpectrum_lambda;
  QVector<double> EmissionSpectrum;
  TH1D* Spectrum = 0;

  double WaveFrom;
  double WaveStep;
  int WaveNodes;

private:
#ifdef GUI
  QPushButton *pbShowRP, *pbShowRPbinned, *pbShowES, *pbShowESbinned;
  void loadReemissionProbability(QWidget *caller); // !!!***
  void loadEmissionSpectrum(QWidget *caller); // !!!***
  void showReemissionProbability(GraphWindowClass* GraphWindow, QWidget *caller); // !!!***
  void showEmissionSpectrum(GraphWindowClass* GraphWindow, QWidget *caller);  // !!!***
  void showBinnedReemissionProbability(GraphWindowClass* GraphWindow, QWidget *caller); // !!!***
  void showBinnedEmissionSpectrum(GraphWindowClass* GraphWindow, QWidget *caller);  // !!!***
  void updateButtons();
#endif
};

#endif // AWAVESHIFTEROVERRIDE_H
