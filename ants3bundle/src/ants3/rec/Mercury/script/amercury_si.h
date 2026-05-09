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

class AEventFilterRecord
{
public:
    bool SuccessRec = false;

    bool ByEnergy = false;
    double EnergyMin = 0;
    double EnergyMax = 1e99;

    bool ByChi2 = false;
    double Chi2Min = 0;
    double Chi2Max = 1e99;

    void clear();
};

class AMercury_si : public AScriptInterface
{
    Q_OBJECT

public:
    AMercury_si();

    AScriptInterface * cloneBase() const override {return new AMercury_si();}

    void abortRun() override;

public slots:
    void newReconstructor(QString type, int numThreads);  // 'COG' 'ML' or 'LS'

    void reconstructEvents(QVariantList sensorSignalsOverAllEvents);
    void reconstructEvents(QVariantList sensorSignalsOverAllEvents, QVariantList ignoreSensorsByEvent);

    //QVariantList getRecXYZ();
    QVariantList getRecXYZE(); // [x y z energy];   energy = 0 if fail rec
    QVariantList getRecStats(); // [status(0 = OK), chi2, cov_xx, cov_yy, cov_xy]

    void clearEventFilter();
    void setFilterByEnergy(double eMin, double eMax);
    void setFilterByChi2(double chi2Min, double chi2Max);
    int  applyFilter();

    void plot(QString what, int bins, double from, double to);
    void configure_plotXY_binning(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);
    void plot_vsRecXY(QString what);
    void configure_plotXY_truePositions(QVariantList truePositions);
    void plot_vsTrueXY(QString what);

    void showReconstructedPositions(QVariantList XYZE_ofEvents, QVariantList goodEvents = QVariantList());
    void showTruePositions(QVariantList XYZ_ofEvents, QVariantList goodEvents = QVariantList());

    void showEventExplorer(QVariantList sensorSignalsOverAllEvents, QVariantList truePositions = QVariantList());

    // --- Low level ---
    void configure_COG(double signalAbsoluteCutoff, double signalRelativeCutoff, double z0);
    void configure_statistical(bool reconstructEnergy, bool reconstructZ, double fixedZ);
    void setCutoffRadius(double val); // !!!*** set both cog and statistical, include the method for stat (CoG result or strongest sensor)
    void configure_statistical_step(double initialStepX, double initialStepY, double initialStepZ, double initialStepEnergy);  //  defaults are 1 1 1 0
    void configure_statistical_Minuit(double tolerance, int maxIterations, int maxFuncCalls);  //  deafults are 0.001, 1000, 500

private:
    // do not make a reference to script hub as AMercury_si object generation is inside the script hub constructor
    ALightResponseHub & LRHub;
    ReconstructorMP   * RecMP = nullptr;

    std::vector<bool> EventsPassingFilter;
    AEventFilterRecord EventFilter;

    int    XBins = 50;
    int    YBins = 50;
    double XFrom = 0;
    double XTo   = 0;
    double YFrom = 0;
    double YTo   = 0;
    double Z0    = 0;

    std::vector<double> XTruePositions, YTruePositions;

    void resetReconstructor();

    enum EPlotOption {ErrorOption, EnergyOption, Chi2Option, StatusOption, DensityOption, BiasXOption, BiasYOption, ResXOption, ResYOption, EachValidOption};
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
    void plotResXYHist    (const std::vector<double> & x, const std::vector<double> & y, QString titleSuffix, bool vsX);
};

#endif // AMERCURY_SI_H
