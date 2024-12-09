#ifndef AWAVESHIFTEROVERRIDE_H
#define AWAVESHIFTEROVERRIDE_H

#include "ainterfacerule.h"

#include <QString>

#include <vector>

class AWaveResSettings;
class TH1D;

class AWaveshifterInterfaceRule : public AInterfaceRule
{
public:
    AWaveshifterInterfaceRule(int MatFrom, int MatTo);
    ~AWaveshifterInterfaceRule();

    void initializeWaveResolved() override;
    EInterfaceRuleResult calculate(APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "SurfaceWLS";}
    QString getAbbreviation() const override {return "WLS";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;
    QString getDescription() const override;

    QString loadReemissionProbability(const QString & fileName);
    QString loadEmissionSpectrum(const QString & fileName);

    int ReemissionModel = 1; //0-isotropic (4Pi), 1-Lamb back (2Pi), 2-Lamb forward (2Pi)
    std::vector<std::pair<double,double>> ReemissionProbability;
    std::vector<double> ReemissionProbabilityBinned;
    std::vector<std::pair<double,double>> EmissionSpectrum;
    TH1D * Spectrum = nullptr;
    bool ConserveEnergy = false;

protected:
    void doWriteToJson(QJsonObject & json) const override;  // !!!***
    bool doReadFromJson(const QJsonObject & json) override; // !!!***

    QString doCheckOverrideData() override;

private:
    const AWaveResSettings & WaveSet;
};

#endif // AWAVESHIFTEROVERRIDE_H
