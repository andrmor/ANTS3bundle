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

    void plot(QString what, int bins, double from, double to);

    void configure_plotXY_binning(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);
    void plot_vsRecXY(QString what);
    void configure_plotXY_truePositions(QVariantList truePositions);
    void plot_vsTrueXY(QString what);

    // --- Low level ---
    void configure_COG(double signalAbsoluteCutoff, double signalRelativeCutoff);
    void configure_statistical(bool reconstructEnergy, bool reconstructZ, double fixedZ);
    void setCutoffRadius(double val); // !!!*** set both cog and statistical, include the method for stat (CoG result or strongest sensor)
    void configure_statistical_step(double initialStepX, double initialStepY, double initialStepZ, double initialStepEnergy);  //  defaults are 1 1 1 0
    void configure_statistical_Minuit(double tolerance, int maxIterations, int maxFuncCalls);  //  deafults are 0.001, 1000, 500

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
    TH2D * create2Dhist();
    void plotEnergyXYHist (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix);
    void plotChi2XYHist   (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix);
    void plotStatusXYHist (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix);
    void plotDensityXYHist(const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix);
    void plotBiasXYHist   (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX);
    void plotSigmaXYHist  (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX);
};

#endif // AMERCURY_SI_H
