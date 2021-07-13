#ifndef SPECTRALBASICOPTICALOVERRIDE_H
#define SPECTRALBASICOPTICALOVERRIDE_H

#include "aopticaloverride.h"
#include "abasicopticaloverride.h"

#include <QString>
#include <QVector>

class A3MatHub;
class ATracerStateful;
class APhoton;
class QJsonObject;
class GraphWindowClass;
class QPushButton;

class SpectralBasicOpticalOverride : public ABasicOpticalOverride
{
public:
  SpectralBasicOpticalOverride(A3MatHub* MatCollection, int MatFrom, int MatTo);
  ~SpectralBasicOpticalOverride() {}

  OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

  const QString getType() const override {return "SimplisticSpectral";}
  const QString getAbbreviation() const override {return "SiSp";}
  const QString getReportLine() const override;
  const QString getLongReportLine() const override;

  // save/load config is not used for this type!
  void writeToJson(QJsonObject &json) const override;
  bool readFromJson(const QJsonObject &json) override;

  void initializeWaveResolved() override;

  const QString loadData(const QString& fileName); // !!!***

#ifdef GUI
  virtual QWidget* getEditWidget(QWidget* caller, GraphWindowClass* GraphWindow) override;
#endif

  virtual const QString checkOverrideData() override;

  //parameters
  QVector<double> Wave;
  QVector<double> ProbLoss; //probability of absorption
  QVector<double> ProbLossBinned; //probability of absorption
  QVector<double> ProbRef;  //probability of specular reflection
  QVector<double> ProbRefBinned;  //probability of specular reflection
  QVector<double> ProbDiff; //probability of scattering
  QVector<double> ProbDiffBinned; //probability of scattering
  double effectiveWavelength = 500; //if waveIndex of photon is -1, index correspinding to this wavelength will be used
  double effectiveWaveIndex;
  bool bWaveResolved;

private:
#ifdef GUI
  QPushButton *pbShow, *pbShowBinned;
  void loadSpectralData(QWidget *caller);  // !!!***
  void showLoaded(GraphWindowClass *GraphWindow); // !!!***
  void showBinned(QWidget *widget, GraphWindowClass *GraphWindow); // !!!***
  void updateButtons();
#endif
};

#endif // SPECTRALBASICOPTICALOVERRIDE_H