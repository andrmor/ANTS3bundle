#ifndef AMERCURY_SI_H
#define AMERCURY_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QVariantList>

class LRModel;
class Reconstructor;

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
    void createReconstructor_LS();

    void reconstructEvent(QVariantList sensSignals);

    double getPositionX();
    double getPositionY();

    void setCogAbsCutoff(double val);
    void setCogRelCutoff(double val);

    // --- LRFs ---
    void createModel(int numSens);
    void addSensor(int iSens, double x, double y);
    void setLRF(int iSens, QString jsonString);

    void clearAllFitData();
    void addFitData(int iSens, QVariantList xyza);
    void fitSensor(int iSens);

private:
    LRModel * Model = nullptr;
    Reconstructor * Rec = nullptr;

};

#endif // AMERCURY_SI_H
