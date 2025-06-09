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
    void clearWorld();
    void updateGeometry(bool CheckOverlaps = false);

    //void box(QString name, double Lx, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void box(QString name, QVariantList fullSizes, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void parallelepiped(QString name, double Lx, double Ly, double Lz, double Alpha, double Theta, double Phi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void parallelepiped(QString name, QVariantList fullSizes, QVariantList angles, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void cylinder(QString name, double outerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void cylinder(QString name, double outerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void coneSegment(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void coneSegment(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void polygon(QString name, int edges, double InscribDiameter, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void polygon(QString name, int edges, double InscribDiameter, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void sphere(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void sphere(QString name, double Dout, double Din, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void sphereSector(QString name, double Dout, double Din, double theta1, double theta2, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void sphereSector(QString name, double Dout, double Din, double theta1, double theta2, double phi1, double phi2, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void composite(QString name, QString compositionString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void composite(QString name, QString compositionString, int iMat, QString container, QVariantList position, QVariantList orientation);
    //void customTGeo(QString name, QString generationString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void customTGeo(QString name, QString generationString, int iMat, QString container, QVariantList position, QVariantList orientation);

    void toScaled(QString name, double xFactor, double yFactor, double zFactor);

    //void monitor(QString name, int shape, double size1, double size2, QString container, double x, double y, double z, double phi, double theta, double psi, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking);
    void monitor(QString name, int shape, double size1, double size2, QString container, QVariantList position, QVariantList orientation, bool SensitiveTop, bool SensitiveBottom, bool StopsTraking);
    void configurePhotonMonitor(QString monitorName, QVariantList position, QVariantList time, QVariantList angle, QVariantList wave);
    void configureParticleMonitor(QString monitorName, QString particle, int both_Primary_Secondary, int both_Direct_Indirect,
                                  QVariantList position, QVariantList time, QVariantList angle, QVariantList energy);

    //void stack(QString name, QString container, double x, double y, double z, double phi, double theta, double psi);
    void stack(QString name, QString container, QVariantList position, QVariantList orientation);
    //void initializeStack(QString StackName, QString MemberName_StackReference); // obsolete

    //void array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double phi, double theta, double psi, bool centerSymmetric, int startIndex);
    void array(QString name, QVariantList numXYZ, QVariantList stepXYZ, QString container, QVariantList position, QVariantList orientation, bool centerSymmetric, int startIndex);
    //void circArray(QString name, int num, double angularStep, double radius, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void circArray(QString name, int num, double angularStep, double radius, QString container, QVariantList position, QVariantList orientation, int startIndex);
    //void hexArray(QString name, int numRings, double pitch, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void hexArray(QString name, int numRings, double pitch, QString container, QVariantList position, QVariantList orientation, int startIndex);
    //void hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipEvenFirst, bool skipOddLast, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipEvenFirst, bool skipOddLast, QString container, QVariantList position, QVariantList orientation, int startIndex);

    void prototype(QString name);
    //void instance(QString name, QString prototype, QString container, double x, double y, double z, double phi, double theta, double psi);
    void instance(QString name, QString prototype, QString container, QVariantList position, QVariantList orientation);

    void setEnabled(QString ObjectOrWildcard, bool flag);

    void setLineProperties(QString name, int color, int width, int style);

    void setLightSensor(QString Object, int iModel = 0);
    void setCalorimeter(QString Object, QVariantList bins, QVariantList origin, QVariantList step);
    void setScintillator(QString Object);
    void setScintillatorByName(QString ObjectNameStartsWith);
    void setSecondaryScintillator(QString Object);

    int countLightSensors();
    QVariantList getLightSensorPositions();

    void setPhotonFunctional(QString Object);
    QVariantMap getDefaultConfigObjectForPhotonFunctionalModel(QString modelName);
    QVariantMap getConfigObjectForPhotonFunctional(int index);
    int countPhotonFunctionals();
    void clearPhotonFunctionalAttribution();
    void configurePhotonFunctional(QString modelName, QVariantMap configObject, int index, int linkedIndex);
    void configurePhotonFunctional(QString modelName, QVariantMap configObject, int index);
    int overrideUnconnectedLinkFunctionals();

    void setParticleAnalyzer(QString object);
    QVariantMap getDefaultParticleAnalyzerProperties();
    QVariantMap getParticleAnalyzerProperties(QString object);
    void configureParticleAnalyzer(QString object, QVariantMap configObject);

    QVariantList getScintillatorProperties();

    void clearHosted(QString Object);
    void removeWithHosted(QString Object);

    void exportToGDML(QString fileName);
    void exportToROOT(QString fileName);

// consider makeing AMaterial_SI and migrate there
    /*
    QString getMaterialName(int materialIndex);
    double  getMaterialDensity(int materialIndex);
    QString getMaterialComposition(int materialIndex, bool byWeight);
    */

    QVariantList trackAndGetPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz);

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
