#ifndef ALIGHTRESPONSE_SI_H
#define ALIGHTRESPONSE_SI_H

#include "ascriptinterface.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class ALightResponseHub;
class LRF;
class TObject;
class ALrfPlotter;

class ALightResponse_SI : public AScriptInterface
{
    Q_OBJECT

public:
    ALightResponse_SI();
    ~ALightResponse_SI();

    AScriptInterface * cloneBase() const override {return new ALightResponse_SI();}

    void abortRun() override {};  // !!!*** TODO

public slots:
    // High-level interface
    void newResponseModel(QVariantList sensorPositions);

    void loadResponseModel(QString fileName);
    void saveResponseModel(QString fileName);

    void defineSensorGroups(QString type, int numNodes = 3);

    QString newLRF_axial(int intervals, double minR, double maxR);
    QString newLRF_axial3D(int intervalsR, double minR, double maxR,
                           int intervalsZ, double minZ, double maxZ);
    QString newLRF_xy(int intervalsX, double minX, double maxX,
                      int intervalsY, double minY, double maxY);
    QString newLRF_xyz(int intervalsX, double minX, double maxX,
                       int intervalsY, double minY, double maxY,
                       int intervalsZ, double minZ, double maxZ);

    QString configureLRF_AxialCompression(QString LRF, double k, double lambda, double r0);
    QString configureLRF_Constrains(QString LRF, bool nonNegative, bool nonIncreasing, bool flattop);

    void setLRF(QString jsonString);

    void addFitData(int iSensor, QVariantList xyza);
    void fitResponse(QVariantList floodSignals, QVariantList floodPositions);

    void plotLRF_radial(int iSensor, bool showNodes = false);
    void plotLRF_xy(int iSensor);
    void configure_plotLRF(bool plotWithSensorData, QVariantList sensorSignals, QVariantList eventPositions);

    void showResponseExplorer();

    // Low-level interface
    void enableSensor(int iSensor, bool enableFlag);

    void clearGroups();
    int  countGroups();
    QVariantList getGroupMembers(int iGroup);

    void setLRF_Sensor(int iSensor, QString jsonString);     // sets x0 y0 of axial if not present in the lrfjson
    void setLRF_Group(int iGroup, QString jsonString);       // sets x0 y0 of axial if not present in the lrfjson

    void         setModelGains(QVariantList gains);
    QVariantList getModelGains();

    void clearFitData();
    void fitSensor(int iSensor);
    void fitGroup(int iGroup);

    double eval(int iSensor, double x, double y, double z);
    double eval(int iSensor, QVariantList xyz);

    QString getModel();
    void    setModel(QString jsonString);

private:
    ALightResponseHub & LRHub;
    ALrfPlotter       * LrfPlotter = nullptr;

    QString CommonJsonString; // set by SetLRF(QString jsonString) to be used in the case when MakeGroups_xxx is used after LRFs are already set

    void clearModel();
    void ifAxialUpdateLrfCenter(LRF * lrf, double x, double y);


};

#endif // ALIGHTRESPONSE_SI_H
