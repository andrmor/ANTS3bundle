#include "ageoshape.h"
#include "ajsontools.h"
#include "ageoconsts.h"

#include <QDebug>
#include <QRegularExpression>

#include "TGeoBBox.h"
#include "TGeoShape.h"
#include "TGeoPara.h"
#include "TGeoSphere.h"
#include "TGeoTube.h"
#include "TGeoTrd1.h"
#include "TGeoTrd2.h"
#include "TGeoPgon.h"
#include "TGeoCone.h"
#include "TGeoParaboloid.h"
#include "TGeoEltu.h"
#include "TGeoArb8.h"
#include "TGeoCompositeShape.h"
#include "TGeoScaledShape.h"
#include "TGeoMatrix.h"
#include "TGeoTorus.h"

#include <math.h>

AGeoShape * AGeoShape::GeoShapeFactory(const QString ShapeType)
{
    if      (ShapeType == "TGeoBBox")
        return new AGeoBox();
    else if (ShapeType == "TGeoPara")
        return new AGeoPara();
    else if (ShapeType == "TGeoSphere")
        return new AGeoSphere();
    else if (ShapeType == "TGeoTube")
        return new AGeoTube();
    else if (ShapeType == "TGeoTubeSeg")
        return new AGeoTubeSeg();
    else if (ShapeType == "TGeoCtub")
        return new AGeoCtub();
    else if (ShapeType == "TGeoEltu")
        return new AGeoEltu();
    else if (ShapeType == "TGeoTrd1")
        return new AGeoTrd1();
    else if (ShapeType == "TGeoTrd2")
        return new AGeoTrd2();
    else if (ShapeType == "TGeoPgon")
        return new AGeoPgon();
    else if (ShapeType == "TGeoPolygon")
        return new AGeoPolygon();
    else if (ShapeType == "TGeoCone")
        return new AGeoCone();
    else if (ShapeType == "TGeoConeSeg")
        return new AGeoConeSeg();
    else if (ShapeType == "TGeoPcon")
        return new AGeoPcon();
    else if (ShapeType == "TGeoParaboloid")
        return new AGeoParaboloid();
    else if (ShapeType == "TGeoArb8")
        return new AGeoArb8();
    else if (ShapeType == "TGeoTorus")
        return new AGeoTorus();
    else if (ShapeType == "TGeoCompositeShape")
        return new AGeoComposite();
    else if (ShapeType == "TGeoScaledShape")
        return new AGeoScaledShape();
    else return nullptr;
}

/*
QList<AGeoShape *> AGeoShape::getAvailableShapes()
{
    QList<AGeoShape *> list;
    list << new AGeoBox << new AGeoPara << new AGeoSphere
         << new AGeoTube << new AGeoTubeSeg << new AGeoCtub << new AGeoEltu
         << new AGeoTrd1 << new AGeoTrd2
         << new AGeoCone << new AGeoConeSeg << new AGeoPcon
         << new AGeoPolygon << new AGeoPgon
         << new AGeoParaboloid << new AGeoTorus
         << new AGeoArb8 << new AGeoComposite
         << new AGeoScaledShape;
    return list;
}
*/

// ----------------------------

AGeoShape * AGeoShape::clone() const
{
    AGeoShape * sh = AGeoShape::GeoShapeFactory(getShapeType());
    if (!sh)
    {
        qWarning() << "Failed to clone AGeoShape";
        return nullptr;
    }
    QJsonObject json;
    writeToJson(json);
    sh->readFromJson(json);
    return sh;
}

bool AGeoShape::extractParametersFromString(QString GenerationString, QStringList &parameters, int numParameters)
{
    GenerationString = GenerationString.simplified();
    if (!GenerationString.startsWith(getShapeType()))
    {
        qWarning() << "Attempt to generate shape using wrong type!";
        return false;
    }
    GenerationString.remove(getShapeType());
    GenerationString = GenerationString.simplified();
    if (GenerationString.endsWith("\\)"))
    {
        qWarning() << "Format error in shape read from string";
        return false;
    }
    GenerationString.chop(1);
    if (GenerationString.startsWith("\\("))
    {
        qWarning() << "Format error in shape read from string";
        return false;
    }
    GenerationString.remove(0, 1);
    GenerationString.remove(QString(" "));

    parameters = GenerationString.split(",", Qt::SkipEmptyParts);

    if (numParameters == -1) return true;

    if (parameters.size() != numParameters)
    {
        qWarning() << "Wrong number of parameters!";
        return false;
    }
    return true;
}

// ====== BOX ======
bool AGeoBox::readFromString(QString GenerationString)
{
    qDebug() <<"bbox readfrom string: generation str" <<GenerationString;
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 3);
    if (!ok) return false;
    qDebug() <<"bbox readfrom string: params" <<params;

    double tmp[3];
    for (int i=0; i<3; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoBox";
            return false;
        }
    }

    dx = tmp[0];
    dy = tmp[1];
    dz = tmp[2];
    qDebug() <<"bbox readfrom string: ds" <<dx <<dy <<dz;
    return true;
}

QString AGeoBox::getHelp() const
{
    return "A box has 3 parameters: dx, dy, dz representing the half-lengths on X, Y and Z axes.\n"
           "The box will range from: -dx to dx on X-axis, from -dy to dy on Y and from -dz to dz on Z.";
}

void AGeoBox::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2dx, dx); if (!ok) errorStr += " in X size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dy, dy); if (!ok) errorStr += " in Y size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz); if (!ok) errorStr += " in Z size\n";
}

bool AGeoBox::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (str2dx.contains(nameRegExp)) return true;
    if (str2dy.contains(nameRegExp)) return true;
    if (str2dz.contains(nameRegExp)) return true;
    return false;
}

void AGeoBox::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    str2dx.replace(nameRegExp, newName);
    str2dy.replace(nameRegExp, newName);
    str2dz.replace(nameRegExp, newName);
}

QString AGeoBox::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoBBox( " +
                QString::number(dx) + ", " +
                QString::number(dy) + ", " +
                QString::number(dz) + " )";
    }
    else
    {
        QString sdx = ( str2dx.isEmpty() ? QString::number(dx) : "' + 0.5*(" + str2dx + ") + '" );
        QString sdy = ( str2dy.isEmpty() ? QString::number(dy) : "' + 0.5*(" + str2dy + ") + '" );
        QString sdz = ( str2dz.isEmpty() ? QString::number(dz) : "' + 0.5*(" + str2dz + ") + '" );
        str = "TGeoBBox( " +
                        sdx + ", " +
                        sdy + ", " +
                        sdz + " )";
    }
    return str;
}

QString AGeoBox::getScriptString(bool useStrings) const
{
    QString sdx, sdy, sdz;
    if (useStrings)
    {
        sdx = ( str2dx.isEmpty() ? QString::number(2.0 * dx) : str2dx );
        sdy = ( str2dy.isEmpty() ? QString::number(2.0 * dy) : str2dy );
        sdz = ( str2dz.isEmpty() ? QString::number(2.0 * dz) : str2dz );
    }
    else
    {
        sdx = QString::number(2.0 * dx);
        sdy = QString::number(2.0 * dy);
        sdz = QString::number(2.0 * dz);
    }

    //void box(QString name, double Lx, double Ly, double Lz, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    return QString("geo.box( $name$,  [%0, %1, %2],  ").arg(sdx, sdy, sdz);
}

double AGeoBox::maxSize() const
{
    double m = std::max(dx, dy);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

double AGeoBox::minSize() const
{
    return std::min(dx, dy);
}

TGeoShape *AGeoBox::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoBBox(dx, dy, dz) : new TGeoBBox(shapeName.toLatin1().data(), dx, dy, dz);
}

void AGeoBox::writeToJson(QJsonObject &json) const
{
    json["dx"] = dx;
    json["dy"] = dy;
    json["dz"] = dz;

    if (!str2dx.isEmpty()) json["str2dx"] = str2dx;
    if (!str2dy.isEmpty()) json["str2dy"] = str2dy;
    if (!str2dz.isEmpty()) json["str2dz"] = str2dz;
}

void AGeoBox::readFromJson(const QJsonObject &json)
{
    dx = json["dx"].toDouble();
    dy = json["dy"].toDouble();
    dz = json["dz"].toDouble();

    if (!jstools::parseJson(json, "str2dx", str2dx)) str2dx.clear();
    if (!jstools::parseJson(json, "str2dy", str2dy)) str2dy.clear();
    if (!jstools::parseJson(json, "str2dz", str2dz)) str2dz.clear();
}

bool AGeoBox::readFromTShape(TGeoShape *Tshape)
{
    TGeoBBox* Tbox = dynamic_cast<TGeoBBox*>(Tshape);
    if (!Tbox) return false;

    dx = Tbox->GetDX();
    dy = Tbox->GetDY();
    dz = Tbox->GetDZ();

    return true;
}

void AGeoBox::scale(double factor)
{
    dx *= factor;
    dy *= factor;
    dz *= factor;
}

// ====== PARA ======
QString AGeoPara::getHelp() const
{
    return "A parallelepiped is a shape having 3 pairs of parallel faces out of which one is parallel with the XY plane (Z"
           " faces). All faces are parallelograms in the general case. The Z faces have 2 edges parallel with the X-axis.\n"
           "The shape has the center in the origin and it is defined by:\n"
           " • dX, dY, dZ: half-lengths of the projections of the edges on X, Y and Z. The lower Z face is "
           "positioned at -dZ, while the upper at +dZ.\n"
           " • alpha: angle between the segment defined by the centers of the X-parallel edges and Y axis [-90,90] in degrees\n"
           " • theta: theta angle of the segment defined by the centers of the Z faces;\n"
           " • phi: phi angle of the same segment";
}

void AGeoPara::introduceGeoConstValues(QString & errorStr)
{
    bool ok;
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, str2dx,   dx);                         if (!ok) errorStr += " in X size\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, str2dy,   dy);                         if (!ok) errorStr += " in Y size\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, str2dz,   dz);                         if (!ok) errorStr += " in Z size\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strAlpha, alpha, false, false, false); if (!ok) errorStr += " in Alpha\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strTheta, theta, false, false, false); if (!ok) errorStr += " in Theta\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strPhi,   phi, false, false, false);   if (!ok) errorStr += " in Phi\n";

    if (-90.0 >= alpha || alpha >= 90.0) errorStr += "alpha must be between -90 and 90\n";
    if (-90.0 >= theta || theta >= 90.0) errorStr += "theta must be between -90 and 90\n";
}

bool AGeoPara::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2dx.contains(nameRegExp))   return true;
    if (str2dy.contains(nameRegExp))   return true;
    if (str2dz.contains(nameRegExp))   return true;
    if (strAlpha.contains(nameRegExp)) return true;
    if (strTheta.contains(nameRegExp)) return true;
    if (strPhi.contains(nameRegExp))   return true;
    return false;
}

void AGeoPara::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2dx  .replace(nameRegExp, newName);
    str2dy  .replace(nameRegExp, newName);
    str2dz  .replace(nameRegExp, newName);
    strAlpha.replace(nameRegExp, newName);
    strTheta.replace(nameRegExp, newName);
    strPhi  .replace(nameRegExp, newName);
}

bool AGeoPara::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 6);
    if (!ok) return false;

    double tmp[6];
    for (int i=0; i<6; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoPara";
            return false;
        }
    }

    dx = tmp[0];
    dy = tmp[1];
    dz = tmp[2];
    alpha = tmp[3];
    theta = tmp[4];
    phi = tmp[5];
    return true;
}

TGeoShape *AGeoPara::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoPara(dx, dy, dz, alpha, theta, phi) : new TGeoPara(shapeName.toLatin1().data(), dx, dy, dz, alpha, theta, phi);
}

QString AGeoPara::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoPara( " +
                QString::number(dx)+", "+
                QString::number(dy)+", "+
                QString::number(dz)+", "+
                QString::number(alpha)+", "+
                QString::number(theta)+", "+
                QString::number(phi)+" )";
    }
    else
    {
        QString sdx    = (str2dx  .isEmpty() ? QString::number(dx)    : "' + 0.5*(" + str2dx    + ") + '");
        QString sdy    = (str2dy  .isEmpty() ? QString::number(dy)    : "' + 0.5*(" + str2dy    + ") + '");
        QString sdz    = (str2dz  .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz    + ") + '");
        QString salpha = (strAlpha.isEmpty() ? QString::number(alpha) : "' + ("      + strAlpha   + ") + '");
        QString stheta = (strTheta.isEmpty() ? QString::number(theta) : "' + ("      + strTheta   + ") + '");
        QString sphi   = (strPhi  .isEmpty() ? QString::number(phi)   : "' + ("      + strPhi     +") + '");

        str = "TGeoPara( " +
                sdx    + ", "+
                sdy    + ", "+
                sdz    + ", "+
                salpha + ", "+
                stheta + ", "+
                sphi   + " )";
    }
    return str;
}

QString AGeoPara::getScriptString(bool useStrings) const
{
    QString sdx, sdy, sdz, sAlpha, sTheta, sPhi;
    if (useStrings)
    {
        sdx    = (str2dx  .isEmpty() ? QString::number(2.0 * dx)    : str2dx);
        sdy    = (str2dy  .isEmpty() ? QString::number(2.0 * dy)    : str2dy);
        sdz    = (str2dz  .isEmpty() ? QString::number(2.0 * dz)    : str2dz);
        sAlpha = (strAlpha.isEmpty() ? QString::number(alpha) : strAlpha);
        sTheta = (strTheta.isEmpty() ? QString::number(theta) : strTheta);
        sPhi   = (strPhi  .isEmpty() ? QString::number(phi)   : strPhi);
    }
    else
    {
        sdx    = QString::number(2.0 * dx);
        sdy    = QString::number(2.0 * dy);
        sdz    = QString::number(2.0 * dz);
        sAlpha = QString::number(alpha);
        sTheta = QString::number(theta);
        sPhi   = QString::number(phi);
    }

    return QString("geo.parallelepiped( $name$, [%0, %1, %2], [%3, %4, %5],  ").arg(sdx, sdy, sdz, sAlpha, sTheta, sPhi);
}

double AGeoPara::maxSize() const
{
    double m = std::max(dx, dy);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoPara::writeToJson(QJsonObject &json) const
{
    json["dx"]    = dx;
    json["dy"]    = dy;
    json["dz"]    = dz;
    json["alpha"] = alpha;
    json["theta"] = theta;
    json["phi"]   = phi;

    if (!str2dx  .isEmpty()) json["str2dx"]   = str2dx;
    if (!str2dy  .isEmpty()) json["str2dy"]   = str2dy;
    if (!str2dz  .isEmpty()) json["str2dz"]   = str2dz;
    if (!strAlpha.isEmpty()) json["strAlpha"] = strAlpha;
    if (!strTheta.isEmpty()) json["strTheta"] = strTheta;
    if (!strPhi  .isEmpty()) json["strPhi"]   = strPhi;
}

void AGeoPara::readFromJson(const QJsonObject &json)
{
    dx    = json["dx"]   .toDouble();
    dy    = json["dy"]   .toDouble();
    dz    = json["dz"]   .toDouble();
    alpha = json["alpha"].toDouble();
    theta = json["theta"].toDouble();
    phi   = json["phi"]  .toDouble();

    if (!jstools::parseJson(json, "str2dx",   str2dx))   str2dx  .clear();
    if (!jstools::parseJson(json, "str2dy",   str2dy))   str2dy  .clear();
    if (!jstools::parseJson(json, "str2dz",   str2dz))   str2dz  .clear();
    if (!jstools::parseJson(json, "strAlpha", strAlpha)) strAlpha.clear();
    if (!jstools::parseJson(json, "strTheta", strTheta)) strTheta.clear();
    if (!jstools::parseJson(json, "strPhi",   strPhi))   strPhi  .clear();
}

bool AGeoPara::readFromTShape(TGeoShape *Tshape)
{
    TGeoPara* p = dynamic_cast<TGeoPara*>(Tshape);
    if (!p) return false;

    dx = p->GetDX();
    dy = p->GetDY();
    dz = p->GetDZ();

    alpha = p->GetAlpha();
    theta = p->GetTheta();
    phi   = p->GetPhi();

    dy = dy - dz * fabs(p->GetTyz());
    dx = dx - dy * fabs(p->GetTxy()) - dz * fabs(p->GetTxz());

    return true;
}

void AGeoPara::scale(double factor)
{
    dx *= factor;
    dy *= factor;
    dz  *= factor;
}

AGeoComposite::AGeoComposite(const QStringList members, QString GenerationString) :
    members(members), GenerationString(GenerationString)
{
    //qDebug() << "new composite!";
    //qDebug() << members;
    //qDebug() << GenerationString;
}

QString AGeoComposite::getHelp() const
{
    return "Composite shapes are Boolean combinations of two or more shape components. The supported Boolean "
           "operations are union (+), intersection (*) and subtraction(-).";
}

#include <QSet>
bool AGeoComposite::readFromString(QString GenerationString)
{
    //qDebug() << "Number of defined members:"<<members.size();
    QString s = GenerationString.simplified();

    s.remove("TGeoCompositeShape");
    s.remove("(");
    s.remove(")");
    QStringList requested = s.split(QRegularExpression("[+*-]"));
    for (int i=0; i<requested.size(); i++) requested[i] = requested[i].simplified();
    //qDebug() << "Requested members in composite generation:"<<requested;
    QSet<QString> SetOfRequested(requested.begin(), requested.end());
    QSet<QString> SetOfMembers(members.begin(), members.end());
    //qDebug() << "Members:"<<SetOfMembers;
    //qDebug() << "Requested:"<<SetOfRequested;
    //qDebug() << "Memebers contain requested?"<<SetOfMembers.contains(SetOfRequested);
    if (!SetOfMembers.contains(SetOfRequested)) return false;

    this->GenerationString = GenerationString.simplified();
    return true;
}

TGeoShape *AGeoComposite::createGeoShape(const QString shapeName)
{
    //qDebug() << "---Create TGeoComposite triggered!";
    //qDebug() << "---Members registered:"<<members;
    QString s = GenerationString;
    s.remove("TGeoCompositeShape(");
    s.chop(1);

    //qDebug() << "--->Gen string before:"<<s;
    //qDebug() << "--->Memebers:"<<members;
    for (int i=0; i<members.size(); i++)
    {
        QString mem = members[i];
        QRegularExpression toReplace("\\b"+mem+"\\b(?!:)");
        QString memMod = mem+":_m"+mem;
        s.replace(toReplace, memMod);
    }
    //qDebug() << "--->Str to be used in composite generation:"<<s;

    return (shapeName.isEmpty()) ? new TGeoCompositeShape(s.toLatin1().data()) : new TGeoCompositeShape(shapeName.toLatin1().data(), s.toLatin1().data());
}

QString AGeoComposite::getScriptString(bool) const
{
    QString s = GenerationString.simplified(); // e.g. "TGeoCompositeShape( (A + B) * (C - D) )"
    s.remove("TGeoCompositeShape(");
    s.chop(1);

    //void composite(QString name, QString compositionString,
    return QString("geo.composite( $name$,  \"%0\",  ").arg(s.simplified());
}

void AGeoComposite::writeToJson(QJsonObject &json) const
{
    json["GenerationString"] = GenerationString;
}

void AGeoComposite::readFromJson(const QJsonObject &json)
{
    GenerationString = json["GenerationString"].toString();
}

QString AGeoSphere::getHelp() const
{
    return "TSphere is a spherical sector with the following parameters:\n"
           " • rmin: internal radius\n"
           " • rmax: external radius\n"
           " • theta1: starting theta value [0, 180) in degrees\n"
           " • theta2: ending theta value (0, 180] in degrees (theta1<theta2)\n"
           " • phi1: starting phi value [0, 360) in degrees\n"
           " • phi2: ending phi value (0, 360] in degrees (phi1<phi2)";
}

void AGeoSphere::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2rmax,  rmax);                         if (!ok) errorStr += " in Outer Diameter\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmin,  rmin,   false);                if (!ok) errorStr += " in Inner Diameter\n";
    ok = GC.updateDoubleParameter(errorStr, strTheta1, theta1, false, false,  false); if (!ok) errorStr += " in Theta1\n";
    ok = GC.updateDoubleParameter(errorStr, strTheta2, theta2, false, false,  false); if (!ok) errorStr += " in Theta2\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi1,   phi1,   false, false, false);  if (!ok) errorStr += " in Phi1\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi2,   phi2,   false, false, false);  if (!ok) errorStr += " in Phi2\n";

    if (rmin   >= rmax)   errorStr += "Inside diameter should be smaller than the outside one!\n";
    if (theta1 >= theta2) errorStr += "Theta2 should be larger than Theta1\n";
    if (phi1   >= phi2)   errorStr += "Phi2 should be larger than Phi1\n";

    if (theta1 <  0 || theta1 >= 180.0) errorStr += "Theta1 should be in the range of [0, 180)\n";
    if (theta2 <= 0 || theta2 >  180.0) errorStr += "Theta2 should be in the range of (0, 180]\n";
    if (phi1   <  0 || phi1   >= 360.0) errorStr += "Phi1 should be in the range of [0, 360)\n";
    if (phi2   <= 0 || phi2   >  360.0) errorStr += "Phi2 should be in the range of (0, 360]\n";
}

