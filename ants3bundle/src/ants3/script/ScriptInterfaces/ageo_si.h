#ifndef AGEO_SI_H
#define AGEO_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QVariantList>
#include <QString>

class AGeoObject;
class DetectorClass;
class AGeometryHub;

class AGeo_SI : public AScriptInterface
{
    Q_OBJECT
public:
    AGeo_SI();
    ~AGeo_SI();

    bool beforeRun() override;

    AScriptInterface * cloneBase() const override {return new AGeo_SI();}

    std::vector<AGeoObject*> GeoObjects;

public slots:
    void clearWorld();  // +
    void updateGeometry(bool CheckOverlaps = false);  // +

    // the volume creation methods work with local GeoObjects container, so they are not included in the new json<->hubs system
    // +
    void box(QString name, QVariantList fullSizes, int iMat, QString container, QVariantList position, QVariantList orientation);
    void parallelepiped(QString name, QVariantList fullSizes, QVariantList angles, int iMat, QString container, QVariantList position, QVariantList orientation);
    void trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation);
    void trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation);
    void arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void cylinder(QString name, double outerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, QVariantList position, QVariantList orientation);
    void cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void coneSegment(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    void pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    void polygon(QString name, int edges, double InscribDiameter, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    void pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    void sphere(QString name, double Dout, double Din, int iMat, QString container, QVariantList position, QVariantList orientation);
    void sphereSector(QString name, double Dout, double Din, double theta1, double theta2, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    void torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    void paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    void composite(QString name, QString compositionString, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tesselated(QString name, QVariantList facetArrayXYZs, int iMat, QString container, QVariantList position, QVariantList orientation);
    void tesselated(QString name, QVariantList vertexArray, QVariantList facetArray, int iMat, QString container, QVariantList position, QVariantList orientation);
    void customTGeo(QString name, QString generationString, int iMat, QString container, QVariantList position, QVariantList orientation);

    void toScaled(QString objectName, double xFactor, double yFactor, double zFactor); // +

    void monitor(QString name, int shape, double size1, double size2, QString container, QVariantList position, QVariantList orientation, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking); // +
    void configurePhotonMonitor(QString monitorName, QVariantList position, QVariantList time, QVariantList angle, QVariantList wave); // +
    void configureParticleMonitor(QString monitorName, QString particle, int both_Primary_Secondary, int both_Direct_Indirect,
                                  QVariantList position, QVariantList time, QVariantList angle, QVariantList energy); // +

    void stack(QString name, QString container, QVariantList position, QVariantList orientation); // +
    void setStackReference(QString stackName, QString stackReferenceObjectName); // +

    void array(QString name, QVariantList numXYZ, QVariantList stepXYZ, QString container, QVariantList position, QVariantList orientation, bool centerSymmetric, int startIndex); // +
    void circArray(QString name, int num, double angularStep, double radius, QString container, QVariantList position, QVariantList orientation, int startIndex); // +
    void hexArray(QString name, int numRings, double pitch, QString container, QVariantList position, QVariantList orientation, int startIndex); // +
    void hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipEvenFirst, bool skipOddLast, QString container, QVariantList position, QVariantList orientation, int startIndex); // +

    void prototype(QString name); // +
    void instance(QString name, QString prototype, QString container, QVariantList position, QVariantList orientation); // +   // !!!*** check existence of prototype

    void setEnabled(QString objectNameOrWildcard, bool flag);  // +

    void setLineProperties(QString objectName, int color, int width, int style); // +

    void setLightSensor(QString objectName, int iModel = 0);  // +
    void setLightSensorByName(QString objectNameStartsWith, int iModel = 0); // +

    void setCalorimeter(QString objectName, QVariantList bins, QVariantList origin, QVariantList step); // +
    void setScintillator(QString objectName); // +
    void setScintillatorByName(QString objectNameStartsWith); // +

    void setSecondaryScintillator(QString objectName); // +

    int countLightSensors(); // +
    QVariantList getLightSensorPositions(); // +

    // !!!*** check adding functionals from script
    // !!!*** check auto-generation script
    void setPhotonFunctional(QString objectName); // +
    QVariantMap getDefaultConfigObjectForPhotonFunctionalModel(QString modelName); // +
    QVariantMap getConfigObjectForPhotonFunctional(int index); // +
    int countPhotonFunctionals(); // +
    void clearPhotonFunctionalAttribution(); // +
    void configurePhotonFunctional(QString modelName, QVariantMap configObject, int index, int linkedIndex); // +
    void configurePhotonFunctional(QString modelName, QVariantMap configObject, int index); // +
    int overrideUnconnectedLinkFunctionals(); // +

    void setParticleAnalyzer(QString objectName); // +
    QVariantMap getDefaultParticleAnalyzerProperties(); // +
    QVariantMap getParticleAnalyzerProperties(QString objectName);  // +
    void configureParticleAnalyzer(QString objectName, QVariantMap configObject); // +

    QVariantList getScintillatorProperties(); // +

    void clearHosted(QString objectName); // +
    void removeWithHosted(QString objectName); // +

    double getGeoConstValue(QString name);  // +
    void   setGeoConstValue(QString name, double value);  // +

    void exportToGDML(QString fileName); // +
    void exportToROOT(QString fileName); // +

// consider makeing AMaterial_SI and migrate there
    /*
    QString getMaterialName(int materialIndex);
    double  getMaterialDensity(int materialIndex);
    QString getMaterialComposition(int materialIndex, bool byWeight);
    */

    QVariantList trackAndGetPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz);  // +

signals:
    void requestUpdateGeoGui();

private:
    AGeometryHub & GeoHub;

    const QString ProrotypeContainerName = "_#_Prototype_#_";

    void clearGeoObjects();
    bool getSectionsPoly(const QVariantList & sections, std::vector<std::array<double, 3> > & vecSections);

    AGeoObject * findObject(const QString & Object);
    bool checkPosOri(QVariantList position, QVariantList orientation, std::array<double, 3> &pos, std::array<double, 3> &ori);
};

#endif // AGEO_SI_H
