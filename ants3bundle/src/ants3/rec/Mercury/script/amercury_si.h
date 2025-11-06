#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariantList>

class LRModel;
class Reconstructor;
class ReconstructorMP;

class AMercury_si : public AScriptInterface
{
    Q_OBJECT

public:
    AMercury_si();

    AScriptInterface * cloneBase() const override {return new AMercury_si();}

    void abortRun() override {};  // !!!*** TODO

public slots:
    void createReconstructor_CoG();
    void createReconstructor_ML();
    void createReconstructor_ML_multi(int numThreads);
    void createReconstructor_LS();

    void reconstructEvent(QVariantList  sensSignals);
    void reconstructEvents(QVariantList sensSignalsOverAllEvents);


    double getPositionX();
    double getPositionY();

    QVariantList getReconstructedPositions();

    void setCogAbsCutoff(double val);
    void setCogRelCutoff(double val);

    // --- LRFs ---
    void createModel(int numSens);
    void addSensor(int iSens, double x, double y);
    void setLRF(int iSens, QString jsonString);

    QString writeModel();
    void    readModel(QString jsonStr);

    void clearAllFitData();
    void addFitData(int iSens, QVariantList xyza);
    void fitSensor(int iSens);

    double eval(int iSens, double x, double y, double z);

private:
    LRModel * Model = nullptr;
    Reconstructor   * Rec   = nullptr;
    ReconstructorMP * RecMP = nullptr;

    void resetReconstructors();

};

#endif // AMERCURY_SI_H