bool AGeoSphere::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2rmin .contains(nameRegExp)) return true;
    if (str2rmax .contains(nameRegExp)) return true;
    if (strTheta1.contains(nameRegExp)) return true;
    if (strTheta2.contains(nameRegExp)) return true;
    if (strPhi1  .contains(nameRegExp)) return true;
    if (strPhi2  .contains(nameRegExp)) return true;

    return false;
}

void AGeoSphere::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2rmin .replace(nameRegExp, newName);
    str2rmax .replace(nameRegExp, newName);
    strTheta1.replace(nameRegExp, newName);
    strTheta2.replace(nameRegExp, newName);
    strPhi1  .replace(nameRegExp, newName);
    strPhi2  .replace(nameRegExp, newName);
}

bool AGeoSphere::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 6);
    if (!ok) return false;

    double tmp[6];
    for (int i=0; i<6; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoSphere";
            return false;
        }
    }

    rmin = tmp[0];
    rmax = tmp[1];
    theta1 = tmp[2];
    theta2 = tmp[3];
    phi1 = tmp[4];
    phi2 = tmp[5];
    return true;
}

TGeoShape *AGeoSphere::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoSphere(rmin, rmax, theta1, theta2, phi1,  phi2) : new TGeoSphere(shapeName.toLatin1().data(), rmin, rmax, theta1, theta2, phi1,  phi2);
}

void AGeoSphere::computeZupZdown(double & Zup, double & Zdown) const
{
    if (theta2 <= 90) // both less= 90
    {
        if (theta1 == 0)
            Zup = rmax;
        else
            Zup = rmax * cos(theta1 * M_PI / 180.0);

        if (theta2 == 90.0 || rmin == 0)
            Zdown = 0;
        else
            Zdown = rmin * cos(theta2 * M_PI / 180.0);
    }
    else if (theta1 < 90.0 && theta2 > 90) // first less than 90, the second is larger than 90
    {
        if (theta1 == 0)
            Zup = rmax;
        else
            Zup = rmax * cos(theta1 * M_PI / 180.0);

        if (theta2 == 180.0)
            Zdown = -rmax;
        else
            Zdown = -rmax * cos( (180.0 - theta2) * M_PI / 180.0);
    }
    else // both above= 90
    {
        if (theta1 == 90.0 || rmin == 0)
            Zup = 0;
        else
            Zup = -rmin * cos( (180.0 - theta1) * M_PI / 180.0);

        if (theta2 == 180.0)
            Zdown = -rmax;
        else
            Zdown = -rmax * cos( (180.0 - theta2) * M_PI / 180.0);
    }
}

double AGeoSphere::getHeight() const
{
    if (theta1 == 0 && theta2 == 180) return rmax;

    double Zup = 0;
    double Zdown = 0;
    computeZupZdown(Zup, Zdown);

    return 0.5 * (Zup - Zdown);
}

QString AGeoSphere::getFullHeightString()
{
    return "";

    /*
    // bad idea: the formula changes depending on the parameter values!
    if (str2rmax.isEmpty() && strTheta1.isEmpty() && strTheta2.isEmpty()) return "";

    QString up, down;

    QString R = (str2rmax.isEmpty() ? QString::number(rmax) : "0.5*"+str2rmax);
    QString r = (str2rmin.isEmpty() ? QString::number(rmin) : "0.5*"+str2rmin);

    QString t1 = (strTheta1.isEmpty() ? QString::number(theta1) : strTheta1);
    QString t2 = (strTheta2.isEmpty() ? QString::number(theta2) : strTheta2);

    if (theta2 <= 90) // both less= 90
    {
        if (theta1 == 0)
            up = R;
        else
            up = R + "*cos("+t1+"*pi/180.0)";

        if (theta2 == 90.0 || rmin == 0)
            down = "0";
        else
            down = r + "*cos("+t2+"*pi/180.0)";
    }
    else if (theta1 < 90.0 && theta2 > 90) // first less than 90, the second is larger than 90
    {
        if (theta1 == 0)
            up = R;
        else
            up = R + "*cos("+t1+"*pi/180.0)";

        if (theta2 == 180.0)
            down = "-"+R;
        else
            down = "-" + R + "*cos((180.0-"+t2+")*pi/180.0)";
    }
    else // both above= 90
    {
        if (theta1 == 90.0 || rmin == 0)
            up = "0";
        else
            up = "-" + r + "*cos((180.0-" + t1 + ")*pi/180.0)";

        if (theta2 == 180.0)
            down = "-" + R;
        else
            down = "-" + R + "*cos((180.0-" + t2 + ")*pi/180.0)";
    }
    return QString("(%0-%1)").arg(up).arg(down);
    */
}

double AGeoSphere::getRelativePosZofCenter() const
{
    if (theta1 == 0 && theta2 == 180) return 0;

    double Zup = 0;
    double Zdown = 0;
    computeZupZdown(Zup, Zdown);

    return 0.5 * (Zup + Zdown);
}

QString AGeoSphere::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoSphere( " +
                QString::number(rmin)+", "+
                QString::number(rmax)+", "+
                QString::number(theta1)+", "+
                QString::number(theta2)+", "+
                QString::number(phi1)+", "+
                QString::number(phi2)+" )";
    }
    else
    {
        QString srmin   = (str2rmin .isEmpty() ? QString::number(rmin)  : "' + 0.5*(" + str2rmin + ") + '");
        QString srmax   = (str2rmax .isEmpty() ? QString::number(rmax)  : "' + 0.5*(" + str2rmax + ") + '");
        QString stheta1 = (strTheta1.isEmpty() ? QString::number(theta1): "' + ("     + strTheta1   + ") + '");
        QString stheta2 = (strTheta2.isEmpty() ? QString::number(theta2): "' + ("     + strTheta2   + ") + '");
        QString sphi1   = (strPhi1  .isEmpty() ? QString::number(phi1)  : "' + ("     + strPhi1   + ") + '");
        QString sphi2   = (strPhi2  .isEmpty() ? QString::number(phi2)  : "' + ("     + strPhi2   + ") + '");

        str = "TGeoSphere( " +
                srmin   + ", "+
                srmax   + ", "+
                stheta1 + ", "+
                stheta2 + ", "+
                sphi1   + ", "+
                sphi2   + " )";
    }
    return str;
}

QString AGeoSphere::getScriptString(bool useStrings) const
{
    QString srmin;
    QString srmax;
    QString sthe1;
    QString sthe2;
    QString sphi1;
    QString sphi2;

    if (useStrings)
    {
        srmin = ( str2rmin.isEmpty()  ? QString::number(2.0 * rmin) : str2rmin );
        srmax = ( str2rmax.isEmpty()  ? QString::number(2.0 * rmax) : str2rmax );
        sthe1 = ( strTheta1.isEmpty() ? QString::number(theta1)     : strTheta1 );
        sthe2 = ( strTheta2.isEmpty() ? QString::number(theta2)     : strTheta2 );
        sphi1 = ( strPhi1.isEmpty()   ? QString::number(phi1)       : strPhi1 );
        sphi2 = ( strPhi2.isEmpty()   ? QString::number(phi2)       : strPhi2 );
    }
    else
    {
        srmin = QString::number(2.0 * rmin);
        srmax = QString::number(2.0 * rmax);
        sthe1 = QString::number(theta1);
        sthe2 = QString::number(theta2);
        sphi1 = QString::number(phi1);
        sphi2 = QString::number(phi2);
    }

    if (theta1 == 0 && theta2 == 180.0 && phi1 == 0 && phi2 == 360.0)
    {
        //void sphere(QString name, double Dout, double Din, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.sphere( $name$,  %0, %1,  ").arg(srmax, srmin);
    }
    else
    {
        //void AGeo_SI::sphereSector(QString name, double Dout, double Din, double Theta1, double Theta2, double Phi1, double Phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
        return QString("geo.sphereSector( $name$,  %0, %1, %2, %3, %4, %5,  ").arg(srmax, srmin, sthe1, sthe2, sphi1, sphi2);
    }
}

void AGeoSphere::writeToJson(QJsonObject &json) const
{
    json["rmin"]   = rmin;
    json["rmax"]   = rmax;
    json["theta1"] = theta1;
    json["theta2"] = theta2;
    json["phi1"]   = phi1;
    json["phi2"]   = phi2;

    if (!str2rmin. isEmpty()) json["str2rmin"]  = str2rmin;
    if (!str2rmax. isEmpty()) json["str2rmax"]  = str2rmax;
    if (!strTheta1.isEmpty()) json["strTheta1"] = strTheta1;
    if (!strTheta2.isEmpty()) json["strTheta2"] = strTheta2;
    if (!strPhi1.  isEmpty()) json["strPhi1"]   = strPhi1;
    if (!strPhi2.  isEmpty()) json["strPhi2"]   = strPhi2;
}

void AGeoSphere::readFromJson(const QJsonObject &json)
{
    rmin   = json["rmin"].  toDouble();
    rmax   = json["rmax"].  toDouble();
    theta1 = json["theta1"].toDouble();
    theta2 = json["theta2"].toDouble();
    phi1   = json["phi1"].  toDouble();
    phi2   = json["phi2"].  toDouble();

    if (!jstools::parseJson(json, "str2rmin",  str2rmin))  str2rmin.clear();
    if (!jstools::parseJson(json, "str2rmax",  str2rmax))  str2rmax.clear();
    if (!jstools::parseJson(json, "strTheta1", strTheta1)) strTheta1.clear();
    if (!jstools::parseJson(json, "strTheta2", strTheta2)) strTheta2.clear();
    if (!jstools::parseJson(json, "strPhi1",   strPhi1))   strPhi1.clear();
    if (!jstools::parseJson(json, "strPhi2",   strPhi2))   strPhi2.clear();
}

bool AGeoSphere::readFromTShape(TGeoShape *Tshape)
{
    TGeoSphere* s = dynamic_cast<TGeoSphere*>(Tshape);
    if (!s) return false;

    rmin   = s->GetRmin();
    rmax   = s->GetRmax();
    theta1 = s->GetTheta1();
    theta2 = s->GetTheta2();
    phi1   = s->GetPhi1();
    phi2   = s->GetPhi2();

    return true;
}

void AGeoSphere::scale(double factor)
{
    rmin *= factor;
    rmax *= factor;
}

QString AGeoTubeSeg::getHelp() const
{
    return "A tube segment is a tube having a range in phi. The general phi convention is "
           "that the shape ranges from phi1 to phi2 going counterclockwise. The angles can be defined with either "
           "negative or positive values. They are stored such that phi1 is converted to [0,360] and phi2 > phi1.\n"
           "Tube segments have Z as their symmetry axis. They have a range in Z, a minimum (rmin) and a maximum (rmax) radius.\n"
           "The full Z range is from -dz to +dz.";
}

void AGeoTubeSeg::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2rmax, rmax);                     if (!ok) errorStr += " in Dmax\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmin, rmin, false);              if (!ok) errorStr += " in Dmin\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz);                         if (!ok) errorStr += " in Height\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi1, phi1, false, false, false); if (!ok) errorStr += " in Phi1\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi2, phi2, false, false, false); if (!ok) errorStr += " in Phi2\n";

    if (rmin >= rmax) errorStr += "Inside diameter should be smaller than the outside one!\n";
}

bool AGeoTubeSeg::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2rmin.contains(nameRegExp)) return true;
    if (str2rmax.contains(nameRegExp)) return true;
    if (str2dz  .contains(nameRegExp)) return true;
    if (strPhi1.contains(nameRegExp)) return true;
    if (strPhi2.contains(nameRegExp)) return true;

    return false;
}

void AGeoTubeSeg::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2rmin.replace(nameRegExp, newName);
    str2rmax.replace(nameRegExp, newName);
    str2dz  .replace(nameRegExp, newName);
    strPhi1.replace(nameRegExp, newName);
    strPhi2.replace(nameRegExp, newName);
}

bool AGeoTubeSeg::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 5);
    if (!ok) return false;

    double tmp[5];
    for (int i=0; i<5; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoTubeSeg";
            return false;
        }
    }

    rmin = tmp[0];
    rmax = tmp[1];
    dz = tmp[2];
    phi1 = tmp[3];
    phi2 = tmp[4];
    return true;
}

TGeoShape *AGeoTubeSeg::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoTubeSeg(rmin, rmax, dz, phi1, phi2) :
                                   new TGeoTubeSeg(shapeName.toLatin1().data(), rmin, rmax, dz, phi1, phi2);
}

QString AGeoTubeSeg::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoTubeSeg( " +
                QString::number(rmin)+", "+
                QString::number(rmax)+", "+
                QString::number(dz)+", "+
                QString::number(phi1)+", "+
                QString::number(phi2)+" )";
    }
    else
    {
        QString srmin = (str2rmin .isEmpty() ? QString::number(rmin)  : "' + 0.5*(" + str2rmin + ") + '");
        QString srmax = (str2rmax .isEmpty() ? QString::number(rmax)  : "' + 0.5*(" + str2rmax + ") + '");
        QString sdz   = (str2dz   .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz   + ") + '");
        QString sphi1 = (strPhi1  .isEmpty() ? QString::number(phi1)  : "' + ("      + strPhi1   + ") + '");
        QString sphi2 = (strPhi2  .isEmpty() ? QString::number(phi2)  : "' + ("      + strPhi2   + ") + '");

        str = "TGeoTubeSeg( " +
                srmin + ", "+
                srmax + ", "+
                sdz   + ", "+
                sphi1 + ", "+
                sphi2 + " )";
    }
    return str;
}

QString AGeoTubeSeg::getScriptString(bool useStrings) const
{
    QString sdmin, sdmax, sh, sphi1, sphi2;

    if (useStrings)
    {
        sdmin = ( str2rmin.isEmpty() ? QString::number(2.0 * rmin) : str2rmin );
        sdmax = ( str2rmax.isEmpty() ? QString::number(2.0 * rmax) : str2rmax );
        sh    = ( str2dz.isEmpty()   ? QString::number(2.0 * dz)   : str2dz );
        sphi1 = ( strPhi1.isEmpty()  ? QString::number(phi1)       : strPhi1 );
        sphi2 = ( strPhi2.isEmpty()  ? QString::number(phi2)       : strPhi2 );
    }
    else
    {
        sdmin = QString::number(2.0 * rmin);
        sdmax = QString::number(2.0 * rmax);
        sh    = QString::number(2.0 * dz);
        sphi1 = QString::number(phi1);
        sphi2 = QString::number(phi2);
    }

    //void tubeSegment(QString name, double outerD, double innerD, double h, double Phi1, double Phi2,
    return QString("geo.tubeSegment( $name$,  %0, %1, %2, %3, %4,  ").arg(sdmax, sdmin, sh, sphi1, sphi2);
}

double AGeoTubeSeg::maxSize() const
{
    double m = std::max(rmax, dz);
    return sqrt(3.0)*m;
}

void AGeoTubeSeg::writeToJson(QJsonObject &json) const
{
    json["rmin"] = rmin;
    json["rmax"] = rmax;
    json["dz"]   = dz;
    json["phi1"] = phi1;
    json["phi2"] = phi2;

    if (!str2rmin.isEmpty()) json["str2rmin"] = str2rmin;
    if (!str2rmax.isEmpty()) json["str2rmax"] = str2rmax;
    if (!str2dz  .isEmpty()) json["str2dz"]   = str2dz;
    if (!strPhi1 .isEmpty())  json["strPhi1"] = strPhi1;
    if (!strPhi2 .isEmpty())  json["strPhi2"] = strPhi2;
}

void AGeoTubeSeg::readFromJson(const QJsonObject &json)
{
    rmin = json["rmin"].toDouble();
    rmax = json["rmax"].toDouble();
    dz   = json["dz"]  .toDouble();
    phi1 = json["phi1"].toDouble();
    phi2 = json["phi2"].toDouble();

    if (!jstools::parseJson(json, "str2rmin", str2rmin)) str2rmin.clear();
    if (!jstools::parseJson(json, "str2rmax", str2rmax)) str2rmax.clear();
    if (!jstools::parseJson(json, "str2dz"  , str2dz))   str2dz.clear();
    if (!jstools::parseJson(json, "strPhi1",  strPhi1))  strPhi1.clear();
    if (!jstools::parseJson(json, "strPhi2",  strPhi2))  strPhi2.clear();
}

