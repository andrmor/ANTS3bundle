#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class LRModel;
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

    void setCOG_AbsCutoff(double val);
    void setCOG_RelCutoff(double val);
    void setCutoffRadius(double val);

    //void reconstructEvent(QVariantList  sensSignals);
    void reconstructEvents(QVariantList sensorSignalsOverAllEvents); // add filter optional parameter? just bool, can set it for sat as well?

    //double getPositionX();
    //double getPositionY();
    QVariantList getRecXYZ();   // status info: what happens with the returned coordinates if the reconstruction fails?
    QVariantList getRecXYZE();
    QVariantList getRecStats(); // [status(0 = OK), chi2, cov_xx, cov_yy, cov_xy]


    // --- LRFs ---
    //void newLightResponseModel(int numSensors);       // --> newLightResponseModel(SensorXYs);
    void newLightResponseModel(QVariantList sensorPositions);
    void setLRF(QString jsonString);

    //void addSensor(int iSensor, double x, double y);
    void setLRF_Sensor(int iSensor, QString jsonString);     // sets x0 y0 of axial if not present in the lrfjson
    void setLRF_Group(int iGroup, QString jsonString);     // sets x0 y0 of axial if not present in the lrfjson

    // to discuss:
    // void changeLRFcenter(iSensor, x, y) - by default at the sensor xy

    QString newLRF_axial(int intervals, double rmin, double rmax);
    QString configureLRF_AxialCompression(QString LRF, double k, double lambda, double r0);
    QString configureLRF_Constrains(QString LRF, bool nonNegative, bool nonIncreasing, bool flattop);

    void clearGroups();
    void makeGroups_OneForAllSensors();
    void makeGroups_ByRadius();
    void MakeGroups_RectanglePattern();
    void MakeGroups_SquarePattern();
    void MakeGroups_HexagonPattern();
    void MakeGroups_NgonPattern(int n);
    int  countGroups();

    void clearAllFitData();
    void addFitData(int iSensor, QVariantList xyza);
    void fitResponse(QVariantList floodSignals, QVariantList floodPositions);
    void fitSensor(int iSensor);
    void fitGroup(int iGroup);

    void enableSensor(int iSensor, bool enableFlag);

    void         setModelGains(QVariantList gains);
    QVariantList getModelGains();

    QString exportLightResponseModel();
    void    importLightResponseModel(QString jsonStr);

    void plotLRF_radial(int iSensor, bool showNodes = false);

    void plotChi2(int bins, double from, double to);
    void plotChi2_XY(int xBins, double xFrom, double xTo, int yBins, double yFrom, double yTo);

    void showLightResponseExplorer();

    double eval(int iSensor, double x, double y, double z);
    double eval(int iSensor, QVariantList xyz);

    void setMinuitParameters(double RMtolerance, int RMmaxIterations, int RMmaxFuncCalls);  //  deafults are 0.001, 1000, 500

private:
    LRModel * Model = nullptr;
    Reconstructor   * Rec   = nullptr;
    ReconstructorMP * RecMP = nullptr;

    QString CommonJsonString; // set by SetLRF(QString jsonString) to be used in the case when MakeGroups_xxx is used after LRFs are already set

    void clearModel();
    void resetReconstructors();
    void ifAxialUpdateLrfCenter(LRF * lrf, double x, double y);
};

#endif // AMERCURY_SI_H
