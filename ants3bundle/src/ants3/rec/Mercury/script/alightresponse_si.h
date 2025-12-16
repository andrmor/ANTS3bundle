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

    void abortRun() override {};

public slots:
    // High-level interface
    void newResponseModel(QVariantList sensorPositions);

    void loadResponseModel(QString fileName);
    void saveResponseModel(QString fileName);

    void defineSensorGroups(QString type, int numNodes = 3); // Common, ByRadius, Rectangle, Square, Hexagon, Polygon

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

    void fitResponse(QVariantList floodSignals, QVariantList floodPositions, QVariantList goodEventFlag = QVariantList());

    void showResponseExplorer();
    void showLrfPlotterWidget(QVariantList sensorSignals, QVariantList eventPositions);
    void showLrfPlotterWidget();

    // Low-level interface
    void enableSensor(int iSensor, bool enableFlag);

    int  countSensors();
    int  countGroups();
    QVariantList getGroupMembers(int iGroup);

    void setLrf_Sensor(int iSensor, QString jsonString);     // sets x0 y0 of axial if not present in the lrfjson
    void setLrf_Group(int iGroup, QString jsonString);       // sets x0 y0 of axial if not present in the lrfjson

    void   setSensorGain(int iSensor, double gain);
    double getSensorGain(int iSensor);

    void clearFitData();
    void addFitData(int iSensor, QVariantList amplitudes, QVariantList positions, QVariantList goodEventFlag = QVariantList());
    void computeGroupGains(int iGroup);
    void fitSensor(int iSensor);
    void fitGroup(int iGroup);

    double evaluateLrf(int iSensor, double x, double y, double z);
    double evaluateLrf(int iSensor, QVariantList xyz);

    QString getModel();
    void    setModel(QString jsonString);

private:
    ALightResponseHub & LRHub;

    QString CommonJsonString; // set by SetLRF(QString jsonString) to be used in the case when defineSensorGroups() is triggered after LRFs are already set

    void clearModel();
    void updateLrfOrigin(LRF * lrf, double x, double y);

    bool checkModel();
    bool checkModelAndSensor(int iSensor);
    bool checkModelAndGroup(int iGroup);

};

#endif // ALIGHTRESPONSE_SI_H