bool AGeoTubeSeg::readFromTShape(TGeoShape *Tshape)
{
    TGeoTubeSeg* s = dynamic_cast<TGeoTubeSeg*>(Tshape);
    if (!s) return false;

    rmin = s->GetRmin();
    rmax = s->GetRmax();
    dz = s->GetDz();
    phi1 = s->GetPhi1();
    phi2 = s->GetPhi2();

    return true;
}

void AGeoTubeSeg::scale(double factor)
{
    rmin *= factor;
    rmax *= factor;
    dz   *= factor;
}

QString AGeoCtub::getHelp() const
{
    return "A cut tube is a tube segment cut with two planes. The centers of the 2 sections are positioned at ±dZ. Each cut "
           "plane is therefore defined by a point (0,0,±dZ) and its normal unit vector pointing outside the shape: "
           "Nlow=(Nx,Ny,Nz<0), Nhigh=(Nx’,Ny’,Nz’>0).\n"
           "The general phi convention is that the shape ranges from phi1 to phi2 going counterclockwise. The angles can be defined with either "
           "negative or positive values. They are stored such that phi1 is converted to [0,360] and phi2 > phi1."
           "The shape has a minimum (rmin) and a maximum (rmax) radius.\n";
}

void AGeoCtub::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, strnxlow, nxlow, false, false, false); if (!ok) errorStr += " in X low\n";
    ok = GC.updateDoubleParameter(errorStr, strnylow, nylow, false, false, false); if (!ok) errorStr += " in Y low\n";
    ok = GC.updateDoubleParameter(errorStr, strnzlow, nzlow, false, false, false); if (!ok) errorStr += " in Z low\n";
    ok = GC.updateDoubleParameter(errorStr, strnxhi, nxhi,   false, false, false); if (!ok) errorStr += " in X heigh\n";
    ok = GC.updateDoubleParameter(errorStr, strnyhi, nyhi,   false, false, false); if (!ok) errorStr += " in Y heigh\n";
    ok = GC.updateDoubleParameter(errorStr, strnzhi, nzhi,   false, false, false); if (!ok) errorStr += " in Z heigh\n";

    if (nzlow >= 0) errorStr += "Lower Nz should be negative\n";
    if (nzhi  <= 0) errorStr += "Upper Nz should be positive\n";
    AGeoTubeSeg::introduceGeoConstValues(errorStr);
}

bool AGeoCtub::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strnxlow.contains(nameRegExp)) return true;
    if (strnylow.contains(nameRegExp)) return true;
    if (strnzlow.contains(nameRegExp)) return true;
    if (strnxhi .contains(nameRegExp)) return true;
    if (strnyhi .contains(nameRegExp)) return true;
    if (strnzhi .contains(nameRegExp)) return true;
    return AGeoTubeSeg::isGeoConstInUse(nameRegExp);
}

void AGeoCtub::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    AGeoTubeSeg::replaceGeoConstName(nameRegExp, newName);

    strnxlow.replace(nameRegExp, newName);
    strnylow.replace(nameRegExp, newName);
    strnzlow.replace(nameRegExp, newName);
    strnxhi .replace(nameRegExp, newName);
    strnyhi .replace(nameRegExp, newName);
    strnzhi .replace(nameRegExp, newName);
}

bool AGeoCtub::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 11);
    if (!ok) return false;

    double tmp[11];
    for (int i=0; i<11; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoCtub";
            return false;
        }
    }

    rmin = tmp[0];
    rmax = tmp[1];
    dz =   tmp[2];
    phi1 = tmp[3];
    phi2 = tmp[4];
    nxlow= tmp[5];
    nylow= tmp[6];
    nzlow= tmp[7];
    nxhi = tmp[8];
    nyhi = tmp[9];
    nzhi = tmp[10];
    return true;
}

TGeoShape *AGeoCtub::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoCtub( rmin, rmax, dz, phi1, phi2, nxlow, nylow, nzlow, nxhi, nyhi, nzhi ) :
                                   new TGeoCtub( shapeName.toLatin1().data(), rmin, rmax, dz, phi1, phi2, nxlow, nylow, nzlow, nxhi, nyhi, nzhi );
}

QString AGeoCtub::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoCtub( " +
                QString::number(rmin)+", "+
                QString::number(rmax)+", "+
                QString::number(dz)+", "+
                QString::number(phi1)+", "+
                QString::number(phi2)+", "+
                QString::number(nxlow)+", "+
                QString::number(nylow)+", "+
                QString::number(nzlow)+", "+
                QString::number(nxhi)+", "+
                QString::number(nyhi)+", "+
                QString::number(nzhi)+" )";
    }
    else
    {
        str = AGeoTubeSeg::getGenerationString(true);
        str.chop(1);
        str.replace("TGeoTubeSeg", "TGeoCtub");

        QString snxlow = (strnxlow.isEmpty() ? QString::number(nxlow) : "' + (" + strnxlow + ") + '");
        QString snylow = (strnylow.isEmpty() ? QString::number(nylow) : "' + (" + strnylow + ") + '");
        QString snzlow = (strnzlow.isEmpty() ? QString::number(nzlow) : "' + (" + strnzlow + ") + '");
        QString snxhi  = (strnxhi .isEmpty() ? QString::number(nxhi)  : "' + (" + strnxhi  + ") + '");
        QString snyhi  = (strnyhi .isEmpty() ? QString::number(nyhi)  : "' + (" + strnyhi  + ") + '");
        QString snzhi  = (strnzhi .isEmpty() ? QString::number(nzhi)  : "' + (" + strnzhi  + ") + '");

        str.append(", "+
                   snxlow +", "+
                   snylow +", "+
                   snzlow +", "+
                   snxhi +", "+
                   snyhi +", "+
                   snzhi +" )");
        qDebug() <<"new " <<str;
    }
    return str;
}

QString AGeoCtub::getScriptString(bool useStrings) const
{
    QString sdmin, sdmax, sh, sphi1, sphi2;
    QString slx, sly, slz,  shx, shy, shz;

    if (useStrings)
    {
        sdmin = ( str2rmin.isEmpty() ? QString::number(2.0 * rmin) : str2rmin );
        sdmax = ( str2rmax.isEmpty() ? QString::number(2.0 * rmax) : str2rmax );
        sh    = ( str2dz.isEmpty()   ? QString::number(2.0 * dz)   : str2dz );
        sphi1 = ( strPhi1.isEmpty()  ? QString::number(phi1)       : strPhi1 );
        sphi2 = ( strPhi2.isEmpty()  ? QString::number(phi2)       : strPhi2 );

        slx   = ( strnxlow.isEmpty() ? QString::number(nxlow)      : strnxlow );
        sly   = ( strnylow.isEmpty() ? QString::number(nylow)      : strnylow );
        slz   = ( strnzlow.isEmpty() ? QString::number(nzlow)      : strnzlow );
        shx   = ( strnxhi.isEmpty()  ? QString::number(nxhi)       : strnxhi );
        shy   = ( strnyhi.isEmpty()  ? QString::number(nyhi)       : strnyhi );
        shz   = ( strnzhi.isEmpty()  ? QString::number(nzhi)       : strnzhi );
    }
    else
    {
        sdmin = QString::number(2.0 * rmin);
        sdmax = QString::number(2.0 * rmax);
        sh    = QString::number(2.0 * dz);
        sphi1 = QString::number(phi1);
        sphi2 = QString::number(phi2);

        slx   = QString::number(nxlow);
        sly   = QString::number(nylow);
        slz   = QString::number(nzlow);
        shx   = QString::number(nxhi);
        shy   = QString::number(nyhi);
        shz   = QString::number(nzhi);
    }

    //void tubeCut(QString name, double outerD, double innerD, double h, double Phi1, double Phi2, QVariantList Nlow, QVariantList Nhigh,
    return QString("geo.tubeCut( $name$,  %0, %1, %2, %3, %4, [%5,%6,%7], [%8,%9,%10],  ").arg(sdmax, sdmin, sh, sphi1, sphi2, slx,sly,slz, shx,shy,shz);
}

double AGeoCtub::maxSize() const
{
    double m = std::max(rmax, dz);
    return sqrt(3.0)*m;
}

void AGeoCtub::writeToJson(QJsonObject &json) const
{
    AGeoTubeSeg::writeToJson(json);

    json["nxlow"] = nxlow;
    json["nylow"] = nylow;
    json["nzlow"] = nzlow;
    json["nxhi"]  = nxhi;
    json["nyhi"]  = nyhi;
    json["nzhi"]  = nzhi;

    if (!strnxlow.isEmpty()) json["strnxlow"] = strnxlow;
    if (!strnylow.isEmpty()) json["strnylow"] = strnylow;
    if (!strnzlow.isEmpty()) json["strnzlow"] = strnzlow;
    if (!strnxhi.isEmpty())  json["strnxhi"]  = strnxhi;
    if (!strnyhi.isEmpty())  json["strnyhi"]  = strnyhi;
    if (!strnzhi.isEmpty())  json["strnzhi"]  = strnzhi;

}

void AGeoCtub::readFromJson(const QJsonObject &json)
{
    AGeoTubeSeg::readFromJson(json);

    nxlow = json["nxlow"].toDouble();
    nylow = json["nylow"].toDouble();
    nzlow = json["nzlow"].toDouble();
    nxhi  = json["nxhi"] .toDouble();
    nyhi  = json["nyhi"] .toDouble();
    nzhi  = json["nzhi"] .toDouble();

    if (!jstools::parseJson(json, "strnxlow", strnxlow)) strnxlow.clear();
    if (!jstools::parseJson(json, "strnylow", strnylow)) strnylow.clear();
    if (!jstools::parseJson(json, "strnzlow", strnzlow)) strnzlow.clear();
    if (!jstools::parseJson(json, "strnxhi",  strnxhi))  strnxhi .clear();
    if (!jstools::parseJson(json, "strnyhi",  strnyhi))  strnyhi .clear();
    if (!jstools::parseJson(json, "strnzhi",  strnzhi))  strnzhi .clear();
}

bool AGeoCtub::readFromTShape(TGeoShape *Tshape)
{
    TGeoCtub* s = dynamic_cast<TGeoCtub*>(Tshape);
    if (!s) return false;

    rmin = s->GetRmin();
    rmax = s->GetRmax();
    dz = s->GetDz();
    phi1 = s->GetPhi1();
    phi2 = s->GetPhi2();
    nxlow = s->GetNlow()[0];
    nylow = s->GetNlow()[1];
    nzlow = s->GetNlow()[2];
    nxhi = s->GetNhigh()[0];
    nyhi = s->GetNhigh()[1];
    nzhi = s->GetNhigh()[2];

    return true;
}

QString AGeoTube::getHelp() const
{
    return "Tubes have Z as their symmetry axis. They have a range in Z, a minimum (rmin) and a maximum (rmax) radius.\n"
           "The full Z range is from -dz to +dz.";
}

void AGeoTube::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2rmax, rmax);        if (!ok) errorStr += "in Rmax\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmin, rmin, false); if (!ok) errorStr += "in Rmin\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz);            if (!ok) errorStr += "in Height\n";
    if (rmin >= rmax) errorStr += "Inside diameter should be smaller than the outside one!\n";
}

bool AGeoTube::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (str2rmax.contains(nameRegExp)) return true;
    if (str2rmin.contains(nameRegExp)) return true;
    if (str2dz.  contains(nameRegExp)) return true;
    return false;
}

void AGeoTube::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    str2rmax.replace(nameRegExp, newName);
    str2rmin.replace(nameRegExp, newName);
    str2dz  .replace(nameRegExp, newName);
}

bool AGeoTube::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 3);
    if (!ok) return false;

    double tmp[3];
    for (int i=0; i<3; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoTube";
            return false;
        }
    }

    rmin = tmp[0];
    rmax = tmp[1];
    dz = tmp[2];
    return true;
}

TGeoShape *AGeoTube::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoTube(rmin, rmax, dz) :
                                   new TGeoTube(shapeName.toLatin1().data(), rmin, rmax, dz);
}

QString AGeoTube::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoTube( " +
                QString::number(rmin)+", "+
                QString::number(rmax)+", "+
                QString::number(dz)+" )";
    }
    else
    {
        QString srmin = (str2rmin.isEmpty() ? QString::number(rmin) : "' + 0.5*(" + str2rmin + ") + '");
        QString srmax = (str2rmax.isEmpty() ? QString::number(rmax) : "' + 0.5*(" + str2rmax + ") + '");
        QString sdz   = (str2dz  .isEmpty() ? QString::number(dz)   : "' + 0.5*(" + str2dz   + ") + '");
        str = "TGeoTube( " +
                srmin +", "+
                srmax +", "+
                sdz   +" )";
    }
    return str;
}

QString AGeoTube::getScriptString(bool useStrings) const
{
    QString Dmin;
    QString Dmax;
    QString H;

    if (useStrings)
    {
        Dmin = ( str2rmin.isEmpty() ? QString::number(2.0 * rmin) : str2rmin );
        Dmax = ( str2rmax.isEmpty() ? QString::number(2.0 * rmax) : str2rmax );
        H    = ( str2dz.isEmpty()   ? QString::number(2.0 * dz)   : str2dz );
    }
    else
    {
        Dmin = QString::number(2.0 * rmin);
        Dmax = QString::number(2.0 * rmax);
        H    = QString::number(2.0 * dz);
    }

    if (Dmin == QStringLiteral("0"))
    {
        //void cylinder(QString name, double D, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.cylinder( $name$,  %0, %1,  ").arg(Dmax, H);
    }
    else
    {
        //void tube(QString name, double outerD, double innerD, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.tube( $name$,  %0, %1, %2,  ").arg(Dmax, Dmin, H);
    }
}

double AGeoTube::maxSize() const
{
    double m = std::max(rmax, dz);
    return sqrt(3.0)*m;
}

double AGeoTube::minSize() const
{
    return rmax;
}

void AGeoTube::writeToJson(QJsonObject &json) const
{
    json["rmin"] = rmin;
    json["rmax"] = rmax;
    json["dz"]   = dz;

    if (!str2rmin.isEmpty()) json["str2rmin"] = str2rmin;
    if (!str2rmax.isEmpty()) json["str2rmax"] = str2rmax;
    if (!str2dz.isEmpty())   json["str2dz"]   = str2dz;
}

void AGeoTube::readFromJson(const QJsonObject &json)
{
    rmax = json["rmax"].toDouble();
    rmin = json["rmin"].toDouble();
    dz   = json["dz"].toDouble();

    if (!jstools::parseJson(json, "str2rmax", str2rmax)) str2rmax.clear();
    if (!jstools::parseJson(json, "str2rmin", str2rmin)) str2rmin.clear();
    if (!jstools::parseJson(json, "str2dz",   str2dz))   str2dz.clear();
}

bool AGeoTube::readFromTShape(TGeoShape *Tshape)
{
    TGeoTube* s = dynamic_cast<TGeoTube*>(Tshape);
    if (!s) return false;

    rmin = s->GetRmin();
    rmax = s->GetRmax();
    dz = s->GetDz();

    return true;
}

void AGeoTube::scale(double factor)
{
    rmin *= factor;
    rmax *= factor;
    dz   *= factor;
}

// ---  Trd1  ---
QString AGeoTrd1::getHelp() const
{
    return "Trapezoid with two of the opposite faces parallel to XY plane and positioned at ± dZ\n"
           "Trd1 is a trapezoid with only X varying with Z. It is defined by the half-length in Z, the half-length in X at the "
           "lowest and highest Z planes and the half-length in Y\n"
           " • dx1: half length in X at -dz\n"
           " • dx2: half length in X at +dz\n"
           " • dy: half length in Y\n"
           " • dz: half length in Z\n";
}

void AGeoTrd1::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2dx1, dx1); if (!ok) errorStr += " in X1 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dx2, dx2); if (!ok) errorStr += " in X2 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dy,  dy);  if (!ok) errorStr += " in Y size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz,  dz);  if (!ok) errorStr += " in Z size\n";
}

bool AGeoTrd1::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2dx1.contains(nameRegExp)) return true;
    if (str2dx2.contains(nameRegExp)) return true;
    if (str2dy .contains(nameRegExp)) return true;
    if (str2dz .contains(nameRegExp)) return true;
    return false;
}

void AGeoTrd1::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2dx1.replace(nameRegExp, newName);
    str2dx2.replace(nameRegExp, newName);
    str2dy .replace(nameRegExp, newName);
    str2dz .replace(nameRegExp, newName);
}

bool AGeoTrd1::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 4);
    if (!ok) return false;

    double tmp[4];
    for (int i=0; i<4; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoTrd1";
            return false;
        }
    }

    dx1 = tmp[0];
    dx2 = tmp[1];
    dy = tmp[2];
    dz = tmp[3];
    return true;
}

TGeoShape *AGeoTrd1::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoTrd1(dx1, dx2, dy, dz) :
                                   new TGeoTrd1(shapeName.toLatin1().data(), dx1, dx2, dy, dz);
}

QString AGeoTrd1::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoTrd1( " +
                QString::number(dx1)+", "+
                QString::number(dx2)+", "+
                QString::number(dy)+", "+
                QString::number(dz)+" )";
    }
    else
    {
        QString sdx1  = (str2dx1.isEmpty() ? QString::number(dx1) : "' + 0.5*(" + str2dx1    + ") + '");
        QString sdx2  = (str2dx2.isEmpty() ? QString::number(dx2) : "' + 0.5*(" + str2dx2    + ") + '");
        QString sdy   = (str2dy .isEmpty() ? QString::number(dy)  : "' + 0.5*(" + str2dy     + ") + '");
        QString sdz   = (str2dz .isEmpty() ? QString::number(dz)  : "' + 0.5*(" + str2dz     + ") + '");

        str = "TGeoTrd1( " +
                sdx1 +", "+
                sdx2 +", "+
                sdy  +", "+
                sdz  +" )";

    }
    return str;
}

