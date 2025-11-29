#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class ALightResponseHub;
class ReconstructorMP;
class TH1D;
class TH2D;

class AMercury_si : public AScriptInterface
{
    Q_OBJECT

public:
    AMercury_si();

    AScriptInterface * cloneBase() const override {return new AMercury_si();}

    void abortRun() override {};  // !!!*** TODO

public slots:
    void newReconstructor(QString type, int numThreads);  // 'COG' 'ML' or 'LS'

    void reconstructEvents(QVariantList sensorSignalsOverAllEvents); // add filter optional parameter? just bool, can set it for sat as well?

    QVariantList getRecXYZ();   // status info: what happens with the returned coordinates if the reconstruction fails?
    QVariantList getRecXYZE();
    QVariantList getRecStats(); // [status(0 = OK), chi2, cov_xx, cov_yy, cov_xy]

    void plot(QString what, int bins);
    void plot(QString what, int bins, double from, double to);

    void configure_plotXY_binning(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);
    void plot_vsRecXY(QString what);
    void configure_plotXY_truePositions(QVariantList truePositions);
    void plot_vsTrueXY(QString what);

    // --- Low level ---
    void setCOG_AbsCutoff(double val);
    void setCOG_RelCutoff(double val);
    void setCutoffRadius(double val);
    void setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls);  //  deafults are 0.001, 1000, 500

private:
    ALightResponseHub & LRHub;
    ReconstructorMP   * RecMP = nullptr;

    int    XBins = 50;
    int    YBins = 50;
    double XFrom = 0;
    double XTo   = 0;
    double YFrom = 0;
    double YTo   = 0;

    std::vector<double> XTruePositions, YTruePositions;

    void resetReconstructor();

    enum EPlotOption {ErrorOption, EnergyOption, Chi2Option, StatusOption, DensityOption, BiasXOption, BiasYOption, ErrorXOption, ErrorYOption, EachValidOption};
    EPlotOption whatFromString(QString what);

    void doPlot_vsXY(bool vsTrue, EPlotOption opt, const std::vector<double> & x, const std::vector<double> & y);

    void plotEnergyHist(int bins, double from, double to);
    void plotChi2Hist(int bins, double from, double to);
    void plotStatusHist();
    void plotEnergyXYHist (const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix);
    void plotChi2XYHist   (const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix);
    void plotStatusXYHist (const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix);
    void plotDensityXYHist(const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix);
    void plotBiasXYHist   (const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix, bool vsX);
    void plotSigmaXYHist  (const std::vector<double> & x, const std::vector<double> & y, TH2D * hist, TH2D * histNorm, QString titleSuffix, bool vsX);
};

#endif // AMERCURY_SI_H
