#ifndef ALIGHTRESPONSE_SI_H
#define ALIGHTRESPONSE_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class ALightResponseHub;
class LRF;
class TObject;

class ALightResponse_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ALightResponse_SI();

    AScriptInterface * cloneBase() const override {return new ALightResponse_SI();}

    void abortRun() override {};  // !!!*** TODO

public slots:
    // High-level interface
    void newLightResponseModel(QVariantList sensorPositions);

    void importLightResponseModel(QString jsonStr);
    QString exportLightResponseModel();

    void makeSensorGroups(QString type, int numNodes = 3);

    QString newLRF_axial(int intervals, double minR, double maxR);
    QString configureLRF_AxialCompression(QString LRF, double k, double lambda, double r0);
    QString configureLRF_Constrains(QString LRF, bool nonNegative, bool nonIncreasing, bool flattop);
    QString newLRF_xy(int intervalsX, double minX, double maxX, int intervalsY, double minY, double maxY);

    void setLRF(QString jsonString);

    void addFitData(int iSensor, QVariantList xyza);
    void fitResponse(QVariantList floodSignals, QVariantList floodPositions);

    void plotLRF_radial(int iSensor, bool showNodes = false);
    void showLightResponseExplorer();

    // Low-level interface
    void enableSensor(int iSensor, bool enableFlag);

    void clearGroups();
    int  countGroups();

    void setLRF_Sensor(int iSensor, QString jsonString);     // sets x0 y0 of axial if not present in the lrfjson
    void setLRF_Group(int iGroup, QString jsonString);       // sets x0 y0 of axial if not present in the lrfjson

    void         setModelGains(QVariantList gains);
    QVariantList getModelGains();

    void clearFitData();
    void fitSensor(int iSensor);
    void fitGroup(int iGroup);

    double eval(int iSensor, double x, double y, double z);
    double eval(int iSensor, QVariantList xyz);



private:
    ALightResponseHub & LRHub;

    QString CommonJsonString; // set by SetLRF(QString jsonString) to be used in the case when MakeGroups_xxx is used after LRFs are already set

    void clearModel();
    void ifAxialUpdateLrfCenter(LRF * lrf, double x, double y);


};

#endif // ALIGHTRESPONSE_SI_H