QString AGeoTrd1::getScriptString(bool useStrings) const
{
    QString sx1, sx2, sy, sz;

    if (useStrings)
    {
        sx1  = (str2dx1.isEmpty() ? QString::number(2.0 * dx1) : str2dx1);
        sx2  = (str2dx2.isEmpty() ? QString::number(2.0 * dx2) : str2dx2);
        sy   = (str2dy .isEmpty() ? QString::number(2.0 * dy)  : str2dy);
        sz   = (str2dz .isEmpty() ? QString::number(2.0 * dz)  : str2dz);
    }
    else
    {
        sx1  = QString::number(2.0 * dx1);
        sx2  = QString::number(2.0 * dx2);
        sy   = QString::number(2.0 * dy);
        sz   = QString::number(2.0 * dz);
    }

    //void AGeo_SI::trap(QString name, double LXlow, double LXup, double Ly, double Lz,
    return QString("geo.trap( $name$,  %0, %1, %2, %3,  ").arg(sx1, sx2, sy, sz);
}

double AGeoTrd1::minSize() const
{
    return std::min(dz, 0.5*(dx1 + dx2));
}

double AGeoTrd1::maxSize() const
{
    double m = std::max(dx1, dx2);
    m = std::max(m, dy);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoTrd1::writeToJson(QJsonObject &json) const
{
    json["dx1"] = dx1;
    json["dx2"] = dx2;
    json["dy"]  = dy;
    json["dz"]  = dz;

    if (!str2dx1.isEmpty()) json["str2dx1"] = str2dx1;
    if (!str2dx2.isEmpty()) json["str2dx2"] = str2dx2;
    if (!str2dy.isEmpty())  json["str2dy"]  = str2dy;
    if (!str2dz.isEmpty())  json["str2dz"]  = str2dz;
}

void AGeoTrd1::readFromJson(const QJsonObject &json)
{
    dx1 = json["dx1"].toDouble();
    dx2 = json["dx2"].toDouble();
    dy  = json["dy"].toDouble();
    dz  = json["dz"].toDouble();

    if (!jstools::parseJson(json, "str2dx1", str2dx1)) str2dx1.clear();
    if (!jstools::parseJson(json, "str2dx2", str2dx2)) str2dx2.clear();
    if (!jstools::parseJson(json, "str2dy",  str2dy))  str2dy .clear();
    if (!jstools::parseJson(json, "str2dz",  str2dz))  str2dz .clear();
}

bool AGeoTrd1::readFromTShape(TGeoShape *Tshape)
{
    TGeoTrd1* s = dynamic_cast<TGeoTrd1*>(Tshape);
    if (!s) return false;

    dx1 = s->GetDx1();
    dx2 = s->GetDx2();
    dy = s->GetDy();
    dz = s->GetDz();

    return true;
}

void AGeoTrd1::scale(double factor)
{
    dx1 *= factor;
    dx2 *= factor;
    dy  *= factor;
    dz  *= factor;
}

// ---  Trd2  ---
QString AGeoTrd2::getHelp() const
{
    return "Trd2 is a trapezoid with both X and Y varying with Z. It is defined by the half-length in Z, the half-length in X at "
           "the lowest and highest Z planes and the half-length in Y at these planes: "
           " • dx1: half length in X at -dz\n"
           " • dx2: half length in X at +dz\n"
           " • dy1: half length in Y at -dz\n"
           " • dy2: half length in Y at +dz\n"
           " • dz: half length in Z\n";
}

void AGeoTrd2::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2dx1, dx1); if (!ok) errorStr += " in X1 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dx2, dx2); if (!ok) errorStr += " in X2 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dy1, dy1); if (!ok) errorStr += " in Y1 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dy2, dy2); if (!ok) errorStr += " in Y2 size\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz,  dz);  if (!ok) errorStr += " in Z size\n";
}

bool AGeoTrd2::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2dx1.contains(nameRegExp)) return true;
    if (str2dx2.contains(nameRegExp)) return true;
    if (str2dy1.contains(nameRegExp)) return true;
    if (str2dy2.contains(nameRegExp)) return true;
    if (str2dz .contains(nameRegExp)) return true;
    return false;
}

void AGeoTrd2::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2dx1.replace(nameRegExp, newName);
    str2dx2.replace(nameRegExp, newName);
    str2dy1.replace(nameRegExp, newName);
    str2dy2.replace(nameRegExp, newName);
    str2dz .replace(nameRegExp, newName);
}

bool AGeoTrd2::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 5);
    if (!ok) return false;

    double tmp[5];
    for (int i=0; i<5; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoTrd2";
            return false;
        }
    }
    dx1 = tmp[0];
    dx2 = tmp[1];
    dy1 = tmp[2];
    dy2 = tmp[3];
    dz = tmp[4];
    return true;
}

TGeoShape *AGeoTrd2::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoTrd2(dx1, dx2, dy1, dy2, dz) :
                                   new TGeoTrd2(shapeName.toLatin1().data(), dx1, dx2, dy1, dy2, dz);
}

QString AGeoTrd2::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoTrd2( " +
                QString::number(dx1)+", "+
                QString::number(dx2)+", "+
                QString::number(dy1)+", "+
                QString::number(dy2)+", "+
                QString::number(dz)+" )";
    }
    else
    {
        QString sdx1  = (str2dx1.isEmpty() ? QString::number(dx1) : "' + 0.5*(" + str2dx1    + ") + '");
        QString sdx2  = (str2dx2.isEmpty() ? QString::number(dx2) : "' + 0.5*(" + str2dx2    + ") + '");
        QString sdy1   = (str2dy1 .isEmpty() ? QString::number(dy1)  : "' + 0.5*(" + str2dy1     + ") + '");
        QString sdy2   = (str2dy2 .isEmpty() ? QString::number(dy2)  : "' + 0.5*(" + str2dy2     + ") + '");
        QString sdz   = (str2dz .isEmpty() ? QString::number(dz)  : "' + 0.5*(" + str2dz     + ") + '");

        str = "TGeoTrd2( " +
                sdx1 +", "+
                sdx2 +", "+
                sdy1  +", "+
                sdy2  +", "+
                sdz  +" )";

    }
    return str;
}

QString AGeoTrd2::getScriptString(bool useStrings) const
{
    QString sx1, sx2, sy1, sy2, sz;

    if (useStrings)
    {
        sx1 = (str2dx1.isEmpty() ? QString::number(2.0 * dx1) : str2dx1);
        sx2 = (str2dx2.isEmpty() ? QString::number(2.0 * dx2) : str2dx2);
        sy1 = (str2dy1.isEmpty() ? QString::number(2.0 * dy1) : str2dy1);
        sy2 = (str2dy2.isEmpty() ? QString::number(2.0 * dy2) : str2dy2);
        sz  = (str2dz .isEmpty() ? QString::number(2.0 * dz)  : str2dz);
    }
    else
    {
        sx1 = QString::number(2.0 * dx1);
        sx2 = QString::number(2.0 * dx2);
        sy1 = QString::number(2.0 * dy1);
        sy2 = QString::number(2.0 * dy2);
        sz  = QString::number(2.0 * dz);
    }

    //void trap2(QString name, double LXlow, double LXup, double LYlow, double LYup, double Lz,
    return QString("geo.trap2( $name$,  %0, %1, %2, %3, %4,  ").arg(sx1, sx2, sy1, sy2, sz);
}

double AGeoTrd2::maxSize() const
{
    double m = std::max(dx1, dx2);
    m = std::max(m, dy1);
    m = std::max(m, dy2);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoTrd2::writeToJson(QJsonObject &json) const
{
    json["dx1"] = dx1;
    json["dx2"] = dx2;
    json["dy1"] = dy1;
    json["dy2"] = dy2;
    json["dz"]  = dz;

    if (!str2dx1.isEmpty()) json["str2dx1"] = str2dx1;
    if (!str2dx2.isEmpty()) json["str2dx2"] = str2dx2;
    if (!str2dy1.isEmpty()) json["str2dy1"] = str2dy1;
    if (!str2dy2.isEmpty()) json["str2dy2"] = str2dy2;
    if (!str2dz .isEmpty()) json["str2dz"]  = str2dz;
}

void AGeoTrd2::readFromJson(const QJsonObject &json)
{
    dx1 = json["dx1"].toDouble();
    dx2 = json["dx2"].toDouble();
    dy1 = json["dy1"].toDouble();
    dy2 = json["dy2"].toDouble();
    dz  = json["dz"] .toDouble();

    if (!jstools::parseJson(json, "str2dx1", str2dx1)) str2dx1.clear();
    if (!jstools::parseJson(json, "str2dx2", str2dx2)) str2dx2.clear();
    if (!jstools::parseJson(json, "str2dy1", str2dy1)) str2dy1.clear();
    if (!jstools::parseJson(json, "str2dy2", str2dy2)) str2dy2.clear();
    if (!jstools::parseJson(json, "str2dz",  str2dz))  str2dz .clear();
}

bool AGeoTrd2::readFromTShape(TGeoShape *Tshape)
{
    TGeoTrd2* s = dynamic_cast<TGeoTrd2*>(Tshape);
    if (!s) return false;

    dx1 = s->GetDx1();
    dx2 = s->GetDx2();
    dy1 = s->GetDy1();
    dy2 = s->GetDy2();
    dz = s->GetDz();

    return true;
}

void AGeoTrd2::scale(double factor)
{
    dx1 *= factor;
    dx2 *= factor;
    dy1 *= factor;
    dy2 *= factor;
    dz  *= factor;
}

// --- GeoPgon ---
QString AGeoPgon::getHelp() const
{
    return "TGeoPgon:\n"
           "nedges - number of edges\n"
           "phi - start angle [0, 360)\n"
           "dphi - range in angle (0, 360]\n"
           "{z : rmin : rmax} - arbitrary number of sections defined with z position, minimum and maximum radii";
}

void AGeoPgon::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateIntParameter(errorStr, strNedges, nedges, true, true); if (!ok) errorStr += " in N edges\n";
    if (nedges < 3)
    {
        errorStr += "There should be at least 3 edges\n";
        return;
    }

    AGeoPcon::introduceGeoConstValues(errorStr);
}

bool AGeoPgon::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strNedges.contains(nameRegExp)) return true;
    return AGeoPcon::isGeoConstInUse(nameRegExp);
}

void AGeoPgon::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strNedges.replace(nameRegExp, newName);
    AGeoPcon::replaceGeoConstName(nameRegExp, newName);
}

bool AGeoPgon::readFromString(QString GenerationString)
{
    Sections.clear();
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, -1);
    if (!ok) return false;

    if (params.size()<5)
    {
        qWarning() << "Not enough parameters to define TGeoPgon";
        return false;
    }

    phi = params[0].toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Syntax error found during extracting parameters of TGeoPgon";
        return false;
    }
    dphi = params[1].toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Syntax error found during extracting parameters of TGeoPgon";
        return false;
    }
    double dnedges = params[2].toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Syntax error found during extracting parameters of TGeoPgon";
        return false;
    }
    nedges = dnedges;

    for (int i=3; i<params.size(); i++)
    {
        APolyCGsection section;
        if (!section.fromString(params.at(i)))
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoPgon";
            return false;
        }
        Sections.push_back(section);
    }

    bool bInc = true;
    for (size_t i = 1; i < Sections.size(); i++)
    {
        if (i == 1)
            if (Sections[0].z > Sections[1].z)
                bInc = false;

        if (Sections[i].z <= Sections[i-1].z && bInc)
        {
            qWarning() << "Non consistent positions of polygon planes";
            return false;
        }

        if (Sections[i].z >= Sections[i-1].z && !bInc)
        {
            qWarning() << "Non consistent positions of polygon planes";
            return false;
        }
    }

    return true;
}

TGeoShape *AGeoPgon::createGeoShape(const QString shapeName)
{
    TGeoPgon* pg = (shapeName.isEmpty()) ? new TGeoPgon(phi, dphi, nedges, Sections.size()) :
                                           new TGeoPgon(shapeName.toLatin1().data(), phi, dphi, nedges, Sections.size());
    for (int i=0; i<Sections.size(); i++)
    {
        const APolyCGsection& s = Sections.at(i);
        pg->DefineSection(i, s.z, s.rmin, s.rmax);
    }
    return pg;
}

QString AGeoPgon::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoPgon( " +
                QString::number(phi)+", "+
                QString::number(dphi) + ", "+
                QString::number(nedges);

        for (const APolyCGsection& s : Sections)  str += ", " + s.toString(false);

        str +=" )";
    }
    else
    {
        QString sphi    = (strPhi    .isEmpty() ? QString::number(phi)   : "' + (" + strPhi    + ") + '");
        QString sdphi   = (strdPhi   .isEmpty() ? QString::number(dphi)  : "' + (" + strdPhi   + ") + '");
        QString snedges = (strNedges .isEmpty() ? QString::number(nedges): "' + (" + strNedges + ") + '");

        str = "TGeoPgon( " +
                sphi  + ", "+
                sdphi + ", "+
                snedges;

        for (const APolyCGsection& s : Sections)  str += ", " + s.toString(true);

        str +=" )";
    }
    return str;
}

QString AGeoPgon::getScriptString(bool useStrings) const
{
    QString sn, sphi, sdphi, sec;
    if (useStrings)
    {
        sn    = ( strNedges.isEmpty() ? QString::number(nedges) : strNedges );
        sphi  = ( strPhi   .isEmpty() ? QString::number(phi)    : strPhi    );
        sdphi = ( strdPhi  .isEmpty() ? QString::number(dphi)   : strdPhi   );
    }
    else
    {
        sn    = QString::number(nedges);
        sphi  = QString::number(phi);
        sdphi = QString::number(dphi);
    }

    for (int i = 0; i < Sections.size(); i++)
    {
        if (i != 0) sec += ", ";
        sec += Sections[i].toScriptString(useStrings);
    }

    //void pGon(QString name, int numEdges, QVariantList sections, double Phi, double dPhi,
    return QString("geo.pGon( $name$,  %0, [ %1 ], %2, %3,  ").arg(sn, sec, sphi, sdphi);
}

double AGeoPgon::maxSize() const
{
    return AGeoPcon::maxSize();
}

void AGeoPgon::writeToJson(QJsonObject &json) const
{
    json["nedges"] = nedges;
    if (!strNedges.isEmpty()) json["strNedges"] = strNedges;

    AGeoPcon::writeToJson(json);
}

void AGeoPgon::readFromJson(const QJsonObject &json)
{
    nedges = json["nedges"].toInt();
    if (!jstools::parseJson(json, "strNedges", strNedges)) strNedges.clear();
    AGeoPcon::readFromJson(json);
}

bool AGeoPgon::readFromTShape(TGeoShape *Tshape)
{
    TGeoPgon* pg = dynamic_cast<TGeoPgon*>(Tshape);
    if (!pg) return false;

    phi = pg->GetPhi1();
    dphi = pg->GetDphi();
    nedges = pg->GetNedges();

    Sections.clear();
    for (int i=0; i<pg->GetNz(); i++)
        Sections.push_back( APolyCGsection(pg->GetZ()[i], pg->GetRmin()[i], pg->GetRmax()[i]) );

    //qDebug() << "Pgone loaded from TShape..."<<phi<<dphi<<nedges;
    //for (int i=0; i<Sections.size(); i++) qDebug() << Sections.at(i).toString();

    return true;
}

QString AGeoConeSeg::getHelp() const
{
    return "Cone segment with the following parameters:\n"
           "dz - half size in Z\n"
           "rminL - internal radius at Z-dz\n"
           "rmaxL - external radius at Z-dz\n"
           "rminU - internal radius at Z+dz\n"
           "rmaxU - external radius at Z+dz\n"
           "phi1 - angle [0, 360)\n"
           "phi2 - angle (0, 360]";
}

void AGeoConeSeg::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, strPhi1, phi1, false, true, false); if (!ok) errorStr += " in Phi1\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi2, phi2, false, true, false); if (!ok) errorStr += " in Phi2\n";

    if (phi1 <  0 || phi1 >= 360.0) errorStr += "Phi1 should be in the range of [0, 360)\n";
    if (phi2 <= 0 || phi2 >  360.0) errorStr += "Phi2 should be in the range of (0, 360]\n";

    AGeoCone::introduceGeoConstValues(errorStr);
}

bool AGeoConeSeg::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strPhi1.contains(nameRegExp)) return true;
    if (strPhi2.contains(nameRegExp)) return true;

    return AGeoCone::isGeoConstInUse(nameRegExp);
}

void AGeoConeSeg::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    AGeoCone::replaceGeoConstName(nameRegExp, newName);

    strPhi1.replace(nameRegExp, newName);
    strPhi2.replace(nameRegExp, newName);
}

bool AGeoConeSeg::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 7);
    if (!ok) return false;

    double tmp[7];
    for (int i=0; i<7; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoConeSeg";
            return false;
        }
    }

    dz =    tmp[0];
    rminL = tmp[1];
    rmaxL = tmp[2];
    rminU = tmp[3];
    rmaxU = tmp[4];
    phi1 =  tmp[5];
    phi2 =  tmp[6];
    return true;
}

TGeoShape *AGeoConeSeg::createGeoShape(const QString shapeName)
{
    TGeoConeSeg* s = (shapeName.isEmpty()) ? new TGeoConeSeg(dz, rminL, rmaxL, rminU, rmaxU, phi1, phi2) :
                                             new TGeoConeSeg(shapeName.toLatin1().data(),
                                                             dz, rminL, rmaxL, rminU, rmaxU, phi1, phi2);
    return s;
}

QString AGeoConeSeg::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoConeSeg( " +
                QString::number(dz)+", "+
                QString::number(rminL)+", "+
                QString::number(rmaxL)+", "+
                QString::number(rminU)+", "+
                QString::number(rmaxU)+", "+
                QString::number(phi1)+", "+
                QString::number(phi2)+" )";
    }
    else
    {
        QString sdz    = (str2dz   .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz    + ") + '");
        QString srminL = (str2rminL.isEmpty() ? QString::number(rminL) : "' + 0.5*(" + str2rminL + ") + '");
        QString srmaxL = (str2rmaxL.isEmpty() ? QString::number(rmaxL) : "' + 0.5*(" + str2rmaxL + ") + '");
        QString srminU = (str2rminU.isEmpty() ? QString::number(rminU) : "' + 0.5*(" + str2rminU + ") + '");
        QString srmaxU = (str2rmaxU.isEmpty() ? QString::number(rmaxU) : "' + 0.5*(" + str2rmaxU + ") + '");
        QString sphi1  = (strPhi1  .isEmpty() ? QString::number(phi1)  : "' + ("      + strPhi1   + ") + '");
        QString sphi2  = (strPhi2  .isEmpty() ? QString::number(phi2)  : "' + ("      + strPhi2   + ") + '");

        str = "TGeoConeSeg( " +
                sdz    + ", "+
                srminL + ", "+
                srmaxL + ", "+
                srminU + ", "+
                srmaxU + ", "+
                sphi1  + ", "+
                sphi2  + " )";

    }
    return str;
}

