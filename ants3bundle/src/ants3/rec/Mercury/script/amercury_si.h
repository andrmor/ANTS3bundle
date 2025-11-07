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
    void createModel(int numSensors);
    void addSensor(int iSensor, double x, double y);
    void setLRF(int iSensor, QString jsonString);

    QString writeModel();
    void    readModel(QString jsonStr);

    void clearAllFitData();
    void addFitData(int iSensor, QVariantList xyza);
    void fitSensor(int iSensor);

    void plotLRF_radial(int iSensor, bool showNodes = false);
    double eval(int iSensor, double x, double y, double z);

private:
    LRModel * Model = nullptr;
    Reconstructor   * Rec   = nullptr;
    ReconstructorMP * RecMP = nullptr;

    void resetReconstructors();

};

#endif // AMERCURY_SI_H
