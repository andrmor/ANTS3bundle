#ifndef AWAVESHIFTEROVERRIDE_H
#define AWAVESHIFTEROVERRIDE_H

#include "ainterfacerule.h"

#include <QString>
#include <QVector>   // !!!***

class AWaveResSettings;
class TH1D;

class AWaveshifterInterfaceRule : public AInterfaceRule
{
public:
    AWaveshifterInterfaceRule(int MatFrom, int MatTo);
    ~AWaveshifterInterfaceRule();

    void initializeWaveResolved() override;
    OpticalOverrideResultEnum calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "SurfaceWLS";}
    QString getAbbreviation() const override {return "WLS";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject &json) const override;  // !!!***
    bool readFromJson(const QJsonObject &json) override; // !!!***

    QString checkOverrideData() override;

    int ReemissionModel = 1; //0-isotropic (4Pi), 1-Lamb back (2Pi), 2-Lamb forward (2Pi)
    QVector<double> ReemissionProbability_lambda;
    QVector<double> ReemissionProbability;
    QVector<double> ReemissionProbabilityBinned;

    QVector<double> EmissionSpectrum_lambda;
    QVector<double> EmissionSpectrum;
    TH1D * Spectrum = nullptr;

private:
    const AWaveResSettings & WaveSet;
};

#endif // AWAVESHIFTEROVERRIDE_H
