#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class LRModel;
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
    void reconstructEvents(QVariantList sensorSignalsOverAllEvents);

    //double getPositionX();
    //double getPositionY();
    QVariantList getRecXYZ();
    QVariantList getRecXYZE();

    void setCOG_AbsCutoff(double val);
    void setCOG_RelCutoff(double val);

    // --- LRFs ---
    void newLightResponseModel(int numSensors);       // --> newLightResponseModel(SensorXYs);
    void addSensor(int iSensor, double x, double y);
    void setLRF(int iSensor, QString jsonString);     // --> ?? setLRF_Axial(iSens, n, rmin, rmax)
                                                      //        setCompression(iSens, k, lam, r0)  --> option to all
                                                      //        setConstrains(iSens, non-neg, non-inc, flat]) --> option to all
                                                      //        shiftLRF(iSensor, x, y) - by default at the sensor xy

    void clearAllFitData();
    void addFitData(int iSensor, QVariantList xyza);
    void fitSensor(int iSensor);

    QString exportLightResponseModel();
    void    importLightResponseModel(QString jsonStr);

    void plotLRF_radial(int iSensor, bool showNodes = false);
    void showLightResponseExplorer();

    double eval(int iSensor, double x, double y, double z);
    double eval(int iSensor, QVariantList xyz);

private:
    LRModel * Model = nullptr;
    Reconstructor   * Rec   = nullptr;
    ReconstructorMP * RecMP = nullptr;

    void resetReconstructors();
};

#endif // AMERCURY_SI_H