QString AGeoConeSeg::getScriptString(bool useStrings) const
{
    QString sminL;
    QString smaxL;
    QString sminU;
    QString smaxU;
    QString sdz;
    QString sphi1;
    QString sphi2;

    if (useStrings)
    {
        sminL = ( str2rminL.isEmpty() ? QString::number(2.0 * rminL) : str2rminL );
        smaxL = ( str2rmaxL.isEmpty() ? QString::number(2.0 * rmaxL) : str2rmaxL );
        sminU = ( str2rminU.isEmpty() ? QString::number(2.0 * rminU) : str2rminU );
        smaxU = ( str2rmaxU.isEmpty() ? QString::number(2.0 * rmaxU) : str2rmaxU );
        sdz   = ( str2dz.isEmpty()    ? QString::number(2.0 * dz)    : str2dz );
        sphi1 = ( strPhi1.isEmpty()   ? QString::number(phi1)        : strPhi1 );
        sphi2 = ( strPhi2.isEmpty()   ? QString::number(phi2)        : strPhi2 );
    }
    else
    {
        sminL = QString::number(2.0 * rminL);
        smaxL = QString::number(2.0 * rmaxL);
        sminU = QString::number(2.0 * rminU);
        smaxU = QString::number(2.0 * rmaxU);
        sdz   = QString::number(2.0 * dz);
        sphi1 = QString::number(2.0 * phi1);
        sphi2 = QString::number(2.0 * phi2);
    }

    //void coneSegment(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, double phi1, double phi2, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    return QString("geo.coneSegment( $name$,  %0, %1, %2, %3, %4, %5, %6,  ").arg(smaxU, sminU, smaxL, sminL, sdz, sphi1, sphi2);
}

double AGeoConeSeg::maxSize() const
{
    double m = std::max(rmaxL, rmaxU);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoConeSeg::writeToJson(QJsonObject &json) const
{
    AGeoCone::writeToJson(json);

    json["phi1"] = phi1;
    json["phi2"] = phi2;

    if (!strPhi1.isEmpty()) json["strPhi1"] = strPhi1;
    if (!strPhi2.isEmpty()) json["strPhi2"] = strPhi2;

}

void AGeoConeSeg::readFromJson(const QJsonObject &json)
{
    AGeoCone::readFromJson(json);

    phi1 = json["phi1"].toDouble();
    phi2 = json["phi2"].toDouble();

    if (!jstools::parseJson(json, "strPhi1", strPhi1)) strPhi1.clear();
    if (!jstools::parseJson(json, "strPhi2", strPhi2)) strPhi2.clear();
}

bool AGeoConeSeg::readFromTShape(TGeoShape *Tshape)
{
    TGeoConeSeg* s = dynamic_cast<TGeoConeSeg*>(Tshape);
    if (!s) return false;

    rminL = s->GetRmin1();
    rmaxL = s->GetRmax1();
    rminU = s->GetRmin2();
    rmaxU = s->GetRmax2();
    phi1 = s->GetPhi1();
    phi2 = s->GetPhi2();
    dz = s->GetDz();

    return true;
}

QString AGeoParaboloid::getHelp() const
{
    return  "A paraboloid is defined by the revolution surface generated by a parabola and is bounded by two planes "
            "perpendicular to Z axis. The parabola equation is taken in the form: z = a·r2 + b, where: r2 = x2 + y2. "
            "The coefficients a and b are computed from the input values which are the radii of the circular sections cut by "
            "the planes at +/-dz:\n"
            " • -dz = a·rlo·rlo + b\n"
            " • +dz = a·rhi·rhi + b";
}

void AGeoParaboloid::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2rlo, rlo, false); if (!ok) errorStr += " in D low\n";
    ok = GC.updateDoubleParameter(errorStr, str2rhi, rhi, false); if (!ok) errorStr += " in D high\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz);          if (!ok) errorStr += " in Height\n";
    if (rlo >= rhi) errorStr += "Lower diameter should be smaller than the upper diameter!\n";
}

bool AGeoParaboloid::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2rlo.contains(nameRegExp)) return true;
    if (str2rhi.contains(nameRegExp)) return true;
    if (str2dz.contains(nameRegExp))  return true;

    return false;
}

void AGeoParaboloid::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2rlo.replace(nameRegExp, newName);
    str2rhi.replace(nameRegExp, newName);
    str2dz .replace(nameRegExp, newName);
}

bool AGeoParaboloid::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 3);
    if (!ok) return false;

    double tmp[3];
    for (int i=0; i<3; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoParaboloid";
            return false;
        }
    }

    rlo = tmp[0];
    rhi = tmp[1];
    dz = tmp[2];
    return true;
}

TGeoShape *AGeoParaboloid::createGeoShape(const QString shapeName)
{
    TGeoParaboloid* s = (shapeName.isEmpty()) ? new TGeoParaboloid(rlo, rhi, dz) :
                                                new TGeoParaboloid(shapeName.toLatin1().data(), rlo, rhi, dz);
    return s;
}

QString AGeoParaboloid::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoParaboloid( " +
                QString::number(rlo)+", "+
                QString::number(rhi)+", "+
                QString::number(dz)+" )";
    }
    else
    {
        QString srlo   = (str2rlo  .isEmpty() ? QString::number(rlo)   : "' + 0.5*(" + str2rlo   + ") + '");
        QString srhi   = (str2rhi  .isEmpty() ? QString::number(rhi)   : "' + 0.5*(" + str2rhi   + ") + '");
        QString sdz    = (str2dz   .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz    + ") + '");

        str = "TGeoParaboloid( " +
                srlo + ", "+
                srhi + ", "+
                sdz  + " )";

    }
    return str;
}

QString AGeoParaboloid::getScriptString(bool useStrings) const
{
    QString sdb, sdu, sdz;
    if (useStrings)
    {
        sdb = (str2rlo.isEmpty() ? QString::number(2.0 * rlo) : str2rlo);
        sdu = (str2rhi.isEmpty() ? QString::number(2.0 * rhi) : str2rhi);
        sdz = (str2dz .isEmpty() ? QString::number(2.0 * dz)  : str2dz);
    }
    else
    {
        sdb = QString::number(2.0 * rlo);
        sdu = QString::number(2.0 * rhi);
        sdz = QString::number(2.0 * dz);
    }

    //void paraboloid(QString name, double Dbot, double Dup, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    return QString("geo.paraboloid( $name$,  %0, %1, %2,  ").arg(sdb, sdu, sdz);
}

double AGeoParaboloid::maxSize() const
{
    double m = std::max(rlo, rhi);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoParaboloid::writeToJson(QJsonObject &json) const
{
    json["rlo"] = rlo;
    json["rhi"] = rhi;
    json["dz"]  = dz;

    if (!str2rlo.isEmpty()) json["str2rlo"] = str2rlo;
    if (!str2rhi.isEmpty()) json["str2rhi"] = str2rhi;
    if (!str2dz.isEmpty())  json["str2dz"]  = str2dz;
}

void AGeoParaboloid::readFromJson(const QJsonObject &json)
{
    rlo = json["rlo"].toDouble();
    rhi = json["rhi"].toDouble();
    dz  = json["dz"].toDouble();

    if (!jstools::parseJson(json, "str2rlo", str2rlo)) str2rlo.clear();
    if (!jstools::parseJson(json, "str2rhi", str2rhi)) str2rhi.clear();
    if (!jstools::parseJson(json, "str2dz", str2dz))   str2dz.clear() ;
}

bool AGeoParaboloid::readFromTShape(TGeoShape *Tshape)
{
    TGeoParaboloid* p = dynamic_cast<TGeoParaboloid*>(Tshape);
    if (!p) return false;

    rlo = p->GetRlo();
    rhi = p->GetRhi();
    dz = p->GetDz();

    return true;
}

void AGeoParaboloid::scale(double factor)
{
    rlo *= factor;
    rhi *= factor;
    dz  *= factor;
}

QString AGeoCone::getHelp() const
{
    return "Cone with the following parameters:\n"
           "dz - half size in Z\n"
           "rminL - internal radius at Z-dz\n"
           "rmaxL - external radius at Z-dz\n"
           "rminU - internal radius at Z+dz\n"
           "rmaxU - external radius at Z+dz";
}

void AGeoCone::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2dz,    dz);           if (!ok) errorStr += " in Height\n";
    ok = GC.updateDoubleParameter(errorStr, str2rminL, rminL, false); if (!ok) errorStr += " in RminL\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmaxL, rmaxL, false); if (!ok) errorStr += " in RmaxL\n";
    ok = GC.updateDoubleParameter(errorStr, str2rminU, rminU, false); if (!ok) errorStr += " in RminU\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmaxU, rmaxU, false); if (!ok) errorStr += " in RmaxU\n";

    if (rminL > rmaxL)                    errorStr += "Inside lower diameter should be equal or smaller than the outside one!\n";
    if (rminU > rmaxU)                    errorStr += "Inside upper diameter should be equal or smaller than the outside one!\n";
    if (rmaxL == 0     && rmaxU == 0)     errorStr += "Upper and lower outside diameters can't be 0 at the same time!\n";
    if (rminL == rmaxL && rminU == rmaxU) errorStr += "Upper and lower outside diameters can't be equal to the inside ones at the same time!\n";
}

bool AGeoCone::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2dz   .contains(nameRegExp)) return true;
    if (str2rminL.contains(nameRegExp)) return true;
    if (str2rmaxL.contains(nameRegExp)) return true;
    if (str2rminU.contains(nameRegExp)) return true;
    if (str2rmaxU.contains(nameRegExp)) return true;

    return false;
}

void AGeoCone::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2dz   .replace(nameRegExp, newName);
    str2rminL.replace(nameRegExp, newName);
    str2rmaxL.replace(nameRegExp, newName);
    str2rminU.replace(nameRegExp, newName);
    str2rmaxU.replace(nameRegExp, newName);
}

bool AGeoCone::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 5);
    if (!ok) return false;

    double tmp[5];
    for (int i=0; i<5; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoCone";
            return false;
        }
    }

    dz =    tmp[0];
    rminL = tmp[1];
    rmaxL = tmp[2];
    rminU = tmp[3];
    rmaxU = tmp[4];
    return true;
}

TGeoShape *AGeoCone::createGeoShape(const QString shapeName)
{
    TGeoCone* s = (shapeName.isEmpty()) ? new TGeoCone(dz, rminL, rmaxL, rminU, rmaxU) :
                                          new TGeoCone(shapeName.toLatin1().data(),
                                                       dz, rminL, rmaxL, rminU, rmaxU);
    return s;
}

QString AGeoCone::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoCone( " +
                QString::number(dz)+", "+
                QString::number(rminL)+", "+
                QString::number(rmaxL)+", "+
                QString::number(rminU)+", "+
                QString::number(rmaxU)+" )";
    }
    else
    {
        QString sdz      = (str2dz   .isEmpty() ? QString::number(dz)      : "' + 0.5*(" + str2dz    + ") + '");
        QString srminL   = (str2rminL.isEmpty() ? QString::number(rminL)   : "' + 0.5*(" + str2rminL + ") + '");
        QString srmaxL   = (str2rmaxL.isEmpty() ? QString::number(rmaxL)   : "' + 0.5*(" + str2rmaxL + ") + '");
        QString srminU   = (str2rminU.isEmpty() ? QString::number(rminU)   : "' + 0.5*(" + str2rminU + ") + '");
        QString srmaxU   = (str2rmaxU.isEmpty() ? QString::number(rmaxU)   : "' + 0.5*(" + str2rmaxU + ") + '");

        str = "TGeoCone( " +
                sdz    + ", "+
                srminL + ", "+
                srmaxL + ", "+
                srminU + ", "+
                srmaxU + " )";


    }
    return str;
}

QString AGeoCone::getScriptString(bool useStrings) const
{
    QString sminL;
    QString smaxL;
    QString sminU;
    QString smaxU;
    QString sdz;

    if (useStrings)
    {
        sminL = ( str2rminL.isEmpty() ? QString::number(2.0 * rminL) : str2rminL );
        smaxL = ( str2rmaxL.isEmpty() ? QString::number(2.0 * rmaxL) : str2rmaxL );
        sminU = ( str2rminU.isEmpty() ? QString::number(2.0 * rminU) : str2rminU );
        smaxU = ( str2rmaxU.isEmpty() ? QString::number(2.0 * rmaxU) : str2rmaxU );
        sdz   = ( str2dz.isEmpty()    ? QString::number(2.0 * dz)    : str2dz );
    }
    else
    {
        sminL = QString::number(2.0 * rminL);
        smaxL = QString::number(2.0 * rmaxL);
        sminU = QString::number(2.0 * rminU);
        smaxU = QString::number(2.0 * rmaxU);
        sdz   = QString::number(2.0 * dz);
    }

    if (sminL == QStringLiteral("0") && sminU == QStringLiteral("0"))
    {
        //void cone(QString name, double Dtop, double Dbot, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.cone( $name$,  %0, %1, %2,  ").arg(smaxU, smaxL, sdz);
    }
    else
    {
        //void conicalTube(QString name, double DtopOut,  double DtopIn, double DbotOut, double DbotIn, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.conicalTube( $name$,  %0, %1, %2, %3, %4,  ").arg(smaxU, sminU, smaxL, sminL, sdz);
    }
}

double AGeoCone::maxSize() const
{
    double m = std::max(rmaxL, rmaxU);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoCone::writeToJson(QJsonObject &json) const
{
    json["dz"]    = dz;
    json["rminL"] = rminL;
    json["rmaxL"] = rmaxL;
    json["rminU"] = rminU;
    json["rmaxU"] = rmaxU;

    if (!str2dz   .isEmpty()) json ["str2dz"]    = str2dz;
    if (!str2rminL.isEmpty()) json ["str2rminL"] = str2rminL;
    if (!str2rmaxL.isEmpty()) json ["str2rmaxL"] = str2rmaxL;
    if (!str2rminU.isEmpty()) json ["str2rminU"] = str2rminU;
    if (!str2rmaxU.isEmpty()) json ["str2rmaxU"] = str2rmaxU;
}

void AGeoCone::readFromJson(const QJsonObject &json)
{
    dz    = json["dz"]   .toDouble();
    rminL = json["rminL"].toDouble();
    rmaxL = json["rmaxL"].toDouble();
    rminU = json["rminU"].toDouble();
    rmaxU = json["rmaxU"].toDouble();

    if (!jstools::parseJson(json, "str2dz",    str2dz))    str2dz   .clear();
    if (!jstools::parseJson(json, "str2rminL", str2rminL)) str2rminL.clear();
    if (!jstools::parseJson(json, "str2rmaxL", str2rmaxL)) str2rmaxL.clear();
    if (!jstools::parseJson(json, "str2rminU", str2rminU)) str2rminU.clear();
    if (!jstools::parseJson(json, "str2rmaxU", str2rmaxU)) str2rmaxU.clear();
}

bool AGeoCone::readFromTShape(TGeoShape *Tshape)
{
    TGeoCone* s = dynamic_cast<TGeoCone*>(Tshape);
    if (!s) return false;

    rminL = s->GetRmin1();
    rmaxL = s->GetRmax1();
    rminU = s->GetRmin2();
    rmaxU = s->GetRmax2();
    dz = s->GetDz();

    return true;
}

void AGeoCone::scale(double factor)
{
    dz    *= factor;
    rminL *= factor;
    rmaxL *= factor;
    rminU *= factor;
    rmaxU *= factor;
}

QString AGeoEltu::getHelp() const
{
    return "An elliptical tube is defined by the two semi-axes a and b. It ranges from –dz to +dz in Z direction.";
}

void AGeoEltu::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2a,  a);  if (!ok) errorStr += " in A\n";
    ok = GC.updateDoubleParameter(errorStr, str2b,  b);  if (!ok) errorStr += " in B\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz); if (!ok) errorStr += " in Height\n";
}

bool AGeoEltu::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2a .contains(nameRegExp)) return true;
    if (str2b .contains(nameRegExp)) return true;
    if (str2dz.contains(nameRegExp)) return true;

    return false;
}

void AGeoEltu::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2a .replace(nameRegExp, newName);
    str2b .replace(nameRegExp, newName);
    str2dz.replace(nameRegExp, newName);
}

bool AGeoEltu::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 3);
    if (!ok) return false;

    double tmp[3];
    for (int i=0; i<3; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoEltu";
            return false;
        }
    }

    a  = tmp[0];
    b  = tmp[1];
    dz = tmp[2];
    return true;
}

TGeoShape *AGeoEltu::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoEltu(a, b, dz) :
                                   new TGeoEltu(shapeName.toLatin1().data(), a, b, dz);
}

QString AGeoEltu::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoEltu( " +
                QString::number(a)+", "+
                QString::number(b)+", "+
                QString::number(dz)+" )";
    }
    else
    {
        QString sdz = (str2dz.isEmpty() ? QString::number(dz) : "' + 0.5*(" + str2dz + ") + '");
        QString sa   = (str2a .isEmpty() ? QString::number(a) : "' + 0.5*(" + str2a  + ") + '");
        QString sb   = (str2b .isEmpty() ? QString::number(b) : "' + 0.5*(" + str2b  + ") + '");

        str = "TGeoEltu( " +
                sa  + ", "+
                sb  + ", "+
                sdz + " )";


    }
    return str;
}

QString AGeoEltu::getScriptString(bool useStrings) const
{
    QString sdx, sdy, sdz;
    if (useStrings)
    {
        sdx = ( str2a.isEmpty()  ? QString::number(2.0 * a)  : str2a );
        sdy = ( str2b.isEmpty()  ? QString::number(2.0 * b)  : str2b );
        sdz = ( str2dz.isEmpty() ? QString::number(2.0 * dz) : str2dz );
    }
    else
    {
        sdx = QString::number(2.0 * a);
        sdy = QString::number(2.0 * b);
        sdz = QString::number(2.0 * dz);
    }

    //void AGeo_SI::tubeElliptical(QString name, double Dx, double Dy, double height, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
    return QString("geo.tubeElliptical( $name$,  %0, %1, %2,  ").arg(sdx, sdy, sdz);
}

