#ifndef SPECTRALBASICOPTICALOVERRIDE_H
#define SPECTRALBASICOPTICALOVERRIDE_H

#include "ainterfacerule.h"
#include "abasicinterfacerule.h"

#include <QVector>

class AWaveResSettings;

#ifdef GUI
class QPushButton;
#endif

class ASpectralBasicInterfaceRule : public ABasicInterfaceRule
{
public:
    ASpectralBasicInterfaceRule(int MatFrom, int MatTo);
    ~ASpectralBasicInterfaceRule() {}

    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "SimplisticSpectral";}
    QString getAbbreviation() const override {return "SiSp";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject &json) const override;
    bool readFromJson(const QJsonObject &json) override;

    void initializeWaveResolved() override;

    QString loadData(const QString & fileName); // !!!***

#ifdef GUI
    QWidget* getEditWidget(QWidget* caller, GraphWindowClass* GraphWindow) override;
#endif

    QString checkOverrideData() override;

    QVector<double> Wave;
    QVector<double> ProbLoss; //probability of absorption
    QVector<double> ProbLossBinned; //probability of absorption
    QVector<double> ProbRef;  //probability of specular reflection
    QVector<double> ProbRefBinned;  //probability of specular reflection
    QVector<double> ProbDiff; //probability of scattering
    QVector<double> ProbDiffBinned; //probability of scattering
    double effectiveWavelength = 500; //if waveIndex of photon is -1, index correspinding to this wavelength will be used
    double effectiveWaveIndex;

private:
#ifdef GUI
    QPushButton *pbShow, *pbShowBinned;
    void loadSpectralData(QWidget *caller);  // !!!***
    void showLoaded(GraphWindowClass *GraphWindow); // !!!***
    void showBinned(QWidget *widget, GraphWindowClass *GraphWindow); // !!!***
    void updateButtons();
#endif

    const AWaveResSettings & WaveSet;
};

#endif // SPECTRALBASICOPTICALOVERRIDE_H
