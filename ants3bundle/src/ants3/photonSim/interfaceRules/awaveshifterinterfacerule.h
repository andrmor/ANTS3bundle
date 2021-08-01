#ifndef AWAVESHIFTEROVERRIDE_H
#define AWAVESHIFTEROVERRIDE_H

#include "ainterfacerule.h"

#include <QString>
#include <QVector>

class AWaveResSettings;
class TH1D;

#ifdef GUI
class QPushButton;
#endif

class AWaveshifterInterfaceRule : public AInterfaceRule
{
public:
    AWaveshifterInterfaceRule(int MatFrom, int MatTo);
    ~AWaveshifterInterfaceRule();

    void initializeWaveResolved() override;
    OpticalOverrideResultEnum calculate(ATracerStateful& Resources, APhoton* Photon, const double* NormalVector) override; //unitary vectors! iWave = -1 if not wavelength-resolved

    QString getType() const override {return "SurfaceWLS";}
    QString getAbbreviation() const override {return "WLS";}
    QString getReportLine() const override;
    QString getLongReportLine() const override;

    void writeToJson(QJsonObject &json) const override;  // !!!***
    bool readFromJson(const QJsonObject &json) override; // !!!***

#ifdef GUI
    QWidget* getEditWidget(QWidget *caller, GraphWindowClass* GraphWindow) override;
#endif
    QString checkOverrideData() override;

    int ReemissionModel = 1; //0-isotropic (4Pi), 1-Lamb back (2Pi), 2-Lamb forward (2Pi)
    QVector<double> ReemissionProbability_lambda;
    QVector<double> ReemissionProbability;
    QVector<double> ReemissionProbabilityBinned;

    QVector<double> EmissionSpectrum_lambda;
    QVector<double> EmissionSpectrum;
    TH1D * Spectrum = nullptr;

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

    const AWaveResSettings & WaveSet;
};

#endif // AWAVESHIFTEROVERRIDE_H