double AGeoEltu::maxSize() const
{
    double m = std::max(a, b);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

void AGeoEltu::writeToJson(QJsonObject &json) const
{
    json["a"]  = a;
    json["b"]  = b;
    json["dz"] = dz;

    if (!str2a .isEmpty()) json["str2a"]  = str2a;
    if (!str2b .isEmpty()) json["str2b"]  = str2b;
    if (!str2dz.isEmpty()) json["str2dz"] = str2dz;
}

void AGeoEltu::readFromJson(const QJsonObject &json)
{
    a  = json["a"] .toDouble();
    b  = json["b"] .toDouble();
    dz = json["dz"].toDouble();

    if (!jstools::parseJson(json, "str2a", str2a))   str2a.clear();
    if (!jstools::parseJson(json, "str2b", str2b))   str2b.clear();
    if (!jstools::parseJson(json, "str2dz", str2dz)) str2dz.clear();
}

bool AGeoEltu::readFromTShape(TGeoShape *Tshape)
{
    TGeoEltu* s = dynamic_cast<TGeoEltu*>(Tshape);
    if (!s) return false;

    a = s->GetA();
    b = s->GetB();
    dz = s->GetDz();

    return true;
}

void AGeoEltu::scale(double factor)
{
    a  *= factor;
    b  *= factor;
    dz *= factor;
}

AGeoArb8::AGeoArb8(double dz, std::array<std::pair<double, double>, 8> NodesList) :
    dz(dz), Vertices(NodesList) {}

AGeoArb8::AGeoArb8() : dz(10.0)
{
    init();
}

QString AGeoArb8::getHelp() const
{
    QString s = "An Arb8 is defined by two quadrilaterals sitting on parallel planes, at ±dZ. These are defined each by 4 vertices "
                "having the coordinates (Xi,Yi,+/-dZ), i=0, 3. The lateral surface of the Arb8 is defined by the 4 pairs of "
                "edges corresponding to vertices (i,i+1) on both -dZ and +dZ. If M and M' are the middles of the segments "
                "(i,i+1) at -dZ and +dZ, a lateral surface is obtained by sweeping the edge at -dZ along MM' so that it will "
                "match the corresponding one at +dZ. Since the points defining the edges are arbitrary, the lateral surfaces are "
                "not necessary planes – but twisted planes having a twist angle linear-dependent on Z.\n"
                "Vertices have to be defined clockwise in the XY pane, both at +dz and –dz. The quadrilateral at -dz is defined "
                "by indices [0,3], whereas the one at +dz by vertices [4,7]. Any two or more vertices in each Z plane can "
                "have the same (X,Y) coordinates. It this case, the top and bottom quadrilaterals become triangles, segments or "
                "points. The lateral surfaces are not necessary defined by a pair of segments, but by pair segment-point (making "
                "a triangle) or point-point (making a line). Any choice is valid as long as at one of the end-caps is at least a "
                "triangle.";
    return s;
}

void AGeoArb8::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2dz, dz); if (!ok) errorStr += " in Height\n";
    for (int i = 0; i < 8; i++)
    {
        ok = GC.updateDoubleParameter(errorStr, strVertices[i].first,  Vertices[i].first,  false, false, false); if (!ok) errorStr += QString(" in X[%0]\n").arg(i);
        ok = GC.updateDoubleParameter(errorStr, strVertices[i].second, Vertices[i].second, false, false, false); if (!ok) errorStr += QString(" in Y[%0]\n").arg(i);
    }

    if (!checkPointsForArb8(Vertices))
        errorStr += "Nodes of AGeoArb8 should be defined clockwise on both planes\n";
}

bool AGeoArb8::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (str2dz.contains(nameRegExp)) return true;

    for (int i =0; i<8; i++)
    {
        if (strVertices[i].first.contains(nameRegExp))  return true;
        if (strVertices[i].second.contains(nameRegExp)) return true;
    }
    return false;
}

void AGeoArb8::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    str2dz.replace(nameRegExp, newName);

    for (int i =0; i<8; i++)
    {
        strVertices[i].first. replace(nameRegExp, newName);
        strVertices[i].second.replace(nameRegExp, newName);
    }
}

bool AGeoArb8::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 17);
    if (!ok) return false;

    double tmp[17];
    for (int i=0; i<17; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoArb8";
            return false;
        }
    }

    dz = tmp[0];
    for (int i = 0; i < 8; i++)
    {
        Vertices[i].first  = tmp[1 + i*2];
        Vertices[i].second = tmp[2 + i*2];
    }
    //  qDebug() << dz << Vertices;

    if (!checkPointsForArb8(Vertices))
    {
        qWarning() << "Nodes of AGeoArb8 should be defined clockwise on both planes";
        return false;
    }

    return true;
}

TGeoShape *AGeoArb8::createGeoShape(const QString shapeName)
{
    double ar[8][2];
    for (int i=0; i<8; i++)
    {
        ar[i][0] = Vertices.at(i).first;
        ar[i][1] = Vertices.at(i).second;
    }

    return (shapeName.isEmpty()) ? new TGeoArb8(dz, (double*)ar) : new TGeoArb8(shapeName.toLatin1().data(), dz, (double*)ar);
}

QString AGeoArb8::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoArb8( " + QString::number(dz)+",  ";

        QString s;
        for (int i=0; i<4; i++)
            s += ", " + QString::number(Vertices.at(i).first) + "," + QString::number(Vertices.at(i).second);
        s.remove(0, 1);
        str += s + ",   ";

        s = "";
        for (int i=4; i<8; i++)
            s += ", " + QString::number(Vertices.at(i).first) + "," + QString::number(Vertices.at(i).second);
        s.remove(0, 1);
        str += s + " )";
    }
    else
    {
        QString sdz     = (str2dz    .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz    + ") + '");
        str = "TGeoArb8( " + sdz + ",  ";

        QString s;
        QString s0, s1;

        for (int i=0; i<8; i++)
        {
            s0 = (strVertices[i].first.isEmpty()  ? QString::number(Vertices[i].first)  : "' + (" + strVertices[i].first  + ") + '");
            s1 = (strVertices[i].second.isEmpty() ? QString::number(Vertices[i].second) : "' + (" + strVertices[i].second + ") + '");
            s +=s0 + "," + s1 + ", ";
        }
        str += s + " )";
    }

    return str;
}

QString AGeoArb8::getScriptString(bool useStrings) const
{
    QString sh;
    QString s0, s1;
    QString nodes = "[ ";
    if (useStrings)
    {
        sh = ( str2dz.isEmpty() ? QString::number(2.0 * dz) : str2dz );

        for (int i = 0; i < 8; i++)
        {
            s0 = (strVertices[i].first.isEmpty()  ? QString::number(Vertices[i].first)  : strVertices[i].first);
            s1 = (strVertices[i].second.isEmpty() ? QString::number(Vertices[i].second) : strVertices[i].second);
            nodes += QString("[%0,%1]").arg(s0, s1);
            if (i != 7) nodes += ", ";
        }
    }
    else
    {
        sh = QString::number(2.0 * dz);

        for (int i = 0; i < 8; i++)
        {
            s0 = QString::number(Vertices[i].first);
            s1 = QString::number(Vertices[i].second);
            nodes += QString("[%0,%1]").arg(s0, s1);
            if (i != 7) nodes += ", ";
        }
    }
    nodes += " ]";

    //void arb8(QString name, QVariantList NodesXY, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
    return QString("geo.arb8( $name$,  %0, %1,  ").arg(nodes, sh);
}

double AGeoArb8::maxSize() const
{
    double max = dz;

    for (const std::pair<double, double> & pair : Vertices)
        max = std::max(max, std::max( fabs(pair.first), fabs(pair.second)) );

    return max;
}

void AGeoArb8::writeToJson(QJsonObject &json) const
{
    json["dz"] = dz;
    if (!str2dz.isEmpty()) json["str2dz"] = str2dz;

    QJsonArray ar;
    for (int i=0; i<8; i++)
    {
        QJsonArray el;
        el << Vertices.at(i).first;
        el << Vertices.at(i).second;
        ar.append(el);
    }
    json["Vertices"] = ar;

    QJsonArray strAr;

    for (int i=0; i<8; i++)
    {
        QJsonArray el;
        el << (strVertices[i].first.isEmpty()  ? "" : strVertices[i].first);
        el << (strVertices[i].second.isEmpty() ? "" : strVertices[i].second);
        strAr.append(el);
    }

    json["StrVertices"] = strAr;

}

void AGeoArb8::readFromJson(const QJsonObject &json)
{
    dz     = json["dz"].toDouble();
    str2dz = json["str2dz"].toString();

    QJsonArray ar = json["Vertices"].toArray();
    for (int i=0; i<8; i++)
    {
        QJsonArray el = ar[i].toArray();
        Vertices[i].first = el[0].toDouble();
        Vertices[i].second = el[1].toDouble();
    }

    QJsonArray strAr = json["StrVertices"].toArray();
    for (int i=0; i<8; i++)
    {
        QJsonArray el = strAr[i].toArray();
        strVertices[i].first  = el[0].toString();
        strVertices[i].second = el[1].toString();
    }
}

bool AGeoArb8::readFromTShape(TGeoShape *Tshape)
{
    TGeoArb8* s = dynamic_cast<TGeoArb8*>(Tshape);
    if (!s) return false;

    dz = s->GetDz();
    const double (*ar)[2] = (const double(*)[2])s->GetVertices(); //fXY[8][2]
    for (int i = 0; i < 8; i++)
        Vertices[i] = {ar[i][0], ar[i][1]};

    return true;
}

bool checkPointsArb8(const std::array<std::pair<double, double>,8> & nodes, bool bFirst)
{
    const int iDelta = (bFirst ? 0 : 4);
    double averageX = 0, averageY = 0;
    for (int i = 0 + iDelta; i < 4 + iDelta; i++)
    {
        averageX += nodes[i].first;
        averageY += nodes[i].second;
    }
    averageX /= 4.0;
    averageY /= 4.0;
    //qDebug() << "Center x,y:"<<X << Y;

    std::array<double, 8> angles; // 8 just to simplify
    int firstNotNAN = -1;
    for (int i = 0 + iDelta; i < 4 + iDelta; i++)
    {
        double dx = nodes[i].first  - averageX;
        double dy = nodes[i].second - averageY;
        double a = atan( fabs(dy)/fabs(dx) ) * 180.0 / 3.1415926535;
        if (a == a && firstNotNAN == -1) firstNotNAN = i;

        if      (dx > 0 && dy > 0) angles[i] = 360.0 - a;
        else if (dx < 0 && dy > 0) angles[i] = 180.0 + a;
        else if (dx < 0 && dy < 0) angles[i] = 180.0 - a;
        else                       angles[i] = a;
    }
    //qDebug() << "Raw angles:" << angles;
    // qDebug() << "First Not NAN:"<< firstNotNAN;
    if (firstNotNAN == -1) return true; //all 4 points are the same

    double delta = angles[firstNotNAN];
    for (int i = 0 + iDelta; i < 4 + iDelta; i++)
    {
        if (angles[i] == angles[i]) // not NAN
        {
            angles[i] -= delta;
            if (angles[i] < 0) angles[i] += 360.0;
        }
    }

    //qDebug() << "Shifted to first"<<angles;
    double A = angles[firstNotNAN];
    for (int i = firstNotNAN+1; i < 4 + iDelta; i++)
    {
        //qDebug() <<i<< angles[i];
        if (angles[i] != angles[i])
        {
            //qDebug() << "NAN, continue";
            continue;
        }
        if (angles[i] < A)
            return false;
        A = angles[i];
    }
    return true;
}

bool AGeoArb8::checkPointsForArb8(std::array<std::pair<double, double>,8> nodes)
{
    bool ok = checkPointsArb8(nodes, true);
    if (!ok) return false;
    return checkPointsArb8(nodes, false);
}

void AGeoArb8::scale(double factor)
{
    dz  *= factor;

    for (std::pair<double, double> & pair : Vertices)
    {
        pair.first  *= factor;
        pair.second *= factor;
    }
}

void AGeoArb8::init()
{
    Vertices[0] = {-20,20};
    Vertices[1] = {20,20};
    Vertices[2] = {20,-20};
    Vertices[3] = {-20,-20};

    Vertices[4] = {-10,10};
    Vertices[5] = {10,10};
    Vertices[6] = {10,-10};
    Vertices[7] = {10,-10};
}

AGeoPcon::AGeoPcon()
    : phi(0), dphi(360)
{
    Sections = { APolyCGsection(-5, 10, 20), APolyCGsection(5, 20, 40) };
}

QString AGeoPcon::getHelp() const
{
    return "TGeoPcon:\n"
           "phi - start angle [0, 360)\n"
           "dphi - range in angle (0, 360]\n"
           "{z : rmin : rmax} - arbitrary number of sections defined with z position, minimum and maximum radii";
}

void AGeoPcon::introduceGeoConstValues(QString & errorStr)
{
    bool ok;

    if (Sections.size() < 2)
    {
        errorStr = "There should be at least 2 sections\n";
        return;
    }

    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strPhi, phi,   false, false, false); if (!ok) errorStr += " in Phi\n";
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strdPhi, dphi, false, false, false); if (!ok) errorStr += " in dPhi\n";

    if ( phi <  0 || phi  >= 360) errorStr += "Phi should be in the range of [0, 360)\n";
    if (dphi <= 0 || dphi >  360) errorStr += "dPhi should be in the range of (0, 360]\n";

    for (int i = 0; i < Sections.size(); i++)
    {
        ok = Sections[i].updateShape(errorStr);
        if (!ok) return;
        if (i > 0 && Sections[i-1].z > Sections[i].z)
        {
            errorStr += "Sections' z coordinates are not in ascending order\n";
            return;
        }
        if ( i > 0  && (Sections[i-1] == Sections[i]))
        {
            errorStr += "Sections can't have duplicates\n";
            return;
        }
    }
    const int lastSection = Sections.size() -1;
    if (Sections[0].z == Sections[1].z || Sections[lastSection].z == Sections[lastSection-1].z)
        errorStr += "Not allowed first two or last two sections at same Z\n";
}

bool AGeoPcon::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strPhi .contains(nameRegExp)) return true;
    if (strdPhi.contains(nameRegExp)) return true;

    for (const APolyCGsection &s : Sections)
    {
        if (s.isGeoConstInUse(nameRegExp)) return true;
    }

    return false;
}

void AGeoPcon::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strPhi .replace(nameRegExp, newName);
    strdPhi.replace(nameRegExp, newName);

    for (APolyCGsection &s : Sections)
    {
        s.replaceGeoConstName(nameRegExp, newName);
    }
}

bool AGeoPcon::readFromString(QString GenerationString)
{
    Sections.clear();

    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, -1);
    if (!ok) return false;

    if (params.size()<4)
    {
        qWarning() << "Not enough parameters to define TGeoPcon";
        return false;
    }

    phi = params[0].toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Syntax error found during extracting parameters of TGeoPcon";
        return false;
    }
    dphi = params[1].toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Syntax error found during extracting parameters of TGeoPcon";
        return false;
    }

    for (int i=2; i<params.size(); i++)
    {
        APolyCGsection section;
        if (!section.fromString(params.at(i)))
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoPcon";
            return false;
        }
        Sections.push_back(section);
    }
    //qDebug() << phi<<dphi;
    //for (int i=0; i<Sections.size(); i++) qDebug() << Sections.at(i).toString();
    return true;
}

TGeoShape *AGeoPcon::createGeoShape(const QString shapeName)
{
    TGeoPcon* pc = (shapeName.isEmpty()) ? new TGeoPcon(phi, dphi, Sections.size()) :
                                           new TGeoPcon(shapeName.toLatin1().data(), phi, dphi, Sections.size());
    for (int i=0; i<Sections.size(); i++)
    {
        const APolyCGsection & s = Sections.at(i);
        pc->DefineSection(i, s.z, s.rmin, s.rmax);
    }
    return pc;
}

double AGeoPcon::getHeight() const
{
    double res = Sections[Sections.size()-1].z - Sections[0].z;
    res *=0.5;
    return res;
}

QString AGeoPcon::getFullHeightString()
{
    const APolyCGsection & botSec = Sections.front();
    const APolyCGsection & topSec = Sections.back();

    if (botSec.strZ.isEmpty() && topSec.strZ.isEmpty()) return "";

    QString strBot = (botSec.strZ.isEmpty() ? QString::number(botSec.z) : botSec.strZ);
    QString strTop = (topSec.strZ.isEmpty() ? QString::number(topSec.z) : topSec.strZ);

    return QString("(%0 - %1)").arg(strTop).arg(strBot);
}

double AGeoPcon::getRelativePosZofCenter() const
{
    double res = Sections[0].z + getHeight();

    return res;
}

QString AGeoPcon::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoPcon( " +
                QString::number(phi)+", "+
                QString::number(dphi);

        for (const APolyCGsection & s : Sections) str += ", " + s.toString(false);

        str +=" )";
    }
    else
    {
        QString sphi    = (strPhi    .isEmpty() ? QString::number(phi)   : "' + (" + strPhi  + ") + '");
        QString sdphi   = (strdPhi   .isEmpty() ? QString::number(dphi)  : "' + (" + strdPhi + ") + '");

        str = "TGeoPcon( " +
                sphi + ", "+
                sdphi ;

        for (const APolyCGsection & s : Sections) str += ", " + s.toString(true);

        str +=" )";
    }
    return str;
}

QString AGeoPcon::getScriptString(bool useStrings) const
{
    QString sphi, sdphi, sec;
    if (useStrings)
    {
        sphi  = (strPhi .isEmpty() ? QString::number(phi)  : strPhi  );
        sdphi = (strdPhi.isEmpty() ? QString::number(dphi) : strdPhi );
    }
    else
    {
        sphi  = QString::number(phi);
        sdphi = QString::number(dphi);
    }

    for (int i = 0; i < Sections.size(); i++)
    {
        if (i != 0) sec += ", ";
        sec += Sections[i].toScriptString(useStrings);
    }

    //void pCone(QString name, QVariantList sections, double Phi, double dPhi,
    return QString("geo.pCone( $name$,  [ %0 ], %1, %2,  ").arg(sec, sphi, sdphi);
}

double AGeoPcon::maxSize() const
{
    double m = 0.5*fabs(Sections.front().z - Sections.back().z);
    for (size_t i = 0; i < Sections.size(); i++)
        m = std::max(m, Sections[i].rmax);
    return sqrt(3.0)*m;
}

double AGeoPcon::minSize() const
{
    double min = 0;
    for (const APolyCGsection & ps : Sections)
        min = std::min(min, ps.rmax);
    return min;
}

void AGeoPcon::writeToJson(QJsonObject &json) const
{
    json["phi"]  = phi;
    json["dphi"] = dphi;

    if (!strPhi .isEmpty()) json["strPhi"]  = strPhi;
    if (!strdPhi.isEmpty()) json["strdPhi"] = strdPhi;

    QJsonArray ar;
    for (const APolyCGsection & s : Sections)
    {
        QJsonObject js;
        s.writeToJson(js);
        ar << js;
    }
    json["Sections"] = ar;
}

