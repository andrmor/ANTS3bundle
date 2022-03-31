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
    bool afterRun() override;

    std::vector<AGeoObject*> GeoObjects;

public slots:
    void clearWorld();
    void updateGeometry(bool CheckOverlaps = true);

    void box(QString name, double Lx, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void cylinder(QString name, double D, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void Polygone(QString name, int edges, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void Cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void Sphere(QString name, double D, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void SphereLayer(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    void Arb8(QString name, QVariant NodesX, QVariant NodesY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);

    void Monitor(QString name, int shape, double size1, double size2,
                 QString container, double x, double y, double z, double phi, double theta, double psi,
                 bool SensitiveTop, bool SensitiveBottom, bool StopsTraking);
    void Monitor_ConfigureForPhotons(QString MonitorName, QVariant Position, QVariant Time, QVariant Angle, QVariant Wave);
    void Monitor_ConfigureForParticles(QString MonitorName, QString Particle, int Both_Primary_Secondary, int Both_Direct_Indirect,
                                       QVariant Position, QVariant Time, QVariant Angle, QVariant Energy);

    void TGeo(QString name, QString generationString, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);

    void MakeStack(QString name, QString container);  // deprecated!!!
    void Stack(QString name, QString container, double x, double y, double z, double phi, double theta, double psi);
    void InitializeStack(QString StackName, QString MemberName_StackReference);
    // !!!*** add posibility to reshape already exisiting stack

    void Array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double psi);
    void Array(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void CircArray(QString name, int num, double angularStep, double radius, QString container, double x, double y, double z, double phi, double theta, double psi, int startIndex);
    void ReconfigureArray(QString name, int numX, int numY, int numZ, double stepX, double stepY, double stepZ);

    void Prototype(QString name);
    void Instance(QString name, QString prototype, QString container, double x, double y, double z, double phi, double theta, double psi);

    void setLineProperties(QString name, int color, int width, int style);

    void Clear(QString Object);
    void Remove(QString Object);
    void RemoveRecursive(QString Object);

    void EnableObject(QString Object);
    void DisableObject(QString Object);

    void setEnable(QString ObjectOrWildcard, bool flag);

// !!!*** to AMaterial_SI
    /*
    QString getMaterialName(int materialIndex);
    double  getMaterialDensity(int materialIndex);
    QString getMaterialComposition(int materialIndex, bool byWeight);
    */

//    QString printOverrides();

    QVariantList getPassedVoulumes(QVariantList startXYZ, QVariantList startVxVyVz);

signals:
    void requestUpdateGeoGui();

private:
    AGeoObject * World = nullptr;
    const QString ProrotypeContainerName = "_#_Prototype_#_";

    void clearGeoObjects();
};

#endif // AGEO_SI_H
