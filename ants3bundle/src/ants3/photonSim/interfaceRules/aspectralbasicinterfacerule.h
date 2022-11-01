#ifndef SPECTRALBASICOPTICALOVERRIDE_H
#define SPECTRALBASICOPTICALOVERRIDE_H

#include "ainterfacerule.h"
#include "abasicinterfacerule.h"

#include <QVector> // !!!***

class AWaveResSettings;

class ASpectralBasicInterfaceRule : public ABasicInterfaceRule
{
public:
    ASpectralBasicInterfaceRule(int MatFrom, int MatTo);
    ~ASpectralBasicInterfaceRule() {}

    OpticalOverrideResultEnum calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "SimplisticSpectral";}
    QString getAbbreviation() const override {return "SimpS";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void initializeWaveResolved() override;

    QString loadData(const QString & fileName); // !!!***

    QVector<double> Wave;
    QVector<double> ProbLoss; //probability of absorption
    QVector<double> ProbLossBinned; //probability of absorption
    QVector<double> ProbRef;  //probability of specular reflection
    QVector<double> ProbRefBinned;  //probability of specular reflection
    QVector<double> ProbDiff; //probability of scattering
    QVector<double> ProbDiffBinned; //probability of scattering
    double effectiveWavelength = 500; //if waveIndex of photon is -1, index correspinding to this wavelength will be used
    double effectiveWaveIndex;

protected:
    void doWriteToJson(QJsonObject & json) const override;
    bool doReadFromJson(const QJsonObject & json) override;

    QString doCheckOverrideData() override;

private:
    const AWaveResSettings & WaveSet;
};

#endif // SPECTRALBASICOPTICALOVERRIDE_H