void AGeoPcon::readFromJson(const QJsonObject &json)
{
    Sections.clear();

    jstools::parseJson(json, "phi", phi);
    jstools::parseJson(json, "dphi", dphi);

    if (!jstools::parseJson(json, "strPhi",  strPhi))  strPhi .clear();
    if (!jstools::parseJson(json, "strdPhi", strdPhi)) strdPhi.clear();

    QJsonArray ar;
    jstools::parseJson(json, "Sections", ar);
    if (ar.size()<2)
    {
        qWarning() << "Error in reading AGeoPcone from json";
        Sections.resize(2);
        return;
    }

    for (int i=0; i<ar.size(); i++)
    {
        QJsonObject js = ar[i].toObject();
        APolyCGsection s;
        s.readFromJson(js);
        Sections.push_back(s);
    }
    if (Sections.size()<2)
    {
        qWarning() << "Error in reading AGeoPcone from json";
        Sections.resize(2);
    }
}

bool AGeoPcon::readFromTShape(TGeoShape *Tshape)
{
    TGeoPcon* pc = dynamic_cast<TGeoPcon*>(Tshape);
    if (!pc) return false;

    phi = pc->GetPhi1();
    dphi = pc->GetDphi();

    Sections.clear();
    for (int i=0; i<pc->GetNz(); i++)
        Sections.push_back( APolyCGsection(pc->GetZ()[i], pc->GetRmin()[i], pc->GetRmax()[i]) );

    //qDebug() << "Pcone loaded from TShape..."<<phi<<dphi;
    //for (int i=0; i<Sections.size(); i++) qDebug() << Sections.at(i).toString();

    return true;
}

void AGeoPcon::scale(double factor)
{
    for (APolyCGsection & sec : Sections) sec.scale(factor);
}

bool APolyCGsection::updateShape(QString &errorStr)
{
    bool ok;

    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, strZ,     z,    false, false, false);
    if (!ok)
    {
        errorStr += " in Z\n";
        return false;
    }
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, str2rmin, rmin, false);
    if (!ok)
    {
        errorStr += " in Dmin\n";
        return false;
    }
    ok = AGeoConsts::getConstInstance().updateDoubleParameter(errorStr, str2rmax, rmax);
    if (!ok)
    {
        errorStr += " in Dmax\n";
        return false;
    }

    if (rmin >= rmax)
    {
        errorStr = "Inside diameter should be smaller than the outside one!\n";
        return false;
    }
    return true;
}

bool APolyCGsection::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strZ    .contains(nameRegExp)) return true;
    if (str2rmin.contains(nameRegExp)) return true;
    if (str2rmax.contains(nameRegExp)) return true;

    return false;
}

void APolyCGsection::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strZ    .replace(nameRegExp, newName);
    str2rmin.replace(nameRegExp, newName);
    str2rmax.replace(nameRegExp, newName);
}

bool APolyCGsection::fromString(QString s)
{
    s = s.simplified();
    s.remove(" ");
    if (!s.startsWith("{") || !s.endsWith("}")) return false;

    s.remove("{");
    s.remove("}");
    QStringList l = s.split(":");
    if (l.size()!=3) return false;

    bool ok;
    z = l[0].toDouble(&ok);
    if (!ok) return false;
    rmin = l[1].toDouble(&ok);
    if (!ok) return false;
    rmax = l[2].toDouble(&ok);
    if (!ok) return false;

    return true;
}

QString APolyCGsection::toString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = QString("{ ") +
                QString::number(z)    + " : " +
                QString::number(rmin) + " : " +
                QString::number(rmax) +
                " }";
    }
    else
    {
        QString sz    = (strZ    .isEmpty() ? QString::number(z)    : "' + (" +      strZ    + ") + '");
        QString srmin = (str2rmin.isEmpty() ? QString::number(rmin) : "' + 0.5*(" + str2rmin + ") + '");
        QString srmax = (str2rmax.isEmpty() ? QString::number(rmax) : "' + 0.5*(" + str2rmax + ") + '");

        str = QString("{ ") +
                sz     + " : " +
                srmin  + " : " +
                srmax  + " }";

    }
    return str;
}

QString APolyCGsection::toScriptString(bool useStrings) const
{
    QString sz, sdmin, sdmax;
    if (useStrings)
    {
        sz    = ( strZ    .isEmpty() ? QString::number(z)          : strZ     );
        sdmin = ( str2rmin.isEmpty() ? QString::number(2.0 * rmin) : str2rmin );
        sdmax = ( str2rmax.isEmpty() ? QString::number(2.0 * rmax) : str2rmax );
    }
    else
    {
        sz    = QString::number(z);
        sdmin = QString::number(2.0 * rmin);
        sdmax = QString::number(2.0 * rmax);
    }
    return QString("[%0, %1, %2]").arg(sz, sdmin, sdmax);
}

void APolyCGsection::writeToJson(QJsonObject &json) const
{
    json["z"]    = z;
    json["rmin"] = rmin;
    json["rmax"] = rmax;

    if (!strZ    .isEmpty()) json["strZ"]     = strZ;
    if (!str2rmin.isEmpty()) json["str2rmin"] = str2rmin;
    if (!str2rmax.isEmpty()) json["str2rmax"] = str2rmax;
}

void APolyCGsection::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "z", z);
    jstools::parseJson(json, "rmin", rmin);
    jstools::parseJson(json, "rmax", rmax);

    if (!jstools::parseJson(json, "strZ",     strZ))     strZ.clear();
    if (!jstools::parseJson(json, "str2rmin", str2rmin)) str2rmin.clear();
    if (!jstools::parseJson(json, "str2rmax", str2rmax)) str2rmax.clear();

    QString errorStr = "";
    updateShape(errorStr);
}

bool APolyCGsection::operator ==(const APolyCGsection &section) const
{
    if ((z == section.z) && (rmin == section.rmin) && (rmax == section.rmax))
        return true;
    return false;
}

void APolyCGsection::scale(double factor)
{
    z    *= factor;
    rmin *= factor;
    rmax *= factor;
}

// --- GeoPolygon ---
QString AGeoPolygon::getHelp() const
{
    return "Simplified TGeoPgon:\n"
           "nedges - number of edges\n"
           "dphi - angle (0, 360]\n"
           "dz - half size in Z\n"
           "rminL - inner size on lower side\n"
           "rmaxL - outer size on lower side\n"
           "rminU - inner size on uppder side\n"
           "rmaxU - outer size on upper side\n";
}

void AGeoPolygon::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateIntParameter(errorStr,    strNedges, nedges, true, true);        if (!ok) errorStr += " in NumEdges\n";
    ok = GC.updateDoubleParameter(errorStr, strdPhi,   dphi, false, false, false); if (!ok) errorStr += " in dPhi\n";
    ok = GC.updateDoubleParameter(errorStr, str2dz,    dz);                        if (!ok) errorStr += " in Height\n";
    ok = GC.updateDoubleParameter(errorStr, str2rminL, rminL, false);              if (!ok) errorStr += " in DminL\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmaxL, rmaxL, false);              if (!ok) errorStr += " in DmaxL\n";
    ok = GC.updateDoubleParameter(errorStr, str2rminU, rminU, false);              if (!ok) errorStr += " in DminU\n";
    ok = GC.updateDoubleParameter(errorStr, str2rmaxU, rmaxU, false);              if (!ok) errorStr += " in DmaxU\n";

    if (nedges < 3)                   errorStr += "There should be at least 3 edges\n";
    if (rminL >= rmaxL)               errorStr += "Inside lower diameter should be smaller than the outside one!\n";
    if (rminU >= rmaxU)               errorStr += "Inside upper diameter should be smaller than the outside one!\n";
    if (dphi  <= 0 || dphi > 360.0)   errorStr += "Phi2 should be in the range of (0, 360]\n";
}

bool AGeoPolygon::isGeoConstInUse(const QRegularExpression &nameRegExp) const
{
    if (strNedges.contains(nameRegExp)) return true;
    if (strdPhi  .contains(nameRegExp)) return true;
    if (str2dz   .contains(nameRegExp)) return true;
    if (str2rminL.contains(nameRegExp)) return true;
    if (str2rmaxL.contains(nameRegExp)) return true;
    if (str2rminU.contains(nameRegExp)) return true;
    if (str2rmaxU.contains(nameRegExp)) return true;
    return false;

}

void AGeoPolygon::replaceGeoConstName(const QRegularExpression &nameRegExp, const QString &newName)
{
    strNedges.replace(nameRegExp, newName);
    strdPhi  .replace(nameRegExp, newName);
    str2dz   .replace(nameRegExp, newName);
    str2rminL.replace(nameRegExp, newName);
    str2rmaxL.replace(nameRegExp, newName);
    str2rminU.replace(nameRegExp, newName);
    str2rmaxU.replace(nameRegExp, newName);
}

bool AGeoPolygon::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 7);
    if (!ok) return false;

    double dn = params[0].toDouble(&ok);
    if (!ok)
    {
        qDebug() <<"right here bug";
        qWarning() << "Syntax error found during extracting parameters of TGeoPolygon";
        return false;
    }

    double tmp[7];
    for (int i=1; i<7; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of TGeoPolygon";
            return false;
        }
    }

    nedges = dn;
    dphi = tmp[1];
    dz = tmp[2];
    rminL = tmp[3];
    rmaxL = tmp[4];
    rminU = tmp[5];
    rmaxU = tmp[6];
    return true;
}

TGeoShape *AGeoPolygon::createGeoShape(const QString shapeName)
{
    TGeoPgon* s = (shapeName.isEmpty()) ? new TGeoPgon(0, dphi, nedges, 2) :
                                          new TGeoPgon(shapeName.toLatin1().data(), 0, dphi, nedges, 2);
    s->DefineSection(0, -dz, rminL, rmaxL);
    s->DefineSection(1, +dz, rminU, rmaxU);
    return s;
}

QString AGeoPolygon::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoPolygon( " +
                QString::number(nedges)+", "+
                QString::number(dphi)+", "+
                QString::number(dz)+", "+
                QString::number(rminL)+", "+
                QString::number(rmaxL)+", "+
                QString::number(rminU)+", "+
                QString::number(rmaxU)+" )";
    }
    else
    {
        QString snedges = (strNedges .isEmpty() ? QString::number(nedges): "' + ("      + strNedges + ") + '");
        QString sdphi   = (strdPhi   .isEmpty() ? QString::number(dphi)  : "' + ("      + strdPhi   + ") + '");
        QString sdz     = (str2dz    .isEmpty() ? QString::number(dz)    : "' + 0.5*(" + str2dz    + ") + '");
        QString srminL  = (str2rminL .isEmpty() ? QString::number(rminL) : "' + 0.5*(" + str2rminL + ") + '");
        QString srmaxL  = (str2rmaxL .isEmpty() ? QString::number(rmaxL) : "' + 0.5*(" + str2rmaxL + ") + '");
        QString srminU  = (str2rminU .isEmpty() ? QString::number(rminU) : "' + 0.5*(" + str2rminU + ") + '");
        QString srmaxU  = (str2rmaxU .isEmpty() ? QString::number(rmaxU) : "' + 0.5*(" + str2rmaxU + ") + '");

        str = "TGeoPolygon( " +
                snedges + ", "+
                sdphi   + ", "+
                sdz     + ", "+
                srminL  + ", "+
                srmaxL  + ", "+
                srminU  + ", "+
                srmaxU  + " )";
    }
    return str;
}

QString AGeoPolygon::getScriptString(bool useStrings) const
{
    QString sNedges, sdPhi, s2dz, s2rminL, s2rmaxL, s2rminU, s2rmaxU;

    if (useStrings)
    {
        sNedges = ( strNedges.isEmpty() ? QString::number(nedges)      : strNedges );
        sdPhi   = ( strdPhi  .isEmpty() ? QString::number(dphi)        : strdPhi );
        s2dz    = ( str2dz   .isEmpty() ? QString::number(2.0 * dz)    : str2dz );
        s2rminL = ( str2rminL.isEmpty() ? QString::number(2.0 * rminL) : str2rminL );
        s2rmaxL = ( str2rmaxL.isEmpty() ? QString::number(2.0 * rmaxL) : str2rmaxL );
        s2rminU = ( str2rminU.isEmpty() ? QString::number(2.0 * rminU) : str2rminU );
        s2rmaxU = ( str2rmaxU.isEmpty() ? QString::number(2.0 * rmaxU) : str2rmaxU );
    }
    else
    {
        sNedges = QString::number(nedges);
        sdPhi   = QString::number(dphi);
        s2dz    = QString::number(2.0 * dz);
        s2rminL = QString::number(2.0 * rminL);
        s2rmaxL = QString::number(2.0 * rmaxL);
        s2rminU = QString::number(2.0 * rminU);
        s2rmaxU = QString::number(2.0 * rmaxU);
    }

    if (sdPhi == "360" && s2rminL == "0" && s2rminU == "0" && s2rmaxL == s2rmaxU)
    {
        //void AGeo_SI::polygon(QString name, int edges, double diameter, double h, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi)
        return QString("geo.polygon( $name$, %0, %1, %2,  ").arg(sNedges, s2rmaxU, s2dz);
    }
    else
    {
        //void polygonSegment(QString name, int edges, double DtopOut, double DtopIn, double DbotOut, double DbotIn, double h, double dPhi, int iMat, QString container, double x, double y, double z, double phi, double theta, double psi);
        return QString("geo.polygonSegment( $name$,  %0,  %1, %2,  %3,  %4, %5, %6,  ").arg(sNedges, s2rmaxU,s2rminU, s2rmaxL,s2rminL, s2dz,  sdPhi);
    }
}

double AGeoPolygon::maxSize() const
{
    double m = std::max(rmaxL, rmaxU);
    m = std::max(m, dz);
    return sqrt(3.0)*m;
}

double AGeoPolygon::minSize() const
{
    return std::min(rmaxL, rmaxU);
}

void AGeoPolygon::writeToJson(QJsonObject &json) const
{
    json["nedges"] = nedges;
    json["dphi"]   = dphi;
    json["dz"]     = dz;
    json["rminL"]  = rminL;
    json["rmaxL"]  = rmaxL;
    json["rminU"]  = rminU;
    json["rmaxU"]  = rmaxU;

    if (!strNedges.isEmpty()) json ["strNedges"] = strNedges;
    if (!strdPhi  .isEmpty()) json ["strdPhi"]   = strdPhi;
    if (!str2dz   .isEmpty()) json ["str2dz"]    = str2dz;
    if (!str2rminL.isEmpty()) json ["str2rminL"] = str2rminL;
    if (!str2rmaxL.isEmpty()) json ["str2rmaxL"] = str2rmaxL;
    if (!str2rminU.isEmpty()) json ["str2rminU"] = str2rminU;
    if (!str2rmaxU.isEmpty()) json ["str2rmaxU"] = str2rmaxU;
}

void AGeoPolygon::readFromJson(const QJsonObject &json)
{
    nedges = json["nedges"].toInt();
    dphi   = json["dphi"]  .toDouble();
    dz     = json["dz"]    .toDouble();
    rminL  = json["rminL"] .toDouble();
    rmaxL  = json["rmaxL"] .toDouble();
    rminU  = json["rminU"] .toDouble();
    rmaxU  = json["rmaxU"] .toDouble();

    if (!jstools::parseJson(json, "strNedges", strNedges)) strNedges.clear();
    if (!jstools::parseJson(json, "strdPhi",   strdPhi))   strdPhi  .clear();
    if (!jstools::parseJson(json, "str2dz",    str2dz))    str2dz   .clear();
    if (!jstools::parseJson(json, "str2rminL", str2rminL)) str2rminL.clear();
    if (!jstools::parseJson(json, "str2rmaxL", str2rmaxL)) str2rmaxL.clear();
    if (!jstools::parseJson(json, "str2rminU", str2rminU)) str2rminU.clear();
    if (!jstools::parseJson(json, "str2rmaxU", str2rmaxU)) str2rmaxU.clear();
}

void AGeoPolygon::scale(double factor)
{
    rminL *= factor;
    rmaxL *= factor;
    rminU *= factor;
    rmaxU *= factor;
    dz    *= factor;
}

AGeoScaledShape::AGeoScaledShape(QString ShapeGenerationString, double scaleX, double scaleY, double scaleZ) :
    BaseShapeGenerationString(ShapeGenerationString),
    scaleX(scaleX), scaleY(scaleY), scaleZ(scaleZ) {}

QString AGeoScaledShape::getHelp() const
{
    return "TGeoShape scaled with TGeoScale transformation";
}

void AGeoScaledShape::updateScalingFactors(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, strScaleX, scaleX, true, true, false); if (!ok) errorStr += " in X Scaling\n";
    ok = GC.updateDoubleParameter(errorStr, strScaleY, scaleY, true, true, false); if (!ok) errorStr += " in Y Scaling\n";
    ok = GC.updateDoubleParameter(errorStr, strScaleZ, scaleZ, true, true, false); if (!ok) errorStr += " in Z Scaling\n";
}

void AGeoScaledShape::introduceGeoConstValues(QString & errorStr)
{
    updateScalingFactors(errorStr);
}

bool AGeoScaledShape::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (strScaleX.contains(nameRegExp)) return true;
    if (strScaleY.contains(nameRegExp)) return true;
    if (strScaleZ.contains(nameRegExp)) return true;

    if (BaseShape) return BaseShape->isGeoConstInUse(nameRegExp);
    return false;
}

void AGeoScaledShape::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    strScaleX.replace(nameRegExp, newName);
    strScaleY.replace(nameRegExp, newName);
    strScaleZ.replace(nameRegExp, newName);

    if (BaseShape) BaseShape->replaceGeoConstName(nameRegExp, newName);
}

