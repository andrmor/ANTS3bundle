#ifndef AGEO_SI_H
#define AGEO_SI_H

#include "ascriptinterface.h"

#include <vector>

#include <QVariant>  // !!!*** QVariant -> QVariantList or double in methods
#include <QVariantList>
#include <QString>

class AGeoObject;
class DetectorClass;

class AGeo_SI : public AScriptInterface
{
    Q_OBJECT
public:
    AGeo_SI();
    ~AGeo_SI();

    bool beforeRun() override;

    std::vector<AGeoObject*> GeoObjects;

public slots:
    void clearWorld();
    void updateGeometry(bool CheckOverlaps = false);

    void box(QString name, double Lx, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void parallelepiped(QString name, double Lx, double Ly, double Lz, double Alpha, double Theta, double Phi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void trap(QString name, double LXlow, double LXup, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void cylinder(QString name, double D, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void coneSegment(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void pCone(QString name, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void polygon(QString name, int edges, double InscribDiameter, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void sphere(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void sphereSector(QString name, double Dout, double Din, double theta1, double theta2, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void torus(QString name, double D, double Dout, double Din, double Phi, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);

    void composite(QString name, QString compositionString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);

    void toScaled(QString name, double xFactor, double yFactor, double zFactor);

    void monitor(QString name, int shape, double size1, double size2,
                 QString container, double x, double y, double z, double phi, double theta, double psi,
                 bool SensitiveTop, bool SensitiveBottom, bool StopsTraking);
    void configurePhotonMonitor(QString MonitorName, QVariant Position, QVariant Time, QVariant Angle, QVariant Wave);
    void configureParticleMonitor(QString MonitorName, QString Particle, int Both_Primary_Secondary, int Both_Direct_Indirect,
                                  QVariant Position, QVariant Time, QVariant Angle, QVariant Energy);

    void customTGeo(QString name, QString generationString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);

    void stack(QString name, QString container, double x, double y, double z, double phi, double theta, double psi);
    void initializeStack(QString StackName, QString MemberName_StackReference);
    // !!!*** add posibility to reshape already exisiting stack

    void array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void circArray(QString name, int num, double angularStep, double radius, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void hexArray(QString name, int numRings, double pitch, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void hexArray_rectangular(QString name, int numX, int numY, double pitch, bool skipLast, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);

    void prototype(QString name);
    void instance(QString name, QString prototype, QString container, double x, double y, double z, double phi, double theta, double psi);

    void setLineProperties(QString name, int color, int width, int style);

    void clearHosted(QString Object);
    void removeWithHosted(QString Object);

    void setLightSensor(QString Object);

    void setEnabled(QString ObjectOrWildcard, bool flag);

// !!!*** to AMaterial_SI
    /*
    QString getMaterialName(int materialIndex);
    double  getMaterialDensity(int materialIndex);
    QString getMaterialComposition(int materialIndex, bool byWeight);
    */

//    QString printOverrides(); // !!!*** to somewhere

    QVariantList getPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz);

signals:
    void requestUpdateGeoGui();

private:
    AGeoObject * World = nullptr;
    const QString ProrotypeContainerName = "_#_Prototype_#_";

    void clearGeoObjects();
    bool getSectionsPoly(const QVariantList & sections, std::vector<std::array<double, 3> > & vecSections);
};

#endif // AGEO_SI_H
