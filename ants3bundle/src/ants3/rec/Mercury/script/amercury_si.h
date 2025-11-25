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
    void plot_vsRecXY(QString what, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);
    void plot_vsTrueXY(QString what, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo, QVariantList truePositions);

    // --- Low level ---
    void setCOG_AbsCutoff(double val);
    void setCOG_RelCutoff(double val);
    void setCutoffRadius(double val);
    void setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls);  //  deafults are 0.001, 1000, 500

private:
    ALightResponseHub & LRHub;
    ReconstructorMP   * RecMP = nullptr;

    void resetReconstructor();

    enum EPlotOption {ErrorOption, EnergyOption, Chi2Option, StatusOption, DensityOption, BiasXOption, BiasYOption, SigmaXOption, SigmaYOption};
    const QString PlotOptions = "Energy, Chi2, Status, Density, BiasX, BiasY, SigmaX, SigmaY";
    EPlotOption whatFromString(QString what);

    void doPlot_vsXY(bool vsTrue, EPlotOption opt, int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo,
                     const std::vector<double> & x, const std::vector<double> & y);
};

#endif // AMERCURY_SI_H