bool AGeoScaledShape::readFromString(QString GenerationString)
{
    GenerationString = GenerationString.simplified();
    //qDebug() << GenerationString;
    if (!GenerationString.startsWith(getShapeType()))
    {
        qWarning() << "Attempt to generate AGeoScaledShape using wrong type!";
        return false;
    }
    GenerationString.remove(getShapeType());
    GenerationString = GenerationString.simplified();
    if (GenerationString.endsWith("\\)"))
    {
        qWarning() << "Format error in AGeoScaledShape read from string";
        return false;
    }
    GenerationString.chop(1);
    if (GenerationString.startsWith("\\("))
    {
        qWarning() << "Format error in AGeoScaledShape read from string";
        return false;
    }
    GenerationString.remove(0, 1);
    GenerationString.remove(QString(" "));

    QStringList l1 = GenerationString.split(')');
    if (l1.size() < 2)
    {
        qWarning() << "Format error in AGeoScaledShape read from string";
        return false;
    }
    qDebug() <<"ageoscaled: read from string" <<l1;

    QString generator = l1.first() + ")";
    TGeoShape* sh = AGeoScaledShape::generateBaseTGeoShape(generator);
    qDebug() <<"is valid" <<sh->IsValidBox();
    if (!sh)
    {
        qWarning() << "Not valid generation string:"<<GenerationString;
        return false;
    }

    QString shapeType = generator.left(generator.indexOf('('));
    AGeoShape* Ashape = AGeoShape::GeoShapeFactory(shapeType);
    if (Ashape)
    {
        qDebug() <<"1" <<"generation str" <<Ashape->getGenerationString(true);
        bool fOK = Ashape->readFromString(generator);
        qDebug() <<"2" <<"ashape" <<"type" <<Ashape->getShapeType() <<"generation str" <<Ashape->getGenerationString(true);
        if (!fOK)
        {
            qWarning() << "failed to create base shape from string:"<<GenerationString;
        }
    }

    BaseShape = Ashape;
    BaseShapeGenerationString = generator;
    qDebug()<<BaseShapeGenerationString;

    QString params = l1.last(); // should be ",scaleX,scaleY,ScaleZ"
    params.remove(0, 1);
    QStringList l2 = params.split(',');
    if (l2.count() != 3)
    {
        qWarning() << "Number of scaling parameters in AGeoScaledShape should be three!";
        return false;
    }
    bool ok = false;
    scaleX = l2.at(0).toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Scaling parameter error in AGeoScaledShape.";
        return false;
    }
    scaleY = l2.at(1).toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Scaling parameter error in AGeoScaledShape.";
        return false;
    }
    scaleZ = l2.at(2).toDouble(&ok);
    if (!ok)
    {
        qWarning() << "Scaling parameter error in AGeoScaledShape.";
        return false;
    }
    qDebug() <<"readFrom string" <<scaleX << scaleY <<scaleZ;
    delete sh;
    return true;
}

TGeoShape* AGeoScaledShape::generateBaseTGeoShape(const QString & BaseShapeGenerationString) const
{
    //qDebug() << "SCALED->: Generating base shape from "<< BaseShapeGenerationString;
    QString shapeType = BaseShapeGenerationString.left(BaseShapeGenerationString.indexOf('('));
    //qDebug() << "SCALED->: base type:"<<shapeType;
    TGeoShape* Tshape = 0;
    qDebug() <<"base generation string shape type" <<shapeType;
    AGeoShape* Ashape = AGeoShape::GeoShapeFactory(shapeType);
    if (Ashape)
    {
        //qDebug() << "SCALED->" << "Created AGeoShape of type" << Ashape->getShapeType();
        qDebug() <<"generation str" <<Ashape->getGenerationString(true);
        bool fOK = Ashape->readFromString(BaseShapeGenerationString);
        qDebug() << "ashape" <<"type" <<Ashape->getShapeType() <<"generation str" <<Ashape->getGenerationString(true);
        if (fOK)
        {
            Tshape = Ashape->createGeoShape();
            if (!Tshape) qWarning() << "TGeoScaledShape processing: Base shape generation fail!";
        }
        else qWarning() << "TGeoScaledShape processing: failed to construct AGeoShape";
        delete Ashape;
    }
    else qWarning() << "TGeoScaledShape processing: unknown base shape type "<< shapeType;

    return Tshape;
}

TGeoShape *AGeoScaledShape::createGeoShape(const QString shapeName)
{
    /*
   TGeoShape* Tshape = generateBaseTGeoShape(BaseShapeGenerationString);
   if (!Tshape)
     {
       qWarning() << "->failed to generate shape using string:"<<BaseShapeGenerationString<<"\nreplacing by default TGeoBBox";
       Tshape = new TGeoBBox(5,5,5);
     }
   QString name = shapeName + "_base";
   Tshape->SetName(name.toLatin1());
   TGeoScale* scale = new TGeoScale(scaleX, scaleY, scaleZ);
   scale->RegisterYourself();

   return (shapeName.isEmpty()) ? new TGeoScaledShape(Tshape, scale) : new TGeoScaledShape(shapeName.toLatin1().data(), Tshape, scale);
   */

    TGeoShape * Tshape = BaseShape->createGeoShape();
    if (!Tshape)
    {
        qWarning() << "->failed to generate shape\nreplacing by default TGeoBBox";
        Tshape = new TGeoBBox(5,5,5);
    }

    QString name = shapeName + "_base";
    Tshape->SetName(name.toLatin1().data());

    TGeoScale* scale = new TGeoScale(scaleX, scaleY, scaleZ);
    scale->RegisterYourself();

    return (shapeName.isEmpty()) ? new TGeoScaledShape(Tshape, scale) : new TGeoScaledShape(shapeName.toLatin1().data(), Tshape, scale);
}

double AGeoScaledShape::getHeight() const
{
    if (!BaseShape) return 0;
    return BaseShape->getHeight() * scaleZ;
}

QString AGeoScaledShape::getFullHeightString()
{
    if (!BaseShape) return "";

    QString txt = BaseShape->getFullHeightString();
    if (txt.isEmpty()) return "";

    return QString("%0*(%1)").arg(scaleZ).arg(txt);
}

double AGeoScaledShape::getRelativePosZofCenter() const
{
    if (!BaseShape) return 0;
    return BaseShape->getRelativePosZofCenter() * scaleZ;
}

void AGeoScaledShape::setHeight(double dz)
{
    if (!BaseShape) return;
    BaseShape->setHeight(dz / scaleZ);
}

QString AGeoScaledShape::getGenerationString(bool useStrings) const
{
    if (!useStrings)
    {
        return QString() + "TGeoScaledShape( " +
                BaseShapeGenerationString + ", " +
                QString::number(scaleX) + ", " +
                QString::number(scaleY) + ", " +
                QString::number(scaleZ) +
                " )";
    }

    else
    {
        QString sscaleX = (strScaleX.isEmpty() ? QString::number(scaleX) : "' + (" + strScaleX + ") + '");
        QString sscaleY = (strScaleY.isEmpty() ? QString::number(scaleY) : "' + (" + strScaleY + ") + '");
        QString sscaleZ = (strScaleZ.isEmpty() ? QString::number(scaleZ) : "' + (" + strScaleZ + ") + '");

        QString bases = BaseShape->getGenerationString(true);

        return QString() + "TGeoScaledShape( " +
                bases + ", " +
                sscaleX + ", " +
                sscaleY + ", " +
                sscaleZ + " )";
    }
}

QString AGeoScaledShape::getScriptString(bool useStrings) const
{
    return BaseShape->getScriptString(useStrings); // the rest is made in the caller
}

QString AGeoScaledShape::getScriptString_Scaled(bool useStrings) const
{
    QString sx;
    QString sy;
    QString sz;

    if (useStrings)
    {
        sx = ( strScaleX.isEmpty() ? QString::number(scaleX) : strScaleX );
        sy = ( strScaleY.isEmpty() ? QString::number(scaleY) : strScaleY );
        sz = ( strScaleZ.isEmpty() ? QString::number(scaleZ) : strScaleZ );
    }
    else
    {
        sx = QString::number(scaleX);
        sy = QString::number(scaleY);
        sz = QString::number(scaleZ);
    }

    //void toScaled(QString name, double xFactor, double yFactor, double zFactor);
    return QString("geo.toScaled( $name$,  %0, %1, %2 )").arg(sx, sy, sz);
}

double AGeoScaledShape::maxSize() const
{
    if (!BaseShape) return 0;

    double size = BaseShape->maxSize();
    double factor = std::max(scaleX, scaleY);
    factor = std::max(factor, scaleZ);
    size *= factor;
    return size;
}

const QString AGeoScaledShape::getBaseShapeType() const
{
    /*
    QStringList sl = BaseShapeGenerationString.split('(', QString::SkipEmptyParts);

    if (sl.size() < 1) return "";
    return sl.first().simplified();
    */

    if (BaseShape) return BaseShape->getShapeType();
    else exit (-7777);
}

void AGeoScaledShape::writeToJson(QJsonObject &json) const
{
    json["scaleX"] = scaleX;
    json["scaleY"] = scaleY;
    json["scaleZ"] = scaleZ;

    if (!strScaleX.isEmpty()) json["strScaleX"] = strScaleX;
    if (!strScaleY.isEmpty()) json["strScaleY"] = strScaleY;
    if (!strScaleZ.isEmpty()) json["strScaleZ"] = strScaleZ;

    //json["CompositeGenerationstring"] = BaseShapeGenerationString;
    if (BaseShape)
    {
        BaseShape->writeToJson(json);
        json["shape"] = BaseShape->getShapeType();
    }
}

#include "aerrorhub.h"
void AGeoScaledShape::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "scaleX", scaleX);
    jstools::parseJson(json, "scaleY", scaleY);
    jstools::parseJson(json, "scaleZ", scaleZ);

    if (!jstools::parseJson(json, "strScaleX", strScaleX)) strScaleX.clear();
    if (!jstools::parseJson(json, "strScaleY", strScaleY)) strScaleY.clear();
    if (!jstools::parseJson(json, "strScaleZ", strScaleZ)) strScaleZ.clear();

    QString type = "TGeoBBox";
    jstools::parseJson(json, "shape", type);
    BaseShape = AGeoShape::GeoShapeFactory(type);
    if (BaseShape) BaseShape->readFromJson(json);

    if (!BaseShape)
    {
        BaseShape = new AGeoBox();
        QString errorStr = "Scaled shape generation failed, replacing with box";
        qWarning() << errorStr;
        AErrorHub::addQError(errorStr);
    }
}

bool AGeoScaledShape::readFromTShape(TGeoShape *Tshape)
{
    TGeoScaledShape* s = dynamic_cast<TGeoScaledShape*>(Tshape);
    if (!s) return false;

    TGeoScale* scale = s->GetScale();
    scaleX = scale->GetScale()[0];
    scaleY = scale->GetScale()[1];
    scaleZ = scale->GetScale()[2];

    TGeoShape* baseTshape = s->GetShape();
    QString stype = baseTshape->ClassName();
    AGeoShape* AShape = AGeoShape::GeoShapeFactory(stype);
    if (!AShape)
    {
        qWarning() << "AGeoScaledShape from TShape: error building AGeoShape";
        return false;
    }
    bool fOK = AShape->readFromTShape(baseTshape);
    if (!fOK)
    {
        qWarning() << "AGeoScaledShape from TShape: error reading base TShape";
        delete AShape;
        return false;
    }
    BaseShapeGenerationString = AShape->getGenerationString();
    BaseShape = AShape;
    return true;
}

void AGeoScaledShape::scale(double factor)
{
    if (BaseShape) BaseShape->scale(factor);
}

bool AGeoScaledShape::isCompatibleWithGeant4() const
{
    return true;
    // checked with root 6.30.06, my request for scaled compatibility with Geant4 seems to be fully implemented
}

QString AGeoTorus::getHelp() const
{
    return QString()+ "Torus segment:\n"
                      "• R - axial radius\n"
                      "• Rmin - inner radius\n"
                      "• Rmax - outer radius\n"
                      "• Phi1 - starting phi\n"
                      "• Dphi - phi extent";
}

void AGeoTorus::introduceGeoConstValues(QString & errorStr)
{
    const AGeoConsts & GC = AGeoConsts::getConstInstance();

    bool ok;
    ok = GC.updateDoubleParameter(errorStr, str2R,    R);                         if (!ok) errorStr += " in Axial Diameter\n";
    ok = GC.updateDoubleParameter(errorStr, str2Rmax, Rmax);                      if (!ok) errorStr += " in Outside Diameter\n";
    ok = GC.updateDoubleParameter(errorStr, str2Rmin, Rmin, false);               if (!ok) errorStr += " in Inner Diameter\n";
    ok = GC.updateDoubleParameter(errorStr, strPhi1,  Phi1, false, false, false); if (!ok) errorStr += " in Phi1\n";
    ok = GC.updateDoubleParameter(errorStr, strDphi,  Dphi, true,  true,  false); if (!ok) errorStr += " in Phi2\n";

    if (R <     Rmax) errorStr += "Axial diameter should be bigger or equal than outside one\n";
    if (Rmin >= Rmax) errorStr += "Inside diameter should be smaller than the outside one!\n";
}

bool AGeoTorus::isGeoConstInUse(const QRegularExpression & nameRegExp) const
{
    if (str2R   .contains(nameRegExp)) return true;
    if (str2Rmin.contains(nameRegExp)) return true;
    if (str2Rmax.contains(nameRegExp)) return true;
    if (strPhi1 .contains(nameRegExp)) return true;
    if (strDphi .contains(nameRegExp)) return true;

    return false;
}

void AGeoTorus::replaceGeoConstName(const QRegularExpression & nameRegExp, const QString & newName)
{
    str2R   .replace(nameRegExp, newName);
    str2Rmin.replace(nameRegExp, newName);
    str2Rmax.replace(nameRegExp, newName);
    strPhi1 .replace(nameRegExp, newName);
    strDphi .replace(nameRegExp, newName);
}

bool AGeoTorus::readFromString(QString GenerationString)
{
    QStringList params;
    bool ok = extractParametersFromString(GenerationString, params, 5);
    if (!ok) return false;

    double tmp[5];
    for (int i=0; i<5; i++)
    {
        tmp[i] = params[i].toDouble(&ok);
        if (!ok)
        {
            qWarning() << "Syntax error found during extracting parameters of AGeoTorus";
            return false;
        }
    }

    R = tmp[0];
    Rmin = tmp[1];
    Rmax = tmp[2];
    Phi1 = tmp[3];
    Dphi = tmp[4];
    return true;
}

TGeoShape *AGeoTorus::createGeoShape(const QString shapeName)
{
    return (shapeName.isEmpty()) ? new TGeoTorus(R, Rmin, Rmax, Phi1, Dphi) : new TGeoTorus(shapeName.toLatin1().data(), R, Rmin, Rmax, Phi1, Dphi);
}

QString AGeoTorus::getGenerationString(bool useStrings) const
{
    QString str;
    if (!useStrings)
    {
        str = "TGeoTorus( " +
                QString::number(R)+", "+
                QString::number(Rmin)+", "+
                QString::number(Rmax)+", "+
                QString::number(Phi1)+", "+
                QString::number(Dphi)+" )";
    }
    else
    {
        QString sR    = (str2R   .isEmpty() ? QString::number(R)     : "' + 0.5*(" + str2R +    ") + '");
        QString sRmin = (str2Rmin.isEmpty() ? QString::number(Rmin)  : "' + 0.5*(" + str2Rmin + ") + '");
        QString sRmax = (str2Rmax.isEmpty() ? QString::number(Rmax)  : "' + 0.5*(" + str2Rmax + ") + '");
        QString sPhi1 = (strPhi1 .isEmpty() ? QString::number(Phi1)  : "' + ("     + strPhi1   + ") + '");
        QString sDphi = (strDphi .isEmpty() ? QString::number(Dphi)  : "' + ("     + strDphi   + ") + '");

        str = "TGeoTorus( " +
                sR    +", "+
                sRmin +", "+
                sRmax +", "+
                sPhi1 +", "+
                sDphi +" )";
    }
    return str;
}

QString AGeoTorus::getScriptString(bool useStrings) const
{
    QString sD, sDmin, sDmax, sPhi, sDphi;
    if (useStrings)
    {
        sD    = (str2R.isEmpty()    ? QString::number(2.0 * R)    : str2R);
        sDmin = (str2Rmin.isEmpty() ? QString::number(2.0 * Rmin) : str2Rmin);
        sDmax = (str2Rmax.isEmpty() ? QString::number(2.0 * Rmax) : str2Rmax);
        sPhi  = (strPhi1 .isEmpty() ? QString::number(Phi1)       : strPhi1);
        sDphi = (strDphi .isEmpty() ? QString::number(Dphi)       : strDphi);
    }
    else
    {
        sD    = QString::number(2.0 * R);
        sDmin = QString::number(2.0 * Rmin);
        sDmax = QString::number(2.0 * Rmax);
        sPhi  = QString::number(Phi1);
        sDphi = QString::number(Dphi);
    }

    //void torus(QString name, double D, double Dout, double Din, double Phi, double dPhi,
    return QString("geo.torus( $name$,  %0, %1, %2, %3, %4,  ").arg(sD, sDmax, sDmin, sPhi, sDphi);
}

double AGeoTorus::maxSize() const
{
    //double m = std::max(R, Rmax);
    double m = R+Rmax;
    return sqrt(3.0)*m;
}

void AGeoTorus::writeToJson(QJsonObject &json) const
{
    json["R"] = R;
    json["Rmin"] = Rmin;
    json["Rmax"] = Rmax;
    json["Phi1"] = Phi1;
    json["Dphi"] = Dphi;

    if (!str2R.   isEmpty()) json["str2R"]    = str2R;
    if (!str2Rmin.isEmpty()) json["str2Rmin"] = str2Rmin;
    if (!str2Rmax.isEmpty()) json["str2Rmax"] = str2Rmax;
    if (!strPhi1. isEmpty()) json["strPhi1"]  = strPhi1;
    if (!strDphi. isEmpty()) json["strDphi"]  = strDphi;
}

void AGeoTorus::readFromJson(const QJsonObject &json)
{
    jstools::parseJson(json, "R", R);
    jstools::parseJson(json, "Rmin", Rmin);
    jstools::parseJson(json, "Rmax", Rmax);
    jstools::parseJson(json, "Phi1", Phi1);
    jstools::parseJson(json, "Dphi", Dphi);

    if (!jstools::parseJson(json, "str2R",    str2R))    str2R.clear();
    if (!jstools::parseJson(json, "str2Rmin", str2Rmin)) str2Rmin.clear();
    if (!jstools::parseJson(json, "str2Rmax", str2Rmax)) str2Rmax.clear();
    if (!jstools::parseJson(json, "strPhi1",  strPhi1))  strPhi1.clear();
    if (!jstools::parseJson(json, "strDphi",  strDphi))  strDphi.clear();
}

bool AGeoTorus::readFromTShape(TGeoShape *Tshape)
{
    TGeoTorus* tor = dynamic_cast<TGeoTorus*>(Tshape);
    if (!tor) return false;

    R = tor->GetR();
    Rmin = tor->GetRmin();
    Rmax = tor->GetRmax();
    Phi1 = tor->GetPhi1();
    Dphi = tor->GetDphi();

    return true;
}
