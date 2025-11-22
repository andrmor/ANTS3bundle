#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class ALightResponseHub;
class LRF;
class Reconstructor;
class ReconstructorMP;
class TObject;

class AMercury_si : public AScriptInterface
{
    Q_OBJECT

public:
    AMercury_si();

    AScriptInterface * cloneBase() const override {return new AMercury_si();}

    void abortRun() override {};  // !!!*** TODO

public slots:
    //void createReconstructor_CoG();
    //void createReconstructor_LS();
    //void createReconstructor_ML();
    void createReconstructor_COG_multi(int numThreads);
    void createReconstructor_LS_multi(int numThreads);
    void createReconstructor_ML_multi(int numThreads);

    //void reconstructEvent(QVariantList  sensSignals);
    void reconstructEvents(QVariantList sensorSignalsOverAllEvents); // add filter optional parameter? just bool, can set it for sat as well?

    //double getPositionX();
    //double getPositionY();
    QVariantList getRecXYZ();   // status info: what happens with the returned coordinates if the reconstruction fails?
    QVariantList getRecXYZE();
    QVariantList getRecStats(); // [status(0 = OK), chi2, cov_xx, cov_yy, cov_xy]

    void plotChi2(int bins, double from, double to);
    void plotChi2_XY(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);

    // --- Low level ---
    void setCOG_AbsCutoff(double val);
    void setCOG_RelCutoff(double val);
    void setCutoffRadius(double val);
    void setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls);  //  deafults are 0.001, 1000, 500

    // --- LRFs ---
    //void addSensor(int iSensor, double x, double y);

private:
    ALightResponseHub & LRHub;
    Reconstructor   * Rec   = nullptr;
    ReconstructorMP * RecMP = nullptr;

    void resetReconstructors();

};

#endif // AMERCURY_SI_H
